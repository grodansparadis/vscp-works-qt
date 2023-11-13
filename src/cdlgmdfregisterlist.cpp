// cdlgmdfregisterlist.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, Grodans Paradis AB
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
#include "cdlgmdfregisterlist.h"
#include "ui_cdlgmdfregisterlist.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfRegisterList::CDlgMdfRegisterList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRegisterList)
{
  m_pmdf  = nullptr;
  m_page = 0;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::addRegister);
  connect(ui->btnEditRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::editRegister);
  connect(ui->btnDupRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::dupRegister);
  connect(ui->btnDelRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::deleteRegister);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRegisterList::~CDlgMdfRegisterList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRegisterList::initDialogData(CMDF* pmdf, uint16_t page)
{
  QString str;

  if (nullptr == pmdf) {
    QMessageBox::critical(this, tr("MDF contact information"), tr("Invalid MDF manufacturing object"));
    spdlog::error("MDF contact information - Invalid MDF manufacturing object");
    return;
  }

  // Save MDF  and page
  m_pmdf = pmdf;
  m_page = page;

  setWindowTitle(tr("Registers on page %1").arg(page));

  // Fill in defined register items
  renderRegisterItems();
}

///////////////////////////////////////////////////////////////////////////////
// renderRegisterItems
//

void
CDlgMdfRegisterList::renderRegisterItems(void)
{
  std::map<uint32_t, CMDF_Register *> pages;

  if (nullptr == m_pmdf) {
    return;
  }

  m_pmdf->getRegisterMap(m_page, pages);

  std::deque<CMDF_Register*>* regs = m_pmdf->getRegisterObjList();

  for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
    QString str = QString("Register  %1 %2").arg((*it)->getOffset()).arg((*it)->getName().c_str());
    ui->listRegister->addItem(str);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRegisterList::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// addRegister
//

void
CDlgMdfRegisterList::addRegister(void)
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
      // m_pContactList->push_back(pitem);
      // ui->listContact->clear();
      // fillContactItems();
    }
    else {
      QMessageBox::warning(this, tr(APPNAME), tr("Memory problem could not add item"), QMessageBox::Ok);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editRegister
//

void
CDlgMdfRegisterList::editRegister(void)
{
  bool ok;

  // if (-1 != ui->listContact->currentRow()) {

  //   // Save the selected row
  //   int idx = ui->listContact->currentRow();

  //   QListWidgetItem* pitem = ui->listContact->currentItem();

  //   std::string str = QInputDialog::getText(this,
  //                                           tr("Edit contact item"),
  //                                           tr("Contact value:"),
  //                                           QLineEdit::Normal,
  //                                           pitem->text(),
  //                                           &ok)
  //                       .toStdString();

  //   if (ok && str.length()) {
  //     pitem->setText(str.c_str());
  //     CMDF_Item* pitem = m_pContactList->at(idx);
  //     if (nullptr != pitem) {
  //       pitem->setValue(str);
  //     }
  //     else {
  //       QMessageBox::warning(this, tr(APPNAME), tr("Contact item was not found"), QMessageBox::Ok);
  //     }
  //     ui->listContact->clear();
  //     fillContactItems();
  //     ui->listContact->setCurrentRow(idx);
  //   }
  // }
  // else {
  //   QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// dupRegister
//

void
CDlgMdfRegisterList::dupRegister(void)
{
  bool ok;

  // if (-1 != ui->listContact->currentRow()) {

  //   // Save the selected row
  //   int idx = ui->listContact->currentRow();

  //   QListWidgetItem* pitem = ui->listContact->currentItem();
  //   QString selstr         = pitem->text().split('_').first().left(2);

  //   std::string str = QInputDialog::getText(this,
  //                                           tr("Edit contact item"),
  //                                           tr("Contact value:"),
  //                                           QLineEdit::Normal,
  //                                           pitem->text(),
  //                                           &ok)
  //                       .toStdString();

  //   if (ok && str.length()) {

  //     CMDF_Item* pitem = new CMDF_Item();
  //     if (nullptr != pitem) {
  //       pitem->setValue(str);
  //       m_pContactList->push_back(pitem);
  //       ui->listContact->clear();
  //       fillContactItems();
  //     }
  //     else {
  //       QMessageBox::warning(this, tr(APPNAME), tr("Memory problem could not add item"), QMessageBox::Ok);
  //     }
  //   }
  // }
  // else {
  //   QMessageBox::warning(this, tr("vscpworks+"), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// deleteRegister
//

void
CDlgMdfRegisterList::deleteRegister(void)
{
  // if (-1 != ui->listContact->currentRow()) {

  //   // Save the row
  //   int idx = ui->listContact->currentRow();

  //   QListWidgetItem* pitem = ui->listContact->currentItem();
  //   ui->listContact->removeItemWidget(pitem);    
  //   m_pContactList->erase(m_pContactList->cbegin()+idx);

  //   ui->listContact->clear();
  //   fillContactItems();
  // }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegisterList::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

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
