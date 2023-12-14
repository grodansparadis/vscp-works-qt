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
  m_preg = nullptr;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::addRegisterBit);
  connect(ui->btnEditRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::editRegisterBit);
  connect(ui->btnDupRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::dupRegisterBit);
  connect(ui->btnDelRegisterBit, &QToolButton::clicked, this, &CDlgMdfRegisterBitList::deleteRegisterBit);

  connect(ui->listRegisterBit, &QListWidget::doubleClicked, this, &CDlgMdfRegisterBitList::editRegisterBit);

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
CDlgMdfRegisterBitList::initDialogData(CMDF_Register* preg)
{
  QString str;

  if (nullptr == preg) {
    QMessageBox::critical(this, tr("MDF register bit information"), tr("Invalid MDF register object"));
    spdlog::error("MDF register information - Invalid MDF register object");
    return;
  }

  // Save register pointer and page
  m_preg = preg;

  // m_pmdf->getRegisterMap(m_page, pages);
  renderBitItems();
}

///////////////////////////////////////////////////////////////////////////////
// renderBitItems
//

void
CDlgMdfRegisterBitList::renderBitItems(void)
{
  std::map<uint32_t, CMDF_Register*> pages;

  if (nullptr == m_preg) {
    return;
  }

  ui->listRegisterBit->clear();
  std::deque<CMDF_Bit*>* pbits = m_preg->getListBits();

  // If no enteries there is nothing to do
  if (!pbits->size()) {
    return;
  }

  int idx = 0;
  for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
    CMDF_Bit* pbit = *it;
    if (nullptr != pbit) {
      QString str = QString("Bit %1(%2)-- %3").arg(pbit->getPos()).arg(pbit->getWidth()).arg(pbit->getName().c_str());
      ui->listRegisterBit->addItem(str);
      // Set data to register index
      ui->listRegisterBit->item(ui->listRegisterBit->count() - 1)->setData(Qt::UserRole, idx);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// addRegisterBit
//

void
CDlgMdfRegisterBitList::addRegisterBit(void)
{
  bool ok;
  CMDF_Bit* pbitnew = new CMDF_Bit();
  if (nullptr == pbitnew) {
    QMessageBox::critical(this, tr("MDF register bit information"), tr("Memory problem"));
    spdlog::error("MDF register information - Memory problem");
    return;
  }

  // Save the selected row
  int idx = ui->listRegisterBit->currentRow();

  CDlgMdfRegisterBit dlg(this);
  dlg.initDialogData(pbitnew);
  dlg.setWindowTitle(tr("Add register bit definition"));

addbitdlg:

  if (QDialog::Accepted == dlg.exec()) {

    uint8_t mask;
    if ((mask = checkIfBitsOverlap(pbitnew))) {
      QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      goto addbitdlg;
    }
    std::deque<CMDF_Bit*>* pbits = m_preg->getListBits();
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
CDlgMdfRegisterBitList::editRegisterBit(void)
{
  bool ok;

  if (-1 != ui->listRegisterBit->currentRow()) {

    // Save the selected row
    int idx = ui->listRegisterBit->currentRow();

    QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
    CMDF_Bit* pbit         = m_preg->getListBits()->at(pitem->data(Qt::UserRole).toUInt());

    CDlgMdfRegisterBit dlg(this);
    dlg.initDialogData(pbit);

  editbitdlg:

    if (QDialog::Accepted == dlg.exec()) {
      uint8_t mask;
      if ((mask = checkIfBitsOverlap(pbit, true))) {
        QMessageBox::warning(this, tr("Edit bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
        goto editbitdlg;
      }
      ui->listRegisterBit->clear();
      renderBitItems();
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

    //   QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
    //   CMDF_Bit* preg    = m_pmdf->getRegister(pitem->data(Qt::UserRole).toUInt(), m_page);

    //   CMDF_Bit* pregnew = new CMDF_Bit();
    //   pregnew->setPage(m_page);

    //   // Make copy
    //   *pregnew = *preg;

    //   CDlgMdfRegisterBit dlg(this);
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
    //     ui->listRegisterBit->clear();
    //     renderRegisterItems();
    //     if (-1 != idx) {
    //       ui->listRegisterBit->setCurrentRow(idx);
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
CDlgMdfRegisterBitList::deleteRegisterBit(void)
{
  if (-1 != ui->listRegisterBit->currentRow()) {

    // Save the row
    int idx = ui->listRegisterBit->currentRow();

    QListWidgetItem* pitem = ui->listRegisterBit->currentItem();
    CMDF_Bit* pbit         = m_preg->getListBits()->at(pitem->data(Qt::UserRole).toUInt());

    std::deque<CMDF_Bit*>::iterator it = m_preg->getListBits()->begin() + pitem->data(Qt::UserRole).toUInt();
    m_preg->getListBits()->erase(it);

    ui->listRegisterBit->clear();
    renderBitItems();
    int sel = idx;
    if (0 == idx) {
      sel = 0;
    }
    else if (m_preg->getListBits()->size() == idx) {
      sel = m_preg->getListBits()->size() - 1;
    }
    else {
      sel = idx + 1;
    }
    ui->listRegisterBit->setCurrentRow(sel);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegisterBitList::accept()
{
  std::string str;
  if (nullptr != m_preg) {

    // str = ui->editName->text().toStdString();
    //  m_pmdf->setName(str);

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
