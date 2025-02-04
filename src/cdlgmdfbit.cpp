// cdlgmdfregisterbit.cpp
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

#include "cdlgmdfbit.h"
#include "ui_cdlgmdfbit.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfBit::pre_str_registerbit[] = "Register bit: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfBit::CDlgMdfBit(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfBit)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pbit            = nullptr;
  m_index           = 0;
  m_type            = mdf_type_unknown;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfBit::~CDlgMdfBit()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfBit::initDialogData(CMDF_Bit* pbit, int index, mdf_record_type type)
{
  QString str;

  if (nullptr == pbit) {
    spdlog::error("MDF register bit information - Invalid MDF register bit object (initDialogData)");
    return;
  }

  m_pbit  = pbit;
  m_index = index;
  m_type  = type;

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

  if (mdf_type_register == type) {
    setWindowTitle("Register bit definitions");
  }
  else if (mdf_type_remotevar == type) {
    setWindowTitle("Remote variable bit definitions");
  }
  else if (mdf_type_alarm == type) {
    setWindowTitle("Alarm bit definitions");
    ui->spinWidth->setValue(1);
    ui->spinWidth->setEnabled(false);
    ui->spinDefault->setValue(0);
    ui->spinDefault->setEnabled(false);
    ui->spinMin->setValue(0);
    ui->spinMin->setEnabled(false);
    ui->spinMax->setValue(1);
    ui->spinMax->setEnabled(false);
    ui->comboAccess->setCurrentIndex(3);
    ui->comboAccess->setEnabled(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfBit::setInitialFocus(void)
{
  // ui->editName->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfBit::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfBit::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

uint8_t
CDlgMdfBit::getPos(void)
{
  return ui->spinPos->value();
}

void
CDlgMdfBit::setPos(uint8_t span)
{
  ui->spinPos->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfBit::getWidth(void)
{
  return ui->spinWidth->value();
}

void
CDlgMdfBit::setWidth(uint8_t span)
{
  ui->spinWidth->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfBit::getDefault(void)
{
  return ui->spinDefault->value();
}

void
CDlgMdfBit::setDefault(uint8_t span)
{
  ui->spinDefault->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfBit::getMin(void)
{
  return ui->spinMin->value();
}

void
CDlgMdfBit::setMin(uint8_t min)
{
  ui->spinMin->setValue(min);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfBit::getMax(void)
{
  return ui->spinMax->value();
}

void
CDlgMdfBit::setMax(uint8_t span)
{
  ui->spinMax->setValue(span);
}

// -----------------------------------------------------------------------

mdf_access_mode
CDlgMdfBit::getAccess(void)
{
  return static_cast<mdf_access_mode>(ui->comboAccess->currentIndex());
}

void
CDlgMdfBit::setAccess(uint8_t access)
{
  if (access <= MDF_REG_ACCESS_READ_WRITE) {
    ui->comboAccess->setCurrentIndex(access);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfBit::accept()
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

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMdfBit::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}