// cdlgselectguid.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "cdlgknownguid.h"

#include "cdlgselectguid.h"
#include "ui_cdlgselectguid.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectGuid::CDlgSelectGuid(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectGuid)
{
    ui->setupUi(this);

    connect(ui->btnFillGuid, &QPushButton::clicked, this, &CDlgSelectGuid::fillGuid );
    connect(ui->btnGetGuid, &QPushButton::clicked, this, &CDlgSelectGuid::getGuid );
    connect(ui->btnAskGuid, &QPushButton::clicked, this, &CDlgSelectGuid::askGuid ); 
    connect(ui->btnFetchKnownGuid, &QPushButton::clicked, this, &CDlgSelectGuid::fetchKnownGuid );

    // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));   
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
            return (uint8_t)vscp_readStringValue(ui->editGuid0->text().toStdString());

        case 1:
            return (uint8_t)vscp_readStringValue(ui->editGuid1->text().toStdString());

        case 2:
            return (uint8_t)vscp_readStringValue(ui->editGuid2->text().toStdString());

        case 3:
            return (uint8_t)vscp_readStringValue(ui->editGuid3->text().toStdString());

        case 4:
            return (uint8_t)vscp_readStringValue(ui->editGuid4->text().toStdString());

        case 5:
            return (uint8_t)vscp_readStringValue(ui->editGuid5->text().toStdString());

        case 6:
            return (uint8_t)vscp_readStringValue(ui->editGuid6->text().toStdString());

        case 7:
            return (uint8_t)vscp_readStringValue(ui->editGuid7->text().toStdString());

        case 8:
            return (uint8_t)vscp_readStringValue(ui->editGuid8->text().toStdString());

        case 9:
            return (uint8_t)vscp_readStringValue(ui->editGuid9->text().toStdString());

        case 10:
            return (uint8_t)vscp_readStringValue(ui->editGuid10->text().toStdString());

        case 11:
            return (uint8_t)vscp_readStringValue(ui->editGuid11->text().toStdString());

        case 12:
            return (uint8_t)vscp_readStringValue(ui->editGuid12->text().toStdString());

        case 13:
            return (uint8_t)vscp_readStringValue(ui->editGuid13->text().toStdString());

        case 14:
            return (uint8_t)vscp_readStringValue(ui->editGuid14->text().toStdString());

        case 15:
            return (uint8_t)vscp_readStringValue(ui->editGuid15->text().toStdString());                                                         
    }
}

///////////////////////////////////////////////////////////////////////////////
// setGuidValue
//

void CDlgSelectGuid::setGuidValue(uint8_t pos, uint8_t value)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance(); 

    switch (pos & 0x00f) {

        default:
        case 0:
            ui->editGuid0->setText(pworks->decimalToStringInBase(value));
            break;

        case 1:
            ui->editGuid1->setText(pworks->decimalToStringInBase(value));
            break;

        case 2:
            ui->editGuid2->setText(pworks->decimalToStringInBase(value));
            break;

        case 3:
            ui->editGuid3->setText(pworks->decimalToStringInBase(value));
            break;

        case 4:
            ui->editGuid4->setText(pworks->decimalToStringInBase(value));
            break;

        case 5:
            ui->editGuid5->setText(pworks->decimalToStringInBase(value));
            break;

        case 6:
            ui->editGuid6->setText(pworks->decimalToStringInBase(value));
            break;

        case 7:
            ui->editGuid7->setText(pworks->decimalToStringInBase(value));
            break;

        case 8:
            ui->editGuid8->setText(pworks->decimalToStringInBase(value));
            break;

        case 9:
            ui->editGuid9->setText(pworks->decimalToStringInBase(value));
            break;

        case 10:
            ui->editGuid10->setText(pworks->decimalToStringInBase(value));
            break;

        case 11:
            ui->editGuid11->setText(pworks->decimalToStringInBase(value));
            break;

        case 12:
            ui->editGuid12->setText(pworks->decimalToStringInBase(value));
            break;

        case 13:
            ui->editGuid13->setText(pworks->decimalToStringInBase(value));
            break;

        case 14:
            ui->editGuid14->setText(pworks->decimalToStringInBase(value));
            break;

        case 15:
            ui->editGuid15->setText(pworks->decimalToStringInBase(value));                                                         
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
// getGuidOperation
//

CSessionFilter::constraint 
CDlgSelectGuid::getGuidConstraint(uint8_t pos)
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

///////////////////////////////////////////////////////////////////////////////
// setGuidConstraint
//

void CDlgSelectGuid::setGuidConstraint(uint8_t pos, CSessionFilter::constraint op)
{
    switch (pos & 0x007) {

        default:
        case 0:
            ui->comboCompare0->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 1:
            ui->comboCompare1->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 2:
            ui->comboCompare2->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 3:
            ui->comboCompare3->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 4:
            ui->comboCompare4->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 5:
            ui->comboCompare5->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 6:
            ui->comboCompare6->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 7:
            ui->comboCompare7->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 8:
            ui->comboCompare8->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 9:
            ui->comboCompare9->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 10:
            ui->comboCompare10->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 11:
            ui->comboCompare11->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 12:
            ui->comboCompare12->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 13:
            ui->comboCompare13->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 14:
            ui->comboCompare14->setCurrentIndex(static_cast<uint8_t>(op));
            break;

        case 15:
            ui->comboCompare15->setCurrentIndex(static_cast<uint8_t>(op));
            break;                                                            
    }    
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// fillGuid
//

void CDlgSelectGuid::fillGuid(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance(); 

    uint8_t guid[16];
    if (!vscp_getGuidFromStringToArray(guid, ui->lineEditGuid->text().toStdString())) {
        QMessageBox::information(this, 
                              tr(APPNAME),
                              tr("Faild to convert GUID string"),
                              QMessageBox::Ok );
        return;                              
    }

    ui->editGuid15->setText(pworks->decimalToStringInBase(guid[0]));    
    ui->editGuid14->setText(pworks->decimalToStringInBase(guid[1]));
    ui->editGuid13->setText(pworks->decimalToStringInBase(guid[2]));
    ui->editGuid12->setText(pworks->decimalToStringInBase(guid[3]));
    ui->editGuid11->setText(pworks->decimalToStringInBase(guid[4]));
    ui->editGuid10->setText(pworks->decimalToStringInBase(guid[5]));
    ui->editGuid9->setText(pworks->decimalToStringInBase(guid[6]));
    ui->editGuid8->setText(pworks->decimalToStringInBase(guid[7]));
    ui->editGuid7->setText(pworks->decimalToStringInBase(guid[8]));
    ui->editGuid6->setText(pworks->decimalToStringInBase(guid[9]));
    ui->editGuid5->setText(pworks->decimalToStringInBase(guid[10]));
    ui->editGuid4->setText(pworks->decimalToStringInBase(guid[11]));
    ui->editGuid3->setText(pworks->decimalToStringInBase(guid[12]));
    ui->editGuid2->setText(pworks->decimalToStringInBase(guid[13]));
    ui->editGuid1->setText(pworks->decimalToStringInBase(guid[14]));
    ui->editGuid0->setText(pworks->decimalToStringInBase(guid[15]));
}

///////////////////////////////////////////////////////////////////////////////
// getGuid
//

void CDlgSelectGuid::getGuid(void)
{
    uint8_t guid[16];

    guid[0] = vscp_readStringValue(ui->editGuid15->text().toStdString());
    guid[1] = vscp_readStringValue(ui->editGuid14->text().toStdString());
    guid[2] = vscp_readStringValue(ui->editGuid13->text().toStdString());
    guid[3] = vscp_readStringValue(ui->editGuid12->text().toStdString());
    guid[4] = vscp_readStringValue(ui->editGuid11->text().toStdString());
    guid[5] = vscp_readStringValue(ui->editGuid10->text().toStdString());
    guid[6] = vscp_readStringValue(ui->editGuid9->text().toStdString());
    guid[7] = vscp_readStringValue(ui->editGuid8->text().toStdString());
    guid[8] = vscp_readStringValue(ui->editGuid7->text().toStdString());
    guid[9] = vscp_readStringValue(ui->editGuid6->text().toStdString());
    guid[10] = vscp_readStringValue(ui->editGuid5->text().toStdString());
    guid[11] = vscp_readStringValue(ui->editGuid4->text().toStdString());
    guid[12] = vscp_readStringValue(ui->editGuid3->text().toStdString());
    guid[13] = vscp_readStringValue(ui->editGuid2->text().toStdString());
    guid[14] = vscp_readStringValue(ui->editGuid1->text().toStdString());
    guid[15] = vscp_readStringValue(ui->editGuid0->text().toStdString());

    std::string strGuid;
    vscp_writeGuidArrayToString(strGuid, guid);
    ui->lineEditGuid->setText(strGuid.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// askGuid
//

void CDlgSelectGuid::askGuid(void)
{
    CDlgKnownGuid dlg;    
    cguid guid;

    dlg.enableAccept(true);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();  

    guid[0] = vscp_readStringValue(ui->editGuid15->text().toStdString());
    guid[1] = vscp_readStringValue(ui->editGuid14->text().toStdString());
    guid[2] = vscp_readStringValue(ui->editGuid13->text().toStdString());
    guid[3] = vscp_readStringValue(ui->editGuid12->text().toStdString());
    guid[4] = vscp_readStringValue(ui->editGuid11->text().toStdString());
    guid[5] = vscp_readStringValue(ui->editGuid10->text().toStdString());
    guid[6] = vscp_readStringValue(ui->editGuid9->text().toStdString());
    guid[7] = vscp_readStringValue(ui->editGuid8->text().toStdString());
    guid[8] = vscp_readStringValue(ui->editGuid7->text().toStdString());
    guid[9] = vscp_readStringValue(ui->editGuid6->text().toStdString());
    guid[10] = vscp_readStringValue(ui->editGuid5->text().toStdString());
    guid[11] = vscp_readStringValue(ui->editGuid4->text().toStdString());
    guid[12] = vscp_readStringValue(ui->editGuid3->text().toStdString());
    guid[13] = vscp_readStringValue(ui->editGuid2->text().toStdString());
    guid[14] = vscp_readStringValue(ui->editGuid1->text().toStdString());
    guid[15] = vscp_readStringValue(ui->editGuid0->text().toStdString());

    std::string strGuid;
    guid.toString(strGuid);
    dlg.selectByGuid(strGuid.c_str());

    if (QDialog::Accepted == dlg.exec()) {   
        
        if (!dlg.getSelectedGuid(guid)) {
            return;
        } 

        ui->editGuid15->setText(pworks->decimalToStringInBase(guid[0]));
        ui->editGuid14->setText(pworks->decimalToStringInBase(guid[1]));
        ui->editGuid13->setText(pworks->decimalToStringInBase(guid[2]));
        ui->editGuid12->setText(pworks->decimalToStringInBase(guid[3]));
        ui->editGuid11->setText(pworks->decimalToStringInBase(guid[4]));
        ui->editGuid10->setText(pworks->decimalToStringInBase(guid[5]));
        ui->editGuid9->setText(pworks->decimalToStringInBase(guid[6]));
        ui->editGuid8->setText(pworks->decimalToStringInBase(guid[7]));
        ui->editGuid7->setText(pworks->decimalToStringInBase(guid[8]));
        ui->editGuid6->setText(pworks->decimalToStringInBase(guid[9]));
        ui->editGuid5->setText(pworks->decimalToStringInBase(guid[10]));
        ui->editGuid4->setText(pworks->decimalToStringInBase(guid[11]));
        ui->editGuid3->setText(pworks->decimalToStringInBase(guid[12]));
        ui->editGuid2->setText(pworks->decimalToStringInBase(guid[13]));
        ui->editGuid1->setText(pworks->decimalToStringInBase(guid[14]));
        ui->editGuid0->setText(pworks->decimalToStringInBase(guid[15]));

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

    dlg.enableAccept(true);

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    if (QDialog::Accepted == dlg.exec()) {        
        if (!dlg.getSelectedGuid(guid)) {
            QMessageBox::information(this, 
                              tr(APPNAME),
                              tr("No GUID selected"),
                              QMessageBox::Ok );
            return;                              
        }        
    }

    ui->editGuid15->setText(pworks->decimalToStringInBase(guid[0]));
    ui->editGuid14->setText(pworks->decimalToStringInBase(guid[1]));
    ui->editGuid13->setText(pworks->decimalToStringInBase(guid[2]));
    ui->editGuid12->setText(pworks->decimalToStringInBase(guid[3]));
    ui->editGuid11->setText(pworks->decimalToStringInBase(guid[4]));
    ui->editGuid10->setText(pworks->decimalToStringInBase(guid[5]));
    ui->editGuid9->setText(pworks->decimalToStringInBase(guid[6]));
    ui->editGuid8->setText(pworks->decimalToStringInBase(guid[7]));
    ui->editGuid7->setText(pworks->decimalToStringInBase(guid[8]));
    ui->editGuid6->setText(pworks->decimalToStringInBase(guid[9]));
    ui->editGuid5->setText(pworks->decimalToStringInBase(guid[10]));
    ui->editGuid4->setText(pworks->decimalToStringInBase(guid[11]));
    ui->editGuid3->setText(pworks->decimalToStringInBase(guid[12]));
    ui->editGuid2->setText(pworks->decimalToStringInBase(guid[13]));
    ui->editGuid1->setText(pworks->decimalToStringInBase(guid[14]));
    ui->editGuid0->setText(pworks->decimalToStringInBase(guid[15]));

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

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectGuid::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/";
  QDesktopServices::openUrl(QUrl(link));
}
