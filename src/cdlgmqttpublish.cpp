// cdlgmqttpublish.cpp
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

#include "cdlgmqttpublish.h"
#include "ui_cdlgmqttpublish.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMqttPublish::CDlgMqttPublish(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgMqttPublish)
{
    ui->setupUi(this);

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





