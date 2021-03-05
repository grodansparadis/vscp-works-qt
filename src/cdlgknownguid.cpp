// cdlgknownguid.cpp
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

#include "mainwindow.h"
#include "cdlgknownguid.h"
#include "../ui/ui_cdlgknownguid.h"

#include "cfrmsession.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgKnownGuid::CDlgKnownGuid(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgKnownGuid)
{
    ui->setupUi(this);

    connect(ui->btnSearch, &QPushButton::clicked, this, &CDlgKnownGuid::btnSearch);
    connect(ui->btnAdd, &QPushButton::clicked, this, &CDlgKnownGuid::btnAdd); 
    connect(ui->btnEdit, &QPushButton::clicked, this, &CDlgKnownGuid::btnEdit); 
    connect(ui->btnClone, &QPushButton::clicked, this, &CDlgKnownGuid::btnClone); 
    connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgKnownGuid::btnDelete);  
    connect(ui->btnLoad, &QPushButton::clicked, this, &CDlgKnownGuid::btnLoad);
    connect(ui->btnSave, &QPushButton::clicked, this, &CDlgKnownGuid::btnSave);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();    


    // Max number of session events
    //ui->editMaxSessionEvents->setText(QString::number(pworks->m_session_maxEvents));

    

    //connect(ui->btnDownLoadNewEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onDownloadEventDb);
    //connect(ui->btnReLoadEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onReLoadEventDb ); 
    

    // Hook to row double clicked
    //connect(ui->listWidgetConnectionTypes, &QListWidget::itemDoubleClicked, this, &CDlgLevel1Filter::onDoubleClicked );           
    
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgKnownGuid::~CDlgKnownGuid()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void CDlgKnownGuid::done(int rv)
{
    if (QDialog::Accepted == rv) { // ok was pressed
        
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();


        // Session window
        //pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
        
    }
    QDialog::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void  CDlgKnownGuid::btnSearch(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void  CDlgKnownGuid::btnAdd(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void  CDlgKnownGuid::btnEdit(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void  CDlgKnownGuid::btnClone(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void  CDlgKnownGuid::btnDelete(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnLoad
//

void  CDlgKnownGuid::btnLoad(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSave
//

void  CDlgKnownGuid::btnSave(void)
{
    int i = 8;
}

