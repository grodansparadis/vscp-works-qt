// cdlgtls.cpp
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

#include "cdlgtls.h"
#include "ui_cdlgtls.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgTLS::CDlgTLS(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgTLS)
{
    ui->setupUi(this);

}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgTLS::~CDlgTLS()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setDebug
//

void CDlgTLS::setDebug(bool bDebug)
{
    //ui->chkEnableDebug->setChecked(bDebug);
}

///////////////////////////////////////////////////////////////////////////////
// getDebug
//

bool CDlgTLS::getDebug(void)
{
    //return ui->chkEnableDebug->isChecked();
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setFd
//

void CDlgTLS::setFd(bool bFd)
{
    //ui->chkEnableFd->setChecked(bFd);
}

///////////////////////////////////////////////////////////////////////////////
// getFd
//

bool CDlgTLS::getFd(void)
{
    //return ui->chkEnableFd->isChecked();
    return false;
}



