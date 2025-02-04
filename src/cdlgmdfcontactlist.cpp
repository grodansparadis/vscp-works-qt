// cdlgmdfcontactlist.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgmdfcontact.h"
#include "cdlgmdfcontactlist.h"
#include "ui_cdlgmdfcontactlist.h"

// #include "cdlgmdfdescription.h"
// #include "cdlgmdfinfourl.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfContactList::CDlgMdfContactList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfContactList)
{
  m_pManufacturer = nullptr;
  m_pContactList  = nullptr;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddContact, &QToolButton::clicked, this, &CDlgMdfContactList::addContact);
  connect(ui->btnEditContact, &QToolButton::clicked, this, &CDlgMdfContactList::editContact);
  connect(ui->btnDupContact, &QToolButton::clicked, this, &CDlgMdfContactList::dupContact);
  connect(ui->btnDelContact, &QToolButton::clicked, this, &CDlgMdfContactList::deleteContact);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfContactList::~CDlgMdfContactList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfContactList::initDialogData(CMDF_Manufacturer* pManufacturer,
                                   mdf_dlg_contact_type type,
                                   QString title)
{
  QString str;

  if (nullptr == pManufacturer) {
    QMessageBox::critical(this, tr("MDF contact information"), tr("Invalid MDF manufacturing object"));
    spdlog::error("MDF contact information - Invalid MDF manufacturing object");
    return;
  }

  // Save type
  m_type = type;

  setWindowTitle(title);

  m_pManufacturer = pManufacturer;

  switch (type) {

    case dlg_type_contact_phone:
      m_pContactList = m_pManufacturer->getPhoneContactList();
      break;

    case dlg_type_contact_fax:
      m_pContactList = m_pManufacturer->getFaxContactList();
      break;

    case dlg_type_contact_email:
      m_pContactList = m_pManufacturer->getEmailContactList();
      break;

    case dlg_type_contact_web:
      m_pContactList = m_pManufacturer->getWebContactList();
      break;

    case dlg_type_contact_social:
      m_pContactList = m_pManufacturer->getSocialContactList();
      break;

    default:
      m_pContactList = nullptr;
      break;
  }

  // Fill in defined contact items
  fillContactItems();
}

///////////////////////////////////////////////////////////////////////////////
// fillContactItems
//

void
CDlgMdfContactList::fillContactItems(void)
{
  if (nullptr == m_pContactList) {
    return;
  }

  for (auto it = m_pContactList->cbegin(); it != m_pContactList->cend(); ++it) {
    ui->listContact->addItem((*it)->getValue().c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfContactList::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addContact
//

void
CDlgMdfContactList::addContact(void)
{
  bool ok;
  std::string str = QInputDialog::getText(this,
                                          tr("Add contact item"),
                                          tr("Contact value:"),
                                          QLineEdit::Normal,
                                          "",
                                          &ok)
                      .toStdString();

  if (ok && str.length()) {
    CMDF_Item* pitem = new CMDF_Item();
    if (nullptr != pitem) {
      pitem->setValue(str);
      m_pContactList->push_back(pitem);
      ui->listContact->clear();
      fillContactItems();
    }
    else {
      QMessageBox::warning(this, tr(APPNAME), tr("Memory problem could not add item"), QMessageBox::Ok);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editContact
//

void
CDlgMdfContactList::editContact(void)
{
  bool ok;

  if (-1 != ui->listContact->currentRow()) {

    // Save the selected row
    int idx = ui->listContact->currentRow();

    QListWidgetItem* pitem = ui->listContact->currentItem();

    std::string str = QInputDialog::getText(this,
                                            tr("Edit contact item"),
                                            tr("Contact value:"),
                                            QLineEdit::Normal,
                                            pitem->text(),
                                            &ok)
                        .toStdString();

    if (ok && str.length()) {
      pitem->setText(str.c_str());
      CMDF_Item* pitem = m_pContactList->at(idx);
      if (nullptr != pitem) {
        pitem->setValue(str);
      }
      else {
        QMessageBox::warning(this, tr(APPNAME), tr("Contact item was not found"), QMessageBox::Ok);
      }
      ui->listContact->clear();
      fillContactItems();
      ui->listContact->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupContact
//

void
CDlgMdfContactList::dupContact(void)
{
  bool ok;

  if (-1 != ui->listContact->currentRow()) {

    // Save the selected row
    int idx = ui->listContact->currentRow();

    QListWidgetItem* pitem = ui->listContact->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    std::string str = QInputDialog::getText(this,
                                            tr("Edit contact item"),
                                            tr("Contact value:"),
                                            QLineEdit::Normal,
                                            pitem->text(),
                                            &ok)
                        .toStdString();

    if (ok && str.length()) {

      CMDF_Item* pitem = new CMDF_Item();
      if (nullptr != pitem) {
        pitem->setValue(str);
        m_pContactList->push_back(pitem);
        ui->listContact->clear();
        fillContactItems();
      }
      else {
        QMessageBox::warning(this, tr(APPNAME), tr("Memory problem could not add item"), QMessageBox::Ok);
      }
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteContact
//

void
CDlgMdfContactList::deleteContact(void)
{
  if (-1 != ui->listContact->currentRow()) {

    // Save the row
    int idx = ui->listContact->currentRow();

    QListWidgetItem* pitem = ui->listContact->currentItem();
    ui->listContact->removeItemWidget(pitem);
    m_pContactList->erase(m_pContactList->cbegin() + idx);

    ui->listContact->clear();
    fillContactItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfContactList::accept()
{
  std::string str;
  if (nullptr != m_pManufacturer) {

    // str = ui->editName->text().toStdString();
    // m_pmdf->setModuleName(str);

    // str = ui->editModel->text().toStdString();
    // m_pmdf->setModuleModel(str);

    // m_pmdf->setModuleLevel(ui->comboModuleLevel->currentIndex());

    // str = ui->editVersion->text().toStdString();
    // m_pmdf->setModuleVersion(str);

    // str = ui->editDate->text().toStdString();
    // m_pmdf->setModuleChangeDate(str);

    // m_pmdf->setModuleBufferSize(ui->editBufferSize->value());

    // str = ui->editCopyright->text().toStdString();
    // m_pmdf->setModuleCopyright(str);
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMdfContactList::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}