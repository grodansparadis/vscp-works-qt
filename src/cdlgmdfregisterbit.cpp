// cdlgmdfregisterbit.cpp
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
#include "ui_cdlgmdfregisterbit.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfRegisterBit::pre_str_registerbit[] = "Register bit: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfRegisterBit::CDlgMdfRegisterBit(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRegisterBit)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pbit = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRegisterBit::~CDlgMdfRegisterBit()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRegisterBit::initDialogData(CMDF* pmdf, CMDF_Bit* pbit, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register bit information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == pbit) {
    spdlog::error("MDF register bit information - Invalid MDF register bit object (initDialogData)");
    return;
  }

  m_pbit = pbit;

  setName(pbit->getName().c_str());
  setPos(pbit->getPos());
  setWidth(pbit->getWidth());
  setDefault(pbit->getDefault());
  setMin(pbit->getMin());
  setMax(pbit->getMax());
  setAccess(pbit->getAccess());

  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_pos:
      ui->spinPos->setFocus();
      break;

    case index_width:
      ui->spinWidth->setFocus();
      break;

    case index_default:
      ui->spinDefault->setFocus();
      break;

    case index_min:
      ui->spinMin->setFocus();
      break;

    case index_max:
      ui->spinMax->setFocus();
      break;

    case index_access:
      ui->spinMin->setFocus();
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
CDlgMdfRegisterBit::setInitialFocus(void)
{
  // ui->editName->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfRegisterBit::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfRegisterBit::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegisterBit::getPos(void)
{
  return ui->spinPos->value();
}

void
CDlgMdfRegisterBit::setPos(uint8_t span)
{
  ui->spinPos->setValue(span);
}


// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegisterBit::getWidth(void)
{
  return ui->spinWidth->value();
}

void
CDlgMdfRegisterBit::setWidth(uint8_t span)
{
  ui->spinWidth->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegisterBit::getDefault(void)
{
  return ui->spinDefault->value();
}

void
CDlgMdfRegisterBit::setDefault(uint8_t span)
{
  ui->spinDefault->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegisterBit::getMin(void)
{
  return ui->spinMin->value();
}

void
CDlgMdfRegisterBit::setMin(uint8_t min)
{
  ui->spinMin->setValue(min);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegisterBit::getMax(void)
{
  return ui->spinMax->value();
}

void
CDlgMdfRegisterBit::setMax(uint8_t span)
{
  ui->spinMax->setValue(span);
}

// -----------------------------------------------------------------------

mdf_access_mode
CDlgMdfRegisterBit::getAccess(void)
{
  return static_cast<mdf_access_mode>(ui->comboAccess->currentIndex());
}

void
CDlgMdfRegisterBit::setAccess(uint8_t access)
{
  if (access <= MDF_REG_ACCESS_READ_WRITE) {
    ui->comboAccess->setCurrentIndex(access);
  }
}



///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegisterBit::accept()
{
  std::string str;
  if (nullptr != m_pbit) {
    m_pbit->setName(getName().toStdString());
    m_pbit->setName(getName().toStdString().c_str());
    m_pbit->setPos(getPos());
    m_pbit->setWidth(getWidth());
    m_pbit->setMin(getMin());
    m_pbit->setMax(getMax());
    m_pbit->setAccess(getAccess());
    m_pbit->setDefault(getDefault());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
