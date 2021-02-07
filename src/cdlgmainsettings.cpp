// cdlgmainsettings.cpp
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

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "cdlgmainsettings.h"
#include "ui_cdlgmainsettings.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMainSettings::CDlgMainSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgMainSettings)
{
    ui->setupUi(this);

    connect(ui->comboNumberBase, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onBaseChange(int))); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();    

    // Numeric base
    //m_baseIndex = pworks->m_base;
    ui->comboNumberBase->setCurrentIndex(static_cast<int>(pworks->m_base));
    //onBaseChange(static_cast<int>(m_baseIndex));

    // VSCP home folder

    //connect(ui->CDlgMainSettings, &QDialog::accepted(), this, &CDlgMainSettings::accepted ); 

    // Hook to row double clicked
    //connect(ui->listWidgetConnectionTypes, &QListWidget::itemDoubleClicked, this, &CDlgLevel1Filter::onDoubleClicked );           
    
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMainSettings::~CDlgMainSettings()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void CDlgMainSettings::done(int r)
{
    if (QDialog::Accepted == r) { // ok was pressed
        
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
        pworks->m_base = static_cast<numerical_base>(ui->comboNumberBase->currentIndex());
        pworks->writeSettings();
    }
    QDialog::done(r);
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void CDlgMainSettings::onBaseChange(int index)
{

}