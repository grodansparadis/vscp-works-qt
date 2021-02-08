// cdlgconnsettingssocketcan.cpp
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

#include "cdlglevel1filter.h"

#include "cdlgconnsettingssocketcan.h"
#include "ui_cdlgconnsettingssocketcan.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsSocketCan::CDlgConnSettingsSocketCan(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsSocketCan)
{
    ui->setupUi(this);

    connect(ui->listFilters, &QListWidget::itemClicked, this, &CDlgConnSettingsSocketCan::onClicked ); 
    connect(ui->listFilters, &QListWidget::itemDoubleClicked, this, &CDlgConnSettingsSocketCan::onDoubleClicked ); 

    connect(ui->btnAddFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onAddFilter ); 
    connect(ui->btnEditFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onEditFilter ); 
    connect(ui->btnDeleteFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onDeleteFilter );
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onTestConnection );  

    addFilterItems();  
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsSocketCan::~CDlgConnSettingsSocketCan()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsSocketCan::setInitialFocus(void)
{
    ui->editDescription->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// addFilterItems
//

void CDlgConnSettingsSocketCan::addFilterItems()
{

}

///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgConnSettingsSocketCan::onClicked(QListWidgetItem* item)
{       
    //m_selected_type = static_cast<CVscpClient::connType>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgConnSettingsSocketCan::onDoubleClicked(QListWidgetItem* item)
{       
    //m_selected_type = static_cast<CVscpClient::connType>(item->type());
    //accept();
}

///////////////////////////////////////////////////////////////////////////////
// onAddFilter
//

void CDlgConnSettingsSocketCan::onAddFilter(void)
{
    CDlgLevel1Filter dlg(this);
    
    dlg.exec();
}

///////////////////////////////////////////////////////////////////////////////
// onDeleteFilter
//

void CDlgConnSettingsSocketCan::onDeleteFilter(void)
{
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("Edit filter"),
                                QMessageBox::Ok ); 
}

///////////////////////////////////////////////////////////////////////////////
// onEditFilter
//

void CDlgConnSettingsSocketCan::onEditFilter(void)
{
    CDlgLevel1Filter dlg(this);
    dlg.exec();
}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void CDlgConnSettingsSocketCan::onTestConnection(void)
{
    QCoreApplication *core = QCoreApplication::instance();
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("Test connection"),
                                QMessageBox::Ok ); 
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

std::string CDlgConnSettingsSocketCan::getName(void)
{
    return (ui->editDescription->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsSocketCan::setName(const std::string& str)
{
    ui->editDescription->setText(str.c_str());
}
