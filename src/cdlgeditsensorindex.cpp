// cdlgeditsensorindex.cpp
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgeditsensorindex.h"
#include "ui_cdlgeditsensorindex.h"

#include <QMessageBox>
#include <QDebug>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgEditSensorIndex::CDlgEditSensorIndex(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgEditSensorIndex)
{
    ui->setupUi(this);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();        

    //connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &CDlgEditGuid::setDummyGuid);
    
    setInitialFocus();             
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgEditSensorIndex::~CDlgEditSensorIndex()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgEditSensorIndex::setInitialFocus(void)
{
    ui->editSensorIndex->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setEditMode
//

void CDlgEditSensorIndex::setEditMode(void)
{
    ui->editSensorIndex->setReadOnly(true);
    ui->editName->setFocus();
}



// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// getSensor
//

uint8_t CDlgEditSensorIndex::getSensor(void)
{
    return (ui->editSensorIndex->text().toInt()); 
}

///////////////////////////////////////////////////////////////////////////////
// setSensor
//

void CDlgEditSensorIndex::setSensor(uint8_t sensorindex)
{
    ui->editSensorIndex->setText(QString::number(sensorindex));
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString CDlgEditSensorIndex::getName(void)
{
    return (ui->editName->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgEditSensorIndex::setName(const QString& str)
{
    ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getDescription
//

QString CDlgEditSensorIndex::getDescription(void)
{
#if QT_VERSION >= 0x050E00     
    return (ui->editDescription->toMarkdown());
#else
    return (ui->editDescription->toPlainText());
#endif    
}

///////////////////////////////////////////////////////////////////////////////
// setDescription
//

void CDlgEditSensorIndex::setDescription(const QString& str)
{
#if QT_VERSION >= 0x050E00    
    //ui->editDescription->setMarkdown(str);
    ui->editDescription->setText(str);
#else
    ui->editDescription->setText(str);
#endif    
}


