// cdlgconnsettingsrawcan.cpp
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


#include "cdlgconnsettingsrawcan.h"
#include "ui_cdlgconnsettingsrawcan.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsRawCan::CDlgConnSettingsRawCan(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsRawCan)
{
    ui->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsRawCan::~CDlgConnSettingsRawCan()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsRawCan::setInitialFocus(void)
{
    ui->m_description->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgConnSettingsRawCan::onClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgConnSettingsRawCan::onDoubleClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
    accept();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

CVscpClient::connType CDlgConnSettingsRawCan::getSelectedType(void) {
    return m_selected_type;
}

// Getters / Setters


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsRawCan::getName(void)
{
    return (ui->m_description->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsRawCan::setName(const QString& str)
{
    ui->m_description->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

QJsonObject CDlgConnSettingsRawCan::getJson(void)
{
    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// setJson
//

void CDlgConnSettingsRawCan::setJson(const QJsonObject* pobj)
{
    m_jsonConfig = *pobj;    
}