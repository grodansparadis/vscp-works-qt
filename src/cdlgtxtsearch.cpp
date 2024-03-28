// cdlgtxtsearch.h
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

#include "cdlgtxtsearch.h"
#include "ui_cdlgtxtsearch.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgTxtSearch::CDlgTxtSearch(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgTxtSearch)
{
  ui->setupUi(this);

  // Fill combo
  ui->comboSearchType->clear();
  ui->comboSearchType->addItem("Exact", 0);
  ui->comboSearchType->addItem("Contains", 1);
  ui->comboSearchType->addItem("Start", 2);
  ui->comboSearchType->addItem("End", 3);
  ui->comboSearchType->addItem("Regular Expression", 4);
  ui->comboSearchType->setCurrentIndex(1);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgTxtSearch::~CDlgTxtSearch()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgTxtSearch::setInitialFocus(void)
{
  ui->editSearch->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// getSearchText
//

std::string
CDlgTxtSearch::getSearchText(void)
{
  return (ui->editSearch->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setCanId
//

void
CDlgTxtSearch::setSearchText(const std::string& str)
{
  ui->editSearch->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getSearchType
//

search_type
CDlgTxtSearch::getSearchType(void)
{
  return static_cast<search_type>(ui->comboSearchType->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setSearchType
//

void
CDlgTxtSearch::setSearchType(search_type type)
{
  ui->comboSearchType->setCurrentIndex(static_cast<int>(type));
}

///////////////////////////////////////////////////////////////////////////////
// isCaseSensitive
//

bool
CDlgTxtSearch::isCaseSensitive(void)
{
  return (2 == ui->checkCaseSensitive->checkState());
}

///////////////////////////////////////////////////////////////////////////////
// setCaseSensitive
//

void
CDlgTxtSearch::setCaseSensitive(bool bon)
{
  ui->checkCaseSensitive->setCheckState((bon ? Qt::CheckState(2) :  Qt::CheckState(0)));
}

///////////////////////////////////////////////////////////////////////////////
// isMarkSearchResults
//

bool
CDlgTxtSearch::isMarkSearchResults(void)
{
  return (2 == ui->checkMarkSearchResults->checkState());
}

///////////////////////////////////////////////////////////////////////////////
// setMarkSearchResults
//

void
CDlgTxtSearch::setMarkSearchResults(bool bon)
{
  ui->checkMarkSearchResults->setCheckState((bon ? Qt::CheckState(2) :  Qt::CheckState(0)));
}