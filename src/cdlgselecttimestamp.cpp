// cdlgselecttimestamp.cpp
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

#include "sessionfilter.h"

#include "cdlgselecttimestamp.h"
#include "ui_cdlgselecttimestamp.h"

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

CDlgSelectTimeStamp::CDlgSelectTimeStamp(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectTimeStamp)
{
    ui->setupUi(this);

    // connect(ui->btnSetCaFile, &QPushButton::clicked, this, &CDlgSelectClass::onSetCaFile );
    // connect(ui->btnSetCaPath, &QPushButton::clicked, this, &CDlgSelectClass::onSetCaPath ); 
    // connect(ui->btnSetCertFile, &QPushButton::clicked, this, &CDlgSelectClass::onSetCertFile ); 
    // connect(ui->btnSetKeyFile, &QPushButton::clicked, this, &CDlgSelectClass::onSetKeyFile ); 
    // connect(ui->btnSetPasswordKeyFile, &QPushButton::clicked, this, &CDlgSelectClass::onSetPwKeyFile );   

    // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectTimeStamp::~CDlgSelectTimeStamp()
{
    delete ui;
}



// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// getTimeStampValue
//

uint32_t CDlgSelectTimeStamp::getTimeStampValue(void)
{
    return vscp_readStringValue(ui->editTimeStamp->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setTimeStampValue
//

void CDlgSelectTimeStamp::setTimeStampValue(uint32_t value)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance(); 

    ui->editTimeStamp->setText(pworks->decimalToStringInBase(value));
}

///////////////////////////////////////////////////////////////////////////////
// getTimeStampConstraint
//

CSessionFilter::constraint CDlgSelectTimeStamp::getTimeStampConstraint(void)
{
    return static_cast<CSessionFilter::constraint>(ui->comboConstraintTimeStamp->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// setTimeStampConstraint
//

void CDlgSelectTimeStamp::setTimeStampConstraint(CSessionFilter::constraint op)
{
    ui->comboConstraintTimeStamp->setCurrentIndex(static_cast<int>(op));
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectTimeStamp::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/";
  QDesktopServices::openUrl(QUrl(link));
}