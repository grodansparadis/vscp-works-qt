// cdlgselectmeasurementvalue.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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
#include "StdAfx.h"
#endif

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "sessionfilter.h"

#include "cdlgselectmeasurementvalue.h"
#include "ui_cdlgselectmeasurementvalue.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectMeasurementValue::CDlgSelectMeasurementValue(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectMeasurementValue)
{
    ui->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectMeasurementValue::~CDlgSelectMeasurementValue()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// getMeasurementValue
//

double CDlgSelectMeasurementValue::getMeasurementValue(void)
{
    return vscp_readStringValue(ui->editMeasurementValue->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setMeasurementValueValue
//

void CDlgSelectMeasurementValue::setMeasurementValue(double value)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance(); 
    ui->editMeasurementValue->setText(pworks->decimalToStringInBase(value));
}

///////////////////////////////////////////////////////////////////////////////
// getMeasurementValueConstraint
//

CSessionFilter::constraint CDlgSelectMeasurementValue::getMeasurementValueConstraint(void)
{
    return static_cast<CSessionFilter::constraint>(ui->comboCompareMeasurementValue->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setMeasurementValueConstraint
//

void CDlgSelectMeasurementValue::setMeasurementValueConstraint(CSessionFilter::constraint op)
{
    ui->comboCompareMeasurementValue->setCurrentIndex(static_cast<int>(op));
}


// ----------------------------------------------------------------------------




