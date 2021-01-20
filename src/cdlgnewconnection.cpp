// cdlgnewconnection.cpp
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

#include "connection_types.h"
#include "cdlgnewconnection.h"
#include "ui_cdlgnewconnection.h"

CDlgNewConnection::CDlgNewConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgNewConnection)
{
    ui->setupUi(this);
    addConnectionItems();
}

CDlgNewConnection::~CDlgNewConnection()
{
    delete ui;
}

void CDlgNewConnection::addConnectionItem(const std::string& str)
{
    const QIcon localIcon = QIcon::fromTheme("document-new", QIcon(":/images/new1.png"));
    QListWidgetItem *item = new QListWidgetItem(localIcon, QString::fromUtf8(str.c_str()), nullptr, NONE);    
    ui->listWidgetConnectionTypes->addItem(item);
    item->setSelected(true);
    item->setToolTip("No connection");
}

void CDlgNewConnection::addConnectionItems(void)
{
    const QIcon localIcon = QIcon::fromTheme("network-offline", QIcon(":/images/new2.png"));
    QListWidgetItem *itemLocal = new QListWidgetItem(localIcon, QString::fromUtf8("Local"), nullptr, NONE);    
    ui->listWidgetConnectionTypes->addItem(itemLocal);
    itemLocal->setToolTip("No connection");
    itemLocal->setSelected(true);

    const QIcon iconCanal = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new3.png"));
    QListWidgetItem *itemCanal = new QListWidgetItem(iconCanal, QString::fromUtf8("CANAL"), nullptr, CANAL);    
    ui->listWidgetConnectionTypes->addItem(itemCanal);
    itemCanal->setToolTip("VSCP over CANAL");
    
    const QIcon iconSocketcan = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new4.png"));
    QListWidgetItem *itemSocketcan = new QListWidgetItem(iconSocketcan, QString::fromUtf8("socketcan"), nullptr, SOCKETCAN);    
    ui->listWidgetConnectionTypes->addItem(itemSocketcan);
    itemSocketcan->setToolTip("VSCP over socketcan");

    const QIcon iconTcpIp = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new5.png"));
    QListWidgetItem *itemTcpIp = new QListWidgetItem(iconTcpIp, QString::fromUtf8("tcp/ip"));    
    ui->listWidgetConnectionTypes->addItem(itemTcpIp);
    itemTcpIp->setToolTip("VSCP over tcp/ip");

    const QIcon iconMqtt = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new6.png"));
    QListWidgetItem *itemMqtt = new QListWidgetItem(iconMqtt, QString::fromUtf8("MQTT"));    
    ui->listWidgetConnectionTypes->addItem(itemMqtt);
    itemMqtt->setToolTip("VSCP over MQTT");

    const QIcon iconWs1 = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new7.png"));
    QListWidgetItem *itemWs1 = new QListWidgetItem(iconWs1, QString::fromUtf8("Websocket WS1"));    
    ui->listWidgetConnectionTypes->addItem(itemWs1);
    itemWs1->setToolTip("VSCP over websocket protocol WS1");

    const QIcon iconWs2 = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new8.png"));
    QListWidgetItem *itemWs2 = new QListWidgetItem(iconWs2, QString::fromUtf8("Websocket WS2"));    
    ui->listWidgetConnectionTypes->addItem(itemWs2);
    itemWs2->setToolTip("VSCP over websocket protocol WS2");    

    const QIcon iconUdp = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new9.png"));
    QListWidgetItem *itemUdp = new QListWidgetItem(iconUdp, QString::fromUtf8("UDP"));    
    ui->listWidgetConnectionTypes->addItem(itemUdp);
    itemUdp->setToolTip("VSCP over UDP");

    const QIcon iconMulticast = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new10.png"));
    QListWidgetItem *itemMulticast = new QListWidgetItem(iconMulticast, QString::fromUtf8("Multicast"));    
    ui->listWidgetConnectionTypes->addItem(itemMulticast);
    itemMulticast->setToolTip("VSCP over Multicast");

    const QIcon iconRest = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new11.png"));
    QListWidgetItem *itemRest = new QListWidgetItem(iconRest, QString::fromUtf8("rest"));    
    ui->listWidgetConnectionTypes->addItem(itemRest);
    itemRest->setToolTip("VSCP rest protocol");

    const QIcon iconRawCan = QIcon::fromTheme("network-transmit-receive", QIcon(":/images/new12.png"));
    QListWidgetItem *itemRawCan = new QListWidgetItem(iconRawCan, QString::fromUtf8("raw CAN"));    
    ui->listWidgetConnectionTypes->addItem(itemRawCan);
    itemRawCan->setToolTip("Raw CAN");

    const QIcon iconRawMqtt = QIcon::fromTheme("xxx", QIcon(":add.png"));
    QListWidgetItem *itemRawMqtt = new QListWidgetItem(iconRawMqtt, QString::fromUtf8("raw MQTT"));    
    ui->listWidgetConnectionTypes->addItem(itemRawMqtt);
    itemRawMqtt->setToolTip("Raw MQTT");
}