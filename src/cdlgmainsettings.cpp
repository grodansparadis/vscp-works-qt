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

#include "mainwindow.h"
#include "cdlgmainsettings.h"
#include "../ui/ui_cdlgmainsettings.h"

#include "cfrmsession.h"

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

    // * * * General * * *

    // Numeric base
    //m_baseIndex = pworks->m_base;
    ui->comboNumberBase->setCurrentIndex(static_cast<int>(pworks->m_base));
    //onBaseChange(static_cast<int>(m_baseIndex));

    // Ask befor delete/clear
    ui->chkAskOnDelete->setChecked(pworks->m_bAskBeforeDelete);    

    // Log level
    ui->comboLogLevel->setCurrentIndex(pworks->m_logLevel);

    // * * * Session Window tab * * *

    // Max number of session events
    ui->editMaxSessionEvents->setText(QString::number(pworks->m_session_maxEvents));

    // Class display format
    ui->comboClassDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_ClassDisplayFormat));

    // Type display format
    ui->comboTypeDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_TypeDisplayFormat));

    // GUID display format
    ui->comboGuidDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_GuidDisplayFormat));

    // Automatic connect  
    ui->chkAutomaticConnect->setChecked(pworks->m_session_bAutoConnect);

    // VSCP type token format  
    ui->chkShowFullToken->setChecked(pworks->m_session_bShowFullTypeToken);
    

    // * * * Data tab * * *

    // Local storage folder
    ui->pathLocalStorage->setText(pworks->m_shareFolder);

    // VSCP home folder
    ui->pathVscpHome->setText(pworks->m_vscpHomeFolder);

    // Path to config folder
    ui->pathConfigFile->setText(pworks->m_configFolder);

    // Path to event db
    {
        QString path = pworks->m_shareFolder;
        path += "vscp_events.sqlite3";
        ui->pathVscpEventDb->setText(path);
    }
    
    // Event DB last download
    QString str = pworks->m_lastEventDbLoadDateTime.toString(Qt::ISODate);
    str += " @ server [";
    str += pworks->m_lastEventDbServerDateTime.toString(Qt::ISODate);
    str += "]";
    ui->lastDownload->setText(str);

    connect(ui->btnDownLoadNewEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onDownloadEventDb);
    connect(ui->btnReLoadEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onReLoadEventDb ); 
    

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

void CDlgMainSettings::done(int rv)
{
    if (QDialog::Accepted == rv) { // ok was pressed
        
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

        // General
        pworks->m_base = static_cast<numerical_base>(ui->comboNumberBase->currentIndex());
        pworks->m_bAskBeforeDelete = ui->chkAskOnDelete->isChecked();
        pworks->m_logLevel = ui->comboLogLevel->currentIndex();

        // Session window
        pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
        pworks->m_session_ClassDisplayFormat = 
            static_cast<CFrmSession::classDisplayFormat>(ui->comboClassDisplayFormat->currentIndex());
        pworks->m_session_TypeDisplayFormat = 
            static_cast<CFrmSession::typeDisplayFormat>(ui->comboTypeDisplayFormat->currentIndex());
        pworks->m_session_GuidDisplayFormat = 
            static_cast<CFrmSession::guidDisplayFormat>(ui->comboGuidDisplayFormat->currentIndex());
        pworks->m_session_bAutoConnect = ui->chkAutomaticConnect->isChecked();
        pworks->m_session_bShowFullTypeToken = ui->chkShowFullToken->isChecked();
        

        // Data

        pworks->writeSettings();
    }
    QDialog::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void CDlgMainSettings::onBaseChange(int index)
{
    
}

///////////////////////////////////////////////////////////////////////////////
// onDownloadEventDb
//

void CDlgMainSettings::onDownloadEventDb(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    //pworks->writeSettings();
    //QWidget* widget = this-> parentWidget();
    MainWindow *main = (MainWindow *)this-> parentWidget();
    main->initForcedRemoteEventDbFetch();
}

///////////////////////////////////////////////////////////////////////////////
// onReLoadEventDb
//

void CDlgMainSettings::onReLoadEventDb(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    if (!pworks->loadEventDb()) {
        QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("Unable to load events from VSCP event database."),
                                QMessageBox::Ok );
        return;                                
    }
    else {
        QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("Events reloaded from event database."),
                                QMessageBox::Ok );
    }
}