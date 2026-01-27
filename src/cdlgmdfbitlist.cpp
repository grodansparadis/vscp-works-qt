// cdlgmdfbitlist.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
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

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

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
    setWindowTitle("Register bit definitions");
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
  else if (mdf_type_event_data_item == type) {
    setWindowTitle("Event data bit definitions");
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

  // Get bitlist (for type)
  if (nullptr == (pbits = getBitList())) {
    return;
  }

  // If no enteries there is nothing to do
  if (!pbits->size()) {
    return;
  }

  // Create set with sorted bit offsets and a map
  // to help find corresponding bit pointer
  // Add registers for page
  m_bitset.clear();
  m_bitmap.clear();
  for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
    m_bitset.insert((*it)->getPos());
    m_bitmap[(*it)->getPos()] = *it;
  }

  for (auto it = m_bitset.cbegin(); it != m_bitset.cend(); ++it) {
    CMDF_Bit* pbit = m_bitmap[*it]; //*it;
    if (nullptr != pbit) {
      QString str = QString("Bit %1(%2)-- %3").arg(pbit->getPos()).arg(pbit->getWidth()).arg(pbit->getName().c_str());
      ui->listBits->addItem(str);
      // Set data to register index
      ui->listBits->item(ui->listBits->count() - 1)->setData(Qt::UserRole, pbit->getPos());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// getBitList
//

std::deque<CMDF_Bit*>*
CDlgMdfBitList::getBitList(void)
{
  std::deque<CMDF_Bit*>* pbits = nullptr;

  // pbits = m_preg->getListBits();
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
  else if (mdf_type_event_data_item == m_type) {
    pbits = ((CMDF_EventData*)m_pobj)->getListBits();
  }

  return pbits;
}

///////////////////////////////////////////////////////////////////////////////
// getBitObj
//

CMDF_Bit*
CDlgMdfBitList::getBitObj(uint8_t pos)
{
  std::deque<CMDF_Bit*>* pbits = getBitList();
  if (nullptr == pbits) {
    return nullptr;
  }

  for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
    CMDF_Bit* pbit = *it;
    if (pbit->getPos() == pos) {
      return pbit;
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// checkIfBitsOverlap
//

uint8_t
CDlgMdfBitList::checkIfBitsOverlap(CMDF_Bit* pbit2test, bool bEdit)
{
  uint8_t result = 0;
  std::deque<CMDF_Bit*>* pbits;

  // Get bitlist (for type)
  if (nullptr == (pbits = getBitList())) {
    return 0;
  }

  for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
    CMDF_Bit* pbit = *it;
    // Don't test the edited bit of in edit mode
    if (bEdit && (pbit2test == pbit)) {
      continue;
    }
    if (nullptr != pbit) {
      result |= pbit->getMask();
    }
  }

  return (result & pbit2test->getMask());
};

///////////////////////////////////////////////////////////////////////////////
// addRegisterBit
//

void
CDlgMdfBitList::addRegisterBit(void)
{
  // bool ok;
  std::deque<CMDF_Bit*>* pbits = nullptr;

  CMDF_Bit* pbitnew = new CMDF_Bit();
  if (nullptr == pbitnew) {
    QMessageBox::critical(this, tr("MDF bit information"), tr("Memory problem"));
    spdlog::error("MDF information - Memory problem");
    return;
  }

  // Save the selected row
  int idx = ui->listBits->currentRow();

  CDlgMdfBit dlg(this);
  dlg.initDialogData(pbitnew);
  dlg.setWindowTitle(tr("Add bit definition"));

  // addbitdlg:

  if (QDialog::Accepted == dlg.exec()) {

    // uint8_t mask;
    //  if ((mask = checkIfBitsOverlap(pbitnew))) {
    //    QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
    //    goto addbitdlg;
    //  }

    // Get bitlist (for type)
    if (nullptr == (pbits = getBitList())) {
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
  // bool ok;
  CMDF_Bit* pbit = nullptr;

  if (-1 != ui->listBits->currentRow()) {

    // Save the selected row
    int idx                = ui->listBits->currentRow();
    QListWidgetItem* pitem = ui->listBits->currentItem();

    // Get bitlist (for type)
    if (nullptr == getBitList()) {
      return;
    }

    // Get the bit
    pbit = getBitObj(pitem->data(Qt::UserRole).toUInt());
    if (nullptr == pbit) {
      return;
    }

    CDlgMdfBit dlg(this);
    dlg.initDialogData(pbit, 0, m_type);

    // editbitdlg:

    if (QDialog::Accepted == dlg.exec()) {
      // uint8_t mask;
      //  if ((mask = checkIfBitsOverlap(pbit, true))) {
      //    QMessageBox::warning(this, tr("Edit bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      //    goto editbitdlg;
      //  }
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
  CMDF_Bit* pbit               = nullptr;
  std::deque<CMDF_Bit*>* pbits = nullptr;

  if (-1 != ui->listBits->currentRow()) {

    // Save the selected row
    int idx = ui->listBits->currentRow();

    QListWidgetItem* pitem = ui->listBits->currentItem();

    CMDF_Bit* pbitnew = new CMDF_Bit();
    if (nullptr == pbitnew) {
      QMessageBox::critical(this, tr("MDF bit information"), tr("Memory problem"));
      spdlog::error("MDF information - Memory problem");
      return;
    }

    // Get the bit
    pbit = getBitObj(pitem->data(Qt::UserRole).toUInt());
    if (nullptr == pbit) {
      return;
    }

    // Make copy
    *pbitnew = *pbit;

    CDlgMdfBit dlg(this);
    dlg.initDialogData(pbit, 0, m_type);
    // dupbitdlg:
    if (QDialog::Accepted == dlg.exec()) {
      // uint8_t mask;
      //  if ((mask = checkIfBitsOverlap(pbitnew))) {
      //    QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      //    goto dupbitdlg;
      //  }

      // Get bitlist (for type)
      if (nullptr == (pbits = getBitList())) {
        return;
      }

      pbits->push_back(pbitnew);
      renderBitItems();
    }
    else {
      delete pbitnew;
    }
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
  CMDF_Bit* pbit = nullptr;
  std::deque<CMDF_Bit*>::iterator it;
  std::deque<CMDF_Bit*>* pbits = nullptr;

  // Get bitlist (for type)
  if (nullptr == (pbits = getBitList())) {
    return;
  }

  if (-1 != ui->listBits->currentRow()) {

    // Save the row
    int idx = ui->listBits->currentRow();

    QListWidgetItem* pitem = ui->listBits->currentItem();

    for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
      CMDF_Bit* pbit = *it;
      if (pbit->getPos() == pitem->data(Qt::UserRole).toUInt()) {
        pbits->erase(it);
      }
    }

    ui->listBits->clear();
    renderBitItems();
    size_t sel = idx;
    if (0 == idx) {
      sel = 0;
    }
    else if (pbits->size() == idx) {
      sel = pbits->size() - 1;
    }
    else {
      sel = idx + 1;
    }
    ui->listBits->setCurrentRow((int)sel);
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

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMdfBitList::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}