// cdlgdatepicker.cpp
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

#include "cdlgdatepicker.h"
#include "ui_cdlgdatepicker.h"

#include <QMessageBox>
#include <QDebug>
#include <QDate>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgDatePicker::CDlgDatePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgDatePicker)
{
    ui->setupUi(this);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();        

    //connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgdatepicker::setDummyGuid);
    
    setInitialFocus();             
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgDatePicker::~CDlgDatePicker()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgDatePicker::setInitialFocus(void)
{
    // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getDate
//

QDate CDlgDatePicker::getDate(void)
{
  return ui->calendar->selectedDate(); 
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

void CDlgDatePicker::setDate(const QDate& date)
{
    ui->calendar->setSelectedDate(date);
}




