// cdlgmdfdm.cpp
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

#include "cdlgmdfdmaction.h"

#include "cdlgmdfdm.h"
#include "ui_cdlgmdfdm.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfDM::pre_str_dm[] = "Decision Matrix: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfDM::CDlgMdfDM(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfDM)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pdm = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfDM::~CDlgMdfDM()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfDM::initDialogData(CMDF* pmdf, CMDF_DecisionMatrix* pdm, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == pdm) {
    spdlog::error("MDF decsion matrix information - Invalid MDF register object (initDialogData)");
    return;
  }

  m_pdm = pdm;

  // Connect edit action
  connect(ui->btnEditAction,
          SIGNAL(clicked()),
          this,
          SLOT(editAction()));

  // Connect add action
  connect(ui->btnAddAction,
          SIGNAL(clicked()),
          this,
          SLOT(addAction()));

  // Connect dup action
  connect(ui->btnDupAction,
          SIGNAL(clicked()),
          this,
          SLOT(dupAction()));

  // Connect dup action
  connect(ui->btnDelAction,
          SIGNAL(clicked()),
          this,
          SLOT(deleteAction()));

  setLevel(pmdf->getLevel());
  ui->comboLevel->setEnabled(false);

  setOffset(pdm->getStartOffset());
  setRows(pdm->getRowCount());

  if (0 == pmdf->getLevel()) {
    setPage(pdm->getStartPage());
    setSize(8);
    ui->spinRowSize->setEnabled(false);
  }
  else {
    setPage(0);
    ui->spinStartPage->setEnabled(false);
    setSize(pdm->getRowSize());
  }

  // Render available actions
  renderActions();

  switch (index) {
    case index_level:
      ui->comboLevel->setFocus();
      break;

    case index_start_page:
      ui->spinStartPage->setFocus();
      break;

    case index_start_offset:
      ui->editStartOffset->setFocus();
      break;

    case index_rows:
      ui->spinRowCnt->setFocus();
      break;

    case index_row_size:
      ui->spinRowSize->setFocus();
      break;

    default:
      ui->comboLevel->setFocus();
      break;
  }

  this->setFixedSize(this->size());
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

int
CDlgMdfDM::getLevel(void)
{
  return ui->comboLevel->currentIndex() + 1;
};

void
CDlgMdfDM::setLevel(int level)
{
  level--;
  if (level < 0) {
    level = 0;
  }
  ui->comboLevel->setCurrentIndex(level);
};

// -----------------------------------------------------------------------

uint16_t
CDlgMdfDM::getPage(void)
{
  return ui->spinStartPage->value();
}

void
CDlgMdfDM::setPage(uint16_t page)
{
  ui->spinStartPage->setValue(page);
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfDM::getOffset(void)
{
  return vscp_readStringValue(ui->editStartOffset->text().toStdString());
}

void
CDlgMdfDM::setOffset(uint32_t offset)
{
  ui->editStartOffset->setText(QString("%1").arg(offset));
}

// -----------------------------------------------------------------------

uint16_t
CDlgMdfDM::getRows(void)
{
  return ui->spinRowCnt->value();
}

void
CDlgMdfDM::setRows(uint16_t rows)
{
  ui->spinRowCnt->setValue(rows);
}

// -----------------------------------------------------------------------

uint16_t
CDlgMdfDM::getSize(void)
{
  return ui->spinRowSize->value();
}

void
CDlgMdfDM::setSize(uint16_t cnt)
{
  ui->spinRowSize->setValue(cnt);
}

// -----------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// renderActions
//

void
CDlgMdfDM::renderActions(void)
{
  if (nullptr == m_pdm) {
    return;
  }

  ui->listActions->clear();

  std::deque<CMDF_Action*>* pActionList = m_pdm->getActionList();

  if (nullptr != pActionList) {
    for (int i = 0; i < pActionList->size(); i++) {
      CMDF_Action* paction = (*pActionList)[i];
      if (nullptr != paction) {
        QString str            = QString("%1 %2").arg(paction->getCode()).arg(paction->getName().c_str());
        QListWidgetItem* pitem = new QListWidgetItem(str, ui->listActions, QListWidgetItem::UserType);
        if (nullptr != pitem) {
          pitem->setData(QListWidgetItem::UserType, paction->getCode());
          ui->listActions->addItem(pitem);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editAction
//

void
CDlgMdfDM::editAction(void)
{
  // bool ok;

  if (-1 != ui->listActions->currentRow()) {

    // Save the selected row
    int idx = ui->listActions->currentRow();

    QListWidgetItem* pitem = ui->listActions->currentItem();
    printf("data=%d\n", pitem->data(QListWidgetItem::UserType).toUInt());
    CMDF_Action* paction = m_pdm->getAction(pitem->data(QListWidgetItem::UserType).toUInt());

    CDlgMdfDmAction dlg(this);
    dlg.initDialogData(m_pmdf, paction);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listActions->clear();
      renderActions();
      ui->listActions->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// addAction
//

void
CDlgMdfDM::addAction(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listActions->currentRow();

  QListWidgetItem* pitem = ui->listActions->currentItem();
  CMDF_Action* paction   = new CMDF_Action; // = m_pdm->getAction(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == paction) {
    return;
  }

  CDlgMdfDmAction dlg(this);
  // adddlg:

  dlg.initDialogData(m_pmdf, paction);
  if (QDialog::Accepted == dlg.exec()) {
    if (!m_pdm->addAction(paction)) {
      QMessageBox::warning(this, tr("MDF add new action"), tr("Action with code %1 is already define. Must be unique.").arg(paction->getCode()));
    }
    ui->listActions->clear();
    renderActions();
    ui->listActions->setCurrentRow(idx);
  }
  else {
    delete paction;
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupAction
//

void
CDlgMdfDM::dupAction(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listActions->currentRow();

  QListWidgetItem* pitem = ui->listActions->currentItem();
  CMDF_Action* paction   = m_pdm->getAction(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == paction) {
    return;
  }

  CMDF_Action* pactionnew = new (CMDF_Action);
  if (nullptr == pactionnew) {
    return;
  }

  pactionnew->setCode(paction->getCode());
  pactionnew->setName(paction->getName());

adddlg:
  CDlgMdfDmAction dlg(this);
  dlg.initDialogData(m_pmdf, pactionnew);
  // If DM is level I only offset 0 is allowd
  if (VSCP_LEVEL1 == m_pmdf->getLevel()) {
    // dlg.setOffsetReadOnly();
  }
  if (QDialog::Accepted == dlg.exec()) {
    if (!m_pdm->addAction(pactionnew)) {
      QMessageBox::warning(this, tr("MDF add new action"), tr("Action with code %1 is already define. Must be unique.").arg(paction->getCode()));
      goto adddlg;
    }
    ui->listActions->clear();
    renderActions();
    ui->listActions->setCurrentRow(idx);
  }
  else {
    delete paction;
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteAction
//

void
CDlgMdfDM::deleteAction(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listActions->currentRow();

  QListWidgetItem* pitem = ui->listActions->currentItem();
  CMDF_Action* paction   = m_pdm->getAction(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == paction) {
    return;
  }

  if (QMessageBox::No == QMessageBox::question(this,
                                               tr("MDF delete action"),
                                               tr("Delete action with code %1.").arg(paction->getCode()))) {
    return;
  }

  if (!m_pdm->deleteAction(paction)) {
    QMessageBox::warning(this, tr("MDF add new action"), tr("Failed to remove action with code %1.").arg(paction->getCode()));
  }

  renderActions();
  ui->listActions->setCurrentRow(idx);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfDM::accept()
{
  std::string str;
  if (nullptr != m_pdm) {
    m_pdm->setLevel(getLevel());
    m_pdm->setStartPage(getPage());
    m_pdm->setStartOffset(getOffset());
    m_pdm->setRowCount(getRows());
    m_pdm->setRowSize(getSize());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
