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
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsLocal::~CDlgConnSettingsLocal()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

std::string CDlgConnSettingsLocal::getName(void)
{
    return (ui->m_description->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsLocal::setName(const std::string& str)
{
    ui->m_description->insert(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getPath
//

std::string CDlgConnSettingsLocal::getPath(void)
{
    return (ui->m_path->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPath
//

void CDlgConnSettingsLocal::setPath(const std::string& str)
{
    ui->m_path->insert(str.c_str());
}