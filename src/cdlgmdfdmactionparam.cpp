// cdlgmdfactionparam.cpp
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

#include "cdlgmdfdmactionparam.h"
#include "ui_cdlgmdfdmactionparam.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfDmActionParam::pre_str_actionparam[] = "Parameter: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfDmActionParam::CDlgMdfDmActionParam(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfDmActionParam)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pactionparam = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfDmActionParam::~CDlgMdfDmActionParam()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfDmActionParam::initDialogData(CMDF* pmdf, CMDF_ActionParameter* pactionparam, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == pactionparam) {
    spdlog::error("MDF decsion matrix information - Invalid MDF register object (initDialogData)");
    return;
  }

  m_pactionparam = pactionparam;

  setName(pactionparam->getName().c_str());
  setOffset(pactionparam->getOffset());
  setMin(pactionparam->getMin());
  setMax(pactionparam->getMax());

  // // Render available actions
  renderActionParams();

  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_offset:
      ui->spinOffset->setFocus();
      break;

    case index_max:
      ui->spinMax->setFocus();
      break;  

    case index_min:
      ui->spinMin->setFocus();
      break;  

    default:
      ui->editName->setFocus();
      break;
  }

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// setOffsetReadOnly
//

void CDlgMdfDmActionParam::setOffsetReadOnly(void)
{
  ui->spinOffset->setValue(0);
  ui->spinOffset->setEnabled(false);
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfDmActionParam::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfDmActionParam::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------


uint32_t
CDlgMdfDmActionParam::getOffset(void)
{
  return ui->spinOffset->value();
};

void
CDlgMdfDmActionParam::setOffset(uint32_t offset)
{
  ui->spinOffset->setValue(offset);
};

// -----------------------------------------------------------------------

uint8_t
CDlgMdfDmActionParam::getMin(void)
{
  return ui->spinMin->value();
};

void
CDlgMdfDmActionParam::setMin(uint8_t min)
{
  ui->spinMin->setValue(min);
};

// -----------------------------------------------------------------------


uint8_t
CDlgMdfDmActionParam::getMax(void)
{
  return ui->spinMax->value();
};

void
CDlgMdfDmActionParam::setMax(uint8_t min)
{
  ui->spinMax->setValue(min);
};

// -----------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// renderActionParams
//

void CDlgMdfDmActionParam::renderActionParams(void)
{
  
}


///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfDmActionParam::accept()
{
  std::string str;
  if (nullptr != m_pactionparam) {
    m_pactionparam->setName(getName().toStdString());
    m_pactionparam->setOffset(getOffset());
    m_pactionparam->setMin(getMin());
    m_pactionparam->setMax(getMax());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
