// cdlgmqttpublish.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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

#include "cdlgmqttpublish.h"
#include "ui_cdlgmqttpublish.h"

#include <QMessageBox>
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

CDlgMqttPublish::CDlgMqttPublish(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgMqttPublish)
{
    ui->setupUi(this);

// Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMqttPublish::~CDlgMqttPublish()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setTopic
//

void CDlgMqttPublish::setTopic(const QString& topic)
{
    ui->editTopic->setText(topic);
}

///////////////////////////////////////////////////////////////////////////////
// getTopic
//

QString CDlgMqttPublish::getTopic(void)
{
    return ui->editTopic->text();
}

///////////////////////////////////////////////////////////////////////////////
// setQos
//

void CDlgMqttPublish::setQos(int qos)
{
    qos &= 0x03;
    return ui->comboQos->setCurrentIndex(qos);
}

///////////////////////////////////////////////////////////////////////////////
// getQos
//

int CDlgMqttPublish::getQos(void)
{
    return ui->comboQos->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// setRetain
//

void CDlgMqttPublish::setRetain(bool bRetain)
{
    ui->chkRetain->setChecked(bRetain);
}

///////////////////////////////////////////////////////////////////////////////
// getRetain
//

bool CDlgMqttPublish::getRetain(void)
{
    return ui->chkRetain->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// setFormat
//

void CDlgMqttPublish::setFormat(enumMqttMsgFormat format)
{
    return ui->comboPublishFormat->setCurrentIndex(static_cast<int>(format));
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

enumMqttMsgFormat CDlgMqttPublish::getFormat(void)
{
    return static_cast<enumMqttMsgFormat>(ui->comboPublishFormat->currentIndex());
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMqttPublish::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=mqtt";
  QDesktopServices::openUrl(QUrl(link));
}


