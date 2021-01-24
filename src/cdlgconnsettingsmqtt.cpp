// cdlgconnsettingsmqtt.cpp
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

#include "connection_types.h"
#include "cdlgconnsettingsmqtt.h"
#include "ui_cdlgconnsettingsmqtt.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsMqtt::CDlgConnSettingsMqtt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsMqtt)
{
    ui->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsMqtt::~CDlgConnSettingsMqtt()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgConnSettingsMqtt::onClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<connection_type>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgConnSettingsMqtt::onDoubleClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<connection_type>(item->type());
    accept();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

connection_type CDlgConnSettingsMqtt::getSelectedType(void) {
    return m_selected_type;
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

std::string CDlgConnSettingsMqtt::getName(void)
{
    return (ui->m_description->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsMqtt::setName(const std::string& str)
{
    ui->m_description->insert(str.c_str());
}