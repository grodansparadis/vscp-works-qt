// cdlgselectdate.cpp
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

#include "cdlgselectdate.h"
#include "ui_cdlgselectdate.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QCalendarWidget>
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

CDlgSelectDate::CDlgSelectDate(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgSelectDate)
{
  ui->setupUi(this);

  connect(ui->calendar, &QCalendarWidget::clicked, this, &CDlgSelectDate::dateClicked);
  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectDate::~CDlgSelectDate()
{
  delete ui;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getYearValue
//

uint16_t
CDlgSelectDate::getYearValue(void)
{
  return ui->editYear->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// setYearValue
//

void
CDlgSelectDate::setYearValue(uint16_t year)
{
  ui->editYear->setText(QString::number(year));
}

///////////////////////////////////////////////////////////////////////////////
// getYearConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getYearConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintYear->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setYearConstraint
//

void
CDlgSelectDate::setYearConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintYear->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// getMonthValue
//

uint8_t
CDlgSelectDate::getMonthValue(void)
{
  return ui->editMonth->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// MonthValue
//

void
CDlgSelectDate::setMonthValue(uint8_t month)
{
  ui->editMonth->setText(QString::number(month));
}

///////////////////////////////////////////////////////////////////////////////
// getMonthConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getMonthConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintMonth->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setMonthConstraint
//

void
CDlgSelectDate::setMonthConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintMonth->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// getDayValue
//

uint8_t
CDlgSelectDate::getDayValue(void)
{
  return ui->editDay->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// setDayValue
//

void
CDlgSelectDate::setDayValue(uint8_t day)
{
  ui->editDay->setText(QString::number(day));
}

///////////////////////////////////////////////////////////////////////////////
// getDayConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getDayConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintDay->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setDayConstraint
//

void
CDlgSelectDate::setDayConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintDay->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// getHourValue
//

uint8_t
CDlgSelectDate::getHourValue(void)
{
  return ui->editHour->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// setHourValue
//

void
CDlgSelectDate::setHourValue(uint8_t hour)
{
  ui->editHour->setText(QString::number(hour));
}

///////////////////////////////////////////////////////////////////////////////
// getHourConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getHourConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintHour->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setHourConstraint
//

void
CDlgSelectDate::setHourConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintHour->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// getMinuteValue
//

uint8_t
CDlgSelectDate::getMinuteValue(void)
{
  return ui->editMinute->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// setMinuteValue
//

void
CDlgSelectDate::setMinuteValue(uint8_t minute)
{
  ui->editMinute->setText(QString::number(minute));
}

///////////////////////////////////////////////////////////////////////////////
// getMinuteConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getMinuteConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintMinute->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setMinuteConstraint
//

void
CDlgSelectDate::setMinuteConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintMinute->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// getSecondValue
//

uint8_t
CDlgSelectDate::getSecondValue(void)
{
  return ui->editSecond->text().toUInt();
}

///////////////////////////////////////////////////////////////////////////////
// setSecondValue
//

void
CDlgSelectDate::setSecondValue(uint8_t second)
{
  ui->editSecond->setText(QString::number(second));
}

///////////////////////////////////////////////////////////////////////////////
// getSecondConstraint
//

CSessionFilter::constraint
CDlgSelectDate::getSecondConstraint(void)
{
  return static_cast<CSessionFilter::constraint>(ui->comboConstraintSecond->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setSecondConstraint
//

void
CDlgSelectDate::setSecondConstraint(CSessionFilter::constraint op)
{
  ui->comboConstraintSecond->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// dateClicked
//

void
CDlgSelectDate::dateClicked(const QDate& date)
{
  // QDate	selectedDate() const
  setYearValue(date.year());
  setMonthValue(date.month());
  setDayValue(date.day());
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectDate::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=mqtt";
  QDesktopServices::openUrl(QUrl(link));
}