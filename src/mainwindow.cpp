// mainwindow.cpp
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

#include <syslog.h>


#include <QtWidgets>
#include <QMessageBox>
#include <QJSEngine>

//#include <QtSerialPort/QSerialPort>
//#include <QCanBus>
// #include <QCanBusDevice>
// #include <QCanBusFactory>
// #include <QCanBusFrame>

#include "vscpworks.h"
#include "filedownloader.h"

#include "version.h"
#include "cfrmsession.h"
#include "vscp_client_base.h"
#include "cdlgmainsettings.h"
#include "cdlgknownguid.h"
#include "cdlgnewconnection.h"
#include "cdlgconnsettingslocal.h"
#include "cdlgconnsettingscanal.h"
#include "cdlgconnsettingstcpip.h"
#include "cdlgconnsettingssocketcan.h"
#include "cdlgconnsettingsmqtt.h"
#include "cdlgconnsettingsws1.h"
#include "cdlgconnsettingsws2.h"
#include "cdlgconnsettingsrest.h"
#include "cdlgconnsettingsudp.h"
#include "cdlgconnsettingsmulticast.h"
#include "cdlgconnsettingsrawcan.h"
#include "cdlgconnsettingsrawmqtt.h"
#include "filedownloader.h"

#include "mainwindow.h"

///////////////////////////////////////////////////////////////////////////////
// treeWidgetItemConn
//


treeWidgetItemConn::treeWidgetItemConn(QTreeWidgetItem *topItem, const QJsonObject& conn) : 
    QTreeWidgetItem(topItem, 1000) 
{
    assert(nullptr != topItem);

    setText(0, conn["name"].toString());
    m_conn = conn;

    const QIcon icon = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    setIcon(0, icon);
    //item_test->setToolTip(0, "This is just a test connection from a snowy country named Sweden.");
    // m_connTreeTable->addTopLevelItem(item_test);
    //topItem->addChild(this);
    topItem->setExpanded(true);
    //topItem->setSelected(false);
    setSelected(true);
    
}

treeWidgetItemConn::~treeWidgetItemConn()
{
    
}




// ----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// MainWindow
//

MainWindow::MainWindow()
    : m_connTreeTable(new QTreeWidget)
{ 
    // Enable custom context menu
    m_connTreeTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Open communication item on double click
    connect(m_connTreeTable, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onDoubleClicked );

    // Open pop up menu on right click
    connect(m_connTreeTable, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showConnectionContextMenu);
    
    // TODO
    // if (QCanBus::instance()->plugins().contains(QStringLiteral("socketcan"))) {
    //     // plugin available
    // }

   
    QJSEngine myEngine;
    QJSValue three = myEngine.evaluate("(1 + 2 + Math.PI);");
    qDebug() << tr("Int Value = ") << three.toInt();
    qDebug() << tr("Value = ") << three.toNumber();
    myEngine.evaluate("var e = {};e.data = [11,22,33];");
    myEngine.evaluate("var ttt=77;");
    //QJSValue fun = myEngine.evaluate("(function(a,b) { var e = {}; e.data = [0,1,2,3,4,5]; switch (e.data[2]) { case 0: return 100; break; case 1: return 222; break; case 2: return 333; break; }; })");
    QJSValue fun = myEngine.evaluate("(function(a,b) { return e.data[2]; })");
    QJSValueList args;
    args << 1 << 2;
    QJSValue threeAgain = fun.call(args);
    qDebug() << threeAgain.toInt();

    qDebug() << "end";
    // QJSEngine engine; if (1) { return 2; } else { return 3 };


  

    setCentralWidget(m_connTreeTable);  // table widget
    createActions();
    createStatusBar();

    readSettings();

    //connect(m_textEdit->document(), &QTextDocument::contentsChanged,
    //        this, &MainWindow::connectionsWasModified);

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);

    QStringList headers(QString(tr("Connection")).split(','));
    m_connTreeTable->setHeaderLabel(tr("VSCP Remote Connections"));

    // Add root items
    
    const QIcon iconConnections = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    const QIcon iconLocal = QIcon::fromTheme("network-offline", QIcon(":info.png"));
    
    //const QIcon iconTest = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    const QIcon iconTest(":process_accept.png");

    // Set font for top-items
    QFont font("" , 10 , QFont::Bold );
	QBrush b (Qt::darkYellow);
    
    // Local
    QStringList strlist_local(QString(tr("Local Connections")).split(','));
    m_topitem_local = new QTreeWidgetItem(strlist_local, static_cast<int>(CVscpClient::connType::LOCAL));
    m_topitem_local->setIcon(0,iconTest);
    m_topitem_local->setToolTip(0,tr("Holds local connections. Typically logfile and debug content containing VSCP events."));
    // Set font
	m_topitem_local->setForeground( 0 , b );
	m_topitem_local->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_local);    

    // canal
    QStringList strlist_canal(QString(tr("CANAL Connections")).split(','));
    m_topitem_canal = new QTreeWidgetItem(strlist_canal, static_cast<int>(CVscpClient::connType::CANAL));
    m_topitem_canal->setIcon(0,iconTest);
    m_topitem_canal->setToolTip(0,"Holds VSCP CANAL connections.");
    // Set font
	m_topitem_canal->setForeground( 0 , b );
	m_topitem_canal->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_canal);

    // Socketcan
    QStringList strlist_socketcan(QString(tr("Socketcan Connections")).split(','));
    m_topitem_socketcan = new QTreeWidgetItem(strlist_socketcan, static_cast<int>(CVscpClient::connType::SOCKETCAN));
    m_topitem_socketcan->setIcon(0,iconTest);
    m_topitem_socketcan->setToolTip(0,"Holds VSCP socketcan connections.");
    // Set font
	m_topitem_socketcan->setForeground( 0 , b );
	m_topitem_socketcan->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_socketcan);

    // tcp/ip
    QStringList strlist_tcpip(QString(tr("TCP/IP Connections")).split(','));
    m_topitem_tcpip = new QTreeWidgetItem(strlist_tcpip, static_cast<int>(CVscpClient::connType::TCPIP));
    m_topitem_tcpip->setIcon(0,iconTest);
    m_topitem_tcpip->setToolTip(0,"Holds VSCP tcp/ip connections.");
    // Set font
	m_topitem_tcpip->setForeground( 0 , b );
	m_topitem_tcpip->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_tcpip);

    // MQTT
    QStringList strlist_mqtt(QString(tr("MQTT Connections")).split(','));
    m_topitem_mqtt = new QTreeWidgetItem(strlist_mqtt, static_cast<int>(CVscpClient::connType::MQTT));
    m_topitem_mqtt->setIcon(0,iconTest);
    m_topitem_mqtt->setToolTip(0,"Holds VSCP MQTT connections.");
    // Set font
	m_topitem_mqtt->setForeground( 0 , b );
	m_topitem_mqtt->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_mqtt);

    // WS1
    QStringList strlist_ws1(QString(tr("WS1 Connections")).split(','));
    m_topitem_ws1 = new QTreeWidgetItem(strlist_ws1, static_cast<int>(CVscpClient::connType::WS1));
    m_topitem_ws1->setIcon(0,iconTest);
    m_topitem_ws1->setToolTip(0,"Holds VSCP websocket ws1 connections.");
    // Set font
	m_topitem_ws1->setForeground( 0 , b );
	m_topitem_ws1->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_ws1);

    // WS2
    QStringList strlist_ws2(QString(tr("WS2 Connections")).split(','));
    m_topitem_ws2 = new QTreeWidgetItem(strlist_ws2, static_cast<int>(CVscpClient::connType::WS2));
    m_topitem_ws2->setIcon(0,iconTest);
    m_topitem_ws2->setToolTip(0,"Holds VSCP websocket ws2 connections.");
    // Set font
	m_topitem_ws2->setForeground( 0 , b );
	m_topitem_ws2->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_ws2);

    // UDP
    QStringList strlist_udp(QString(tr("UDP Connections")).split(','));
    m_topitem_udp = new QTreeWidgetItem(strlist_udp, static_cast<int>(CVscpClient::connType::UDP));
    m_topitem_udp->setIcon(0,iconTest);
    m_topitem_udp->setToolTip(0,"Holds VSCP UDP connections.");
    // Set font
	m_topitem_udp->setForeground( 0 , b );
	m_topitem_udp->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_udp);

    // Multicast
    QStringList strlist_multicast(QString(tr("Multicast Connections")).split(','));
    m_topitem_multicast = new QTreeWidgetItem(strlist_multicast, static_cast<int>(CVscpClient::connType::MULTICAST));
    m_topitem_multicast->setIcon(0,iconTest);
    m_topitem_multicast->setToolTip(0,"Holds VSCP multicast connections.");
    // Set font
	m_topitem_multicast->setForeground( 0 , b );
	m_topitem_multicast->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_multicast);

    // REST
    QStringList strlist_rest(QString(tr("REST Connections")).split(','));
    m_topitem_rest = new QTreeWidgetItem(strlist_rest, static_cast<int>(CVscpClient::connType::REST));
    m_topitem_rest->setIcon(0,iconTest);
    m_topitem_rest->setToolTip(0,"Holds VSCP REST connections.");
    // Set font
	m_topitem_rest->setForeground( 0 , b );
	m_topitem_rest->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_rest);

    // RAWCAN
    QStringList strlist_rawcan(QString(tr("RAWCAN Connections")).split(','));
    m_topitem_rawcan = new QTreeWidgetItem(strlist_rawcan, static_cast<int>(CVscpClient::connType::RAWCAN));
    m_topitem_rawcan->setIcon(0,iconTest);
    m_topitem_rawcan->setToolTip(0,"Holds generic CAN connections.");
    // Set font
	m_topitem_rawcan->setForeground( 0 , b );
	m_topitem_rawcan->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_rawcan);

    // RAWMQTT
    QStringList strlist_rawmqtt(QString(tr("RAWMQTT Connections")).split(','));
    m_topitem_rawmqtt = new QTreeWidgetItem(strlist_rawmqtt, static_cast<int>(CVscpClient::connType::RAWMQTT));
    m_topitem_rawmqtt->setIcon(0,iconTest);
    m_topitem_rawmqtt->setToolTip(0,"Holds generic MQTT connections.");
    // Set font
	m_topitem_rawmqtt->setForeground( 0 , b );
	m_topitem_rawmqtt->setFont( 0,  font );
    m_connTreeTable->addTopLevelItem(m_topitem_rawmqtt);

    // TEST
    // QStringList strlist_test(QString(tr("Fluorine")).split(',')); 
    // // QTreeWidgetItem *item_test = new QTreeWidgetItem(m_topitem_tcpip, strlist_test); 
    // QTreeWidgetItem *item_test = new QTreeWidgetItem(strlist_test);
    // const QIcon iconFluorine = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    // item_test->setIcon(0, iconFluorine);
    // item_test->setToolTip(0, "This is just a test connection from a snowy country named Sweden.");
    // // m_connTreeTable->addTopLevelItem(item_test);
    // m_topitem_tcpip->addChild(item_test);

    //addChildItemToConnectionTree(m_topitem_tcpip, "Kalle tupp");

    // Load connections
    addLoadedConnections();

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    initRemoteEventDbFetch();

    if (!pworks->loadEventDb()) {
        QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to load remote event data. Will not be used."),
                                    QMessageBox::Ok );
    }

}

///////////////////////////////////////////////////////////////////////////////
// initRemoteEventDbFetch
//

void MainWindow::initRemoteEventDbFetch()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    // Get version for remote event database
    QUrl versionUrl("https://vscp.org/events/version.json");
    pworks->m_pVersionCtrl = new FileDownloader(versionUrl, this);

    bool success = connect(pworks->m_pVersionCtrl, &FileDownloader::downloaded,
                            this, &MainWindow::checkRemoteEventDbVersion);
}

///////////////////////////////////////////////////////////////////////////////
// initRemoteEventDbFetch
//

void MainWindow::initForcedRemoteEventDbFetch()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    // Get version for remote events
    QUrl eventUrl("https://www.vscp.org/events/vscp_events.sqlite3");
    pworks->m_pVersionCtrl = new FileDownloader(eventUrl, this);

    bool success = connect(pworks->m_pVersionCtrl, &FileDownloader::downloaded, 
                            this, &MainWindow::downloadedEventDb);
}

///////////////////////////////////////////////////////////////////////////////
// checkRemoteEventDbVersion
//

void MainWindow::checkRemoteEventDbVersion()
{
    json j;
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    QString ver(pworks->m_pVersionCtrl->downloadedData());
    qDebug() << "Data: " << ver;
    if ( -1 != ver.indexOf("<title>404 Not Found</title>")) {
        QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("Unable to get version for remote VSCP event data. Will not be downloaded."),
                                QMessageBox::Ok );
        return;
    }

    try {
        j = json::parse(ver.toStdString().c_str());   

        QString dd(j["generated"].get<std::string>().c_str());
        pworks->m_lastEventDbServerDateTime = QDateTime::fromString(dd, Qt::ISODateWithMs);
        // qDebug() << dt;
        // qDebug() << pworks->m_lastEventDbLoadDateTime;
        // qDebug() << dt.toTime_t() << " " << pworks->m_lastEventDbLoadDateTime.toTime_t();

        QString path = pworks->m_shareFolder;
        path += "vscp_events.sqlite3";

        // If there is a newer version we should download it
        if ( !QFile::exists(path) || 
             ( pworks->m_lastEventDbServerDateTime.toTime_t() > 
               pworks->m_lastEventDbLoadDateTime.toTime_t() ) ) {
            
            // Get version for remote events
            QUrl eventUrl("https://www.vscp.org/events/vscp_events.sqlite3");
            pworks->m_pVersionCtrl = new FileDownloader(eventUrl, this);

            bool success = connect(pworks->m_pVersionCtrl, &FileDownloader::downloaded, 
                                    this, &MainWindow::downloadedEventDb);
        } 
    }
    catch (...) {
        syslog(LOG_ERR,"Parsing VSCP Event version information failed");
    }
}


///////////////////////////////////////////////////////////////////////////////
// downloadedEventDb
//

void MainWindow::downloadedEventDb()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    QFile file("/tmp/vscp_events.sqlite3");
    file.open(QIODevice::WriteOnly);
    file.write(pworks->m_pVersionCtrl->downloadedData());
    file.close();
    qDebug() << "A new event database file has been download";
    
    QString path = pworks->m_shareFolder;
    path += "vscp_events.sqlite3";
    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    QFile::copy("/tmp/vscp_events.sqlite3", path);    

    //pworks->m_lastEventDbLoadDateTime = QDateTime::currentDateTime();
    pworks->m_lastEventDbLoadDateTime = pworks->m_lastEventDbServerDateTime;
    pworks->writeSettings();
    QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("A new VSCP event database has been downloaded. Restart your application to use the new database."),
                                QMessageBox::Ok );
}

///////////////////////////////////////////////////////////////////////////////
// addChildItemToConnectionTree
//

// void MainWindow::addChildItemToConnectionTree(QTreeWidgetItem *topitem, std::string name) 
// {
//     QStringList strlstname(QString(name.c_str()).split(',')); 
//     // QTreeWidgetItem *item_test = new QTreeWidgetItem(m_topitem_tcpip, strlist_test); 
//     QTreeWidgetItem *childitem = new QTreeWidgetItem(topitem, strlstname, 1000 + 1);
//     const QIcon icon = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
//     childitem->setIcon(0, icon);
//     //item_test->setToolTip(0, "This is just a test connection from a snowy country named Sweden.");
//     // m_connTreeTable->addTopLevelItem(item_test);
//     topitem->addChild(childitem);
// }

///////////////////////////////////////////////////////////////////////////////
// addChildItemToConnectionTree
//

void MainWindow::addChildItemToConnectionTree(QTreeWidgetItem *topitem, 
                                                const QJsonObject& conn) 
{
    // Check pointers
    assert(nullptr != topitem);

    //QJsonObject conn = m_mapConn[(*pconn)["uuid"]];
    treeWidgetItemConn *childitem = new treeWidgetItemConn(topitem, conn);
    m_connTreeTable->clearSelection();
    topitem->addChild(childitem);
    childitem->setSelected(true);    
}

///////////////////////////////////////////////////////////////////////////////
// addLoadedConnections
//

void MainWindow::addLoadedConnections(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    //m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
    //m_connTreeTable->setSortingEnabled(true);             // should cause sort on add


    //for (QMap<QString,QJsonObject>::iterator it = pworks->m_mapConn.begin(); it != pworks->m_mapConn.end(); ++it){ 
    QMap<QString,QJsonObject>::const_iterator it = pworks->m_mapConn.constBegin();
    while (it != pworks->m_mapConn.constEnd()) {

        if ( !(it.value())["type"].isUndefined() && !(it.value())["type"].isNull() ) {

            switch(static_cast<CVscpClient::connType>((it.value())["type"].toInt())) {

                case CVscpClient::connType::LOCAL:
                    {
                        // Create a new local communication object
                        // vscpClientLocal *pClient = new vscpClientLocal();

                        // pClient->setName((it->second)["name"].toString().toStdString());
                        // pClient->setPath((it->second)["path"].toString().toStdString());
                        // pClient->initFromJson(QJsonDocument(it->second).getConfigAsJson(QJsonDocument::Compact).toStdString());

                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_local, it.value());

                        m_topitem_local->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::TCPIP:
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_tcpip, it.value());

                        m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::CANAL: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_canal, it.value());

                        m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::SOCKETCAN: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_socketcan, it.value());

                        m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::WS1: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_ws1, it.value());

                        m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::WS2: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_ws2, it.value());

                        m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::MQTT: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_mqtt, it.value());

                        m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::UDP: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_udp, it.value());

                        m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::MULTICAST: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_multicast, it.value());

                        m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::REST: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_rest, it.value());

                        m_topitem_rest->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::RAWCAN: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_rawcan, it.value());

                        m_topitem_rawcan->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;

                case CVscpClient::connType::RAWMQTT: 
                    {
                        // Add connection to connection tree
                        addChildItemToConnectionTree(m_topitem_rawmqtt, it.value());

                        m_topitem_rawmqtt->sortChildren(0, Qt::AscendingOrder);
                        // m_connTreeTable->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
                        // m_connTreeTable->setSortingEnabled(true);             // should cause sort on add
                    }
                    break;
                                                        
                default:
                    break;  

            }    

        }

        it++;

    }
}

///////////////////////////////////////////////////////////////////////////////
// closeEvent
//

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } 
    else {
        event->ignore();
    }
}

///////////////////////////////////////////////////////////////////////////////
// openConnectionSettingsDialog
//

void MainWindow::openConnectionSettingsDialog(CVscpClient::connType type)
{
    switch (type) { 
        
        case CVscpClient::connType::LOCAL:
            newLocalConnection();
            break;

        case CVscpClient::connType::TCPIP:
            newTcpipConnection();
            break;

        case CVscpClient::connType::CANAL: 
            newCanalConnection();
            break;

        case CVscpClient::connType::SOCKETCAN: 
            newSocketCanConnection();
            break;

        case CVscpClient::connType::WS1: 
            newWs1Connection();
            break;

        case CVscpClient::connType::WS2: 
            newWs2Connection();
            break;

        case CVscpClient::connType::MQTT: 
            newMqttConnection();
            break;

        case CVscpClient::connType::UDP: 
            newUdpConnection();
            break;

        case CVscpClient::connType::MULTICAST: 
            newMulticastConnection();
            break;

        case CVscpClient::connType::REST: 
            newRestConnection();
            break;

        case CVscpClient::connType::RAWCAN: 
            newRawCanConnection();
            break;

        case CVscpClient::connType::RAWMQTT: 
            newRawMqttConnection();
            break;
                                                
        default:
            break;    
    }    
}



///////////////////////////////////////////////////////////////////////////////
// newConnection
//

void MainWindow::newConnection()
{
    if (maybeSave()) {
        //m_textEdit->clear();
        setCurrentFile(QString());
    }

    CDlgNewConnection dlg(this);

    if (QDialog::Accepted == dlg.exec()) {         
        openConnectionSettingsDialog( dlg.getSelectedType() ); 
    }
}

///////////////////////////////////////////////////////////////////////////////
// editConnectionItem  uint32_t connectionIndex
//

void MainWindow::editConnectionItem(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    QList<QTreeWidgetItem *> itemList;
    itemList = m_connTreeTable->selectedItems();
    
    foreach(QTreeWidgetItem *item, itemList) {

        // Not intereste din top level items
        if (NULL != item->parent()) {

            // Get item
            treeWidgetItemConn *itemConn = (treeWidgetItemConn *)item;

            // Get the connection object  
            QJsonObject *pconn = itemConn->getJson();

            switch ((*pconn)["type"].toInt()) {

                case static_cast<int>(CVscpClient::connType::LOCAL):
                    editLocalConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::TCPIP):
                    editTcpipConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::CANAL): 
                    editCanalConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                    editSocketCanConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::WS1): 
                    editWs1Connection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::WS2): 
                    editWs2Connection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::MQTT): 
                    editMqttConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::UDP): 
                    editUdpConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::MULTICAST): 
                    editMulticastConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::REST): 
                    editRestConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::RAWCAN): 
                    editRawCanConnection(itemConn);
                    break;

                case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                    editRawMqttConnection(itemConn);
                    break;
                                                        
                default:
                    break;    
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// cloneConnectionItem  uint32_t connectionIndex
//

void MainWindow::cloneConnectionItem(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    QList<QTreeWidgetItem *> itemList;
    itemList = m_connTreeTable->selectedItems();
    
    foreach(QTreeWidgetItem *item, itemList) {

        // Not intereste din top level items
        if (NULL != item->parent()) {

            // Get item
            treeWidgetItemConn *itemConn = (treeWidgetItemConn *)item;            

            // Get the connection object  
            QJsonObject *pconn = itemConn->getJson();

            // Make copy of config
            QJsonObject conn_copy = *pconn;

            int type = 0 ;
            if (!(*pconn)["type"].isNull()) {
                type = (*pconn)["type"].toInt();
            }

            // Add to main table
            pworks->addConnection(conn_copy, true);

            switch (type) {

                case static_cast<int>(CVscpClient::connType::LOCAL):
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_local, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::TCPIP):
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_tcpip, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::CANAL): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_canal, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_socketcan, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::WS1): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_ws1, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::WS2): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_ws2, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::MQTT): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_mqtt, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::UDP): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_udp, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::MULTICAST): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_multicast, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::REST): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_rest, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::RAWCAN): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_rawcan, conn_copy);
                    break;

                case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                    // Add connection to connection tree
                    addChildItemToConnectionTree(m_topitem_rawmqtt, conn_copy);
                    break;
                                                        
                default:
                    break;    
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// removeConnectionItem  
//

void MainWindow::removeConnectionItem(void)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    // Context Menu Creation
    // QModelIndex selected = m_connTreeTable->indexAt(pos);
    // QModelIndex parent = selected.parent();
    //QTreeWidgetItem *item = m_connTreeTable->selectedItems();
    
    QList<QTreeWidgetItem *> itemList;
    itemList = m_connTreeTable->selectedItems();
    foreach(QTreeWidgetItem *item, itemList) {
        treeWidgetItemConn * itemConn = (treeWidgetItemConn *)item;
        if (itemConn->isSelected()) {
            QJsonObject *pconn = itemConn->getJson();

            QString str = tr("Are you sure that you want to delete [");
            str += (*pconn)["name"].toString();
            str += "]";
            
            if ( QMessageBox::Yes == QMessageBox::question(this, tr("vscpworks+"), str,
                                                            QMessageBox::Yes|QMessageBox::No) ) {                
                //qDebug() << (*pconn)["uuid"].toString();
                pworks->removeConnection((*pconn)["uuid"].toString(), true);
                QTreeWidgetItem *parent = item->parent();
                parent->removeChild(item);
                delete item;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void MainWindow::onDoubleClicked(QTreeWidgetItem* item)
{
    // Get parent of item if any
    //QTreeWidgetItem *itemParent = item->parent();

    // If this is a top level item allow just new connection
    if (NULL != item->parent()) {

        // Get item
        treeWidgetItemConn * itemConn = (treeWidgetItemConn *)item;

        // Get the connection object  
        QJsonObject *pconn = itemConn->getJson();

        switch ((*pconn)["type"].toInt()) {

            case static_cast<int>(CVscpClient::connType::LOCAL):
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::TCPIP):

                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::CANAL): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::WS1): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::WS2): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::MQTT): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::UDP): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::MULTICAST): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::REST): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::RAWCAN): 
                newSession();
                break;

            case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                newSession();
                break;
                                                    
            default:
                break;    
        }
    }

    // This is a top level item
    else {

        switch (item->type()) {

            case static_cast<int>(CVscpClient::connType::LOCAL):
                newLocalConnection();
                break;

            case static_cast<int>(CVscpClient::connType::TCPIP):
                newTcpipConnection();
                break;

            case static_cast<int>(CVscpClient::connType::CANAL): 
                newCanalConnection();
                break;

            case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                newSocketCanConnection();
                break;

            case static_cast<int>(CVscpClient::connType::WS1): 
                newWs1Connection();
                break;

            case static_cast<int>(CVscpClient::connType::WS2): 
                newWs2Connection();
                break;

            case static_cast<int>(CVscpClient::connType::MQTT): 
                newMqttConnection();
                break;

            case static_cast<int>(CVscpClient::connType::UDP): 
                newUdpConnection();
                break;

            case static_cast<int>(CVscpClient::connType::MULTICAST): 
                newMulticastConnection();
                break;

            case static_cast<int>(CVscpClient::connType::REST): 
                newRestConnection();
                break;

            case static_cast<int>(CVscpClient::connType::RAWCAN): 
                newRawCanConnection();
                break;

            case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                newRawMqttConnection();
                break;
                                                    
            default:
                newConnection();
                break;    
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// showConnectionContextMenu
//

void MainWindow::showConnectionContextMenu(const QPoint& pos)
{
    // Context Menu Creation
    QModelIndex selected = m_connTreeTable->indexAt(pos);
    QModelIndex parent = selected.parent();
    QTreeWidgetItem *item = m_connTreeTable->itemAt(pos);

    if (nullptr != item) {
        statusBar()->showMessage(item->text(0));
    }

    int row = selected.row();

    QMenu *menu = new QMenu(this);

    // If this is a top level item allow just new connection
    if (QModelIndex() == parent) {
        switch (item->type()) {

            case static_cast<int>(CVscpClient::connType::LOCAL):
                menu->addAction(QString("Add new local connection..."), this, SLOT(newLocalConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::TCPIP):
                menu->addAction(QString("Add new tcp/ip connection..."), this, SLOT(newTcpipConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::CANAL): 
                menu->addAction(QString("Add new CANAL connection..."), this, SLOT(newCanalConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                menu->addAction(QString("Add new Socketcan connection..."), this, SLOT(newSocketCanConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::WS1): 
                menu->addAction(QString("Add new websocket WS1 connection..."), this, SLOT(newWs1Connection()));
                break;

            case static_cast<int>(CVscpClient::connType::WS2): 
                menu->addAction(QString("Add new websocket WS2 connection..."), this, SLOT(newWs2Connection()));
                break;

            case static_cast<int>(CVscpClient::connType::MQTT): 
                menu->addAction(QString("Add new MQTT connection..."), this, SLOT(newMqttConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::UDP): 
                menu->addAction(QString("Add new UDP connection..."), this, SLOT(newUdpConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::MULTICAST): 
                menu->addAction(QString("Add new multicast connection..."), this, SLOT(newMulticastConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::REST): 
                menu->addAction(QString("Add new REST connection..."), this, SLOT(newRestConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::RAWCAN): 
                menu->addAction(QString("Add new raw CAN connection..."), this, SLOT(newRawCanConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                menu->addAction(QString("Add new raw MQTT connection..."), this, SLOT(newRawMqttConnection()));
                break;
                                                    
            default:
                menu->addAction(QString("Add new connection..."), this, SLOT(newConnection()));
                break;    
        }
    }
    else {
        switch (item->parent()->type()) {

            case static_cast<int>(CVscpClient::connType::LOCAL):
                menu->addAction(QString("Add new local connection"), this, SLOT(newLocalConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::TCPIP):
                menu->addAction(QString("Add new tcp/ip connection"), this, SLOT(newTcpipConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::CANAL): 
                menu->addAction(QString("Add new CANAL connection"), this, SLOT(newCanalConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::SOCKETCAN): 
                menu->addAction(QString("Add new Socketcan connection"), this, SLOT(newSocketCanConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::WS1): 
                menu->addAction(QString("Add new websocket WS1 connection"), this, SLOT(newWs1Connection()));
                break;

            case static_cast<int>(CVscpClient::connType::WS2): 
                menu->addAction(QString("Add new websocket WS2 connection"), this, SLOT(newWs2Connection()));
                break;

            case static_cast<int>(CVscpClient::connType::MQTT): 
                menu->addAction(QString("Add new MQTT connection"), this, SLOT(newMqttConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::UDP): 
                menu->addAction(QString("Add new UDP connection"), this, SLOT(newUdpConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::MULTICAST): 
                menu->addAction(QString("Add new multicast connection"), this, SLOT(newMulticastConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::REST): 
                menu->addAction(QString("Add new REST connection"), this, SLOT(newRestConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::RAWCAN): 
                menu->addAction(QString("Add new raw CAN connection"), this, SLOT(newRawCanConnection()));
                break;

            case static_cast<int>(CVscpClient::connType::RAWMQTT): 
                menu->addAction(QString("Add new raw MQTT connection"), this, SLOT(newRawMqttConnection()));
                break;
                                                    
            default:
                menu->addAction(QString("Add new connection..."), this, SLOT(newConnection()));
                break;    
        }

        // Connections are stored in a list and there position is the index. item->type() is
        // this index + 1000 therefore we need to subtract 1000 to get the correct index
        // (uint32_t)(item->type()))
        menu->addAction(QString("Edit this connection"), this, SLOT(editConnectionItem()));
        menu->addAction(QString("Remove this connection"),this, SLOT(removeConnectionItem()));
        menu->addAction(QString("Clone this connection"),this, SLOT(cloneConnectionItem()));
        menu->addAction(QString("Add new connection"), this, SLOT(newConnection()));
    }

    menu->popup(m_connTreeTable->viewport()->mapToGlobal(pos));

}

///////////////////////////////////////////////////////////////////////////////
// open
//

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadConfiguration(fileName);
    }
}

///////////////////////////////////////////////////////////////////////////////
// save
//

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

///////////////////////////////////////////////////////////////////////////////
// saveAs
//

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

///////////////////////////////////////////////////////////////////////////////
// about
//

void MainWindow::about()
{
   QString str;
   QTextStream(&str) << tr("<b>VSCP Works+</b> is the second generation toolbox for general VSCP configuration, " 
            "diagnostics and other VSCP related tasks.<br/>"
            "<p><b>Version</b>: ") <<
            tr(VSCPWORKS_DISPLAY_VERSION) <<
            "</p><p>" <<
            tr(VSCPWORKS_COPYRIGHT_HTML) <<
            "</p>";
   QMessageBox::about(this, tr("About VSCP Works+"), str );
}



///////////////////////////////////////////////////////////////////////////////
// connectionsWasModified
//

void MainWindow::connectionsWasModified()
{
    //setWindowModified(m_textEdit->document()->isModified());
}

///////////////////////////////////////////////////////////////////////////////
// createActions
//

void MainWindow::createActions()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New connection..."), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New connection"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newConnection);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);


    // const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    // QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    // openAct->setShortcuts(QKeySequence::Open);
    // openAct->setStatusTip(tr("Open an existing file"));
    // connect(openAct, &QAction::triggered, this, &MainWindow::open);
    // fileMenu->addAction(openAct);
    // fileToolBar->addAction(openAct);

    // const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    // QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    // saveAct->setShortcuts(QKeySequence::Save);
    // saveAct->setStatusTip(tr("Save the document to disk"));
    // connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    // fileMenu->addAction(saveAct);
    // fileToolBar->addAction(saveAct);

    // const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    // QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    // saveAsAct->setShortcuts(QKeySequence::SaveAs);
    // saveAsAct->setStatusTip(tr("Save the document under a new name"));

    fileMenu->addSeparator();

    const QIcon newSessionIcon = QIcon::fromTheme("emblem-symbolic-link");
    QAction *newSessionAct = new QAction(newSessionIcon, tr("&Session window..."), this);
    newSessionAct->setShortcuts(QKeySequence::SaveAs);
    newSessionAct->setStatusTip(tr("Open a new VSCP Session window"));
    connect(newSessionAct, &QAction::triggered, this, &MainWindow::newSession);
    fileMenu->addAction(newSessionAct);
    fileToolBar->addAction(newSessionAct);

    const QIcon newDevConfigIcon = QIcon::fromTheme("document-properties");
    QAction *newDevConfigAct = new QAction(newDevConfigIcon, tr("&Configuration..."), this);
    newDevConfigAct->setShortcuts(QKeySequence::SaveAs);
    newDevConfigAct->setStatusTip(tr("Open a new VSCP configuration window"));
    connect(newDevConfigAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(newDevConfigAct);
    fileToolBar->addAction(newDevConfigAct);    

    const QIcon scanForDeviceIcon = QIcon::fromTheme("edit-find"); 
    QAction *scanForDeviceAct = new QAction(scanForDeviceIcon, tr("Scan for &Device..."), this); 
    scanForDeviceAct->setShortcuts(QKeySequence::SaveAs);
    scanForDeviceAct->setStatusTip(tr("Open a new device scan window."));
    connect(scanForDeviceAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(scanForDeviceAct);
    fileToolBar->addAction(scanForDeviceAct);

    const QIcon bootloaderIcon = QIcon::fromTheme("emblem-downloads");
    QAction *bootloaderAct = new QAction(bootloaderIcon, tr("&Bootloader wizard..."), this);
    bootloaderAct->setShortcuts(QKeySequence::SaveAs);
    bootloaderAct->setStatusTip(tr("Open a new VSCP bootloader wizard."));
    connect(bootloaderAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(bootloaderAct);
    fileToolBar->addAction(bootloaderAct);

    const QIcon newMdfEditorIcon = QIcon::fromTheme("emblem-documents"); // applications-office
    QAction *newMdfEditorAct = new QAction(newMdfEditorIcon, tr("&MDF editor..."), this); 
    newMdfEditorAct->setShortcuts(QKeySequence::SaveAs);
    newMdfEditorAct->setStatusTip(tr("Open a new NDF editor"));
    connect(newMdfEditorAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(newMdfEditorAct);
    fileToolBar->addAction(newMdfEditorAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);

    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    //connect(cutAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    //connect(copyAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    //connect(pasteAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();

#endif // !QT_NO_CLIPBOARD

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    QAction *settingsAct = toolsMenu->addAction(tr("&Settings"), this, &MainWindow::showMainsettings);
    settingsAct->setStatusTip(tr("Open settins..."));
    QAction *knownGuidAct = toolsMenu->addAction(tr("&Known GUID's"), this, &MainWindow::knownGuids);
    knownGuidAct->setStatusTip(tr("Edit/Add known GUID's..."));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *helpAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    helpAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    //connect(m_textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    //connect(m_textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// createStatusBar *
//

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

///////////////////////////////////////////////////////////////////////////////
// readSettings *
//

void MainWindow::readSettings()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    // QList <QScreen*> screens = QGuiApplication::screens();  // Multiscreen
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }
    else {
        restoreGeometry(geometry);
    }

    int size = settings.beginReadArray("hosts/connections");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        std::string conn = settings.value("conn", "").toString().toStdString();
    }
    settings.endArray();

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    

    //int margin = settings.value("editor/wrapMargin").toInt();
    //int margin = settings.value("editor/wrapMargin", 80).toInt();

    //std::string source { "Hello, World!" };
    //QVariant destination;        
    //destination = QString::fromStdString(source);

    // Configuration folder vscpworks
    //pworks->m_cfgfolder = settings.value("cfgfolder", "").toString();

    

    // Default numerical base
    //default_base_ numerical_base = settings.value("general/numerical-base", 0).toInt();

}

///////////////////////////////////////////////////////////////////////////////
// writeSettings *
//

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    settings.setValue("geometry", saveGeometry());
 
}

///////////////////////////////////////////////////////////////////////////////
// maybeSave
//

bool MainWindow::maybeSave()
{
    // if (!m_textEdit->document()->isModified()) {
    //     return true;
    // }

    // const QMessageBox::StandardButton ret
    //     = QMessageBox::warning(this, tr("Application"),
    //                            tr("The document has been modified.\n"
    //                               "Do you want to save your changes?"),
    //                            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    // switch (ret) {
    //     case QMessageBox::Save:
    //         return save();
    //     case QMessageBox::Cancel:
    //         return false;
    //     default:
    //         break;
    // }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadConfiguration
//

void MainWindow::loadConfiguration(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    //m_textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

///////////////////////////////////////////////////////////////////////////////
// saveFile
//

bool MainWindow::saveFile(const QString &fileName)
{
    /*
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setCurrentFile
//

void MainWindow::setCurrentFile(const QString &fileName)
{
    // curFile = fileName;
    // m_textEdit->document()->setModified(false);
    // setWindowModified(false);

    // QString shownName = curFile;
    // if (curFile.isEmpty())
    //     shownName = "untitled.txt";
    // setWindowFilePath(shownName);
}

///////////////////////////////////////////////////////////////////////////////
// strippedName
//

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

///////////////////////////////////////////////////////////////////////////////
// commitData
//

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        // if (m_textEdit->document()->isModified()) {
        //     save();
        // }
    }
}

///////////////////////////////////////////////////////////////////////////////
// newSession
//

void MainWindow::newSession()
{
    QList<QTreeWidgetItem *> itemList;
    itemList = m_connTreeTable->selectedItems();
    
    foreach(QTreeWidgetItem *item, itemList) {

        // Not intereste din top level items
        if (NULL != item->parent()) {

            // Get item
            treeWidgetItemConn *itemConn = (treeWidgetItemConn *)item;

            // Get the connection object  
            QJsonObject *pconn = itemConn->getJson();

            CFrmSession *w = new CFrmSession(nullptr, pconn);
            w->setAttribute(Qt::WA_DeleteOnClose);  // Make window close on exit
            w->setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            w->setWindowFlags(Qt::Window);
            w->show();
            w->raise();
            // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
            w->activateWindow();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// showMainsettings
//

void MainWindow::showMainsettings(void)
{
    CDlgMainSettings *dlg = new CDlgMainSettings(this);
    dlg->show();
}

///////////////////////////////////////////////////////////////////////////////
// showMainsettings
//

void MainWindow::knownGuids(void)
{
    CDlgKnownGuid *dlg = new CDlgKnownGuid();
    dlg->show();
}

///////////////////////////////////////////////////////////////////////////////
// newLocalConnection
//

void MainWindow::newLocalConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsLocal dlg(this);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Add the new connection
        //pworks->m_mapConn.push_back(dlg.getJson());
        QJsonObject conn = dlg.getJson();
        // QString uuid = QUuid::createUuid().toString();
        // conn["uuid"] = uuid;
        // pworks->m_mapConn[uuid] = conn;
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientLocal *pClient = new vscpClientLocal();

        // pClient->setName(strName.toStdString());
        // pClient->setPath(dlg.getPath().toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_local, conn);
        m_topitem_local->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editLocalConnection
//

void MainWindow::editLocalConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsLocal dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_local->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newCanalConnection
//

void MainWindow::newCanalConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsCanal dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientCanal *pClient = new vscpClientCanal();
        
        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        // pClient->init(dlg.getPath().toStdString(), dlg.getConfig().toStdString(), dlg.getFlags() );

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_canal, conn);
        m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
    } 
}

///////////////////////////////////////////////////////////////////////////////
// editCanalConnection
//

void MainWindow::editCanalConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsCanal dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newTcpipConnection
//

void MainWindow::newTcpipConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsTcpip dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientTcp *pClient = new vscpClientTcp();
        // pClient->setName(strName.toStdString());

        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_tcpip, conn);
        m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
    } 
}

///////////////////////////////////////////////////////////////////////////////
// editTcpipConnection
//

void MainWindow::editTcpipConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsTcpip dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newSocketCanConnection
//

void MainWindow::newSocketCanConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsSocketCan dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientSocketCan *pClient = new vscpClientSocketCan();
        // pClient->setName(strName.toStdString());

        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_socketcan, conn);
        m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editSocketCanConnection
//

void MainWindow::editSocketCanConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsSocketCan dlg(this);

    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newMqttConnection
//

void MainWindow::newMqttConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsMqtt dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientMqtt *pClient = new vscpClientMqtt();
        // pClient->setName(strName.toStdString());

        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_mqtt, conn);
        m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editMqttConnection
//

void MainWindow::editMqttConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsMqtt dlg(this);

    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newWs1Connection
//

void MainWindow::newWs1Connection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsWs1 dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientWs1 *pClient = new vscpClientWs1();

        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_ws1, conn);
        m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editWs1Connection
//

void MainWindow::editWs1Connection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsWs1 dlg(this);

    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newWs2Connection
//

void MainWindow::newWs2Connection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsWs2 dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientWs2 *pClient = new vscpClientWs2();

        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_ws2, conn);
        m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editWs2Connection
//

void MainWindow::editWs2Connection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsWs2 dlg(this);

    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newUdpConnection
//

void MainWindow::newUdpConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsUdp dlg(this);

    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientUdp *pClient = new vscpClientUdp();

        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_udp, conn);
        m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editUdpConnection
//

void MainWindow::editUdpConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsUdp dlg(this);

    dlg.setJson(pconn);    
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newMulticastConnection
//

void MainWindow::newMulticastConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsMulticast dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientMulticast *pClient = new vscpClientMulticast();

        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_multicast, conn);
        m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editMulticatsConnection
//

void MainWindow::editMulticastConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsMulticast dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newRestConnection
//

void MainWindow::newRestConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsRest dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientRest *pClient = new vscpClientRest();

        // pClient->setName(strName.toStdString());
        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_rest, conn);
        m_topitem_rest->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editRestConnection
//

void MainWindow::editRestConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsRest dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_rest->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newRawCanConnection
//

void MainWindow::newRawCanConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsRawCan dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientRawCan *pClient = new vscpClientRawCan();

        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        // pClient->setName(strName.toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_rawcan, conn);
        m_topitem_rawcan->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editRawCanConnection
//

void MainWindow::editRawCanConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsRawCan dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_rawcan->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// newRawMqttConnection
//

void MainWindow::newRawMqttConnection()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    CDlgConnSettingsRawMqtt dlg(this);
    dlg.setInitialFocus();

restart:
    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        QJsonObject conn = dlg.getJson();
        if (!pworks->addConnection(conn, true)) {
            QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    tr("Failed to add new connection."),
                                    QMessageBox::Ok );
        }

        // Create a new local communication object
        // vscpClientRawMqtt *pClient = new vscpClientRawMqtt();

        // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
        // pClient->setName(strName.toStdString());
        //pClient->setPath(dlg.getPath());
        //m_mapConn.push_back(pClient);

        // Add connection to connection tree
        addChildItemToConnectionTree(m_topitem_rawmqtt, conn);
        m_topitem_rawmqtt->sortChildren(0, Qt::AscendingOrder);
    }
}

///////////////////////////////////////////////////////////////////////////////
// editRawMqttConnection
//

void MainWindow::editRawMqttConnection(treeWidgetItemConn *itemConn)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    
    // Get the connection object  
    QJsonObject *pconn = itemConn->getJson();

    CDlgConnSettingsRawMqtt dlg(this);
    dlg.setJson(pconn);
    dlg.setInitialFocus();

restart:

    if (QDialog::Accepted == dlg.exec()) {
        QString strName = dlg.getName();
        if (!strName.length()) {
            QMessageBox::warning(this, tr("vscpworks+"),
                               tr("A connection needs a description"),
                               QMessageBox::Ok);
            goto restart;
        }

        // Set the new connection
        QJsonObject conn = dlg.getJson();
        
        // Update listTree data
        itemConn->setJson(conn);

        // Update main table data
        pworks->m_mapConn[conn["uuid"].toString()] = conn;
        pworks->writeConnections();

        // Replace any possible new name
        itemConn->setText(0, conn["name"].toString());

        m_topitem_rawmqtt->sortChildren(0, Qt::AscendingOrder);
    }
}

#endif


