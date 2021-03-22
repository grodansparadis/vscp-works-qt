// cdlgselectclass.cpp
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

#include "cdlgselectclass.h"
#include "ui_cdlgselectclass.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectClass::CDlgSelectClass(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectClass)
{
    ui->setupUi(this);

    connect(ui->btnClearClasses, &QPushButton::clicked, this, &CDlgSelectClass::clearClassSelections );
    connect(ui->btnClearTypes, &QPushButton::clicked, this, &CDlgSelectClass::clearTypeSelections ); 
    connect(ui->listClass, &QListWidget::itemClicked, this, &CDlgSelectClass::itemClassClicked );   
    connect(ui->listType, &QListWidget::itemClicked, this, &CDlgSelectClass::itemTypeClicked );  

    fillVscpClasses();
    fillVscpTypes();   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectClass::~CDlgSelectClass()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedClasses
//

QList<QListWidgetItem *>
CDlgSelectClass::getSelectedClasses(void)
{
    QList<QListWidgetItem *> selected = ui->listClass->selectedItems();
    return selected;
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedTypes
//

QList<QListWidgetItem *>
CDlgSelectClass::getSelectedTypes(void)
{
    QList<QListWidgetItem *> selected = ui->listType->selectedItems();
    return selected;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// clearClassSelections
//

void
CDlgSelectClass::clearClassSelections(void)
{
    // Clear selections
    ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);
}

///////////////////////////////////////////////////////////////////////////////
// clearTypeSelections
//

void
CDlgSelectClass::clearTypeSelections(void)
{
    // Clear selections
    ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);
}

///////////////////////////////////////////////////////////////////////////////
// itemTypeClicked
//

void
CDlgSelectClass::itemClassClicked(QListWidgetItem *item)
{
    fillVscpTypes();
}

///////////////////////////////////////////////////////////////////////////////
// itemTypeClicked
//

void
CDlgSelectClass::itemTypeClicked(QListWidgetItem *item)
{
    int iiii = 01;
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpClasses
//

void
CDlgSelectClass::fillVscpClasses(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear selections
    ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);

    std::map<uint16_t, QString>::iterator it;
    for (it = pworks->m_mapVscpClassToToken.begin();
         it != pworks->m_mapVscpClassToToken.end();
         ++it) {

        uint16_t classId   = it->first;
        QString classToken = it->second;

        QString listItem =
            vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
        // while (listItem.length() < 30) listItem += " ";
        listItem +=
            vscp_str_format(" -- (%d / 0x%04x)", (int)classId, (int)classId)
            .c_str();
        QListWidgetItem *item = new QListWidgetItem(listItem, ui->listClass);
        item->setData(Qt::UserRole, classId);
        ui->listClass->addItem(item);        
    }
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpTypes
//

void
CDlgSelectClass::fillVscpTypes(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear selections
    //ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);

    ui->listType->clear();

    // Get selected classes
    QList<QListWidgetItem *> selected = ui->listClass->selectedItems();
    if (!selected.size()) return;

    std::map<uint32_t, QString>::iterator it;
    for (it = pworks->m_mapVscpTypeToToken.begin();
         it != pworks->m_mapVscpTypeToToken.end();
         ++it) {

        uint16_t classId  = (it->first >> 16) & 0xffff;
        uint16_t typeId   = it->first & 0xfff;
        QString typeToken = it->second;

        bool bFound = false;
        for (int i=0; i<selected.size(); i++) {
            if (classId == selected[i]->data(Qt::UserRole)) {
                bFound = true;
                break;
            }
        }
        if (!bFound) continue;

        QString listItem =
            vscp_str_format(
            "%s ",
            typeToken.toStdString().c_str())
            .c_str();

        listItem +=
            vscp_str_format(" -- (%d / 0x%04x)", (int)typeId, (int)typeId)
            .c_str();
        QListWidgetItem *item = new QListWidgetItem(listItem, ui->listType);

        item->setData(Qt::UserRole, (((uint32_t)classId) << 16) + typeId);    
        ui->listType->addItem(item);
    }
}

