// cdlgmdfaction.cpp
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

#include "cdlgmdfdmactionparam.h"

#include "cdlgmdfdmaction.h"
#include "ui_cdlgmdfdmaction.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfDmAction::pre_str_dm_action[] = "Actions: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfDmAction::CDlgMdfDmAction(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfDmAction)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_paction = nullptr;

  // Connect edit action
  connect(ui->btnEditActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(editActionParam()));

  // Connect add action
  connect(ui->btnAddActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(addActionParam()));

  // Connect dup action
  connect(ui->btnDupActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(dupActionParam()));

  // Connect dup action
  connect(ui->btnDelActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(delActionParam()));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfDmAction::~CDlgMdfDmAction()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfDmAction::initDialogData(CMDF* pmdf, CMDF_Action* paction, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == paction) {
    spdlog::error("MDF decsion matrix information - Invalid MDF register object (initDialogData)");
    return;
  }

  m_paction = paction;

  // Connect edit action
  connect(ui->btnEditActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(editAction()));

  // Connect add action
  connect(ui->btnAddActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(addAction()));

  // Connect dup action
  connect(ui->btnDupActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(dupAction()));

  // Connect dup action
  connect(ui->btnDelActionParam,
          SIGNAL(clicked()),
          this,
          SLOT(delAction()));

  setCode(paction->getCode());
  setName(paction->getName().c_str());

  // Render available actions
  renderActionParams();

  switch (index) {
    case index_code:
      ui->spinCode->setFocus();
      break;

    case index_name:
      ui->editName->setFocus();
      break;

    default:
      ui->spinCode->setFocus();
      break;
  }

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// setReadOnly
//

void
CDlgMdfDmAction::setReadOnly(void)
{
  ui->spinCode->setEnabled(false);
  ui->editName->setEnabled(false);
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

uint8_t
CDlgMdfDmAction::getCode(void)
{
  return ui->spinCode->value();
};

void
CDlgMdfDmAction::setCode(uint8_t code)
{
  ui->spinCode->setValue(code);
};

// -----------------------------------------------------------------------

QString
CDlgMdfDmAction::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfDmAction::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// renderActions
//

void
CDlgMdfDmAction::renderActionParams(void)
{
  if (nullptr == m_paction) {
    return;
  }

  ui->listActionParams->clear();

  std::deque<CMDF_ActionParameter*>* pActionParamList = m_paction->getListActionParameter();

  if (nullptr != pActionParamList) {
    for (int i = 0; i < pActionParamList->size(); i++) {
      CMDF_ActionParameter* pactionparam = (*pActionParamList)[i];
      if (nullptr != pactionparam) {
        QString str = QString("Offset: %1 - %2").arg(pactionparam->getOffset()).arg(pactionparam->getName().c_str());
        QListWidgetItem* pitem = new QListWidgetItem(str, ui->listActionParams);
        if (nullptr != pitem) {
          pitem->setData(QListWidgetItem::UserType,pactionparam->getOffset());
          ui->listActionParams->addItem(pitem);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editActionParam
//

void
CDlgMdfDmAction::editActionParam(void)
{
  bool ok;

  if (-1 != ui->listActionParams->currentRow()) {

    // Save the selected row
    int idx = ui->listActionParams->currentRow();

    QListWidgetItem* pitem = ui->listActionParams->currentItem();
    printf("data=%d\n",pitem->data(QListWidgetItem::UserType).toUInt());
    CMDF_ActionParameter* pactionparam   = m_paction->getActionParam(pitem->data(QListWidgetItem::UserType).toUInt());

    CDlgMdfDmActionParam dlg(this);
    dlg.initDialogData(m_pmdf, pactionparam);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listActionParams->clear();
      renderActionParams();
      ui->listActionParams->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// addActionParam
//

void
CDlgMdfDmAction::addActionParam(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// dupActionParam
//

void
CDlgMdfDmAction::dupActionParam(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// deleteActionParam
//

void
CDlgMdfDmAction::deleteActionParam(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfDmAction::accept()
{
  std::string str;
  if (nullptr != m_paction) {
    m_paction ->setCode(getCode());
    m_paction->setName(getName().toStdString());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
