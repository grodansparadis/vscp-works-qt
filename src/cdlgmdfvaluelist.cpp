// cdlgmdfvaluelist.cpp
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

#include "cdlgmdfvalue.h"
#include "cdlgmdfvaluelist.h"
#include "ui_cdlgmdfvaluelist.h"

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

CDlgMdfValueList::CDlgMdfValueList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfValueList)
{
  m_pobj = nullptr;

  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRegisterValue, &QToolButton::clicked, this, &CDlgMdfValueList::addValue);
  connect(ui->btnEditRegisterValue, &QToolButton::clicked, this, &CDlgMdfValueList::editValue);
  connect(ui->btnDupRegisterValue, &QToolButton::clicked, this, &CDlgMdfValueList::dupValue);
  connect(ui->btnDelRegisterValue, &QToolButton::clicked, this, &CDlgMdfValueList::deleteValue);

  connect(ui->listValues, &QListWidget::doubleClicked, this, &CDlgMdfValueList::editValue);

  this->setFixedSize(this->size());

}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfValueList::~CDlgMdfValueList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfValueList::initDialogData(CMDF_Object* pobj, mdf_record_type type)
{
  QString str;

  if (nullptr == pobj) {
    QMessageBox::critical(this, tr("MDF value information"), tr("Invalid MDF object"));
    spdlog::error("MDF value information - Invalid MDF object");
    return;
  }

  // Save register pointer and type
  m_pobj = pobj;
  m_type = type;

  // m_pmdf->getRegisterMap(m_page, pages);
  renderValueItems();
}

///////////////////////////////////////////////////////////////////////////////
// renderValueItems
//

void
CDlgMdfValueList::renderValueItems(void)
{
  if (nullptr == m_pobj) {
    return;
  }

  ui->listValues->clear();
  std::deque<CMDF_Value*>* pvalues = getValueList();

  if (nullptr == pvalues) {
    QMessageBox::critical(this, tr("APPNAME"), tr("Invalid firmware object"));
    spdlog::error("MDF values: Can't get valuelist");
    return;
  }

  // If no enteries there is nothing to do
  if (!pvalues->size()) {
    return;
  }

  int idx = 0;
  for (auto it = pvalues->cbegin(); it != pvalues->cend(); ++it) {
    CMDF_Value* pvalue = *it;
    if (nullptr != pvalue) {
      QString str = QString("Value: %1").arg(pvalue->getName().c_str());
      ui->listValues->addItem(str);
      // Set data to register index
      ui->listValues->item(ui->listValues->count() - 1)->setData(Qt::UserRole, idx);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// getValueList
//

std::deque<CMDF_Value*>*
CDlgMdfValueList::getValueList(void)
{
  std::deque<CMDF_Value*>* pvalues = nullptr;

  // pbits = m_pobj->getListBits();
  if (mdf_type_register == m_type) {
    pvalues = ((CMDF_Register*)m_pobj)->getListValues();
  }
  else if (mdf_type_remotevar == m_type) {
    pvalues = ((CMDF_RemoteVariable*)m_pobj)->getListValues();
  }
  else if (mdf_type_action_param == m_type) {
    pvalues = ((CMDF_ActionParameter*)m_pobj)->getListValues();
  }
  else if (mdf_type_event_data_item == m_type) {
    pvalues = ((CMDF_EventData*)m_pobj)->getListValues();
  }
  // Bit definitions values
  else if ((mdf_type_bit_item == m_type) || (mdf_type_bit_sub_item == m_type)) {
    pvalues = ((CMDF_Bit*)m_pobj)->getListValues();
  }

  return pvalues;
}



///////////////////////////////////////////////////////////////////////////////
// addValue
//

void
CDlgMdfValueList::addValue(void)
{
  bool ok;
  CMDF_Value* pvaluenew = new CMDF_Value();
  if (nullptr == pvaluenew) {
    QMessageBox::critical(this, tr("MDF value information"), tr("Memory problem"));
    spdlog::error("MDF information - Memory problem");
    return;
  }

  // Save the selected row
  int idx = ui->listValues->currentRow();

  CDlgMdfValue dlg(this);
  dlg.initDialogData(pvaluenew);
  dlg.setWindowTitle(tr("Add value"));

addvaluedlg:

  if (QDialog::Accepted == dlg.exec()) {
    uint8_t mask;
    // if ((mask = checkIfBitsOverlap(pvaluenew))) {
    //   QMessageBox::warning(this, tr("Add new bit definition"), tr("Can not add bit definition. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
    //   goto addbitdlg;
    // }
    std::deque<CMDF_Value*>* pvalues = getValueList();
    if (nullptr == pvalues) {
      QMessageBox::warning(this, tr(APPNAME), tr("Unable to add value as there is no valuelist [%s] ").arg(static_cast<int>(m_type)));
      delete pvaluenew;
      return;
    }
    pvalues->push_back(pvaluenew);
    renderValueItems();
  }
  else {
    delete pvaluenew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editValue
//

void
CDlgMdfValueList::editValue(void)
{
  bool ok;

  qDebug() << "Type = " << m_type;

  if (-1 != ui->listValues->currentRow()) {

    // Save the selected row
    int idx = ui->listValues->currentRow();

    QListWidgetItem* pitem = ui->listValues->currentItem();
    CMDF_Value* pvalue         = getValueList()->at(pitem->data(Qt::UserRole).toUInt());

    CDlgMdfValue dlg(this);
    dlg.initDialogData(pvalue);

  editvaluedlg:

    if (QDialog::Accepted == dlg.exec()) {
      uint8_t mask;
      // if ((mask = checkIfBitsOverlap(pvalue, true))) {
      //   QMessageBox::warning(this, tr("Edit register value"), tr("Can not add register value. Bits overlap with already defined bits 0b%1").arg(mask, 8, 2, QChar('0')));
      //   goto editvaluedlg;
      // }
      ui->listValues->clear();
      renderValueItems();
      ui->listValues->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupValue
//

void
CDlgMdfValueList::dupValue(void)
{
  if (-1 != ui->listValues->currentRow()) {

    // Save the selected row
    int idx = ui->listValues->currentRow();

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
// deleteValue
//

void
CDlgMdfValueList::deleteValue(void)
{
  if (-1 != ui->listValues->currentRow()) {

    // Save the row
    int idx = ui->listValues->currentRow();

    QListWidgetItem* pitem = ui->listValues->currentItem();
    CMDF_Value* pvalue         = getValueList()->at(pitem->data(Qt::UserRole).toUInt());

    std::deque<CMDF_Value*>::iterator it = getValueList()->begin() + pitem->data(Qt::UserRole).toUInt();
    getValueList()->erase(it);

    ui->listValues->clear();
    renderValueItems();
    int sel = idx;
    if (0 == idx) {
      sel = 0;
    }
    else if (getValueList()->size() == idx) {
      sel = getValueList()->size() - 1;
    }
    else {
      sel = idx + 1;
    }
    ui->listValues->setCurrentRow(sel);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfValueList::accept()
{
  std::string str;
  if (nullptr != m_pobj) {

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
