// cdlgnewconnection.cpp
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

#include "cdlgnewconnection.h"
#include "ui_cdlgnewconnection.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgNewConnection::CDlgNewConnection(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgNewConnection)
{
    ui->setupUi(this);

    // Hook to row clicked
    connect(ui->listWidgetConnectionTypes, &QListWidget::itemClicked, this, &CDlgNewConnection::onClicked ); 

    // Hook to row double clicked
    connect(ui->listWidgetConnectionTypes, &QListWidget::itemDoubleClicked, this, &CDlgNewConnection::onDoubleClicked );           
    
    addConnectionItems();

    m_selected_type = CVscpClient::connType::LOCAL;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgNewConnection::~CDlgNewConnection()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// addConnectionItems
//

void CDlgNewConnection::addConnectionItems(void)
{
    const QIcon localIcon = QIcon::fromTheme("network-offline", QIcon(":info.png"));
    QListWidgetItem *itemLocal = new QListWidgetItem(localIcon, QString::fromUtf8("Local"), nullptr, static_cast<int>(CVscpClient::connType::LOCAL));    
    ui->listWidgetConnectionTypes->addItem(itemLocal);
    itemLocal->setToolTip("No connection (logfile or similar)");
    itemLocal->setSelected(true);

    const QIcon iconCanal = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemCanal = new QListWidgetItem(iconCanal, QString::fromUtf8("CANAL"), nullptr, static_cast<int>(CVscpClient::connType::CANAL));    
    ui->listWidgetConnectionTypes->addItem(itemCanal);
    itemCanal->setToolTip("VSCP over CANAL (CAN Abstraction Layer)");
    
    const QIcon iconSocketcan = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemSocketcan = new QListWidgetItem(iconSocketcan, QString::fromUtf8("Socketcan"), nullptr, static_cast<int>(CVscpClient::connType::SOCKETCAN));    
    ui->listWidgetConnectionTypes->addItem(itemSocketcan);
    itemSocketcan->setToolTip("VSCP over socketcan");

    const QIcon iconTcpIp = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemTcpIp = new QListWidgetItem(iconTcpIp, QString::fromUtf8("TCP/IP"), nullptr, static_cast<int>(CVscpClient::connType::TCPIP));    
    ui->listWidgetConnectionTypes->addItem(itemTcpIp);
    itemTcpIp->setToolTip("VSCP over tcp/ip");

    const QIcon iconMqtt = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemMqtt = new QListWidgetItem(iconMqtt, QString::fromUtf8("MQTT"), nullptr, static_cast<int>(CVscpClient::connType::MQTT));    
    ui->listWidgetConnectionTypes->addItem(itemMqtt);
    itemMqtt->setToolTip("VSCP over MQTT");

    const QIcon iconWs1 = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemWs1 = new QListWidgetItem(iconWs1, QString::fromUtf8("Websocket WS1"), nullptr, static_cast<int>(CVscpClient::connType::WS1));    
    ui->listWidgetConnectionTypes->addItem(itemWs1);
    itemWs1->setToolTip("VSCP over websocket protocol ws1");

    const QIcon iconWs2 = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemWs2 = new QListWidgetItem(iconWs2, QString::fromUtf8("Websocket WS2"), nullptr, static_cast<int>(CVscpClient::connType::WS2));    
    ui->listWidgetConnectionTypes->addItem(itemWs2);
    itemWs2->setToolTip("VSCP over websocket protocol ws2");    

    const QIcon iconUdp = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemUdp = new QListWidgetItem(iconUdp, QString::fromUtf8("UDP"), nullptr, static_cast<int>(CVscpClient::connType::UDP));    
    ui->listWidgetConnectionTypes->addItem(itemUdp);
    itemUdp->setToolTip("VSCP over UDP");

    const QIcon iconMulticast = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemMulticast = new QListWidgetItem(iconMulticast, QString::fromUtf8("Multicast"), nullptr, static_cast<int>(CVscpClient::connType::MULTICAST));    
    ui->listWidgetConnectionTypes->addItem(itemMulticast);
    itemMulticast->setToolTip("VSCP over Multicast");

    const QIcon iconRest = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemRest = new QListWidgetItem(iconRest, QString::fromUtf8("REST"), nullptr, static_cast<int>(CVscpClient::connType::REST));    
    ui->listWidgetConnectionTypes->addItem(itemRest);
    itemRest->setToolTip("VSCP REST protocol");

    const QIcon iconRawCan = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemRawCan = new QListWidgetItem(iconRawCan, QString::fromUtf8("Raw CAN"), nullptr, static_cast<int>(CVscpClient::connType::RAWCAN));    
    ui->listWidgetConnectionTypes->addItem(itemRawCan);
    itemRawCan->setToolTip("Raw CAN");

    const QIcon iconRawMqtt = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    QListWidgetItem *itemRawMqtt = new QListWidgetItem(iconRawMqtt, QString::fromUtf8("Raw MQTT"), nullptr, static_cast<int>(CVscpClient::connType::RAWMQTT));    
    ui->listWidgetConnectionTypes->addItem(itemRawMqtt);
    itemRawMqtt->setToolTip("Raw MQTT");
}

///////////////////////////////////////////////////////////////////////////////
// onClicked
//

void CDlgNewConnection::onClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void CDlgNewConnection::onDoubleClicked(QListWidgetItem* item)
{       
    m_selected_type = static_cast<CVscpClient::connType>(item->type());
    accept();
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedType
//

CVscpClient::connType CDlgNewConnection::getSelectedType(void) {
    return m_selected_type;
}

