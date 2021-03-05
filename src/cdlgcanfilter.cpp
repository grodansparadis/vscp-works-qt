// cdlgcanfilter.h
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

#include "cdlgcanfilter.h"
#include "../ui/ui_cdlgcanfilter.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgCanFilter::CDlgCanFilter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgCanFilter)
{
    ui->setupUi(this);
    
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

void CDlgCanFilter::setInitialFocus(void)
{
    ui->canid->setFocus();
}


///////////////////////////////////////////////////////////////////////////////
// getCanId
//

std::string CDlgCanFilter::getCanId(void)
{
    return (ui->canid->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setCanId
//

void CDlgCanFilter::setCanId(const std::string& str)
{
    ui->canid->setText(str.c_str());
}


///////////////////////////////////////////////////////////////////////////////
// getCanMask
//

std::string CDlgCanFilter::getCanMask(void)
{
    return (ui->canmask->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setCanMask
//

void CDlgCanFilter::setCanMask(const std::string& str)
{
    ui->canmask->setText(str.c_str());
}

