// cdlgselectsensorindex.cpp
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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "sessionfilter.h"

#include "cdlgselectsensorindex.h"
#include "ui_cdlgselectsensorindex.h"

#include <QMessageBox>
#include <QFileDialog>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectSensorIndex::CDlgSelectSensorIndex(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectSensorIndex)
{

}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectSensorIndex::~CDlgSelectSensorIndex()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getSensorIndexValue
//

uint8_t CDlgSelectSensorIndex::getSensorIndexValue(void)
{
    return vscp_readStringValue(ui->editSensorIndex->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setSensorIndexValue
//

void CDlgSelectSensorIndex::setSensorIndexValue(uint8_t value)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance(); 
    ui->editSensorIndex->setText(pworks->decimalToStringInBase(value));
}

///////////////////////////////////////////////////////////////////////////////
// getSensorIndexConstraint
//

CSessionFilter::constraint 
CDlgSelectSensorIndex::getSensorIndexConstraint(void)
{
    return static_cast<CSessionFilter::constraint>(ui->comboCompareSensorIndex->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setSensorIndexConstraint
//

void CDlgSelectSensorIndex::setSensorIndexConstraint(CSessionFilter::constraint op)
{
    ui->comboCompareSensorIndex->setCurrentIndex(static_cast<int>(op));
}


// ----------------------------------------------------------------------------





