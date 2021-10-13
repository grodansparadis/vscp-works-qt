// cdlgmqttsubscribe.cpp
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

#include "cdlgmqttsubscribe.h"
#include "ui_cdlgmqttsubscribe.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMqttSubscribe::CDlgMqttSubscribe(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgMqttSubscribe)
{
    ui->setupUi(this);

}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMqttSubscribe::~CDlgMqttSubscribe()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setTopic
//

void CDlgMqttSubscribe::setTopic(const QString& topic)
{
    ui->editTopic->setText(topic);
}

///////////////////////////////////////////////////////////////////////////////
// getTopic
//

QString CDlgMqttSubscribe::getTopic(void)
{
    return ui->editTopic->text();
}

///////////////////////////////////////////////////////////////////////////////
// setFormat
//

void CDlgMqttSubscribe::setFormat(enumMqttMsgFormat format)
{
    return ui->comboSubscribeFormat->setCurrentIndex(static_cast<int>(format));
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

enumMqttMsgFormat CDlgMqttSubscribe::getFormat(void)
{
    return static_cast<enumMqttMsgFormat>(ui->comboSubscribeFormat->currentIndex());
}





