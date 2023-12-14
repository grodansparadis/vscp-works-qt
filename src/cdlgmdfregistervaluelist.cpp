// cdlgmdfregistervaluelist.cpp
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

#include "cdlgmdfregistervalue.h"
#include "cdlgmdfregistervaluelist.h"
#include "ui_cdlgmdfregistervaluelist.h"

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

CDlgMdfRegisterValueList::CDlgMdfRegisterValueList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRegisterValueList)
{
  m_preg = nullptr;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegisterValue, &QToolButton::clicked, this, &CDlgMdfRegisterValueList::addRegisterValue);
  connect(ui->btnEditRegisterValue, &QToolButton::clicked, this, &CDlgMdfRegisterValueList::editRegisterValue);
  connect(ui->btnDupRegisterValue, &QToolButton::clicked, this, &CDlgMdfRegisterValueList::dupRegisterValue);
  connect(ui->btnDelRegisterValue, &QToolButton::clicked, this, &CDlgMdfRegisterValueList::deleteRegisterValue);

  connect(ui->listRegisterValues, &QListWidget::doubleClicked, this, &CDlgMdfRegisterValueList::editRegisterValue);

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRegisterValueList::~CDlgMdfRegisterValueList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRegisterValueList::initDialogData(CMDF_Register* preg)
{
  QString str;

  if (nullptr == preg) {
    QMessageBox::critical(this, tr("MDF register value information"), tr("Invalid MDF register object"));
    spdlog::error("MDF register value information - Invalid MDF register object");
    return;
  }

  // Save register pointer and page
  m_preg = preg;

  // m_pmdf->getRegisterMap(m_page, pages);
  renderValueItems();
}

///////////////////////////////////////////////////////////////////////////////
// renderValueItems
//

void
CDlgMdfRegisterValueList::renderValueItems(void)
{
  if (nullptr == m_preg) {
    return;
  }

  ui->listRegisterValues->clear();
  std::deque<CMDF_Value*>* pvalues = m_preg->getListValues();

  // If no enteries there is nothing to do
  if (!pvalues->size()) {
    return;
  }

  int idx = 0;
  for (auto it = pvalues->cbegin(); it != pvalues->cend(); ++it) {
    CMDF_Value* pvalue = *it;
    if (nullptr != pvalue) {
      QString str = QString("Value: %1").arg(pvalue->getName().c_str());
      ui->listRegisterValues->addItem(str);
      // Set data to register index
      ui->listRegisterValues->item(ui->listRegisterValues->count() - 1)->setData(Qt::UserRole, idx);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// addRegisterValue
//

void
CDlgMdfRegisterValueList::addRegisterValue(void)
{
  bool ok;
  CMDF_Value* pvaluenew = new CMDF_Value();
  if (nullptr == pvaluenew) {
    QMessageBox::critical(this, tr("MDF register bit information"), tr("Memory problem"));
    spdlog::error("MDF register information - Memory problem");
    return;
  }

  // Save the selected row
  int idx = ui->listRegisterValues->currentRow();

  CDlgMdfRegisterValue dlg(this);
  dlg.initDialogData(pvaluenew);
  dlg.setWindowTitle(tr("Add register value"));

addbitdlg:

  if (QDialog::Accepted == dlg.exec()) {

    uint8_t mask;
    // if ((mask = checkIfBitsOverlap(pvaluenew))) {
    //   QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
    //   goto addbitdlg;
    // }
    std::deque<CMDF_Value*>* pvalues = m_preg->getListValues();
    pvalues->push_back(pvaluenew);
    renderValueItems();
  }
  else {
    delete pvaluenew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editRegisterValue
//

void
CDlgMdfRegisterValueList::editRegisterValue(void)
{
  bool ok;

  if (-1 != ui->listRegisterValues->currentRow()) {

    // Save the selected row
    int idx = ui->listRegisterValues->currentRow();

    QListWidgetItem* pitem = ui->listRegisterValues->currentItem();
    CMDF_Value* pvalue         = m_preg->getListValues()->at(pitem->data(Qt::UserRole).toUInt());

    CDlgMdfRegisterValue dlg(this);
    dlg.initDialogData(pvalue);

  editvaluedlg:

    if (QDialog::Accepted == dlg.exec()) {
      uint8_t mask;
      // if ((mask = checkIfBitsOverlap(pvalue, true))) {
      //   QMessageBox::warning(this, tr("Edit register value"), tr("Can not add register value. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      //   goto editvaluedlg;
      // }
      ui->listRegisterValues->clear();
      renderValueItems();
      ui->listRegisterValues->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupRegisterValue
//

void
CDlgMdfRegisterValueList::dupRegisterValue(void)
{
  if (-1 != ui->listRegisterValues->currentRow()) {

    // Save the selected row
    int idx = ui->listRegisterValues->currentRow();

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
// deleteRegisterValue
//

void
CDlgMdfRegisterValueList::deleteRegisterValue(void)
{
  if (-1 != ui->listRegisterValues->currentRow()) {

    // Save the row
    int idx = ui->listRegisterValues->currentRow();

    QListWidgetItem* pitem = ui->listRegisterValues->currentItem();
    CMDF_Value* pvalue         = m_preg->getListValues()->at(pitem->data(Qt::UserRole).toUInt());

    std::deque<CMDF_Value*>::iterator it = m_preg->getListValues()->begin() + pitem->data(Qt::UserRole).toUInt();
    m_preg->getListValues()->erase(it);

    ui->listRegisterValues->clear();
    renderValueItems();
    int sel = idx;
    if (0 == idx) {
      sel = 0;
    }
    else if (m_preg->getListValues()->size() == idx) {
      sel = m_preg->getListValues()->size() - 1;
    }
    else {
      sel = idx + 1;
    }
    ui->listRegisterValues->setCurrentRow(sel);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegisterValueList::accept()
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
