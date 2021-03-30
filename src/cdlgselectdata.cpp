// cdlgselectdata.cpp
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
#include "StdAfx.h"
#endif

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgselectdata.h"
#include "ui_cdlgselectdata.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectData::CDlgSelectData(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectData)
{
    ui->setupUi(this);

    connect(ui->btnAddValue, &QPushButton::clicked, this, &CDlgSelectData::onAddValue );
    connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgSelectData::onDeleteValue );   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectData::~CDlgSelectData()
{
    delete ui;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// onAddValue
//

void CDlgSelectData::addValue(uint16_t pos, uint8_t value, uint8_t op)
{
    QString str;
        
    switch (op) {
        
        default:
        case 0:
            str = "[%1] ANY %2";
            break;

        case 1:
            str = "[%1] EQ %2";
            break; 

        case 2:
            str = "[%1] NEQ %2";
            break;

        case 3:
            str = "[%1] LT %2";
            break;

        case 4:
            str = "[%1] LTEQ %2";
            break;

        case 5:
            str = "[%1] GT %2";
            break;

        case 6:
            str = "[%1] GTEQ %2";
            break;                 
    }
    
    QListWidgetItem *newitem = new QListWidgetItem;
    if (nullptr == newitem) return;
    
    newitem->setText(str.arg(pos).arg(value));
    newitem->setData(Qt::UserRole, 
                        (((uint32_t)pos) << 16) +                                           // pos
                        (((uint32_t)ui->comboCompareConstraint->currentIndex()) << 8) +     // op
                        value);                                                             // value
    ui->listValues->addItem(newitem); 
}

///////////////////////////////////////////////////////////////////////////////
// onAddValue
//

void CDlgSelectData::onAddValue(void)
{
    // op : val : pos
    // Check if there is a value for the position already
    for (int i=0; i < ui->listValues->count(); i++) {
        QListWidgetItem * item = ui->listValues->item(i);
        if ((item->data(Qt::UserRole).toUInt() & 0xff) == ui->spinPos->value()) {
            QMessageBox::information(this, 
                        tr("vscpworks+"),
                        tr("Data for this data item is already set. Delete item first to set a new value."),
                        QMessageBox::Ok );
            return;
        }       
    }

    addValue(ui->spinPos->value(),
                (uint8_t)vscp_readStringValue(ui->editDataValue->text().toStdString()),
                ui->comboCompareConstraint->currentIndex());                       
}

///////////////////////////////////////////////////////////////////////////////
// onDeleteValue
//

void CDlgSelectData::onDeleteValue(void)
{
    QListWidgetItem *item = ui->listValues->currentItem();
    if (nullptr != item) {
        delete item;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setData
//

void CDlgSelectData::setData(std::deque<uint32_t> listData)
{
    for (int i=0; i < listData.size(); i++) {
        // listData: pos (16) : op (8) : value (8)
        qDebug() << listData[i];
        addValue((listData[i] >> 16) & 0x1ff,   // pos
                    listData[i] & 0xff,         // value
                    (listData[i] >> 8) & 0xff); // opt
    }
}

///////////////////////////////////////////////////////////////////////////////
// getData
//

std::deque<uint32_t> CDlgSelectData::getData(void)
{
    std::deque<uint32_t> listData;
    // listData: pos (16) : op (8) : value (8)
    for (int i=0; i < ui->listValues->count(); i++) {
        QListWidgetItem *item = ui->listValues->item(i);
        uint32_t addData = item->data(Qt::UserRole).toUInt();
        listData.push_back(addData);
    }
    return listData;
}

