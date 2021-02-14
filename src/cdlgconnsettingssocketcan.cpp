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

#include <vscphelper.h>

#include "cdlglevel1filter.h"

#include "cdlgconnsettingssocketcan.h"
#include "ui_cdlgconnsettingssocketcan.h"

#include <QMessageBox>
#include <QJsonArray>
#include <QMenu>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CFilterListItem::CFilterListItem(const QString &name, uint32_t filter, uint32_t mask, bool bInvert) :
    QListWidgetItem(name) 
{
    m_name = name;
    m_filter = filter;
    m_mask = mask;
    m_bInvert = bInvert;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFilterListItem::~CFilterListItem()
{
    m_bInvert = true;
}

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
    //connect(ui->listFilters, &QListWidget::customContextMenuRequested, this, &CDlgConnSettingsSocketCan::showContextMenu);
    connect(ui->listFilters, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    
    connect(ui->btnAddFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onAddFilter ); 
    connect(ui->btnEditFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onEditFilter ); 
    connect(ui->btnDeleteFilter, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onDeleteFilter );
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsSocketCan::onTestConnection );   
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
// showContextMenu
//

void CDlgConnSettingsSocketCan::showContextMenu(const QPoint& pos)
{
    // Context Menu Creation
    QModelIndex selected = ui->listFilters->indexAt(pos);
    QModelIndex parent = selected.parent();
    CFilterListItem *item = (CFilterListItem *)ui->listFilters->itemAt(pos);

    if (nullptr != item) {
        //statusBar()->showMessage(item->text(0));
    }

    int row = selected.row();

    QMenu *menu = new QMenu(ui->listFilters);

    menu->addAction(QString("Edit this filter"), this, SLOT(onEditFilter()));
    menu->addAction(QString("Remove  filter"),this, SLOT(onDeleteFilter()));
    menu->addAction(QString("Clone this filter"),this, SLOT(onEditFilter()));
    menu->addAction(QString("Add new filter"), this, SLOT(onAddFilter()));

    menu->popup(ui->listFilters->viewport()->mapToGlobal(pos));

}


// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsSocketCan::getName(void)
{
    return (ui->editDescription->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsSocketCan::setName(const QString& str)
{
    ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getDevice
//

QString CDlgConnSettingsSocketCan::getDevice(void)
{
    return (ui->editDevice->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDevice
//

void CDlgConnSettingsSocketCan::setDevice(const QString& str)
{
    ui->editDevice->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimoeut
//

uint32_t CDlgConnSettingsSocketCan::getResponseTimeout(void)
{
    return vscp_readStringValue(ui->editResponseTimeout->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void CDlgConnSettingsSocketCan::setResponseTimout(uint32_t timeout)
{
    std::string str = vscp_str_format("%lu", (unsigned long)timeout);
    ui->editResponseTimeout->setText(str.c_str());   
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

QJsonObject CDlgConnSettingsSocketCan::getJson(void)
{
    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::SOCKETCAN);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["device"] = getDevice();
    m_jsonConfig["resonse-timeout"] = (int)getResponseTimeout();

    QJsonArray filterArray;

    for(int i = 0; i < ui->listFilters->count(); ++i) {
        CFilterListItem *item = (CFilterListItem *)ui->listFilters->item(i);
        QJsonObject obj;
        obj["name"] = item->m_name;
        obj["filter"] = (int)item->m_filter;
        obj["mask"] = (int)item->m_mask;
        obj["binvert"] = item->m_bInvert;
        filterArray.append(obj);
    }

    m_jsonConfig["filters"] = filterArray;

    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// setJson
//

void CDlgConnSettingsSocketCan::setJson(const QJsonObject *pobj)
{
    m_jsonConfig = *pobj;    

    if (!m_jsonConfig["name"].isNull()) setName(m_jsonConfig["name"].toString());
    if (!m_jsonConfig["device"].isNull()) setDevice(m_jsonConfig["device"].toString());
    if (!m_jsonConfig["response-timeout"].isNull()) setResponseTimout((uint32_t)(m_jsonConfig["response-timeout"].toInt()));

    if (m_jsonConfig["filters"].isArray()) {
        QJsonArray filterArray = m_jsonConfig["filters"].toArray();  

        for (auto v : filterArray) {

            QString strName;
            uint32_t filter;
            uint32_t mask;
            bool bInvert;

            QJsonObject item = v.toObject();
            if (!item["name"].isNull()) strName = item["name"].toString();
            if (!item["filter"].isNull()) filter = (uint32_t)item["filter"].toInt();
            if (!item["mask"].isNull()) mask = (uint32_t)item["mask"].toInt();
            if (!item["binvert"].isNull()) bInvert = item["binvert"].toBool();
            
            ui->listFilters->addItem(new CFilterListItem(strName, filter, mask, bInvert));
        }   
    }
}


// ----------------------------------------------------------------------------



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
    onEditFilter();
}

///////////////////////////////////////////////////////////////////////////////
// onAddFilter
//

void CDlgConnSettingsSocketCan::onAddFilter(void)
{
    CDlgLevel1Filter dlg(this);

restart:    
    if (QDialog::Accepted == dlg.exec() ) {

        std::string strName = dlg.getDescription();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                                tr("The filter need a description"),
                                QMessageBox::Ok);
            goto restart;
        }

        // Save the filter
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

        uint32_t canid = vscp_getCANALidFromData(dlg.getVscpPriorityFilter(),
                                                    dlg.getVscpClassFilter(),
                                                    dlg.getVscpTypeFilter());
        canid += dlg.getVscpNodeIdFilter();
        if (dlg.getHardcoded()) {
            canid |= (1 << 25);
        }

        uint32_t mask = vscp_getCANALidFromData(dlg.getVscpPriorityMask(),
                                                dlg.getVscpClassMask(),
                                                dlg.getVscpTypeMask());
        mask += dlg.getVscpNodeIdMask();
        if (dlg.getHardcoded()) {
            mask |= (1 << 25);
        }        

        ui->listFilters->addItem(new CFilterListItem(strName.c_str(), canid, mask, dlg.getInverted()));
    }
}

///////////////////////////////////////////////////////////////////////////////
// onDeleteFilter
//

void CDlgConnSettingsSocketCan::onDeleteFilter(void)
{
    if ( QMessageBox::Yes == QMessageBox::question(this, 
                                    tr("vscpworks+"), 
                                    tr("Do you really want to delete filter?"),
                                    QMessageBox::Yes|QMessageBox::No) ) {

        CFilterListItem *item = (CFilterListItem *)ui->listFilters->takeItem(ui->listFilters->currentRow());
        delete item;
    }
}

///////////////////////////////////////////////////////////////////////////////
// onEditFilter
//

void CDlgConnSettingsSocketCan::onEditFilter(void)
{

    CFilterListItem *item = (CFilterListItem *)ui->listFilters->currentItem();

    CDlgLevel1Filter dlg(this);

    dlg.setDescription(item->m_name.toStdString());

    dlg.setVscpPriorityFilter(vscp_getHeadFromCANALid(item->m_filter) >> 5);
    dlg.setVscpPriorityMask(vscp_getHeadFromCANALid(item->m_mask) >> 5);

    if (vscp_getHeadFromCANALid(item->m_mask) & VSCP_HEADER_HARD_CODED) {
        dlg.setHardcoded(true);
    }
    else {
        dlg.setHardcoded(false);
    }

    dlg.setVscpClassFilter(vscp_getVscpClassFromCANALid(item->m_filter));
    dlg.setVscpClassMask(vscp_getVscpClassFromCANALid(item->m_mask));

    dlg.setVscpTypeFilter(vscp_getVscpTypeFromCANALid(item->m_filter));
    dlg.setVscpTypeMask(vscp_getVscpTypeFromCANALid(item->m_mask)); 

    dlg.setVscpNodeIdFilter(item->m_filter & 0xff);
    dlg.setVscpNodeIdMask(item->m_mask & 0xff);

 restart:   
    if ( QDialog::Accepted == dlg.exec() ) {

        std::string strName = dlg.getDescription();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                                tr("The filter need a description"),
                                QMessageBox::Ok);
            goto restart;
        }

        item->m_name = strName.c_str();
        item->setText(strName.c_str());

        // Save the filter
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

        item->m_filter = vscp_getCANALidFromData(dlg.getVscpPriorityFilter(),
                                                    dlg.getVscpClassFilter(),
                                                    dlg.getVscpTypeFilter());
        item->m_filter += dlg.getVscpNodeIdFilter();
        if (dlg.getHardcoded()) {
            item->m_filter |= (1 << 25);
        }

        item->m_mask = vscp_getCANALidFromData(dlg.getVscpPriorityMask(),
                                                dlg.getVscpClassMask(),
                                                dlg.getVscpTypeMask());
        item->m_mask += dlg.getVscpNodeIdMask();
        if (dlg.getHardcoded()) {
            item->m_mask |= (1 << 25);
        }  

        item->m_bInvert = dlg.getInverted();

    }

}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void CDlgConnSettingsSocketCan::onTestConnection(void)
{
    CDlgLevel1Filter dlg(this);

restart:    
    if (QDialog::Accepted == dlg.exec() ) {
        std::string strName = dlg.getDescription();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                                tr("The filter need a description"),
                                QMessageBox::Ok);
            goto restart;
        }
    }
}

