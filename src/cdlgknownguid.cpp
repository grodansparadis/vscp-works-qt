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
#include <guid.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "mainwindow.h"
#include "cdlgknownguid.h"
#include "ui_cdlgknownguid.h"

#include "cdlgeditguid.h"

#include <QMessageBox>
#include <QMenu>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgKnownGuid::CDlgKnownGuid(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgKnownGuid)
{
    ui->setupUi(this);

    ui->btnSave->setVisible(false);
    ui->btnLoad->setVisible(false); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    connect(ui->listGuid, &QTableWidget::itemClicked, this, &CDlgKnownGuid::listItemClicked);
    connect(ui->listGuid, &QTableWidget::itemDoubleClicked, this, &CDlgKnownGuid::listItemDoubleClicked);

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listGuid,
            &QTableWidget::customContextMenuRequested,
            this,
            &CDlgKnownGuid::showContextMenu);

    connect(ui->btnSearch, &QPushButton::clicked, this, &CDlgKnownGuid::btnSearch);
    connect(ui->btnAdd, &QPushButton::clicked, this, &CDlgKnownGuid::btnAdd); 
    connect(ui->btnEdit, &QPushButton::clicked, this, &CDlgKnownGuid::btnEdit); 
    connect(ui->btnClone, &QPushButton::clicked, this, &CDlgKnownGuid::btnClone); 
    connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgKnownGuid::btnDelete);  
    connect(ui->btnLoad, &QPushButton::clicked, this, &CDlgKnownGuid::btnLoad);
    connect(ui->btnSave, &QPushButton::clicked, this, &CDlgKnownGuid::btnSave);

    ui->textDescription->acceptRichText();

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

        insertGuidItem(guid, name);
    }

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
// insertGuidItem
//

void CDlgKnownGuid::insertGuidItem(QString guid, QString name) 
{
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

    // Make all rows equal length
    ui->listGuid->setUpdatesEnabled(false);
    for (int i = 0; i < ui->listGuid->rowCount(); i++) {
        ui->listGuid->setRowHeight(i, 10);
    }
    ui->listGuid->setUpdatesEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// listItemClicked
//

void CDlgKnownGuid::listItemClicked(QTableWidgetItem *item)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int currentRow = ui->listGuid->selectionModel()->currentIndex().row();
    if (-1 == currentRow) {
        currentRow = 0; // First row
    }

    QTableWidgetItem *itemGuid = ui->listGuid->item(currentRow, 0);
    QString strguid = itemGuid->text(); 
    
    // Search db record for description
    QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';");
    pworks->m_mutexGuidMaps.lock();
    qDebug() << strQuery.arg(strguid);
    QSqlQuery query(strQuery.arg(strguid), pworks->m_worksdb);

    qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
    if (QSqlError::NoError != query.lastError().type()) {
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->log(pworks->LOG_LEVEL_ERROR,
                        tr("Unable to find record in database. Err =") + 
                        query.lastError().text());
        pworks->m_mutexGuidMaps.unlock();                
        return;                            
    }

    if (query.next()) {
        ui->textDescription->setMarkdown(query.value(3).toString());
    }

    pworks->m_mutexGuidMaps.unlock(); 
}

///////////////////////////////////////////////////////////////////////////////
// listItemDoubleClicked
//

void CDlgKnownGuid::listItemDoubleClicked(QTableWidgetItem *item)
{
    btnEdit();
}

///////////////////////////////////////////////////////////////////////////////
// showContextMenu
//

void CDlgKnownGuid::showContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString(tr("Add...")), this, SLOT(btnAdd()));
    menu->addAction(QString(tr("Edit...")), this, SLOT(btnEdit()));
    menu->addAction(QString(tr("Clone...")), this, SLOT(btnClone())); 
    menu->addAction(QString(tr("Delete")), this, SLOT(btnDelete())); 
    menu->addSeparator();
    menu->addAction(QString(tr("Load from file file...")), this, SLOT(btnLoad()));
    menu->addAction(QString(tr("Save to file...")), this, SLOT(btnSave()));

    menu->popup(ui->listGuid->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// selectByGuid
//

bool CDlgKnownGuid::selectByGuid(const QString& guid)
{
    for (int i=0; i < ui->listGuid->rowCount(); i++) {
        
        QTableWidgetItem * itemGuid = ui->listGuid->item(i,0);
        QTableWidgetItem * itemName = ui->listGuid->item(i,1);
        
        if (itemGuid->text() == guid) {
            ui->listGuid->selectRow(i);
            return true;
        }
        else {
            ui->listGuid->clearSelection();
        }   
    }

    return false;
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
// btnAdd
//

void  CDlgKnownGuid::btnAdd(void)
{
    CDlgEditGuid dlg;
    dlg.setWindowTitle(tr("Add new known GUID")); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    dlg.setGuid(m_addGuid);
    m_addGuid = "";

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strguid = dlg.getGuid();
        strguid = strguid.trimmed();

        // Validate length
        if (47 != strguid.length()) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Invalid GUID. Length is wrong."),
                              QMessageBox::Ok ); 
            goto again;                               
        }

        // Validate format
        int cntColon = 0;
        for (int i=0; i<strguid.length(); i++) {
            if (':' == strguid[i]) cntColon++;
        }

        // Validate # of colons
        if (15 != cntColon) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Invalid GUID. Format is wrong. Should be like 'FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:0A:00:02:00:05'"),
                              QMessageBox::Ok ); 
            goto again;                               
        }

        QString strQuery = tr("INSERT INTO guid (guid, name, description) VALUES ('%1', '%2', '%3');");

        pworks->m_mutexGuidMaps.lock();
        QSqlQuery query(strQuery
                        .arg(strguid)
                        .arg(dlg.getName())
                        .arg(dlg.getDescription()), 
                        pworks->m_worksdb);
        qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
        if (QSqlError::NoError != query.lastError().type()) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save GUID into database (duplicate?).\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->log(pworks->LOG_LEVEL_ERROR,
                            tr("Unable to save GUID into database (duplicate?). Err =") + 
                            query.lastError().text());
            pworks->m_mutexGuidMaps.unlock();                
            goto again;                            
        }

        // Add to the internal table
        pworks->m_mapGuidToSymbolicName[strguid] = dlg.getName();
        pworks->m_mutexGuidMaps.unlock();

        // Add to dialog List
        insertGuidItem(strguid, dlg.getName());
        ui->textDescription->setMarkdown(dlg.getDescription());
        ui->listGuid->sortItems(0, Qt::AscendingOrder);

        // Select added item
        for (int i=0; i < ui->listGuid->rowCount(); i++) {
        
            QTableWidgetItem * itemGuid = ui->listGuid->item(i,0);
        
            if (itemGuid->text() == strguid) {
                ui->listGuid->selectRow(i);
                break;;
            }

        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnEdit
//

void  CDlgKnownGuid::btnEdit(void)
{
    CDlgEditGuid dlg;
    dlg.setWindowTitle(tr("Edit known GUID"));
    dlg.setEditMode();

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listGuid->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No row is selected"),
                              QMessageBox::Ok );
        return;
    }
    QTableWidgetItem * itemGuid = ui->listGuid->item(row, 0);
    QString strguid = itemGuid->text();
    strguid = strguid.trimmed();

    QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';");
    pworks->m_mutexGuidMaps.lock();
    qDebug() << strQuery.arg(strguid);
    QSqlQuery query(strQuery.arg(strguid), pworks->m_worksdb);

    qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
    if (QSqlError::NoError != query.lastError().type()) {
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->log(pworks->LOG_LEVEL_ERROR,
                        tr("Unable to find record in database. Err =") + 
                        query.lastError().text());
        pworks->m_mutexGuidMaps.unlock();                
        return;                            
    }

    if (query.next()) {
        dlg.setGuid(query.value(1).toString());
        dlg.setName(query.value(2).toString());
        dlg.setDescription(query.value(3).toString());
    }

    pworks->m_mutexGuidMaps.unlock();

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strname = dlg.getName();
        QString strdescription = dlg.getDescription();

        QString strQuery = tr("UPDATE guid SET name='%1', description='%2' WHERE guid='%3';");
        qDebug() << strQuery.arg(strname)
                            .arg(strdescription)
                            .arg(strguid);
        pworks->m_mutexGuidMaps.lock();
        QSqlQuery query(strQuery
                            .arg(strname)
                            .arg(strdescription)
                            .arg(strguid), 
                            pworks->m_worksdb);
        qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
        if (QSqlError::NoError != query.lastError().type()) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save edited GUID into database.\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->log(pworks->LOG_LEVEL_ERROR,
                            tr("Unable to save edited GUID into database. Err =") + 
                            query.lastError().text());
            pworks->m_mutexGuidMaps.unlock();                
            goto again;                            
        }

        // Add to the internal table
        pworks->m_mapGuidToSymbolicName[strguid] = dlg.getName();
        pworks->m_mutexGuidMaps.unlock();

        // Add to dialog List
        QTableWidgetItem * itemName = ui->listGuid->item(row, 1);
        itemName->setText(dlg.getName());
        ui->textDescription->setMarkdown(dlg.getDescription());
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnClone
//

void  CDlgKnownGuid::btnClone(void)
{
    CDlgEditGuid dlg;
    dlg.setWindowTitle(tr("Clone GUID")); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listGuid->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No row is selected"),
                              QMessageBox::Ok );
        return;
    }
    QTableWidgetItem * itemGuid = ui->listGuid->item(row, 0);
    QString strguid = itemGuid->text();
    strguid = strguid.trimmed();

    QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';");
    pworks->m_mutexGuidMaps.lock();
    qDebug() << strQuery.arg(strguid);
    QSqlQuery query(strQuery.arg(strguid), pworks->m_worksdb);

    qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
    if (QSqlError::NoError != query.lastError().type()) {
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->log(pworks->LOG_LEVEL_ERROR,
                        tr("Unable to find record in database. Err =") + 
                        query.lastError().text());
        pworks->m_mutexGuidMaps.unlock();                
        return;                            
    }

    if (query.next()) {
        dlg.setGuid(query.value(1).toString());
        dlg.setName(query.value(2).toString());
        dlg.setDescription(query.value(3).toString());
    }

    pworks->m_mutexGuidMaps.unlock();

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strguid = dlg.getGuid();
        strguid = strguid.trimmed();

        // Validate length
        if (47 != strguid.length()) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Invalid GUID. Length is wrong."),
                              QMessageBox::Ok ); 
            goto again;                               
        }

        // Validate format
        int cntColon = 0;
        for (int i=0; i<strguid.length(); i++) {
            if (':' == strguid[i]) cntColon++;
        }

        // Validate # of colons
        if (15 != cntColon) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Invalid GUID. Format is wrong. Should be like 'FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:0A:00:02:00:05'"),
                              QMessageBox::Ok ); 
            goto again;                               
        }

        QString strQuery = tr("INSERT INTO guid (guid, name, description) VALUES ('%1', '%2', '%3');");

        pworks->m_mutexGuidMaps.lock();
        QSqlQuery query(strQuery
                        .arg(strguid)
                        .arg(dlg.getName())
                        .arg(dlg.getDescription()), 
                        pworks->m_worksdb);
        qDebug() << "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().type();                        
        if (QSqlError::NoError != query.lastError().type()) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save GUID into database (duplicate?).\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->log(pworks->LOG_LEVEL_ERROR,
                            tr("Unable to save GUID into database (duplicate?). Err =") + 
                            query.lastError().text());
            pworks->m_mutexGuidMaps.unlock();                
            goto again;                            
        }

        // Add to the internal table
        pworks->m_mapGuidToSymbolicName[strguid] = dlg.getName();
        pworks->m_mutexGuidMaps.unlock();

        // Add to dialog List
        insertGuidItem(strguid, dlg.getName());
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnDelete
//

void  CDlgKnownGuid::btnDelete(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listGuid->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No row is selected"),
                              QMessageBox::Ok );
        return;
    }
    QTableWidgetItem * item = ui->listGuid->item(row, 0);
    QString strguid = item->text();
    strguid = strguid.trimmed();

    QString strQuery = tr("DELETE FROM guid WHERE guid='%1';");
    pworks->m_mutexGuidMaps.lock();
    QSqlQuery query(strQuery.arg(strguid), pworks->m_worksdb);

    if (QSqlError::NoError != query.lastError().type()) {
        QMessageBox::information(this, 
                            tr("vscpworks+"),
                            tr("Unable to delete GUID.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );    
        pworks->log(pworks->LOG_LEVEL_ERROR,
                        tr("Unable to delete GUID. Err =") + 
                        query.lastError().text());                                          
    }
    else {

        // Delete row
        ui->listGuid->removeRow(row);

        // Delete from internal table
        pworks->m_mapGuidToSymbolicName.erase(strguid);
        // std::map<QString, QString>::iterator it = pworks->m_mapGuidToSymbolicName.find(strguid);
        // if (std::map::end != it) {
             
        // }
    }

    pworks->m_mutexGuidMaps.unlock();                        
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

