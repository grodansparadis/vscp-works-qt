// cdlgselectunit.cpp
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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "sessionfilter.h"

#include "cdlgselectunit.h"
#include "ui_cdlgselectunit.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectMeasurementUnit::CDlgSelectMeasurementUnit(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgSelectMeasurementUnit)
{
  ui->setupUi(this);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectMeasurementUnit::~CDlgSelectMeasurementUnit()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getMeasurementValue
//

uint8_t
CDlgSelectMeasurementUnit::getMeasurementUnitValue(void)
{
  return vscp_readStringValue(ui->editUnit->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setMeasurementValue
//

void
CDlgSelectMeasurementUnit::setMeasurementUnitValue(uint8_t value)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  ui->editUnit->setText(pworks->decimalToStringInBase(value));
}

///////////////////////////////////////////////////////////////////////////////
// getMeasurementUnitConstraint
//

CSessionFilter::constraint
CDlgSelectMeasurementUnit::getMeasurementUnitConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboCompareUnit->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setMeasurementUnitConstraint
//

void
CDlgSelectMeasurementUnit::setMeasurementUnitConstraint(CSessionFilter::constraint op)
{
  ui->comboCompareUnit->setCurrentIndex(static_cast<int>(op));
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectMeasurementUnit::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=mqtt";
  QDesktopServices::openUrl(QUrl(link));
}