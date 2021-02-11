// cdlgconnsettingsrawmqtt.cpp
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


#include "cdlgconnsettingsrawmqtt.h"
#include "ui_cdlgconnsettingsrawmqtt.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsRawMqtt::CDlgConnSettingsRawMqtt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsRawMqtt)
{
    ui->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsRawMqtt::~CDlgConnSettingsRawMqtt()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsRawMqtt::setInitialFocus(void)
{
    ui->m_description->setFocus();
}


///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgConnSettingsRawMqtt::onClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgConnSettingsRawMqtt::onDoubleClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
    accept();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

CVscpClient::connType CDlgConnSettingsRawMqtt::getSelectedType(void) {
    return m_selected_type;
}

// Getters / Setters



///////////////////////////////////////////////////////////////////////////////
// getJsonObj
//

QJsonObject CDlgConnSettingsRawMqtt::getJsonObj(void)
{
    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// SetFromJsonObj
//

void CDlgConnSettingsRawMqtt::SetJsonObj(const QJsonObject& obj)
{
    m_jsonConfig = obj;    
}


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsRawMqtt::getName(void)
{
    return (ui->m_description->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsRawMqtt::setName(const QString& str)
{
    ui->m_description->setText(str);
}

