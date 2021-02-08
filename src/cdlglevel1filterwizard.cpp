// cdlglevel1filterwizard.cpp
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

#include "vscpworks.h"
#include "vscphelper.h"

#include "cdlglevel1filterwizard.h"
#include "ui_cdlglevel1filterwizard.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel1FilterWizard::CDlgLevel1FilterWizard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgLevel1FilterWizard)
{
    ui->setupUi(this);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    m_baseIndex = pworks->m_base;
    ui->comboNumberBase->setCurrentIndex(static_cast<int>(m_baseIndex));
    onBaseChange(static_cast<int>(m_baseIndex));

    connect(ui->btnTransferRight, &QPushButton::clicked, this, &CDlgLevel1FilterWizard::transferToVisual); 
    connect(ui->btnTransferLeft, &QPushButton::clicked, this, &CDlgLevel1FilterWizard::transferFromVisual);

    connect(ui->comboNumberBase, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onBaseChange(int)));
    
    setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgLevel1FilterWizard::~CDlgLevel1FilterWizard()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgLevel1FilterWizard::setInitialFocus(void)
{
    ui->editVscpPriorityFilter->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setNumBaseComboIndex
//

void CDlgLevel1FilterWizard::setNumBaseComboIndex(uint8_t index)
{
    if (index > 3) index = 0;
    ui->comboNumberBase->setCurrentIndex(index);
}

///////////////////////////////////////////////////////////////////////////////
// getNumComboIndex
//

uint8_t CDlgLevel1FilterWizard::getNumComboIndex(void)
{
    return ui->comboNumberBase->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void CDlgLevel1FilterWizard::onBaseChange(int index)
{
    int base = 10;
    QString qstr;
    QString prefix;
    numerical_base numbase = static_cast<numerical_base>(index);
    
    switch (numbase) {
        case numerical_base::HEX:
            prefix = "0x";
            base = 16;
            break;
        case numerical_base::DECIMAL:
        default:
            prefix = "";
            base = 10;
            break;
        case numerical_base::OCTAL:
            prefix = "0o";
            base = 8;
            break;
        case numerical_base::BINARY:
            prefix = "0b";
            base = 2;
            break;
    }

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpPriorityFilter->text().toStdString()), base);
    ui->editVscpPriorityFilter->setText(qstr);

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpPriorityMask->text().toStdString()), base);
    ui->editVscpPriorityMask->setText(qstr);

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpClassFilter->text().toStdString()), base);
    ui->editVscpClassFilter->setText(qstr);

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpClassMask->text().toStdString()), base);
    ui->editVscpClassMask->setText(qstr);

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpTypeFilter->text().toStdString()), base);
    ui->editVscpTypeFilter->setText(qstr);

    qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpTypeMask->text().toStdString()), base);
    ui->editVscpTypeMask->setText(qstr);

    qstr = prefix + QString::number(vscp_readStringValue(ui->editVscpNodeIdFilter->text().toStdString()),base);
    ui->editVscpNodeIdFilter->setText(qstr);

    qstr = prefix + QString::number(vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()),base);
    ui->editVscpNodeIdMask->setText(qstr);                            
}

///////////////////////////////////////////////////////////////////////////////
// transferToVisual
//

void CDlgLevel1FilterWizard::transferToVisual(void)
{
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("To Visual"),
                                QMessageBox::Ok ); 
}

///////////////////////////////////////////////////////////////////////////////
// transferFromVisual
//

void CDlgLevel1FilterWizard::transferFromVisual(void)
{
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("From Visual"),
                                QMessageBox::Ok ); 
}



// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityFilter
//

void CDlgLevel1FilterWizard::setVscpPriorityFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityFilter
//

uint8_t CDlgLevel1FilterWizard::getVscpPriorityFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpPriorityFilter->text().toStdString() ) & 0x07); 
}


///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityMask
//

void CDlgLevel1FilterWizard::setVscpPriorityMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityMask
//

uint8_t CDlgLevel1FilterWizard::getVscpPriorityMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpPriorityMask->text().toStdString() ) & 0x07); 
}


///////////////////////////////////////////////////////////////////////////////
// setVscpClassFilter
//

void CDlgLevel1FilterWizard::setVscpClassFilter(uint16_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassFilter
//

uint16_t CDlgLevel1FilterWizard::getVscpClassFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpClassFilter->text().toStdString() ) & 0x1ff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpClassMask
//

void CDlgLevel1FilterWizard::setVscpClassMask(uint16_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassMask
//

uint16_t CDlgLevel1FilterWizard::getVscpClassMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpClassMask->text().toStdString() ) & 0x1ff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpTypeFilter
//

void CDlgLevel1FilterWizard::setVscpTypeFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpTypeFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeFilter
//

uint8_t CDlgLevel1FilterWizard::getVscpTypeFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpTypeFilter->text().toStdString() ) & 0xff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpTypeMask
//

void CDlgLevel1FilterWizard::setVscpTypeMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpTypeMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeMask
//

uint8_t CDlgLevel1FilterWizard::getVscpTypeMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpTypeMask->text().toStdString() )  & 0xff); 
}




///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdFilter
//

void CDlgLevel1FilterWizard::setVscpNodeIdFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdFilter
//

uint8_t CDlgLevel1FilterWizard::getVscpNodeIdFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpNodeIdFilter->text().toStdString() ) & 0xff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdMask
//

void CDlgLevel1FilterWizard::setVscpNodeIdMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdMask
//

uint8_t CDlgLevel1FilterWizard::getVscpNodeIdMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpNodeIdMask->text().toStdString() ) & 0xff); 
}


