// cdlgconnsettingstcpip.cpp
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

#include "vscpworks.h"

#include "cdlgconnsettingstcpip.h"
#include "ui_cdlgconnsettingstcpip.h"

#include "cdlglevel2filter.h"
#include "cdlgtls.h"

#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsTcpip::CDlgConnSettingsTcpip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsTcpip)
{
    ui->setupUi(this);

    // Set defaults
    m_bTLS = false;

    connect(ui->btnTLS, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onTLSSettings );
    connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onSetFilter );
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onTestConnection );
    connect(ui->btnGetInterfaces, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onGetInterfaces );

    // Help button
    connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &CDlgConnSettingsTcpip::onGetHelp );
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsTcpip::~CDlgConnSettingsTcpip()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void 
CDlgConnSettingsTcpip::setInitialFocus(void)
{
    ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

CVscpClient::connType CDlgConnSettingsTcpip::getSelectedType(void) 
{
    return m_selected_type;
}

///////////////////////////////////////////////////////////////////////////////
// onGetHelp
//

void 
CDlgConnSettingsTcpip::onGetHelp() 
{
    QUrl helpUrl("https://docs.vscp.org/");
    QDesktopServices::openUrl(helpUrl);
}


// Getters / Setters


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString 
CDlgConnSettingsTcpip::getName(void)
{
    return (ui->editName->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void 
CDlgConnSettingsTcpip::setName(const QString& str)
{
    ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getHost
//

QString 
CDlgConnSettingsTcpip::getHost(void)
{
    return (ui->editHost->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setHost
//

void 
CDlgConnSettingsTcpip::setHost(const QString& str)
{
    ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPort
//

short 
CDlgConnSettingsTcpip::getPort(void)
{
    short port = vscp_readStringValue(ui->editPort->text().toStdString());
    return port; 
}

///////////////////////////////////////////////////////////////////////////////
// setPort
//

void 
CDlgConnSettingsTcpip::setPort(short port)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString str = pworks->decimalToStringInBase(port);
    ui->editPort->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString 
CDlgConnSettingsTcpip::getUser(void)
{
    return (ui->editUser->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void 
CDlgConnSettingsTcpip::setUser(const QString& str)
{
    ui->editUser->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString 
CDlgConnSettingsTcpip::getPassword(void)
{
    return (ui->editPassword->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void 
CDlgConnSettingsTcpip::setPassword(const QString& str)
{
    ui->editPassword->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getInterface
//

QString 
CDlgConnSettingsTcpip::getInterface(void)
{
    return ui->comboInterface->currentText(); 
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//

void 
CDlgConnSettingsTcpip::setInterface(const QString& str)
{
    ui->comboInterface->setCurrentText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getFullL2
//

bool 
CDlgConnSettingsTcpip::getFullL2(void)
{
    return ui->chkFullLevel2->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// setFullL2
//

void 
CDlgConnSettingsTcpip::setFullL2(bool l2)
{
    ui->chkFullLevel2->setChecked(l2);
}

///////////////////////////////////////////////////////////////////////////////
// isTLSEnabled
//

bool 
CDlgConnSettingsTcpip::isTLSEnabled(void)
{
    return m_bTLS; 
}

///////////////////////////////////////////////////////////////////////////////
// enableTLS
//

void 
CDlgConnSettingsTcpip::enableTLS(bool btls)
{
    m_bTLS = btls;
}

///////////////////////////////////////////////////////////////////////////////
// isVerifyPeerEnabled
//

bool 
CDlgConnSettingsTcpip::isVerifyPeerEnabled(void)
{
    return m_bTLS; 
}

///////////////////////////////////////////////////////////////////////////////
// enableVerifyPeer
//

void 
CDlgConnSettingsTcpip::enableVerifyPeer(bool bverifypeer)
{
    m_bVerifyPeer = bverifypeer;
}

///////////////////////////////////////////////////////////////////////////////
// getCaFile
//

QString 
CDlgConnSettingsTcpip::getCaFile(void)
{
    return m_cafile; 
}

///////////////////////////////////////////////////////////////////////////////
// setCaFile
//

void 
CDlgConnSettingsTcpip::setCaFile(const QString& str)
{
    m_cafile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCaPath
//

QString 
CDlgConnSettingsTcpip::getCaPath(void)
{
    return m_capath; 
}

///////////////////////////////////////////////////////////////////////////////
// setCaPath
//

void 
CDlgConnSettingsTcpip::setCaPath(const QString& str)
{
    m_capath = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCertFile
//

QString 
CDlgConnSettingsTcpip::getCertFile(void)
{
    return m_certfile; 
}

///////////////////////////////////////////////////////////////////////////////
// setCertFile
//

void 
CDlgConnSettingsTcpip::setCertFile(const QString& str)
{
    m_certfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getKeyFile
//

QString 
CDlgConnSettingsTcpip::getKeyFile(void)
{
    return m_keyfile; 
}

///////////////////////////////////////////////////////////////////////////////
// setKeyFile
//

void 
CDlgConnSettingsTcpip::setKeyFile(const QString& str)
{
    m_keyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getPwKeyFile
//

QString 
CDlgConnSettingsTcpip::getPwKeyFile(void)
{
    return m_pwkeyfile; 
}

///////////////////////////////////////////////////////////////////////////////
// setPwKeyFile
//

void 
CDlgConnSettingsTcpip::setPwKeyFile(const QString& str)
{
    m_pwkeyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getJsonObj
//

QJsonObject 
CDlgConnSettingsTcpip::getJson(void)
{
    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::TCPIP);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["host"] = getHost();
    m_jsonConfig["port"] = getPort();
    m_jsonConfig["user"] = getUser();
    m_jsonConfig["password"] = getPassword();
    m_jsonConfig["bfull-l2"] = getFullL2();
    m_jsonConfig["interface"] = getInterface();
    m_jsonConfig["btls"] = isTLSEnabled();

    m_jsonConfig["bverifypeer"] = isVerifyPeerEnabled();
    m_jsonConfig["cafile"] = getCaFile();
    m_jsonConfig["capath"] = getCaPath();
    m_jsonConfig["certfile"] = getCertFile();
    m_jsonConfig["keyfile"] = getKeyFile();
    m_jsonConfig["pwkeyfile"] = getPwKeyFile();
    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// SetFromJsonObj
//

void 
CDlgConnSettingsTcpip::setJson(const QJsonObject *pobj)
{
    m_jsonConfig = *pobj; 

    if (!m_jsonConfig["name"].isNull()) setName(m_jsonConfig["name"].toString());
    if (!m_jsonConfig["host"].isNull()) setHost(m_jsonConfig["host"].toString());
    if (!m_jsonConfig["port"].isNull()) setPort((short)m_jsonConfig["port"].toInt());
    if (!m_jsonConfig["user"].isNull()) setUser(m_jsonConfig["user"].toString());
    if (!m_jsonConfig["password"].isNull()) setPassword(m_jsonConfig["password"].toString());
    if (!m_jsonConfig["interface"].isNull()) setInterface(m_jsonConfig["interface"].toString());
    if (!m_jsonConfig["bfull-l2"].isNull()) setFullL2((short)m_jsonConfig["bfull-l2"].toBool());
    if (!m_jsonConfig["btls"].isNull()) enableTLS((short)m_jsonConfig["btls"].toBool());

    if (!m_jsonConfig["bverifypeer"].isNull()) enableVerifyPeer((short)m_jsonConfig["bverifypeer"].toBool());
    if (!m_jsonConfig["cafile"].isNull()) setCaFile(m_jsonConfig["cafile"].toString());
    if (!m_jsonConfig["capath"].isNull()) setCaPath(m_jsonConfig["capath"].toString());
    if (!m_jsonConfig["certfile"].isNull()) setCertFile(m_jsonConfig["certfile"].toString());
    if (!m_jsonConfig["keyfile"].isNull()) setKeyFile(m_jsonConfig["keyfile"].toString());
    if (!m_jsonConfig["pwkeyfile"].isNull()) setPwKeyFile(m_jsonConfig["pwkeyfile"].toString());
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// onSetFilter
//

void 
CDlgConnSettingsTcpip::onSetFilter(void)
{
    CDlgLevel2Filter dlg;

    if (QDialog::Accepted == dlg.exec() ) {
        
    }
}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void 
CDlgConnSettingsTcpip::onTestConnection(void)
{
    // Initialize host connection
    if ( VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
                                                getPort(),
                                                getUser().toStdString().c_str(),
                                                getPassword().toStdString().c_str() ) ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to initialize tcp/ip object"));
        return;                                                
    }

    // Connect to remote host
    if ( VSCP_ERROR_SUCCESS != m_client.connect() ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to connect to remote tcp/ip host"));
        m_client.disconnect();
        return;
    }

    // Get server version
    uint8_t major_ver;
    uint8_t minor_ver;
    uint8_t release_ver;
    uint8_t build_ver;
    QString strVersion;
    if ( VSCP_ERROR_SUCCESS == m_client.getversion( &major_ver,
                                                    &minor_ver,
                                                    &release_ver,
                                                    &build_ver ) ) {
        
        strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
                                        (int)major_ver,
                                        (int)minor_ver,
                                        (int)release_ver,
                                        (int)build_ver ).c_str();      
    }
    else {
        strVersion = tr("Failed to get version from server");
    }

    // Disconnect from remote host
    if ( VSCP_ERROR_SUCCESS != m_client.disconnect() ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to disconnect from remote tcp/ip host"));
        return;
    }    

    QString msg = tr("Connection test was successful");
    msg += "\n";
    msg += strVersion;
    QMessageBox::information(this, tr("vscpworks+"), msg );
}

///////////////////////////////////////////////////////////////////////////////
// onGetInterfaces
//

void 
CDlgConnSettingsTcpip::onGetInterfaces(void)
{
    // Initialize host connection
    if ( VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
                                                getPort(),
                                                getUser().toStdString().c_str(),
                                                getPassword().toStdString().c_str() ) ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to initialize tcp/ip object"));
        return;                                                
    }

    m_client.setResponseTimeout(2000);

    // Connect to remote host
    if ( VSCP_ERROR_SUCCESS != m_client.connect() ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to connect to remote tcp/ip host"));
        m_client.disconnect();
        return;
    }

    ui->comboInterface->clear();

    // Get interfaces
    std::deque<std::string> iflist;
    if ( VSCP_ERROR_SUCCESS == m_client.getinterfaces(iflist) ) {
        for (int i = 0; i < iflist.size(); i++) {           
            ui->comboInterface->addItem(iflist[i].c_str(), i);
            qDebug() << iflist.size();
            qDebug() << iflist[i].c_str();
        }                
    }
    else {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to get interfaces from server") );
        m_client.disconnect();
        return;
    }

    // Disconnect from remote host
    if ( VSCP_ERROR_SUCCESS != m_client.disconnect() ) {
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to disconnect from remote tcp/ip host"));
        return;
    }    

    QMessageBox::information(this, tr("vscpworks+"), tr("Interfaces fetched from remote server") );
}

///////////////////////////////////////////////////////////////////////////////
// onTLSSettings
//

void 
CDlgConnSettingsTcpip::onTLSSettings(void)
{
    CDlgTLS dlg;

    dlg.enableTLS(m_bTLS);
    dlg.enableVerifyPeer(m_bVerifyPeer);
    dlg.setCaFile(m_cafile);
    dlg.setCaPath(m_capath);
    dlg.setCertFile(m_certfile);
    dlg.setKeyFile(m_keyfile);
    dlg.setPwKeyFile(m_pwkeyfile);

    if (QDialog::Accepted == dlg.exec() ) {
        m_bTLS = dlg.isTLSEnabled();
        m_bVerifyPeer = dlg.isVerifyPeerEnabled();
        m_cafile = dlg.getCaFile();
        m_capath = dlg.getCaPath();
        m_certfile = dlg.getCertFile();
        m_keyfile = dlg.getKeyFile();
        m_pwkeyfile = dlg.getPwKeyFile();
    }
}
