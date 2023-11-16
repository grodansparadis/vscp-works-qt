// cdlgmdfregisterbitlist.cpp
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

#include "cdlgmdfregisterbit.h"
#include "cdlgmdfregisterbitlist.h"
#include "ui_cdlgmdfregisterbitlist.h"

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

CDlgMdfRegisterBitList::CDlgMdfRegisterBitList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRegisterBitList)
{
  m_pmdf = nullptr;
  m_page = 0;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::addRegisterBit);
  connect(ui->btnEditRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::editRegisterBit);
  connect(ui->btnDupRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::dupRegisterBit);
  connect(ui->btnDelRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::deleteRegisterBit);

  connect(ui->listRegisterBit, &QListWidget::doubleClicked, this, &CDlgMdfRegisterBitList::editRegisterBit);
  

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRegisterBitList::~CDlgMdfRegisterBitList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRegisterBitList::initDialogData(CMDF* pmdf, uint16_t page)
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


}



///////////////////////////////////////////////////////////////////////////////
// renderRegisterItems
//

void
CDlgMdfRegisterBitList::renderRegisterItems(void)
{
  std::map<uint32_t, CMDF_Register*> pages;

  if (nullptr == m_pmdf) {
    return;
  }

  ui->listRegisterBit->clear();

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
      ui->listRegisterBit->addItem(str);
      // Set data to register index
      ui->listRegisterBit->item(ui->listRegisterBit->count() - 1)->setData(Qt::UserRole, preg->getOffset());
    }
  }

  // for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
  //   m_registersSet.insert((*it)->getOffset());
  //   if ((*it)->getPage() == m_page) {
  //     QString str = QString("Register  %1 -- %2").arg((*it)->getOffset()).arg((*it)->getName().c_str());
  //     ui->listRegisterBit->addItem(str);
  //     // Set data to register index
  //     ui->listRegisterBit->item(ui->listRegisterBit->count() - 1)->setData(Qt::UserRole, (*it)->getOffset());
  //   }
  // }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRegisterBitList::setInitialFocus(void)
{
  //ui->editName->setFocus();
}



///////////////////////////////////////////////////////////////////////////////
// addRegisterBit
//

void
CDlgMdfRegisterBitList::addRegisterBit(void)
{
  bool ok;
  CMDF_Register* pregnew = new CMDF_Register();
  pregnew->setPage(m_page);

  // Save the selected row
  int idx = ui->listRegisterBit->currentRow();

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
    ui->listRegisterBit->clear();
    renderRegisterItems();
    if (-1 != idx) {
      ui->listRegisterBit->setCurrentRow(idx);
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
// editRegisterBit
//

void
CDlgMdfRegisterBitList::editRegisterBit(void)
{
  bool ok;

  if (-1 != ui->listRegisterBit->currentRow()) {

    // Save the selected row
    int idx = ui->listRegisterBit->currentRow();

    QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
    CMDF_Register* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);

    CDlgMdfRegister dlg(this);
    dlg.initDialogData(m_pmdf, preg);
    // Don't allow editing of page and offset
    dlg.setReadOnly();
    if (QDialog::Accepted == dlg.exec()) {
      ui->listRegisterBit->clear();
      renderRegisterItems();
      ui->listRegisterBit->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupRegisterBit
//

void
CDlgMdfRegisterBitList::dupRegisterBit(void)
{
  if (-1 != ui->listRegisterBit->currentRow()) {

    // Save the selected row
    int idx = ui->listRegisterBit->currentRow();

    QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
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
      ui->listRegisterBit->clear();
      renderRegisterItems();
      if (-1 != idx) {
        ui->listRegisterBit->setCurrentRow(idx);
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
  //   QMessageBox::warning(this, tr("vscpworks+"), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// deleteRegisterBit
//

void
CDlgMdfRegisterBitList::deleteRegisterBit(void)
{
  if (-1 != ui->listRegisterBit->currentRow()) {

    // Save the row
    int idx = ui->listRegisterBit->currentRow();

    QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
    CMDF_Register* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);
    m_pmdf->deleteRegister(preg);
    delete preg;
    ui->listRegisterBit->removeItemWidget(pitem);
    renderRegisterItems();
    ui->listRegisterBit->setCurrentRow(idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegisterBitList::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

    // str = ui->editName->text().toStdString();
    // m_pmdf->setName(str);

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
