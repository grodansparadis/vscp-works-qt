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
#include "ui_cdlgknownguid.h"

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

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    connect(ui->btnSearch, &QPushButton::clicked, this, &CDlgKnownGuid::btnSearch);
    connect(ui->btnAdd, &QPushButton::clicked, this, &CDlgKnownGuid::btnAdd); 
    connect(ui->btnEdit, &QPushButton::clicked, this, &CDlgKnownGuid::btnEdit); 
    connect(ui->btnClone, &QPushButton::clicked, this, &CDlgKnownGuid::btnClone); 
    connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgKnownGuid::btnDelete);  
    connect(ui->btnLoad, &QPushButton::clicked, this, &CDlgKnownGuid::btnLoad);
    connect(ui->btnSave, &QPushButton::clicked, this, &CDlgKnownGuid::btnSave);
  

    // Max number of session events
    //ui->editMaxSessionEvents->setText(QString::number(pworks->m_session_maxEvents));

    //connect(ui->btnDownLoadNewEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onDownloadEventDb);
    //connect(ui->btnReLoadEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onReLoadEventDb ); 
    

    // Hook to row double clicked
    //connect(ui->listWidgetConnectionTypes, &QListWidget::itemDoubleClicked, this, &CDlgLevel1Filter::onDoubleClicked );

    QStringList headers(
      QString(tr("GUID, Name")).split(','));
    ui->listGuid->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu
    ui->listGuid->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listGuid->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->listGuid->setColumnCount(2);
    ui->listGuid->setColumnWidth(0, 350);  // GUID
    ui->listGuid->setColumnWidth(1, 200);  // Name
    ui->listGuid->horizontalHeader()->setStretchLastSection(true);
    ui->listGuid->setHorizontalHeaderLabels(headers);

    // Fill in GUID's
     
    pworks->m_mutexGuidMaps.lock();

    QSqlQuery queryClass("SELECT * FROM guid order by name", pworks->m_worksdb);

    while (queryClass.next()) {
        QString guid = queryClass.value(1).toString();
        QString name = queryClass.value(2).toString();

        int row = ui->listGuid->rowCount();
        ui->listGuid->insertRow(row);

        // * * * GUID
        QTableWidgetItem* itemGuid = new QTableWidgetItem(guid);

        // Not editable
        itemGuid->setFlags(itemGuid->flags() & ~Qt::ItemIsEditable);

        ui->listGuid->setItem(ui->listGuid->rowCount() - 1, 0, itemGuid);

        // * * * Name
        QTableWidgetItem* itemName = new QTableWidgetItem(name);

        // Not editable
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);

        ui->listGuid->setItem(ui->listGuid->rowCount() - 1, 1, itemName);
    }

    ui->listGuid->setUpdatesEnabled(false);
    for (int i = 0; i < ui->listGuid->rowCount(); i++) {
        ui->listGuid->setRowHeight(i, 10);
    }
    ui->listGuid->setUpdatesEnabled(true);

    pworks->m_mutexGuidMaps.unlock();
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
    /*!
        Search for GUID if ":" is in search term other wise search for name
        which must start with a letter.
    */    
    int searchType = ui->comboSearchType->currentIndex();  // 0-exact, 1=start, 2=contains

    
    int currentRow = ui->listGuid->selectionModel()->currentIndex().row();
    if (-1 == currentRow) {
        currentRow = 0; // First row
    }
    else {
        currentRow++;   // Row after the selected one
    }

    QString strsearch = ui->editSearch->text();

    for (int i=currentRow; i < ui->listGuid->rowCount(); i++) {
        
        QTableWidgetItem * itemGuid = ui->listGuid->item(i,0);
        QTableWidgetItem * itemName = ui->listGuid->item(i,1);
        
        // GUID exact match
        if (0 == searchType) {
            if (itemGuid->text() == ui->editSearch->text()) {
                //itemGuid->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            }   
        }
        // GUID starts with
        else if (1 == searchType) {
            if (itemGuid->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
                //itemGuid->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            }
        }
        // GUID contains
        else if (2 == searchType) {
            if (itemGuid->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
                itemGuid->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            }
        }
        // Name Exact match
        else if (3 == searchType) {
            if (itemName->text() == ui->editSearch->text()) {
                //itemName->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            } 
        }
        // Name starts with
        else if (4 == searchType) {
            if (itemName->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
                //itemName->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            }
        }
        // Name contains
        else if (5 == searchType) {
            if (itemName->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
                //itemName->setSelected(true); 
                ui->listGuid->selectRow(i);
                break;
            }
            else {
                ui->listGuid->clearSelection();
            }
        }
    } 
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

