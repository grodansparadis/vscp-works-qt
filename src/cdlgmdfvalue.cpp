// cdlgmdfvalue.cpp
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

#include "cdlgmdfvalue.h"
#include "ui_cdlgmdfvalue.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfValue::pre_str_registerbit[] = "Register value: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfValue::CDlgMdfValue(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfValue)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pvalue = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfValue::~CDlgMdfValue()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfValue::initDialogData(CMDF_Value* pvalue, int index)
{
  QString str;

  if (nullptr == pvalue) {
    spdlog::error("MDF register value information - Invalid MDF register value object (initDialogData)");
    return;
  }

  m_pvalue = pvalue;

  setName(pvalue->getName().c_str());
  str = pvalue->getValue().c_str();
  setValue(str);

  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_value:
      ui->editValue->setFocus();
      break;

    default:
      ui->editName->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfValue::setInitialFocus(void)
{
  // ui->editName->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfValue::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfValue::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

QString
CDlgMdfValue::getValue(void)
{
  return ui->editValue->text();
};

void
CDlgMdfValue::setValue(const QString& name)
{
  ui->editValue->setText(name);
};


///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfValue::accept()
{
  std::string str;
  if (nullptr != m_pvalue) {
    m_pvalue->setName(getName().toStdString());
    m_pvalue->setValue(getValue().toStdString());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
