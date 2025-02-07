// cdlgcanfilter.h
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

#include "cdlgcanfilter.h"
#include "ui_cdlgcanfilter.h"

#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QDesktopServices>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgCanFilter::CDlgCanFilter(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgCanFilter)
{
  ui->setupUi(this);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgCanFilter::~CDlgCanFilter()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgCanFilter::setInitialFocus(void)
{
  ui->canid->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// getCanId
//

std::string
CDlgCanFilter::getCanId(void)
{
  return (ui->canid->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setCanId
//

void
CDlgCanFilter::setCanId(const std::string& str)
{
  ui->canid->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getCanMask
//

std::string
CDlgCanFilter::getCanMask(void)
{
  return (ui->canmask->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setCanMask
//

void
CDlgCanFilter::setCanMask(const std::string& str)
{
  ui->canmask->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgCanFilter::showHelp()
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=filter";
  QDesktopServices::openUrl(QUrl(link));
}
