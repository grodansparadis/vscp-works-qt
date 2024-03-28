// cdlgmdfregisterlist.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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
#include "cdlgmdfregister.h"
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
  m_pmdf = nullptr;
  m_page = 0;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::addRegister);
  connect(ui->btnEditRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::editRegister);
  connect(ui->btnDupRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::dupRegister);
  connect(ui->btnDelRegister, &QToolButton::clicked, this, &CDlgMdfRegisterList::deleteRegister);

  connect(ui->listRegister, &QListWidget::doubleClicked, this, &CDlgMdfRegisterList::editRegister);
  

  setInitialFocus();
  this->setFixedSize(this->size());
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

  // m_pmdf->getRegisterMap(m_page, pages);

  // Fill in defined register items
  renderRegisterItems();

  // Fill the page combo with page information
  renderComboPage();

  connect(ui->comboPage, SIGNAL(currentIndexChanged(int)), this, SLOT(onPageComboChange(int)));
}

///////////////////////////////////////////////////////////////////////////////
// renderComboPage
//

void
CDlgMdfRegisterList::renderComboPage(void)
{
  setWindowTitle(tr("Registers for %1").arg(m_page));

  // Fill available pages in combo
  std::set<uint16_t> pages;
  uint32_t cnt = m_pmdf->getPages(pages);
  ui->comboPage->clear();
  int pos = 0;
  for (std::set<uint16_t>::iterator it = pages.begin(); it != pages.end(); ++it) {
    ui->comboPage->addItem(QString("Page %1").arg(*it), *it);
    if (m_page == *it) {
      ui->comboPage->setCurrentIndex(pos);
    }
    pos++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// renderRegisterItems
//

void
CDlgMdfRegisterList::renderRegisterItems(void)
{
  std::map<uint32_t, CMDF_Register*> pages;

  if (nullptr == m_pmdf) {
    return;
  }

  ui->listRegister->clear();

  // Make sorted set of registers on this page
  std::deque<CMDF_Register*>* regset = m_pmdf->getRegisterObjList();
  for (auto it = regset->cbegin(); it != regset->cend(); ++it) {
    if (m_page == (*it)->getPage()) {
      m_registersSet.insert((*it)->getOffset());
    }
  }

  m_pmdf->getRegisterMap(m_page, pages);

  std::deque<CMDF_Register*>* regs = m_pmdf->getRegisterObjList();

  for (auto it = m_registersSet.cbegin(); it != m_registersSet.cend(); ++it) {
    // m_registersSet.insert((*it)->getOffset());
    CMDF_Register* preg = m_pmdf->getRegister(*it, m_page);
    if (nullptr != preg) {
      QString str = QString("Register  %1 -- %2").arg(preg->getOffset()).arg(preg->getName().c_str());
      ui->listRegister->addItem(str);
      // Set data to register index
      ui->listRegister->item(ui->listRegister->count() - 1)->setData(Qt::UserRole, preg->getOffset());
    }
  }

  // for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
  //   m_registersSet.insert((*it)->getOffset());
  //   if ((*it)->getPage() == m_page) {
  //     QString str = QString("Register  %1 -- %2").arg((*it)->getOffset()).arg((*it)->getName().c_str());
  //     ui->listRegister->addItem(str);
  //     // Set data to register index
  //     ui->listRegister->item(ui->listRegister->count() - 1)->setData(Qt::UserRole, (*it)->getOffset());
  //   }
  // }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRegisterList::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onPageComboChange
//

void
CDlgMdfRegisterList::onPageComboChange(int idx)
{
  m_page = ui->comboPage->currentData().toInt();
  renderRegisterItems();
  setWindowTitle(tr("Registers for %1").arg(m_page));
  ui->listRegister->setCurrentRow(0);
}

///////////////////////////////////////////////////////////////////////////////
// addRegister
//

void
CDlgMdfRegisterList::addRegister(void)
{
  bool ok;
  CMDF_Register* pregnew = new CMDF_Register();
  pregnew->setPage(m_page);

  // Save the selected row
  int idx = ui->listRegister->currentRow();

  CDlgMdfRegister dlg(this);
  dlg.initDialogData(m_pmdf, pregnew);
addregdlg:
  if (QDialog::Accepted == dlg.exec()) {
    // Check if register is already defined
    CMDF_Register* preg = m_pmdf->getRegister(pregnew->getOffset(), pregnew->getPage());
    if (nullptr != preg) {
      QMessageBox::warning(this, tr("MDF add new register"), tr("Register page=%1 offset=%2 is already define. Must be unique.").arg(pregnew->getPage()).arg(pregnew->getOffset()));
      goto addregdlg;
    }
    qDebug() << "Page=" << pregnew->getPage() << " Offset=" << pregnew->getOffset();
    m_pmdf->getRegisterObjList()->push_back(pregnew);
    if (m_page == pregnew->getPage()) {
      m_registersSet.insert(pregnew->getOffset());
    }
    ui->listRegister->clear();
    renderRegisterItems();
    if (-1 != idx) {
      ui->listRegister->setCurrentRow(idx);
    }

    // Warn if page is not the same as for dialog
    if (pregnew->getPage() != m_page) {
      QMessageBox::information(this,
                               tr("MDF duplicate register"),
                               tr("Register page=%1 offset=%2 is not on same page [%3] as registers and will be added but not shown.").arg(pregnew->getPage()).arg(pregnew->getOffset()).arg(m_page));
    }
  }
  else {
    delete pregnew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editRegister
//

void
CDlgMdfRegisterList::editRegister(void)
{
  bool ok;

  if (-1 != ui->listRegister->currentRow()) {

    // Save the selected row
    int idx = ui->listRegister->currentRow();

    QListWidgetItem* pitem = ui->listRegister->currentItem();
    CMDF_Register* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);

    CDlgMdfRegister dlg(this);
    dlg.initDialogData(m_pmdf, preg);
    // Don't allow editing of page and offset
    dlg.setReadOnly();
    if (QDialog::Accepted == dlg.exec()) {
      ui->listRegister->clear();
      renderRegisterItems();
      ui->listRegister->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupRegister
//

void
CDlgMdfRegisterList::dupRegister(void)
{
  if (-1 != ui->listRegister->currentRow()) {

    // Save the selected row
    int idx = ui->listRegister->currentRow();

    QListWidgetItem* pitem = ui->listRegister->currentItem();
    CMDF_Register* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);

    CMDF_Register* pregnew = new CMDF_Register();
    pregnew->setPage(m_page);

    // Make copy
    *pregnew = *preg;

    CDlgMdfRegister dlg(this);
    dlg.initDialogData(m_pmdf, pregnew);
  dupregdlg:
    if (QDialog::Accepted == dlg.exec()) {
      // Check if register is already defined
      CMDF_Register* pregold = m_pmdf->getRegister(pregnew->getOffset(), pregnew->getPage());
      if (nullptr != pregold) {
        QMessageBox::warning(this, tr("MDF duplicate register"), tr("Register page=%1 offset=%2 is already define. Must be unique.").arg(pregnew->getPage()).arg(pregnew->getOffset()));
        goto dupregdlg;
      }
      qDebug() << "Page=" << pregnew->getPage() << " Offset=" << pregnew->getOffset();
      m_pmdf->getRegisterObjList()->push_back(pregnew);
      if (m_page == pregnew->getPage()) {
        m_registersSet.insert(pregnew->getOffset());
      }
      ui->listRegister->clear();
      renderRegisterItems();
      if (-1 != idx) {
        ui->listRegister->setCurrentRow(idx);
      }
      // Warn if page is not the same as for dialog
      if (pregnew->getPage() != m_page) {
        QMessageBox::information(this,
                                 tr("MDF duplicate register"),
                                 tr("Register page=%1 offset=%2 is not on same page [%3] as registers and will be added but not shown.").arg(pregnew->getPage()).arg(pregnew->getOffset()).arg(m_page));
      }
    }
    else {
      delete pregnew;
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }

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
  //   QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// deleteRegister
//

void
CDlgMdfRegisterList::deleteRegister(void)
{
  if (-1 != ui->listRegister->currentRow()) {

    // Save the row
    int idx = ui->listRegister->currentRow();

    QListWidgetItem* pitem = ui->listRegister->currentItem();
    CMDF_Register* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);
    m_pmdf->deleteRegister(preg);
    delete preg;
    ui->listRegister->removeItemWidget(pitem);
    renderRegisterItems();
    ui->listRegister->setCurrentRow(idx);
  }
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
