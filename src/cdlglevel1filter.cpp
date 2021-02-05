// cdlglevel1filter.cpp
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

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>
#include "cdlglevel1filterwizard.h"

#include "cdlglevel1filter.h"
#include "ui_cdlglevel1filter.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel1Filter::CDlgLevel1Filter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgLevel1Filter)
{
    ui->setupUi(this);

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
// onWizard
//

void CDlgLevel1Filter::onWizard(void)
{
    CDlgLevel1FilterWizard dlg(this);
    dlg.exec();
}

///////////////////////////////////////////////////////////////////////////////
// onIdMask
//

void CDlgLevel1Filter::onIdMask(void)
{
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("ID/MASK"),
                                QMessageBox::Ok );
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
    
    switch (numbase) {
        case HEX:
            prefix = "0x";
            base = 16;
            break;
        case DECIMAL:
        default:
            prefix = "";
            base = 10;
            break;
        case OCTAL:
            prefix = "0o";
            base = 8;
            break;
        case BINARY:
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

    qstr = prefix + QString::number(vscp_readStringValue(ui->editVscpNodeIdFilter->text().toStdString()),16);
    ui->editVscpNodeIdFilter->setText(qstr);

    qstr = prefix + QString::number(vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()),16);
    ui->editVscpNodeIdMask->setText(qstr);                            
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

// void CDlgLevel1Filter::onBaseChange(const QString& str)
// {
//     QMessageBox::information(this, 
//                                 tr("vscpworks+"),
//                                 str,
//                                 QMessageBox::Ok );   
// }