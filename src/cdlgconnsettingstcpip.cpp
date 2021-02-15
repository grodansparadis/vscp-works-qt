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


#include "cdlgconnsettingstcpip.h"
#include "ui_cdlgconnsettingstcpip.h"
#include "vscpworks.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsTcpip::CDlgConnSettingsTcpip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsTcpip)
{
    ui->setupUi(this);
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

void CDlgConnSettingsTcpip::setInitialFocus(void)
{
    ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgConnSettingsTcpip::onClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgConnSettingsTcpip::onDoubleClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
    accept();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

CVscpClient::connType CDlgConnSettingsTcpip::getSelectedType(void) {
    return m_selected_type;
}



// Getters / Setters


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsTcpip::getName(void)
{
    return (ui->editName->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsTcpip::setName(const QString& str)
{
    ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getHost
//

QString CDlgConnSettingsTcpip::getHost(void)
{
    return (ui->editHost->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setHost
//

void CDlgConnSettingsTcpip::setHost(const QString& str)
{
    ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPort
//

short CDlgConnSettingsTcpip::getPort(void)
{
    short port = vscp_readStringValue(ui->editPort->text().toStdString());
    return port; 
}

///////////////////////////////////////////////////////////////////////////////
// setPort
//

void CDlgConnSettingsTcpip::setPort(short port)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString str = pworks->decimalToStringInBase(port);
    ui->editPort->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString CDlgConnSettingsTcpip::getUser(void)
{
    return (ui->editUser->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void CDlgConnSettingsTcpip::setUser(const QString& str)
{
    ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString CDlgConnSettingsTcpip::getPassword(void)
{
    return (ui->editPassword->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void CDlgConnSettingsTcpip::setPassword(const QString& str)
{
    ui->editPassword->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getInterface
//

QString CDlgConnSettingsTcpip::getInterface(void)
{
    return ui->comboInterface->currentText(); 
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//

void CDlgConnSettingsTcpip::setInterface(const QString& str)
{
    ui->comboInterface->setCurrentText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getFullL2
//

bool CDlgConnSettingsTcpip::getFullL2(void)
{
    return ui->chkFullLevel2->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// setFullL2
//

void CDlgConnSettingsTcpip::setFullL2(bool l2)
{
    ui->chkFullLevel2->setChecked(l2);
}

///////////////////////////////////////////////////////////////////////////////
// getJsonObj
//

QJsonObject CDlgConnSettingsTcpip::getJson(void)
{
    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::TCPIP);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["host"] = getHost();
    m_jsonConfig["port"] = getPort();
    m_jsonConfig["user"] = getUser();
    m_jsonConfig["password"] = getPassword();
    m_jsonConfig["bfull-l2"] = getFullL2();
    m_jsonConfig["interface"] = getInterface();
    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// SetFromJsonObj
//

void CDlgConnSettingsTcpip::setJson(const QJsonObject *pobj)
{
    m_jsonConfig = *pobj; 

    if (!m_jsonConfig["name"].isNull()) setName(m_jsonConfig["name"].toString());
    if (!m_jsonConfig["host"].isNull()) setHost(m_jsonConfig["host"].toString());
    if (!m_jsonConfig["port"].isNull()) setPort((short)m_jsonConfig["port"].toInt());
    if (!m_jsonConfig["user"].isNull()) setUser(m_jsonConfig["user"].toString());
    if (!m_jsonConfig["password"].isNull()) setPassword(m_jsonConfig["password"].toString());
    if (!m_jsonConfig["interface"].isNull()) setInterface(m_jsonConfig["interface"].toString());
    if (!m_jsonConfig["bfull-l2"].isNull()) setFullL2((short)m_jsonConfig["bfull-l2"].toBool());
}




