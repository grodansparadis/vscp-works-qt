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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "cdlgtxedit.h"
#include "ui_cdlgtxedit.h"

#include <QDesktopServices>
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

    connect(ui->btnClassHelp, &QToolButton::clicked, this, &CDlgTxEdit::showVscpClassInfo);
    connect(ui->btnClassHelp, &QToolButton::clicked, this, &CDlgTxEdit::showVscpTypeInfo);

    connect(ui->comboClass, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &CDlgTxEdit::currentVscpClassIndexChanged );

    fillVscpClass(0);
    setInitialFocus();  
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgTxEdit::~CDlgTxEdit()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgTxEdit::setInitialFocus(void)
{
    ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// showVscpClassInfo
//

void CDlgTxEdit::showVscpClassInfo()
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString str = pworks->getHelpUrlForClass(getVscpClass());
    QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}

///////////////////////////////////////////////////////////////////////////////
// showVscpClassInfo
//

void CDlgTxEdit::showVscpTypeInfo()
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString str =  pworks->getHelpUrlForType(getVscpClass(), getVscpType());
    QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpClass
//

void CDlgTxEdit::fillVscpClass(uint16_t vscpclass)
{
    int selidx;
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear all items
    ui->comboType->clear();

    std::map<uint16_t, QString>::iterator it;
    for (it = pworks->m_mapVscpClassToToken.begin();
         it != pworks->m_mapVscpClassToToken.end();
         ++it) {

        uint16_t classId   = it->first;
        QString classToken = it->second;

        QString listItem =
            vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
        listItem +=
            vscp_str_format(" -- (%d / 0x%04x)", (int)classId, (int)classId)
            .c_str();
        ui->comboClass->addItem(listItem, classId);

        // Save index for requested sel
        if (classId == vscpclass) {
            selidx = ui->comboClass->count()-1;
        }

    }    

    // Select the requested item
    ui->comboClass->setCurrentIndex(selidx);
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpType
//

void CDlgTxEdit::fillVscpType(uint16_t vscpclass, uint16_t vscptype)
{
    int selidx;
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear all items
    ui->comboType->clear();

    std::map<uint32_t, QString>::iterator it;
    for (it = pworks->m_mapVscpTypeToToken.begin();
         it != pworks->m_mapVscpTypeToToken.end();
         ++it) {

        uint16_t classId  = (it->first >> 16) & 0xffff;
        uint16_t typeId   = it->first & 0xfff;
        QString typeToken = it->second;

        if ((classId == vscpclass)) {
            QString listItem =
                vscp_str_format(
                "%s ",
                typeToken.toStdString().c_str())
                .c_str();
            // while (listItem.length() < 30) listItem += " ";
            listItem +=
                vscp_str_format(" -- (%d / 0x%04x)", (int)typeId, (int)typeId)
                .c_str();
            ui->comboType->addItem(listItem, typeId);
            
            // Save index for requested sel
            if (typeId == vscptype) {
                selidx = ui->comboType->count()-1;
            }
        }
    }

    // Select the requested item
    ui->comboType->setCurrentIndex(selidx);
}

///////////////////////////////////////////////////////////////////////////////
// currentVscpClassIndexChanged
//

void CDlgTxEdit::currentVscpClassIndexChanged(int index)
{
    int selected = ui->comboClass->currentIndex();
    fillVscpType(ui->comboClass->itemData(selected).toInt(), 0);
}


///////////////////////////////////////////////////////////////////////////////
// geEnable
//

bool 
CDlgTxEdit::getEnable(void)
{
    return ui->chkActive->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// setEnable
//

void 
CDlgTxEdit::setEnable(bool bActive)
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
    int selected = ui->comboClass->currentIndex();
    return ui->comboClass->itemData(selected).toInt(); 
}

///////////////////////////////////////////////////////////////////////////////
// getVscpType
//

uint16_t 
CDlgTxEdit::getVscpType(void)
{
    int selected = ui->comboType->currentIndex();
    return ui->comboType->itemData(selected).toInt(); 
}

///////////////////////////////////////////////////////////////////////////////
// setVscpClassType
//

void 
CDlgTxEdit::setVscpClassType(uint16_t vscpClass, uint16_t vscpType)
{
    // Select requested VSCP Class item
    for (int i=0; i<ui->comboClass->count(); i++ ) {
        if (vscpClass == ui->comboClass->itemData(i).toInt()) {
            ui->comboClass->setCurrentIndex(i);
            break;
        }        
    }

    // Select requested VSCP Type item
    for (int i=0; i<ui->comboType->count(); i++ ) {
        if (vscpType == ui->comboType->itemData(i).toInt()) {
            ui->comboType->setCurrentIndex(i);
            break;
        }        
    }
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
    return ui->textData->toPlainText().simplified(); 
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



