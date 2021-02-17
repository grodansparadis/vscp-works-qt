// cdlgconnsettingsrest.cpp
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

#include "cdlgconnsettingsrest.h"
#include "ui_cdlgconnsettingsrest.h"

#include "cdlglevel2filter.h"

#include <QMessageBox>
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsRest::CDlgConnSettingsRest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsRest)
{
    ui->setupUi(this);
    
    setInitialFocus();

    connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsRest::onSetFilter );
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsRest::onTestConnection );
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsRest::~CDlgConnSettingsRest()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsRest::setInitialFocus(void)
{
    ui->editDescription->setFocus();
}



// ----------------------------------------------------------------------------
// Getters / Setters
// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsRest::getName(void)
{
    return (ui->editDescription->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsRest::setName(const QString& str)
{
    ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString CDlgConnSettingsRest::getUrl(void)
{
    return (ui->editUrl->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void CDlgConnSettingsRest::setUrl(const QString& str)
{
    ui->editUrl->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString CDlgConnSettingsRest::getUser(void)
{
    return (ui->editUsername->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void CDlgConnSettingsRest::setUser(const QString& str)
{
    ui->editUsername->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString CDlgConnSettingsRest::getPassword(void)
{
    return (ui->editUsername->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void CDlgConnSettingsRest::setPassword(const QString& str)
{
    ui->editUsername->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t CDlgConnSettingsRest::getConnectionTimeout(void)
{
    return m_client.getConnectionTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void CDlgConnSettingsRest::setConnectionTimeout(uint32_t timeout)
{
    m_client.setConnectionTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t CDlgConnSettingsRest::getResponseTimeout(void)
{
    return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void CDlgConnSettingsRest::setResponseTimeout(uint32_t timeout)
{
    m_client.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

QJsonObject CDlgConnSettingsRest::getJson(void)
{
    std::string str;

    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::REST);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["url"] = getUrl();
    m_jsonConfig["user"] = getUser();
    m_jsonConfig["password"] = getPassword();

    // Filter
    m_jsonConfig["priority-filter"] = m_filter.filter_priority;
    m_jsonConfig["priority-mask"] = m_filter.mask_priority;
    m_jsonConfig["class-filter"] = m_filter.filter_class;
    m_jsonConfig["class-mask"] = m_filter.mask_class;
    m_jsonConfig["type-filter"] = m_filter.filter_type;
    m_jsonConfig["type-mask"] = m_filter.mask_type;
    vscp_writeGuidArrayToString(str, m_filter.filter_GUID);
    m_jsonConfig["guid-filter"] = str.c_str();
    vscp_writeGuidArrayToString(str, m_filter.mask_GUID);
    m_jsonConfig["guid-mask"] = str.c_str();

    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// setJson
//

void CDlgConnSettingsRest::setJson(const QJsonObject *pobj)
{
    m_jsonConfig = *pobj; 

    if (!m_jsonConfig["name"].isNull()) setName(m_jsonConfig["name"].toString());
    if (!m_jsonConfig["url"].isNull()) setUrl(m_jsonConfig["url"].toString());
    if (!m_jsonConfig["user"].isNull()) setUser(m_jsonConfig["user"].toString());
    if (!m_jsonConfig["password"].isNull()) setPassword(m_jsonConfig["password"].toString()); 
    if (!m_jsonConfig["connection-timeout"].isNull()) setConnectionTimeout((uint32_t)m_jsonConfig["connection-timeout"].toInt()); 
    if (!m_jsonConfig["response-timeout"].isNull()) setResponseTimeout((uint32_t)m_jsonConfig["response-timeout"].toInt());  

    // Get main filter
    memset(&m_filter, 0, sizeof(vscpEventFilter));
    if (!m_jsonConfig["priority-filter"].isNull()) {
        m_filter.filter_priority = (uint8_t)m_jsonConfig["priority-filter"].toInt();
    }

    if (!m_jsonConfig["priority-mask"].isNull()) {
        m_filter.mask_priority = (uint8_t)m_jsonConfig["priority-mask"].toInt();
    }

    if (!m_jsonConfig["class-filter"].isNull()) {
        m_filter.filter_class = (uint16_t)m_jsonConfig["class-filter"].toInt();
    }
    
    if (!m_jsonConfig["class-mask"].isNull()) {
        m_filter.mask_class = (uint16_t)m_jsonConfig["class-mask"].toInt();
    }

    if (!m_jsonConfig["type-filter"].isNull()) {
        m_filter.filter_type = (uint16_t)m_jsonConfig["type-filter"].toInt();
    }
    
    if (!m_jsonConfig["type-mask"].isNull()) {
        m_filter.mask_type = (uint16_t)m_jsonConfig["type-mask"].toInt();
    }

    if (!m_jsonConfig["guid-filter"].isNull()) {
        vscp_getGuidFromStringToArray(m_filter.filter_GUID, 
                                        m_jsonConfig["guid-filter"].toString().toStdString());
    }
    
    if (!m_jsonConfig["guid-mask"].isNull()) {
        vscp_getGuidFromStringToArray(m_filter.mask_GUID, 
                                        m_jsonConfig["guid-mask"].toString().toStdString());
    }       
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// onSetFilter
//

void 
CDlgConnSettingsRest::onSetFilter(void)
{
    CDlgLevel2Filter dlg;

    dlg.setFilter(&m_filter);

    if (QDialog::Accepted == dlg.exec() ) {
        dlg.getFilter(&m_filter);    
    }
}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void 
CDlgConnSettingsRest::onTestConnection(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // // Initialize host connection
    // if ( VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
    //                                             getPort(),
    //                                             getUser().toStdString().c_str(),
    //                                             getPassword().toStdString().c_str() ) ) {
    //     QApplication::restoreOverrideCursor();                                                        
    //     QMessageBox::information(this, tr("vscpworks+"), tr("Failed to initialize tcp/ip client"));        
    //     return;                                                
    // }

    // // Connect to remote host
    // if ( VSCP_ERROR_SUCCESS != m_client.connect() ) {
    //     QApplication::restoreOverrideCursor();
    //     QMessageBox::information(this, tr("vscpworks+"), tr("Failed to connect to remote tcp/ip host"));
    //     m_client.disconnect();        
    //     return;
    // }

    // // Get server version
    // uint8_t major_ver;
    // uint8_t minor_ver;
    // uint8_t release_ver;
    // uint8_t build_ver;
    // QString strVersion;
    // if ( VSCP_ERROR_SUCCESS == m_client.getversion( &major_ver,
    //                                                 &minor_ver,
    //                                                 &release_ver,
    //                                                 &build_ver ) ) {
        
    //     strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
    //                                     (int)major_ver,
    //                                     (int)minor_ver,
    //                                     (int)release_ver,
    //                                     (int)build_ver ).c_str();      
    // }
    // else {
    //     strVersion = tr("Failed to get version from server");
    // }

    // // Disconnect from remote host
    // if ( VSCP_ERROR_SUCCESS != m_client.disconnect() ) {
    //     QApplication::restoreOverrideCursor();
    //     QMessageBox::information(this, tr("vscpworks+"), tr("Failed to disconnect from remote tcp/ip host"));        
    //     return;
    // }    

    QApplication::restoreOverrideCursor();

    QString msg = tr("Connection test was successful");
    msg += "\n";
    msg += "NOT IMPLEMENTED YET!";
    QMessageBox::information(this, tr("vscpworks+"), msg );
}

