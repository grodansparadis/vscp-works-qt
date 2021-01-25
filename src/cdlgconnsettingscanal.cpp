// cdlgconnsettingscanal.cpp
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


#include "cdlgconnsettingscanal.h"
#include "ui_cdlgconnsettingscanal.h"
#include "vscphelper.h"

#include <QMessageBox>

#include <string>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsCanal::CDlgConnSettingsCanal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsCanal)
{
    ui->setupUi(this);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsCanal::~CDlgConnSettingsCanal()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgConnSettingsCanal::setInitialFocus(void)
{
    ui->m_description->setFocus();
}


///////////////////////////////////////////////////////////////////////////////
// getName
//

std::string CDlgConnSettingsCanal::getName(void)
{
    return (ui->m_description->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsCanal::setName(const std::string& str)
{
    ui->m_description->insert(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getPath
//

std::string CDlgConnSettingsCanal::getPath(void)
{
    return (ui->m_path->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPath
//

void CDlgConnSettingsCanal::setPath(const std::string& str)
{
    ui->m_path->insert(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getConfig
//

std::string CDlgConnSettingsCanal::getConfig(void)
{
    return (ui->m_config->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setConfig
//

void CDlgConnSettingsCanal::setConfig(const std::string& str)
{
    ui->m_config->insert(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getFlags
//

uint32_t CDlgConnSettingsCanal::getFlags(void)
{
    return vscp_readStringValue(ui->m_flags->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setFlags
//

void CDlgConnSettingsCanal::setFlags(uint32_t flags)
{
    std::string str = vscp_str_format("%lu", (unsigned long)flags);
    ui->m_flags->insert(str.c_str());
}
