// cdlgsensorindex.cpp
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
#include <pch.h>
#endif

#include <vscp.h>
#include <guid.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "mainwindow.h"
#include "cdlgsensorindex.h"
#include "ui_cdlgsensorindex.h"

#include "cdlgeditsensorindex.h"

#include <QMessageBox>
#include <QMenu>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSensorIndex::CDlgSensorIndex(QWidget *parent, int link_to_guid) :
        QDialog(parent),
        ui(new Ui::CDlgSensorIndex)
{
    ui->setupUi(this);

    ui->listSensors->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listSensors->setSelectionMode(QAbstractItemView::SingleSelection);

    m_link_to_guid = link_to_guid;

    ui->btnSave->setVisible(false);
    ui->btnLoad->setVisible(false); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    connect(ui->listSensors, &QTableWidget::itemClicked, this, &CDlgSensorIndex::listItemClicked);
    connect(ui->listSensors, &QTableWidget::itemDoubleClicked, this, &CDlgSensorIndex::listItemDoubleClicked);

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listSensors,
            &QTableWidget::customContextMenuRequested,
            this,
            &CDlgSensorIndex::showContextMenu);

    connect(ui->btnAdd, &QPushButton::clicked, this, &CDlgSensorIndex::btnAdd); 
    connect(ui->btnEdit, &QPushButton::clicked, this, &CDlgSensorIndex::btnEdit); 
    connect(ui->btnClone, &QPushButton::clicked, this, &CDlgSensorIndex::btnClone); 
    connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgSensorIndex::btnDelete);  
    connect(ui->btnLoad, &QPushButton::clicked, this, &CDlgSensorIndex::btnLoad);
    connect(ui->btnSave, &QPushButton::clicked, this, &CDlgSensorIndex::btnSave);

    ui->textDescription->acceptRichText();

    QStringList headers(
      QString(tr("Sensor, Name")).split(','));
    ui->listSensors->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu
    ui->listSensors->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listSensors->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->listSensors->setColumnCount(2);
    ui->listSensors->setColumnWidth(0, 100);    // Sensor index
    ui->listSensors->setColumnWidth(1, 200);    // Name
    ui->listSensors->horizontalHeader()->setStretchLastSection(true);
    ui->listSensors->setHorizontalHeaderLabels(headers);

    // Fill in GUID's
     
    pworks->m_mutexSensorIndexMap.lock(); 

    QString strQuery = "SELECT * FROM sensorindex  WHERE link_to_guid = %1 ORDER BY sensor;"; 
    QSqlQuery query(strQuery.arg(m_link_to_guid), pworks->m_worksdb);

    while (query.next()) {
        QString sensor = query.value(2).toString();
        QString name = query.value(3).toString();

        insertSensorIndexItem(sensor, name);
    }

    pworks->m_mutexSensorIndexMap.unlock();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSensorIndex::~CDlgSensorIndex()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// insertSensorIndexItem
//

void CDlgSensorIndex::insertSensorIndexItem(QString sensorindex, QString name) 
{
    int row = ui->listSensors->rowCount();
    ui->listSensors->insertRow(row);

    // * * * Sensor index
    QTableWidgetItem* itemGuid = new QTableWidgetItem(sensorindex);

    // Not editable
    itemGuid->setFlags(itemGuid->flags() & ~Qt::ItemIsEditable);

    ui->listSensors->setItem(ui->listSensors->rowCount() - 1, 0, itemGuid);

    // * * * Name
    QTableWidgetItem* itemName = new QTableWidgetItem(name);

    // Not editable
    itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);

    ui->listSensors->setItem(ui->listSensors->rowCount() - 1, 1, itemName);

    // Make all rows equal height
    ui->listSensors->setUpdatesEnabled(false);
    for (int i = 0; i < ui->listSensors->rowCount(); i++) {
        ui->listSensors->setRowHeight(i, 10);
    }
    ui->listSensors->setUpdatesEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// listItemClicked
//

void CDlgSensorIndex::listItemClicked(QTableWidgetItem *item)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int currentRow = ui->listSensors->selectionModel()->currentIndex().row();
    if (-1 == currentRow) {
        currentRow = 0; // First row
    }

    QTableWidgetItem *itemSensor = ui->listSensors->item(currentRow, 0);
    QString strsensor = itemSensor->text(); 
    
    // Search db record for description
    QString strQuery = tr("SELECT * FROM sensorindex WHERE sensor=%1 AND link_to_guid=%2;");
    pworks->m_mutexSensorIndexMap.lock();
    QSqlQuery query(strQuery.arg(strsensor.toInt()).arg(m_link_to_guid), pworks->m_worksdb);

    // Check for database operation error
    if (QSqlError::NoError != query.lastError().type()) {
        spdlog::error(std::string(tr("Unable to find record in database. Err =").toStdString() 
                        + query.lastError().text().toStdString()));
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->m_mutexSensorIndexMap.unlock();                
        return;                            
    }

    if (query.next()) {
#if QT_VERSION >= 0x050E00    
        ui->textDescription->setMarkdown(query.value(4).toString());
#else
        ui->textDescription->setText(query.value(3).toString());
#endif 
    }

    pworks->m_mutexSensorIndexMap.unlock(); 
}

///////////////////////////////////////////////////////////////////////////////
// listItemDoubleClicked
//

void CDlgSensorIndex::listItemDoubleClicked(QTableWidgetItem *item)
{
    btnEdit();
}

///////////////////////////////////////////////////////////////////////////////
// showContextMenu
//

void CDlgSensorIndex::showContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString(tr("Add...")), this, SLOT(btnAdd()));
    menu->addAction(QString(tr("Edit...")), this, SLOT(btnEdit()));
    menu->addAction(QString(tr("Clone...")), this, SLOT(btnClone())); 
    menu->addAction(QString(tr("Delete")), this, SLOT(btnDelete())); 
    // menu->addSeparator();
    // menu->addAction(QString(tr("Load from file file...")), this, SLOT(btnLoad()));
    // menu->addAction(QString(tr("Save to file...")), this, SLOT(btnSave()));

    menu->popup(ui->listSensors->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// selectBySensorIndex
//

bool CDlgSensorIndex::selectBySensorIndex(const QString& sensorindex)
{
    for (int i=0; i < ui->listSensors->rowCount(); i++) {
        
        QTableWidgetItem * itemSensorIndex = ui->listSensors->item(i,0);
        QTableWidgetItem * itemName = ui->listSensors->item(i,1);
        
        if (itemSensorIndex->text() == sensorindex) {
            ui->listSensors->selectRow(i);
            return true;
        }
        else {
            ui->listSensors->clearSelection();
        }   
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setGuid
//

void CDlgSensorIndex::setGuid(const QString& strguid )
{
    ui->lblGuid->setText(strguid);
}

///////////////////////////////////////////////////////////////////////////////
// getGuidName
//

QString CDlgSensorIndex::getGuidName(void)
{
    return ui->lblGuidName->text();
}

///////////////////////////////////////////////////////////////////////////////
// setGuidName
//

void CDlgSensorIndex::setGuidName(const QString& strguid )
{
    ui->lblGuidName->setText(strguid);
}

///////////////////////////////////////////////////////////////////////////////
// getGuid
//

QString CDlgSensorIndex::getGuid(void)
{
    return ui->lblGuid->text();
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void CDlgSensorIndex::done(int rv)
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

void CDlgSensorIndex::btnSearch(void)
{
    /*!
        Search for GUID if ":" is in search term other wise search for name
        which must start with a letter.
    */    
    // int searchType = ui->comboSearchType->currentIndex();  // 0-exact, 1=start, 2=contains

    
    // int currentRow = ui->listSensors->selectionModel()->currentIndex().row();
    // if (-1 == currentRow) {
    //     currentRow = 0; // First row
    // }
    // else {
    //     currentRow++;   // Row after the selected one
    // }

    // QString strsearch = ui->editSearch->text();

    // for (int i=currentRow; i < ui->listSensors->rowCount(); i++) {
        
    //     QTableWidgetItem * itemGuid = ui->listSensors->item(i,0);
    //     QTableWidgetItem * itemName = ui->listSensors->item(i,1);
        
    //     // GUID exact match
    //     if (0 == searchType) {
    //         if (itemGuid->text() == ui->editSearch->text()) {
    //             //itemGuid->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         }   
    //     }
    //     // GUID starts with
    //     else if (1 == searchType) {
    //         if (itemGuid->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
    //             //itemGuid->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         }
    //     }
    //     // GUID contains
    //     else if (2 == searchType) {
    //         if (itemGuid->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
    //             itemGuid->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         }
    //     }
    //     // Name Exact match
    //     else if (3 == searchType) {
    //         if (itemName->text() == ui->editSearch->text()) {
    //             //itemName->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         } 
    //     }
    //     // Name starts with
    //     else if (4 == searchType) {
    //         if (itemName->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
    //             //itemName->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         }
    //     }
    //     // Name contains
    //     else if (5 == searchType) {
    //         if (itemName->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
    //             //itemName->setSelected(true); 
    //             ui->listSensors->selectRow(i);
    //             break;
    //         }
    //         else {
    //             ui->listSensors->clearSelection();
    //         }
    //     }
    // } 
}

///////////////////////////////////////////////////////////////////////////////
// btnAdd
//

void CDlgSensorIndex::btnAdd(void)
{
    CDlgEditSensorIndex dlg;
    dlg.setWindowTitle(tr("Add new sensor")); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    //dlg.setGuid(m_addGuid);
    //m_addGuid = "";

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strQuery = tr("INSERT INTO sensorindex (link_to_guid, sensor, name, description) VALUES (%1, %2, '%3', '%4');");

        pworks->m_mutexSensorIndexMap.lock();
        QSqlQuery query(strQuery
                        .arg(m_link_to_guid)
                        .arg(dlg.getSensor())
                        .arg(dlg.getName())
                        .arg(dlg.getDescription()), 
                        pworks->m_worksdb);
        
        // Check for database operation error
        if (QSqlError::NoError != query.lastError().type()) {
            spdlog::error(std::string(tr("Unable to save GUID into database (duplicate?). Err =").toStdString() 
                        + query.lastError().text().toStdString()));
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save sensor into database (duplicate?).\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->m_mutexSensorIndexMap.unlock();                
            goto again;                            
        }

        // Add to the internal table
        pworks->m_mapSensorIndexToSymbolicName[(m_link_to_guid << 8) + dlg.getSensor()] = dlg.getName();

        pworks->m_mutexSensorIndexMap.unlock();

        // Add to dialog List
        insertSensorIndexItem(QString::number(dlg.getSensor()), dlg.getName());
        
        ui->listSensors->sortItems(0, Qt::AscendingOrder);
        ui->listSensors->clearSelection();

        // Select added item
        for (int i=0; i < ui->listSensors->rowCount(); i++) {
            QTableWidgetItem * itemSensor = ui->listSensors->item(i, 0);
            if (itemSensor->text() == QString::number(dlg.getSensor())) {
                ui->listSensors->selectRow(i);
                #if QT_VERSION >= 0x050E00    
                    ui->textDescription->setMarkdown(dlg.getDescription());
                #else
                    ui->textDescription->setText(dlg.getDescription());
                #endif
                break;
            }

        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnEdit
//

void CDlgSensorIndex::btnEdit(void)
{
    CDlgEditSensorIndex dlg;
    dlg.setWindowTitle(tr("Edit sensor"));
    dlg.setEditMode();

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listSensors->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No sensor is selected"),
                              QMessageBox::Ok );
        return;
    }

    QTableWidgetItem * itemSensor = ui->listSensors->item(row, 0);
    QString strsensor = itemSensor->text();

    QString strQuery = tr("SELECT * FROM sensorindex WHERE sensor=%1 AND link_to_guid=%2;");
    pworks->m_mutexSensorIndexMap.lock();
    QSqlQuery query(strQuery.arg(strsensor).arg(m_link_to_guid), pworks->m_worksdb);

    // Check for database operation error
    if (QSqlError::NoError != query.lastError().type()) {
        spdlog::error(std::string(tr("Unable to find record in database. Err =").toStdString() 
                        + query.lastError().text().toStdString()));
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->m_mutexSensorIndexMap.unlock();                
        return;                            
    }

    if (query.next()) {
        dlg.setSensor(query.value(2).toInt());
        dlg.setName(query.value(3).toString());
        dlg.setDescription(query.value(4).toString());
    }

    pworks->m_mutexSensorIndexMap.unlock();

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strname = dlg.getName();
        QString strdescription = dlg.getDescription();        

        QString strQuery = tr("UPDATE sensorindex SET name='%1', description='%2' WHERE sensor='%3' AND link_to_guid=%4;");
        pworks->m_mutexSensorIndexMap.lock();
        QSqlQuery query(strQuery
                            .arg(strname)
                            .arg(strdescription)
                            .arg(strsensor)
                            .arg(m_link_to_guid), 
                            pworks->m_worksdb);

        // Check for database operation error
        if (QSqlError::NoError != query.lastError().type()) {
            spdlog::error(std::string(tr("Unable to save edited GUID into database. Err =").toStdString() 
                        + query.lastError().text().toStdString()));
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save edited GUID into database.\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->m_mutexSensorIndexMap.unlock();                
            goto again;                            
        }

        // Add to the in memory table
        pworks->m_mapSensorIndexToSymbolicName[(m_link_to_guid << 8) + dlg.getSensor()] = dlg.getName();

        pworks->m_mutexSensorIndexMap.unlock();

        // Add to dialog List
        QTableWidgetItem * itemName = ui->listSensors->item(row, 1);
        itemName->setText(dlg.getName());        
#if QT_VERSION >= 0x050E00    
        ui->textDescription->setMarkdown(dlg.getDescription());
#else
        ui->textDescription->setText(dlg.getDescription());
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnClone
//

void CDlgSensorIndex::btnClone(void)
{
    CDlgEditSensorIndex dlg;
    dlg.setWindowTitle(tr("Clone sensor")); 

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listSensors->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No sensor is selected"),
                              QMessageBox::Ok );
        return;
    }

    QTableWidgetItem * itemSensor = ui->listSensors->item(row, 0);
    QString strsensor = itemSensor->text();

    QString strQuery = tr("SELECT * FROM sensorindex WHERE sensor=%1 AND link_to_guid=%2;");
    pworks->m_mutexSensorIndexMap.lock();
    QSqlQuery query(strQuery.arg(strsensor).arg(m_link_to_guid), pworks->m_worksdb);

    // Check for database operation error
    if (QSqlError::NoError != query.lastError().type()) {
        spdlog::error(std::string(tr("Unable to find record in database Err =").toStdString() 
                        + query.lastError().text().toStdString()));
        QMessageBox::information(this,
                            tr("vscpworks+"),
                            tr("Unable to find record in database.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );
        pworks->m_mutexSensorIndexMap.unlock();                
        return;                            
    }

    if (query.next()) {
        //dlg.setSensor(query.value(2).toInt());
        dlg.setName(query.value(3).toString());
        dlg.setDescription(query.value(4).toString());
    }

    pworks->m_mutexSensorIndexMap.unlock();

again:    
    if (QDialog::Accepted == dlg.exec()) {

        QString strQuery = tr("INSERT INTO sensorindex (link_to_guid, sensor, name, description) VALUES (%1, %2, '%3', '%4');");

        pworks->m_mutexSensorIndexMap.lock();
        QSqlQuery query(strQuery
                        .arg(m_link_to_guid)
                        .arg(dlg.getSensor())
                        .arg(dlg.getName())
                        .arg(dlg.getDescription()), 
                        pworks->m_worksdb);
                       
        // Check for database operation error
        if (QSqlError::NoError != query.lastError().type()) {
            spdlog::error(std::string(tr("Unable to save GUID into database (duplicate?). Err =").toStdString() 
                        + query.lastError().text().toStdString()));
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Unable to save sensor into database (duplicate?).\n\n Error =") + query.lastError().text(),
                              QMessageBox::Ok );    
            pworks->m_mutexSensorIndexMap.unlock();                
            goto again;                            
        }

        // Add to the in memory table
        pworks->m_mapSensorIndexToSymbolicName[(m_link_to_guid << 8) + dlg.getSensor()] = dlg.getName();

        pworks->m_mutexSensorIndexMap.unlock();

        // Add to dialog List
        insertSensorIndexItem(QString::number(dlg.getSensor()), dlg.getName());
        
        ui->listSensors->sortItems(0, Qt::AscendingOrder);
        ui->listSensors->clearSelection();

        // Select added item
        for (int i=0; i < ui->listSensors->rowCount(); i++) {
            QTableWidgetItem * itemSensor = ui->listSensors->item(i,0);
        
            if (itemSensor->text() == QString::number(dlg.getSensor())) {
                ui->listSensors->selectRow(i);
                #if QT_VERSION >= 0x050E00    
                    ui->textDescription->setMarkdown(dlg.getDescription());
                #else
                    ui->textDescription->setText(dlg.getDescription());
                #endif
                break;;
            }

        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// btnDelete
//

void CDlgSensorIndex::btnDelete(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    int row = ui->listSensors->currentRow();
    if (-1 == row) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No sensor is selected"),
                              QMessageBox::Ok );
        return;
    }
    QTableWidgetItem * item = ui->listSensors->item(row, 0);
    QString strsensor = item->text();

    QString strQuery = tr("DELETE FROM sensorindex WHERE sensor='%1' AND link_to_guid=%2;");
    pworks->m_mutexSensorIndexMap.lock();
    QSqlQuery query(strQuery.arg(strsensor).arg(m_link_to_guid), pworks->m_worksdb);

    // Check for database operation error
    if (QSqlError::NoError != query.lastError().type()) {
        spdlog::error(std::string(tr("Unable to delete sensor. Err =").toStdString() 
                        + query.lastError().text().toStdString()));
        QMessageBox::information(this, 
                            tr("vscpworks+"),
                            tr("Unable to delete sensor.\n\n Error =") + query.lastError().text(),
                            QMessageBox::Ok );                                            
    }
    else {

        // Delete row
        ui->listSensors->removeRow(row);

        // Delete from in memory table
        pworks->m_mapSensorIndexToSymbolicName.erase((m_link_to_guid << 8) + strsensor.toInt());
    }

    pworks->m_mutexSensorIndexMap.unlock();                        
}

///////////////////////////////////////////////////////////////////////////////
// btnLoad
//

void CDlgSensorIndex::btnLoad(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// btnSave
//

void CDlgSensorIndex::btnSave(void)
{
    int i = 8;
}

