// cdlgselectguid.cpp
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

#include "cdlgknownguid.h"

#include "cdlgselectguid.h"
#include "ui_cdlgselectguid.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectGuid::CDlgSelectGuid(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectGuid)
{
    ui->setupUi(this);

    connect(ui->btnFillGuid, &QPushButton::clicked, this, &CDlgSelectGuid::fillGuid );
    connect(ui->btnAskGuid, &QPushButton::clicked, this, &CDlgSelectGuid::askGuid ); 
    connect(ui->btnFetchKnownGuid, &QPushButton::clicked, this, &CDlgSelectGuid::fetchKnownGuid );
   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectGuid::~CDlgSelectGuid()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getGuidValue
//

uint8_t CDlgSelectGuid::getGuidValue(uint8_t pos)
{
    switch (pos & 0x00f) {

        default:
        case 0:
            return (uint8_t)ui->spinBoxBox0->text().toUInt();

        case 1:
            return (uint8_t)ui->spinBoxBox1->text().toUInt();

        case 2:
            return (uint8_t)ui->spinBoxBox2->text().toUInt();

        case 3:
            return (uint8_t)ui->spinBoxBox3->text().toUInt();

        case 4:
            return (uint8_t)ui->spinBoxBox4->text().toUInt();

        case 5:
            return (uint8_t)ui->spinBoxBox5->text().toUInt();

        case 6:
            return (uint8_t)ui->spinBoxBox6->text().toUInt();

        case 7:
            return (uint8_t)ui->spinBoxBox7->text().toUInt();

        case 8:
            return (uint8_t)ui->spinBoxBox8->text().toUInt();

        case 9:
            return (uint8_t)ui->spinBoxBox9->text().toUInt();

        case 10:
            return (uint8_t)ui->spinBoxBox10->text().toUInt();

        case 11:
            return (uint8_t)ui->spinBoxBox11->text().toUInt();

        case 12:
            return (uint8_t)ui->spinBoxBox12->text().toUInt();

        case 13:
            return (uint8_t)ui->spinBoxBox13->text().toUInt();

        case 14:
            return (uint8_t)ui->spinBoxBox14->text().toUInt();

        case 15:
            return (uint8_t)ui->spinBoxBox15->text().toUInt();                                                         
    }
}


///////////////////////////////////////////////////////////////////////////////
// getOperation
//

CSessionFilter::constraint 
CDlgSelectGuid::getOperation(uint8_t pos)
{
    switch (pos & 0x007) {

        default:
        case 0:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare0->currentIndex());

        case 1:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare1->currentIndex());

        case 2:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare2->currentIndex());

        case 3:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare3->currentIndex());

        case 4:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare4->currentIndex());

        case 5:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare5->currentIndex());

        case 6:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare5->currentIndex());

        case 7:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare6->currentIndex());

        case 8:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare7->currentIndex());

        case 9:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare8->currentIndex());

        case 10:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare9->currentIndex());

        case 11:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare10->currentIndex());

        case 12:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare11->currentIndex());

        case 13:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare12->currentIndex());

        case 14:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare13->currentIndex());

        case 15:
            return static_cast<CSessionFilter::constraint>(ui->comboCompare14->currentIndex());                                                            
    }
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// fillGuid
//

void CDlgSelectGuid::fillGuid(void)
{
    uint8_t guid[16];
    if (!vscp_getGuidFromStringToArray(guid, ui->lineEditGuid->text().toStdString())) {
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("Faild to convert GUID string"),
                              QMessageBox::Ok );
        return;                              
    }

    ui->spinBoxBox15->setValue(guid[0]);    
    ui->spinBoxBox14->setValue(guid[1]);
    ui->spinBoxBox13->setValue(guid[2]);
    ui->spinBoxBox12->setValue(guid[3]);
    ui->spinBoxBox11->setValue(guid[4]);
    ui->spinBoxBox10->setValue(guid[5]);
    ui->spinBoxBox9->setValue(guid[6]);
    ui->spinBoxBox8->setValue(guid[7]);
    ui->spinBoxBox7->setValue(guid[8]);
    ui->spinBoxBox6->setValue(guid[9]);
    ui->spinBoxBox5->setValue(guid[10]);
    ui->spinBoxBox4->setValue(guid[11]);
    ui->spinBoxBox3->setValue(guid[12]);
    ui->spinBoxBox2->setValue(guid[13]);
    ui->spinBoxBox1->setValue(guid[14]);
    ui->spinBoxBox0->setValue(guid[15]);
}

///////////////////////////////////////////////////////////////////////////////
// askGuid
//

void CDlgSelectGuid::askGuid(void)
{
    CDlgKnownGuid dlg;
    cguid guid;

    guid[0] = ui->spinBoxBox15->value();
    guid[1] = ui->spinBoxBox14->value();
    guid[2] = ui->spinBoxBox13->value();
    guid[3] = ui->spinBoxBox12->value();
    guid[4] = ui->spinBoxBox11->value();
    guid[5] = ui->spinBoxBox10->value();
    guid[6] = ui->spinBoxBox9->value();
    guid[7] = ui->spinBoxBox8->value();
    guid[8] = ui->spinBoxBox7->value();
    guid[9] = ui->spinBoxBox6->value();
    guid[10] = ui->spinBoxBox5->value();
    guid[11] = ui->spinBoxBox4->value();
    guid[12] = ui->spinBoxBox3->value();
    guid[13] = ui->spinBoxBox2->value();
    guid[14] = ui->spinBoxBox1->value();
    guid[15] = ui->spinBoxBox0->value();

    std::string strGuid;
    guid.toString(strGuid);
    dlg.selectByGuid(strGuid.c_str());

    if (QDialog::Accepted == dlg.exec()) {   
        
        if (!dlg.getSelectedGuid(guid)) {
            return;
        } 

        ui->spinBoxBox15->setValue(guid[0]);
        ui->spinBoxBox14->setValue(guid[1]);
        ui->spinBoxBox13->setValue(guid[2]);
        ui->spinBoxBox12->setValue(guid[3]);
        ui->spinBoxBox11->setValue(guid[4]);
        ui->spinBoxBox10->setValue(guid[5]);
        ui->spinBoxBox9->setValue(guid[6]);
        ui->spinBoxBox8->setValue(guid[7]);
        ui->spinBoxBox7->setValue(guid[8]);
        ui->spinBoxBox6->setValue(guid[9]);
        ui->spinBoxBox5->setValue(guid[10]);
        ui->spinBoxBox4->setValue(guid[11]);
        ui->spinBoxBox3->setValue(guid[12]);
        ui->spinBoxBox2->setValue(guid[13]);
        ui->spinBoxBox1->setValue(guid[14]);
        ui->spinBoxBox0->setValue(guid[15]);

        if (!ui->chkDontChangeCheck->checkState()) {
            ui->comboCompare15->setCurrentIndex(1);
            ui->comboCompare14->setCurrentIndex(1);
            ui->comboCompare13->setCurrentIndex(1);
            ui->comboCompare12->setCurrentIndex(1);
            ui->comboCompare11->setCurrentIndex(1);
            ui->comboCompare10->setCurrentIndex(1);
            ui->comboCompare9->setCurrentIndex(1);
            ui->comboCompare8->setCurrentIndex(1);
            ui->comboCompare7->setCurrentIndex(1);
            ui->comboCompare6->setCurrentIndex(1);
            ui->comboCompare5->setCurrentIndex(1);
            ui->comboCompare4->setCurrentIndex(1);
            ui->comboCompare3->setCurrentIndex(1);
            ui->comboCompare2->setCurrentIndex(1);
            ui->comboCompare1->setCurrentIndex(1);
            ui->comboCompare0->setCurrentIndex(1);
        }
    }   
}

///////////////////////////////////////////////////////////////////////////////
// fetchKnownGuid
//

void CDlgSelectGuid::fetchKnownGuid(void)
{
    CDlgKnownGuid dlg;
    cguid guid;

    if (QDialog::Accepted == dlg.exec()) {        
        if (!dlg.getSelectedGuid(guid)) {
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No GUID selected"),
                              QMessageBox::Ok );
            return;                              
        }        
    }

    ui->spinBoxBox15->setValue(guid[0]);
    ui->spinBoxBox14->setValue(guid[1]);
    ui->spinBoxBox13->setValue(guid[2]);
    ui->spinBoxBox12->setValue(guid[3]);
    ui->spinBoxBox11->setValue(guid[4]);
    ui->spinBoxBox10->setValue(guid[5]);
    ui->spinBoxBox9->setValue(guid[6]);
    ui->spinBoxBox8->setValue(guid[7]);
    ui->spinBoxBox7->setValue(guid[8]);
    ui->spinBoxBox6->setValue(guid[9]);
    ui->spinBoxBox5->setValue(guid[10]);
    ui->spinBoxBox4->setValue(guid[11]);
    ui->spinBoxBox3->setValue(guid[12]);
    ui->spinBoxBox2->setValue(guid[13]);
    ui->spinBoxBox1->setValue(guid[14]);
    ui->spinBoxBox0->setValue(guid[15]);

    if (!ui->chkDontChangeCheck->checkState()) {
        ui->comboCompare15->setCurrentIndex(1);
        ui->comboCompare14->setCurrentIndex(1);
        ui->comboCompare13->setCurrentIndex(1);
        ui->comboCompare12->setCurrentIndex(1);
        ui->comboCompare11->setCurrentIndex(1);
        ui->comboCompare10->setCurrentIndex(1);
        ui->comboCompare9->setCurrentIndex(1);
        ui->comboCompare8->setCurrentIndex(1);
        ui->comboCompare7->setCurrentIndex(1);
        ui->comboCompare6->setCurrentIndex(1);
        ui->comboCompare5->setCurrentIndex(1);
        ui->comboCompare4->setCurrentIndex(1);
        ui->comboCompare3->setCurrentIndex(1);
        ui->comboCompare2->setCurrentIndex(1);
        ui->comboCompare1->setCurrentIndex(1);
        ui->comboCompare0->setCurrentIndex(1);
    }
}


