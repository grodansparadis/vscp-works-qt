// cdlglevel1filter.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, Grodans Paradis AB
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
#include "cdlgcanfilter.h"
#include "cdlglevel1filterwizard.h"

#include "cdlglevel1filter.h"
#include "ui_cdlglevel1filter.h"

#include <QMessageBox>
#include <QDebug>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel1Filter::CDlgLevel1Filter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgLevel1Filter)
{
    ui->setupUi(this);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    m_baseIndex = pworks->m_base;
    ui->comboNumberBase->setCurrentIndex(static_cast<int>(m_baseIndex));
    onBaseChange(static_cast<int>(m_baseIndex));

    connect(ui->btnIdMask, &QPushButton::clicked, this, &CDlgLevel1Filter::onIdMask );
    connect(ui->btnWizard, &QPushButton::clicked, this, &CDlgLevel1Filter::onWizard );
    //connect(ui->comboNumberBase, SIGNAL(currentIndexChanged(const QString&)),
    //            this,SLOT(onBaseChange(const QString&)));
    connect(ui->comboNumberBase, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onBaseChange(int)));                

    setInitialFocus();             
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgLevel1Filter::~CDlgLevel1Filter()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgLevel1Filter::setInitialFocus(void)
{
    ui->editDescription->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setNumBaseComboIndex
//

void CDlgLevel1Filter::setNumBaseComboIndex(uint8_t index)
{
    if (index > 3) index = 0;
    ui->comboNumberBase->setCurrentIndex(index);
}

///////////////////////////////////////////////////////////////////////////////
// getNumComboIndex
//

uint8_t CDlgLevel1Filter::getNumComboIndex(void)
{
    return ui->comboNumberBase->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// onWizard
//

void CDlgLevel1Filter::onWizard(void)
{
    CDlgLevel1FilterWizard dlg(this);

    dlg.setVscpPriorityFilter( getVscpPriorityFilter() );
    dlg.setVscpPriorityMask( getVscpPriorityMask() );

    dlg.setVscpClassFilter( getVscpClassFilter() );
    dlg.setVscpClassMask( getVscpClassMask() );
    
    dlg.setVscpTypeFilter( getVscpTypeFilter() );
    dlg.setVscpTypeMask( getVscpTypeMask() );
    
    dlg.setVscpNodeIdFilter( getVscpNodeIdFilter() );
    dlg.setVscpNodeIdMask( getVscpNodeIdMask() );

    dlg.setNumBaseComboIndex(ui->comboNumberBase->currentIndex());
    
    if (QDialog::Accepted == dlg.exec()) {

        setVscpPriorityFilter( dlg.getVscpPriorityFilter() );
        setVscpPriorityMask( dlg.getVscpPriorityMask() );

        setVscpClassFilter( dlg.getVscpClassFilter() );
        setVscpClassMask( dlg.getVscpClassMask() );

        setVscpTypeFilter( dlg.getVscpTypeFilter() );
        setVscpTypeMask( dlg.getVscpTypeMask() );

        setVscpNodeIdFilter( dlg.getVscpNodeIdFilter() );
        setVscpNodeIdMask( dlg.getVscpNodeIdMask() );

        ui->comboNumberBase->setCurrentIndex(dlg.getNumComboIndex());
    }
}

///////////////////////////////////////////////////////////////////////////////
// onIdMask
//

void CDlgLevel1Filter::onIdMask(void)
{
    CDlgCanFilter dlg(this);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    uint32_t canid = vscp_getCANALidFromData(vscp_readStringValue(ui->editVscpPriorityFilter->text().toStdString()) & 0x07,
                                            vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) & 0x1ff,
                                            vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()) & 0xff);
    canid += vscp_readStringValue(ui->editVscpNodeIdFilter->text().toStdString()) & 0xff;
    if (ui->chkHardcoded->isChecked()) {
        canid |= (1 << 25);
    }

    uint32_t mask = vscp_getCANALidFromData(vscp_readStringValue(ui->editVscpPriorityMask->text().toStdString()) & 0x07,
                                            vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0x1ff,
                                            vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()) & 0xff);
    mask += vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()) & 0xff;
    if (ui->chkHardcoded->isChecked()) {
        mask |= (1 << 25);
    }

    dlg.setCanId(pworks->decimalToStringInBase(canid, ui->comboNumberBase->currentIndex()).toStdString());
    dlg.setCanMask(pworks->decimalToStringInBase(mask,ui->comboNumberBase->currentIndex()).toStdString());

    if (QDialog::Accepted == dlg.exec()) {

        canid = vscp_readStringValue(dlg.getCanId());
        mask = vscp_readStringValue(dlg.getCanMask());

        ui->editVscpPriorityFilter->setText(pworks->decimalToStringInBase((vscp_getHeadFromCANALid(canid) >> 5), 
                                            ui->comboNumberBase->currentIndex()));

        ui->editVscpPriorityMask->setText(pworks->decimalToStringInBase((vscp_getHeadFromCANALid(mask) >> 5), 
                                            ui->comboNumberBase->currentIndex()));

        if (vscp_getHeadFromCANALid(mask) & VSCP_HEADER_HARD_CODED) {
            ui->chkHardcoded->setChecked(true);
        }
        else {
            ui->chkHardcoded->setChecked(false);
        }

        ui->editVscpClassFilter->setText(pworks->decimalToStringInBase(vscp_getVscpClassFromCANALid(canid), 
                                            ui->comboNumberBase->currentIndex()));

        ui->editVscpClassMask->setText(pworks->decimalToStringInBase(vscp_getVscpClassFromCANALid(mask), 
                                            ui->comboNumberBase->currentIndex()));


        ui->editVscpTypeFilter->setText(pworks->decimalToStringInBase(vscp_getVscpTypeFromCANALid(canid), 
                                            ui->comboNumberBase->currentIndex()));

        ui->editVscpTypeMask->setText(pworks->decimalToStringInBase(vscp_getVscpTypeFromCANALid(mask), 
                                            ui->comboNumberBase->currentIndex())); 


        ui->editVscpNodeIdFilter->setText(pworks->decimalToStringInBase(canid & 0xff, 
                                            ui->comboNumberBase->currentIndex()));

        ui->editVscpNodeIdMask->setText(pworks->decimalToStringInBase(mask & 0xff, 
                                            ui->comboNumberBase->currentIndex()));                                                                                        
    }
}


///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void CDlgLevel1Filter::onBaseChange(int index)
{
    int base = 10;
    QString qstr;
    QString prefix;
    numerical_base numbase = static_cast<numerical_base>(index);
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
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



// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// getDescription
//

std::string CDlgLevel1Filter::getDescription(void)
{
    return (ui->editDescription->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDescription
//

void CDlgLevel1Filter::setDescription(const std::string& str)
{
    ui->editDescription->setText(str.c_str());
}


///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityFilter
//

void CDlgLevel1Filter::setVscpPriorityFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityFilter
//

uint8_t CDlgLevel1Filter::getVscpPriorityFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpPriorityFilter->text().toStdString() ) & 0x07); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityMask
//

void CDlgLevel1Filter::setVscpPriorityMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityMask
//

uint8_t CDlgLevel1Filter::getVscpPriorityMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpPriorityMask->text().toStdString() ) & 0x07); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpClassFilter
//

void CDlgLevel1Filter::setVscpClassFilter(uint16_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassFilter
//

uint16_t CDlgLevel1Filter::getVscpClassFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpClassFilter->text().toStdString() ) & 0x1ff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpClassMask
//

void CDlgLevel1Filter::setVscpClassMask(uint16_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassMask
//

uint16_t CDlgLevel1Filter::getVscpClassMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpClassMask->text().toStdString() ) & 0x1ff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpTypeFilter
//

void CDlgLevel1Filter::setVscpTypeFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpTypeFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeFilter
//

uint8_t CDlgLevel1Filter::getVscpTypeFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpTypeFilter->text().toStdString() ) & 0xff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpTypeMask
//

void CDlgLevel1Filter::setVscpTypeMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpTypeMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeMask
//

uint8_t CDlgLevel1Filter::getVscpTypeMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpTypeMask->text().toStdString() )  & 0xff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdFilter
//

void CDlgLevel1Filter::setVscpNodeIdFilter(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdFilter
//

uint8_t CDlgLevel1Filter::getVscpNodeIdFilter(void) 
{ 
    return (vscp_readStringValue( ui->editVscpNodeIdFilter->text().toStdString() ) & 0xff); 
}



///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdMask
//

void CDlgLevel1Filter::setVscpNodeIdMask(uint8_t value) 
{ 
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString qstr = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdMask
//

uint8_t CDlgLevel1Filter::getVscpNodeIdMask(void) 
{ 
    return (vscp_readStringValue( ui->editVscpNodeIdMask->text().toStdString() ) & 0xff); 
}

///////////////////////////////////////////////////////////////////////////////
// setHardCoded
//

void CDlgLevel1Filter::setHardcoded(bool val)
{
    ui->chkHardcoded->setChecked(val);   
}

///////////////////////////////////////////////////////////////////////////////
// getHardcoded
//

bool CDlgLevel1Filter::getHardcoded(void)
{
    return ui->chkHardcoded->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// setInverted
//

void CDlgLevel1Filter::setInverted(bool val)
{
    ui->chkInvert->setChecked(val);
}

///////////////////////////////////////////////////////////////////////////////
// getInverted
//

bool CDlgLevel1Filter::getInverted(void)
{
    return ui->chkInvert->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// disableDescriptionField
//

void CDlgLevel1Filter::disableDescriptionField(void)
{
    ui->editDescription->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// enableDescriptionField
//

void CDlgLevel1Filter::enableDescriptionField(void)
{
    ui->editDescription->setEnabled(true);
}


