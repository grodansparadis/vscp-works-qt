// cdlgmdfbitlist.cpp
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

#include "cdlgmdfbit.h"
#include "cdlgmdfbitlist.h"
#include "ui_cdlgmdfbitlist.h"

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

CDlgMdfBitList::CDlgMdfBitList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfBitList)
{
  m_pobj = nullptr;
  m_type = mdf_type_unknown;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegisterBit, &QToolButton::clicked, this, &CDlgMdfBitList::addRegisterBit);
  connect(ui->btnEditRegisterBit, &QToolButton::clicked, this, &CDlgMdfBitList::editRegisterBit);
  connect(ui->btnDupRegisterBit, &QToolButton::clicked, this, &CDlgMdfBitList::dupRegisterBit);
  connect(ui->btnDelRegisterBit, &QToolButton::clicked, this, &CDlgMdfBitList::deleteRegisterBit);

  connect(ui->listBits, &QListWidget::doubleClicked, this, &CDlgMdfBitList::editRegisterBit);

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfBitList::~CDlgMdfBitList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfBitList::initDialogData(CMDF_Object* pobj, mdf_record_type type)
{
  QString str;

  if (nullptr == pobj) {
    QMessageBox::critical(this, tr("MDF bit information"), tr("Invalid MDF object"));
    spdlog::error("MDF bit information - Invalid MDF object");
    return;
  }

  // Save object pointer and type
  m_pobj = pobj;
  m_type = type;

  // Disable items that are not used
  if (mdf_type_register == type) {
    setWindowTitle("Register bit definitionsöööö");
  }
  else if (mdf_type_remotevar == type) {
    setWindowTitle("Remote variable bit definitions");
  }
  else if (mdf_type_alarm == type) {
    setWindowTitle("Alarm bit definitions");
  }
  else if (mdf_type_action_param == type) {
    setWindowTitle("Action parameter bit definitions");
  }

  // m_pmdf->getRegisterMap(m_page, pages);
  renderBitItems();
}

///////////////////////////////////////////////////////////////////////////////
// getBits
//

std::deque<CMDF_Bit*>*
CDlgMdfBitList::getBits(void)
{
  if (mdf_type_register == m_type) {
    return ((CMDF_Register*)m_pobj)->getListBits();
  }
  else if (mdf_type_remotevar == m_type) {
    return ((CMDF_RemoteVariable*)m_pobj)->getListBits();
  }
  else if (mdf_type_alarm == m_type) {
    return ((CMDF*)m_pobj)->getAlarmListBits();
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// renderBitItems
//

void
CDlgMdfBitList::renderBitItems(void)
{
  std::deque<CMDF_Bit*>* pbits = nullptr;
  std::map<uint32_t, CMDF_Register*> pages;

  if (nullptr == m_pobj) {
    return;
  }

  ui->listBits->clear();
  if (mdf_type_register == m_type) {
    pbits = ((CMDF_Register*)m_pobj)->getListBits();
  }
  else if (mdf_type_remotevar == m_type) {
    pbits = ((CMDF_RemoteVariable*)m_pobj)->getListBits();
  }
  else if (mdf_type_alarm == m_type) {
    pbits = ((CMDF*)m_pobj)->getAlarmListBits();
  }
  else if (mdf_type_action_param == m_type) {
    pbits = ((CMDF_ActionParameter*)m_pobj)->getListBits();
  }
  else {
    return;
  }

  // If no enteries there is nothing to do
  if (!pbits->size()) {
    return;
  }

  int idx = 0;
  for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
    CMDF_Bit* pbit = *it;
    if (nullptr != pbit) {
      QString str = QString("Bit %1(%2)-- %3").arg(pbit->getPos()).arg(pbit->getWidth()).arg(pbit->getName().c_str());
      ui->listBits->addItem(str);
      // Set data to register index
      ui->listBits->item(ui->listBits->count() - 1)->setData(Qt::UserRole, idx);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// addRegisterBit
//

void
CDlgMdfBitList::addRegisterBit(void)
{
  bool ok;
  std::deque<CMDF_Bit*>* pbits;

  CMDF_Bit* pbitnew = new CMDF_Bit();
  if (nullptr == pbitnew) {
    QMessageBox::critical(this, tr("MDF register bit information"), tr("Memory problem"));
    spdlog::error("MDF register information - Memory problem");
    return;
  }

  // Save the selected row
  int idx = ui->listBits->currentRow();

  CDlgMdfBit dlg(this);
  dlg.initDialogData(pbitnew);
  dlg.setWindowTitle(tr("Add register bit definition"));

addbitdlg:

  if (QDialog::Accepted == dlg.exec()) {

    uint8_t mask;
    if ((mask = checkIfBitsOverlap(pbitnew))) {
      QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      goto addbitdlg;
    }
    if (mdf_type_register == m_type) {
      pbits = ((CMDF_Register*)m_pobj)->getListBits();
    }
    else if (mdf_type_remotevar == m_type) {
      pbits = ((CMDF_RemoteVariable*)m_pobj)->getListBits();
    }
    else if (mdf_type_alarm == m_type) {
      pbits = ((CMDF*)m_pobj)->getAlarmListBits();
    }
    else if (mdf_type_action_param == m_type) {
      pbits = ((CMDF_ActionParameter*)m_pobj)->getListBits();
    }
    else {
      return;
    }

    pbits->push_back(pbitnew);
    renderBitItems();
  }
  else {
    delete pbitnew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editRegisterBit
//

void
CDlgMdfBitList::editRegisterBit(void)
{
  bool ok;
  CMDF_Bit* pbit = nullptr;

  if (-1 != ui->listBits->currentRow()) {

    // Save the selected row
    int idx = ui->listBits->currentRow();

    QListWidgetItem* pitem = ui->listBits->currentItem();
    // CMDF_Bit* pbit         = m_preg->getListBits()->at(pitem->data(Qt::UserRole).toUInt());

    if (mdf_type_register == m_type) {
      pbit = ((CMDF_Register*)m_pobj)->getListBits()->at(pitem->data(Qt::UserRole).toUInt());
    }
    else if (mdf_type_remotevar == m_type) {
      pbit = ((CMDF_RemoteVariable*)m_pobj)->getListBits()->at(pitem->data(Qt::UserRole).toUInt());
    }
    else if (mdf_type_alarm == m_type) {
      pbit = ((CMDF*)m_pobj)->getAlarmListBits()->at(pitem->data(Qt::UserRole).toUInt());
    }
    else if (mdf_type_action_param == m_type) {
      pbit = ((CMDF_ActionParameter*)m_pobj)->getListBits()->at(pitem->data(Qt::UserRole).toUInt());
    }
    else {
      return;
    }

    CDlgMdfBit dlg(this);
    dlg.initDialogData(pbit, 0, m_type);

  editbitdlg:

    if (QDialog::Accepted == dlg.exec()) {
      uint8_t mask;
      if ((mask = checkIfBitsOverlap(pbit, true))) {
        QMessageBox::warning(this, tr("Edit bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
        goto editbitdlg;
      }
      ui->listBits->clear();
      renderBitItems();
      ui->listBits->setCurrentRow(idx);
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
CDlgMdfBitList::dupRegisterBit(void)
{
  if (-1 != ui->listBits->currentRow()) {

    // Save the selected row
    int idx = ui->listBits->currentRow();

    //   QListWidgetItem* pitem = ui->listBits->currentItem();
    //   CMDF_Bit* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);

    //   CMDF_Bit* pregnew = new CMDF_Bit();
    //   pregnew->setPage(m_page);

    //   // Make copy
    //   *pregnew = *preg;

    //   CDlgMdfBit dlg(this);
    //   dlg.initDialogData(m_pmdf, pregnew);
    // dupregdlg:
    //   if (QDialog::Accepted == dlg.exec()) {
    //     // Check if register is already defined
    //     CMDF_Bit* pregold = m_pmdf->getRegister(pregnew->getOffset(), pregnew->getPage());
    //     if (nullptr != pregold) {
    //       QMessageBox::warning(this, tr("MDF duplicate register"), tr("Register page=%1 offset=%2 is already define. Must be unique.").arg(pregnew->getPage()).arg(pregnew->getOffset()));
    //       goto dupregdlg;
    //     }
    //     qDebug() << "Page=" << pregnew->getPage() << " Offset=" << pregnew->getOffset();
    //     m_pmdf->getRegisterObjList()->push_back(pregnew);
    //     if (m_page == pregnew->getPage()) {
    //       m_registersSet.insert(pregnew->getOffset());
    //     }
    //     ui->listBits->clear();
    //     renderRegisterItems();
    //     if (-1 != idx) {
    //       ui->listBits->setCurrentRow(idx);
    //     }
    //     // Warn if page is not the same as for dialog
    //     if (pregnew->getPage() != m_page) {
    //       QMessageBox::information(this,
    //                                tr("MDF duplicate register"),
    //                                tr("Register page=%1 offset=%2 is not on same page [%3] as registers and will be added but not shown.").arg(pregnew->getPage()).arg(pregnew->getOffset()).arg(m_page));
    //     }
    //   }
    //   else {
    //     delete pregnew;
    //   }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteRegisterBit
//

void
CDlgMdfBitList::deleteRegisterBit(void)
{
  CMDF_Bit* pbit;
  std::deque<CMDF_Bit*>::iterator it;
  std::deque<CMDF_Bit*>* pbits;

  if (mdf_type_register == m_type) {
    pbits = ((CMDF_Register*)m_pobj)->getListBits();
  }
  else if (mdf_type_remotevar == m_type) {
    pbits = ((CMDF_RemoteVariable*)m_pobj)->getListBits();
  }
  else if (mdf_type_alarm == m_type) {
    pbits = ((CMDF*)m_pobj)->getAlarmListBits();
  }
  else if (mdf_type_action_param == m_type) {
    pbits = ((CMDF_ActionParameter*)m_pobj)->getListBits();
  }
  else {
    return;
  }

  if (-1 != ui->listBits->currentRow()) {

    // Save the row
    int idx = ui->listBits->currentRow();

    QListWidgetItem* pitem = ui->listBits->currentItem();
    // CMDF_Bit* pbit         = m_preg->getListBits()->at(pitem->data(Qt::UserRole).toUInt());
    pbit = pbits->at(pitem->data(Qt::UserRole).toUInt());

    // std::deque<CMDF_Bit*>::iterator it = m_preg->getListBits()->begin() + pitem->data(Qt::UserRole).toUInt();

    it = pbits->begin() + pitem->data(Qt::UserRole).toUInt();
    pbits->erase(it);

    // m_preg->getListBits()->erase(it);

    ui->listBits->clear();
    renderBitItems();
    int sel = idx;
    if (0 == idx) {
      sel = 0;
    }
    else if (pbits->size() == idx) {
      sel = pbits->size() - 1;
    }
    else {
      sel = idx + 1;
    }
    ui->listBits->setCurrentRow(sel);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfBitList::accept()
{
  std::string str;
  if (nullptr != m_pobj) {
    ;
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
