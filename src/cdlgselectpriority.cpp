// cdlgselectpriority.cpp
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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "sessionfilter.h"

#include "cdlgselectpriority.h"
#include "ui_cdlgselectpriority.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectPriority::CDlgSelectPriority(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectPriority)
{
    ui->setupUi(this);
  
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectPriority::~CDlgSelectPriority()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getPriorityValue
//

uint8_t CDlgSelectPriority::getPriorityValue(void)
{
    return ui->comboPriority->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// setPriorityValue
//

void CDlgSelectPriority::setPriorityValue(uint8_t value)
{
    ui->comboPriority->setCurrentIndex(value & 7);    
}

///////////////////////////////////////////////////////////////////////////////
// getPriorityConstraint
//

CSessionFilter::constraint 
CDlgSelectPriority::getPriorityConstraint(void)
{
    return static_cast<CSessionFilter::constraint>(ui->comboComparePriority->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setPriorityConstraint
//

void CDlgSelectPriority::setPriorityConstraint(CSessionFilter::constraint op)
{
    ui->comboComparePriority->setCurrentIndex(static_cast<int>(op));
}


// ----------------------------------------------------------------------------




