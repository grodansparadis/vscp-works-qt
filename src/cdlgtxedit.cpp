// cdlgtxedit.cpp
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

#include "cdlgtxedit.h"
#include "ui_cdlgtxedit.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgTxEdit::CDlgTxEdit(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgTxEdit)
{
    ui->setupUi(this);

    // connect(ui->btnSetCaFile, &QPushButton::clicked, this, &CDlgTLS::onSetCaFile );
    // connect(ui->btnSetCaPath, &QPushButton::clicked, this, &CDlgTLS::onSetCaPath ); 
    // connect(ui->btnSetCertFile, &QPushButton::clicked, this, &CDlgTLS::onSetCertFile ); 
    // connect(ui->btnSetKeyFile, &QPushButton::clicked, this, &CDlgTLS::onSetKeyFile ); 
    // connect(ui->btnSetPasswordKeyFile, &QPushButton::clicked, this, &CDlgTLS::onSetPwKeyFile );   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgTxEdit::~CDlgTxEdit()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getActive
//

bool 
CDlgTxEdit::getActive(void)
{
    return ui->chkActive->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// setActive
//

void 
CDlgTxEdit::setActive(bool bActive)
{
    return ui->chkActive->setChecked(bActive);
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString 
CDlgTxEdit::getName(void)
{
    return ui->editName->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void 
CDlgTxEdit::setName(const QString& str)
{
    ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getCount
//

uint8_t
CDlgTxEdit::getCount(void)
{
    return (uint8_t)ui->spinCount->value(); 
}

///////////////////////////////////////////////////////////////////////////////
// setCount
//

void 
CDlgTxEdit::setCount(uint8_t cnt)
{
    ui->spinCount->setValue(cnt);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClass
//

uint16_t 
CDlgTxEdit::getVscpClass(void)
{
    return ui->comboClass->currentIndex(); 
}

///////////////////////////////////////////////////////////////////////////////
// getVscpType
//

uint16_t 
CDlgTxEdit::getVscpType(void)
{
    return ui->comboType->currentIndex(); 
}

///////////////////////////////////////////////////////////////////////////////
// setVscpClassType
//

void 
CDlgTxEdit::setVscpClassType(uint16_t vscpClass, uint16_t vscpType)
{
    ui->comboClass->setCurrentIndex(vscpClass); // TODO
    ui->comboType->setCurrentIndex(vscpClass); // TODO
}

///////////////////////////////////////////////////////////////////////////////
// getPriority
//

uint8_t
CDlgTxEdit::getPriority(void)
{
    return ui->comboPriority->currentIndex(); 
}

///////////////////////////////////////////////////////////////////////////////
// setPriority
//

void 
CDlgTxEdit::setPriority(uint8_t priority)
{
    ui->comboPriority->setCurrentIndex(priority & 7);
}

///////////////////////////////////////////////////////////////////////////////
// getGuid
//

QString 
CDlgTxEdit::getGuid(void)
{
    return ui->editGuid->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setGuid
//

void 
CDlgTxEdit::setGuid(const QString& guid)
{
    ui->editGuid->setText(guid);
}

///////////////////////////////////////////////////////////////////////////////
// getData
//

QString 
CDlgTxEdit::getData(void)
{
    return ui->textData->toPlainText(); 
}

///////////////////////////////////////////////////////////////////////////////
// setData
//

void 
CDlgTxEdit::setData(const QString& data)
{
    ui->textData->setPlainText(data);
}


///////////////////////////////////////////////////////////////////////////////
// getPeriod
//

uint32_t
CDlgTxEdit::getPeriod(void)
{
    return ui->spinPeriod->value(); 
}

///////////////////////////////////////////////////////////////////////////////
// setPeriod
//

void 
CDlgTxEdit::setPeriod(uint32_t period)
{
    ui->spinPeriod->setValue(period);
}


// ----------------------------------------------------------------------------



