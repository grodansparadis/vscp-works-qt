// cdlgmdfeventdata.cpp
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

#include "cdlgmdfeventdata.h"
#include "ui_cdlgmdfeventdata.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfEventData::CDlgMdfEventData(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfEventData)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_pmdf       = nullptr;
  m_peventdata = nullptr;

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfEventData::~CDlgMdfEventData()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfEventData::initDialogData(CMDF* pmdf, CMDF_EventData* peventdata, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  if (nullptr == peventdata) {
    spdlog::error("MDF module information - Invalid event data object (initDialogData)");
    return;
  }

  m_pmdf       = pmdf;
  m_peventdata = peventdata;

  setName(peventdata->getName().c_str());
  setOffset(peventdata->getOffset());

  switch (index) {
    case index_offset:
      ui->spinOffset->setFocus();
      break;

    case index_name:
      ui->editName->setFocus();
      break;

    default:
      ui->spinOffset->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfEventData::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setLevel1
//

void
CDlgMdfEventData::setLevel1(void)
{
  ui->spinOffset->setMaximum(7);
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

void
CDlgMdfEventData::setOffset(uint8_t offset)
{
  ui->spinOffset->setValue(offset);
}

uint8_t
CDlgMdfEventData::getOffset(void)
{
  return ui->spinOffset->value();
}

//-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

void
CDlgMdfEventData::setName(const QString& str)
{
  ui->editName->setText(str);
}

QString
CDlgMdfEventData::getName(void)
{
  return ui->editName->text();
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfEventData::accept()
{
  std::string str;

  str = vscp_trim_copy(getName().toStdString()).c_str();
  if (!str.size()) {
    QMessageBox::warning(this, tr(APPNAME), tr("Event data must have a name"), QMessageBox::Ok);
    return;
  }

  if (nullptr != m_peventdata) {
    m_peventdata->setOffset(getOffset());
    m_peventdata->setName(getName().toStdString());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
