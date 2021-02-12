// cdlgconnsettingslocal.cpp
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


#include "cdlgconnsettingslocal.h"
#include "ui_cdlgconnsettingslocal.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsLocal::CDlgConnSettingsLocal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsLocal)
{
    ui->setupUi(this);

    // {
    //  "type": drv_enum_local,
    // 	"name": "name of connection item",
    // 	"path": "Path to driver",
    // };

    m_jsonConfig.insert("type", static_cast<int>(CVscpClient::connType::LOCAL));
    m_jsonConfig.insert("name", "local driver name");
    m_jsonConfig.insert("path", "path to local file");
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsLocal::~CDlgConnSettingsLocal()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsLocal::setInitialFocus(void)
{
    ui->m_description->setFocus();
}



// Getters / Setters



///////////////////////////////////////////////////////////////////////////////
// getJson
//

QJsonObject CDlgConnSettingsLocal::getJson(void)
{
    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::LOCAL);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["path"] = getPath();
    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// setJson
//

void CDlgConnSettingsLocal::setJson(const QJsonObject *pobj)
{
    m_jsonConfig = *pobj;

    setName(m_jsonConfig["name"].toString()); 
    setPath(m_jsonConfig["path"].toString()); 
}


///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgConnSettingsLocal::getName(void)
{
    return (ui->m_description->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsLocal::setName(const QString& str)
{
    ui->m_description->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPath
//

QString CDlgConnSettingsLocal::getPath(void)
{
    return (ui->m_path->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPath
//

void CDlgConnSettingsLocal::setPath(const QString& str)
{
    ui->m_path->setText(str);
}