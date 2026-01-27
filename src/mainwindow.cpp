// mainwindow.cpp
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include <QJSEngine>
#include <QMessageBox>
#include <QtWidgets>

#include <QtSerialPort/QSerialPort>
#include <QtSerialBus/QCanBus>
#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanBusFactory>
#include <QtSerialBus/QCanBusFrame>

#include "filedownloader.h"
#include "vscpworks.h"

#include "bootloaderwizard.h"
#include "cdlgconnsettingscanal.h"
#include "cdlgconnsettingsmqtt.h"
#include "cdlgconnsettingsmulticast.h"
#include "cdlgconnsettingssocketcan.h"
#include "cdlgconnsettingstcpip.h"
#include "cdlgconnsettingsudp.h"
#include "cdlgconnsettingsws1.h"
#include "cdlgconnsettingsws2.h"
#include "cdlgknownguid.h"
#include "cdlgmainsettings.h"
#include "cdlgnewconnection.h"
#include "cdlgsessionfilter.h"
#include "cfrmmdf.h"
#include "cfrmnodeconfig.h"
#include "cfrmnodescan.h"
#include "cfrmsession.h"
#include "filedownloader.h"
#include "version.h"
#include "vscp-client-base.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <fstream>

#include "mainwindow.h"

///////////////////////////////////////////////////////////////////////////////
// treeWidgetItemConn
//

treeWidgetItemConn::treeWidgetItemConn(QTreeWidgetItem* topItem,
                                       const json& conn)
  : QTreeWidgetItem(topItem, static_cast<int>(itemType::CONNECTION))
{
  assert(nullptr != topItem);

  setText(0, conn["name"].get<std::string>().c_str());
  m_conn = conn;

  const QIcon icon =
    QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));

  setIcon(0, icon);
  // item_test->setToolTip(0, "This is just a te1st connection from a snowy
  // country named Sweden.");
  // m_connTreeTable->addTopLevelItem(item_test);
  // topItem->addChild(this);
  topItem->setExpanded(true);
  // topItem->setSelected(false);
  setSelected(true);
}

treeWidgetItemConn::~treeWidgetItemConn() {}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// MainWindow
//

MainWindow::MainWindow()
  : m_connTreeTable(new QTreeWidget)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Enable custom context menu
  m_connTreeTable->setContextMenuPolicy(Qt::CustomContextMenu);

  // Open communication item on double click
  connect(m_connTreeTable,
          &QTreeWidget::itemDoubleClicked,
          this,
          &MainWindow::onDoubleClicked);

  // Open pop up menu on right click
  connect(m_connTreeTable,
          &QTreeWidget::customContextMenuRequested,
          this,
          &MainWindow::showConnectionContextMenu);

  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  // TODO
  // if (QCanBus::instance()->plugins().contains(QStringLiteral("socketcan")))
  // {
  //     // plugin available
  // }

  // QJSEngine myEngine;
  // QJSValue three = myEngine.evaluate("(1 + 2 + Math.PI);");
  // qDebug() << tr("Int Value = ") << three.toInt();
  // qDebug() << tr("Value = ") << three.toNumber();
  // myEngine.evaluate("var e = {};e.data = [11,22,33];");
  // myEngine.evaluate("var ttt=77;");
  // //QJSValue fun = myEngine.evaluate("(function(a,b) { var e = {}; e.data =
  // [0,1,2,3,4,5]; switch (e.data[2]) { case 0: return 100; break; case 1:
  // return 222; break; case 2: return 333; break; }; })"); QJSValue fun =
  // myEngine.evaluate("(function(a,b) { return e.data[2]; })"); QJSValueList
  // args; args << 1 << 2; QJSValue threeAgain = fun.call(args); qDebug() <<
  // threeAgain.toInt();

  // qDebug() << "end";
  // QJSEngine engine; if (1) { return 2; } else { return 3 };

  setCentralWidget(m_connTreeTable); // table widget
  createActions();
  createStatusBar();

  readSettings();

  // connect(m_textEdit->document(), &QTextDocument::contentsChanged,
  //        this, &MainWindow::connectionsWasModified);

  // https://forum.qt.io/topic/125035/what-is-was-qguiapplication-setfallbacksessionmanagementenabled/2
  // #ifndef QT_NO_SESSIONMANAGER
  //   QGuiApplication::setFallbackSessionManagementEnabled(false);
  //   connect(qApp,
  //           &QGuiApplication::commitDataRequest,
  //           this,
  //           &MainWindow::commitData);
  // #endif

  setCurrentFile(QString());
  setUnifiedTitleAndToolBarOnMac(true);

  QStringList headers(QString(tr("Connection")).split(','));
  m_connTreeTable->setHeaderLabel(tr("VSCP Remote Connections"));

  // Add root items

  const QIcon iconConnections =
    QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
  const QIcon iconLocal =
    QIcon::fromTheme("network-offline", QIcon(":info.png"));

  // const QIcon iconTest = QIcon::fromTheme("network-transmit-receive",
  // QIcon(":add.png"));
  const QIcon iconTest(":process_accept.png");

  // Set font for top-items
  QFont font("", 10, QFont::Bold);
  QBrush b(Qt::darkYellow);

  // CANAL
  QStringList strlist_canal(QString(tr("CANAL Connections")).split(','));
  m_topitem_canal =
    new QTreeWidgetItem(strlist_canal,
                        static_cast<int>(CVscpClient::connType::CANAL));
  m_topitem_canal->setIcon(0, iconTest);
  m_topitem_canal->setToolTip(0, "Holds VSCP CANAL connections.");
  // Set font
  m_topitem_canal->setForeground(0, b);
  m_topitem_canal->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_canal);

#ifndef WIN32
  // Socketcan
  QStringList strlist_socketcan(
    QString(tr("Socketcan Connections")).split(','));
  m_topitem_socketcan =
    new QTreeWidgetItem(strlist_socketcan,
                        static_cast<int>(CVscpClient::connType::SOCKETCAN));
  m_topitem_socketcan->setIcon(0, iconTest);
  m_topitem_socketcan->setToolTip(0, "Holds VSCP socketcan connections.");
  // Set font
  m_topitem_socketcan->setForeground(0, b);
  m_topitem_socketcan->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_socketcan);
#endif

  // tcp/ip
  QStringList strlist_tcpip(QString(tr("TCP/IP Connections")).split(','));
  m_topitem_tcpip =
    new QTreeWidgetItem(strlist_tcpip,
                        static_cast<int>(CVscpClient::connType::TCPIP));
  m_topitem_tcpip->setIcon(0, iconTest);
  m_topitem_tcpip->setToolTip(0, "Holds VSCP tcp/ip connections.");
  // Set font
  m_topitem_tcpip->setForeground(0, b);
  m_topitem_tcpip->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_tcpip);

  // MQTT
  QStringList strlist_mqtt(QString(tr("MQTT Connections")).split(','));
  m_topitem_mqtt =
    new QTreeWidgetItem(strlist_mqtt,
                        static_cast<int>(CVscpClient::connType::MQTT));
  m_topitem_mqtt->setIcon(0, iconTest);
  m_topitem_mqtt->setToolTip(0, "Holds VSCP MQTT connections.");
  // Set font
  m_topitem_mqtt->setForeground(0, b);
  m_topitem_mqtt->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_mqtt);

  // WS1
  QStringList strlist_ws1(QString(tr("WS1 Connections")).split(','));
  m_topitem_ws1 =
    new QTreeWidgetItem(strlist_ws1,
                        static_cast<int>(CVscpClient::connType::WS1));
  m_topitem_ws1->setIcon(0, iconTest);
  m_topitem_ws1->setToolTip(0, "Holds VSCP websocket ws1 connections.");
  // Set font
  m_topitem_ws1->setForeground(0, b);
  m_topitem_ws1->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_ws1);

  // WS2
  QStringList strlist_ws2(QString(tr("WS2 Connections")).split(','));
  m_topitem_ws2 =
    new QTreeWidgetItem(strlist_ws2,
                        static_cast<int>(CVscpClient::connType::WS2));
  m_topitem_ws2->setIcon(0, iconTest);
  m_topitem_ws2->setToolTip(0, "Holds VSCP websocket ws2 connections.");
  // Set font
  m_topitem_ws2->setForeground(0, b);
  m_topitem_ws2->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_ws2);

  // UDP
  QStringList strlist_udp(QString(tr("UDP Connections")).split(','));
  m_topitem_udp =
    new QTreeWidgetItem(strlist_udp,
                        static_cast<int>(CVscpClient::connType::UDP));
  m_topitem_udp->setIcon(0, iconTest);
  m_topitem_udp->setToolTip(0, "Holds VSCP UDP connections.");
  // Set font
  m_topitem_udp->setForeground(0, b);
  m_topitem_udp->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_udp);

  // Multicast
  QStringList strlist_multicast(
    QString(tr("Multicast Connections")).split(','));
  m_topitem_multicast =
    new QTreeWidgetItem(strlist_multicast,
                        static_cast<int>(CVscpClient::connType::MULTICAST));
  m_topitem_multicast->setIcon(0, iconTest);
  m_topitem_multicast->setToolTip(0, "Holds VSCP multicast connections.");
  // Set font
  m_topitem_multicast->setForeground(0, b);
  m_topitem_multicast->setFont(0, font);
  m_connTreeTable->addTopLevelItem(m_topitem_multicast);

  // TEST
  // QStringList strlist_test(QString(tr("Fluorine")).split(','));
  // // QTreeWidgetItem *item_test = new QTreeWidgetItem(m_topitem_tcpip,
  // strlist_test); QTreeWidgetItem *item_test = new
  // QTreeWidgetItem(strlist_test); const QIcon iconFluorine =
  // QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
  // item_test->setIcon(0, iconFluorine);
  // item_test->setToolTip(0, "This is just a test connection from a snowy
  // country named Sweden.");
  // // m_connTreeTable->addTopLevelItem(item_test);
  // m_topitem_tcpip->addChild(item_test);

  // addChildItemToConnectionTree(m_topitem_tcpip, "Kalle tupp");

  // Load connections
  addLoadedConnections();

  initRemoteEventDbFetch();

  // On first start-up wait a while for the
  // event database to be downloaded
  // QString pathEventDb = pworks->m_shareFolder;
  // pathEventDb += "vscp_events.sqlite3";
  // qint64 start = QDateTime::currentMSecsSinceEpoch();
  // while (true) {
  //     if (QFile::exists(pathEventDb) ||
  //         ((QDateTime::currentMSecsSinceEpoch()-start) > 5000)) {
  //         break;
  //     }
  //     sleep(1);
  // }

  if (!pworks->loadEventDb()) {
    statusBar()->showMessage(tr("Failed to load remote event data. Will "
                                "try to load from external source."));
    // QMessageBox::information(this,
    //                             tr(APPNAME),
    //                             tr("Failed to load remote event data.
    //                             Will try to load from external source."),
    //                             QMessageBox::Ok );
  }

  // m_connTreeTable->selectionModel()->select(1, QItemSelectionModel::Select
  // | QItemSelectionModel::Rows);
  // m_connTreeTable->setCurrentIndex(QModelIndex())

  setWindowTitle(tr("VSCP Works+"));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

MainWindow::~MainWindow()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  foreach (QTreeWidgetItem* item, itemList) {

    // Not intereste din top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// initRemoteEventDbFetch
//

void
MainWindow::initRemoteEventDbFetch()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get version for remote event database
  QUrl versionUrl(pworks->URL_EVENT_VERSION);
  pworks->m_pVersionEventDbCtrl = new FileDownloader(versionUrl, this);

  bool success = connect(pworks->m_pVersionEventDbCtrl,
                         &FileDownloader::downloaded,
                         this,
                         &MainWindow::checkRemoteEventDbVersion);
}

///////////////////////////////////////////////////////////////////////////////
// initRemoteEventDbFetch
//

void
MainWindow::initForcedRemoteEventDbFetch()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get version for remote events
  QUrl eventUrl(tr("https://www.vscp.org/events/vscp_events.sqlite3"));
  pworks->m_pVersionEventDbCtrl = new FileDownloader(eventUrl, this);

  bool success = connect(pworks->m_pVersionEventDbCtrl,
                         &FileDownloader::downloaded,
                         this,
                         &MainWindow::downloadedEventDb);
}

///////////////////////////////////////////////////////////////////////////////
// checkRemoteEventDbVersion
//

void
MainWindow::checkRemoteEventDbVersion()
{
  json j;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QString ver(pworks->m_pVersionEventDbCtrl->downloadedData());
  spdlog::debug("Remote event db version is {}", ver.toStdString());
  // qDebug() << "__Data: " << ver;
  if (-1 != ver.indexOf("<title>404 Not Found</title>")) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Unable to get version for remote VSCP "
                                "event data. Will not be downloaded."),
                             QMessageBox::Ok);
    return;
  }

  try {
    j = json::parse(ver.toStdString().c_str());

    QString dd(j["generated"].get<std::string>().c_str());
    pworks->m_lastEventDbServerDateTime =
      QDateTime::fromString(dd, Qt::ISODateWithMs);

    QString path = pworks->m_shareFolder;
    path += "vscp_events.sqlite3";

    // If there is a newer version or it does not exist we should download it
    if (!QFile::exists(path) ||
        (pworks->m_lastEventDbServerDateTime.toSecsSinceEpoch() >
         pworks->m_lastEventDbLoadDateTime.toSecsSinceEpoch())) {

      // Get version for remote events
      QUrl eventUrl("https://www.vscp.org/events/vscp_events.sqlite3");
      pworks->m_pVersionEventDbCtrl = new FileDownloader(eventUrl, this);

      bool success = connect(pworks->m_pVersionEventDbCtrl,
                             &FileDownloader::downloaded,
                             this,
                             &MainWindow::downloadedEventDb);
    }
  }
  catch (...) {
    fprintf(stderr, "Parsing VSCP Event version information failed");
  }
}

///////////////////////////////////////////////////////////////////////////////
// downloadedEventDb
//

void
MainWindow::downloadedEventDb()
{
  QString tmpPath;
  QFile file;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QTemporaryDir dir;
  if (dir.isValid()) {
    tmpPath = dir.path() + tr("/vscp_events.sqlite3");
  }
  else {
#ifdef WIN32
    // C:\Users\<Username>\AppData\Local\Temp
    tmpPath = tr("C:/WINDOWS/Temp/vscp_events.sqlite3");
#else
    tmpPath = tr("/tmp/vscp_events.sqlite3");
#endif
  }

  file.setFileName(tmpPath);
  file.open(QIODevice::WriteOnly);
  file.write(pworks->m_pVersionEventDbCtrl->downloadedData());
  file.close();
  qDebug() << "A new event database file has been download";

  QString path = pworks->m_shareFolder;
  path += "vscp_events.sqlite3";
  if (QFile::exists(path)) {
    QFile::remove(path);
  }

  QFile::copy(tmpPath, path);

  pworks->m_lastEventDbLoadDateTime = pworks->m_lastEventDbServerDateTime;
  pworks->writeSettings();
  pworks->loadEventDb();
  statusBar()->showMessage(
    tr("A new VSCP event database has automatically been downloaded."));
  QMessageBox::information(
    this,
    tr(APPNAME),
    tr("A new VSCP event database has automatically been downloaded."),
    QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// addChildItemToConnectionTree
//

void
MainWindow::addChildItemToConnectionTree(QTreeWidgetItem* topitem,
                                         const json& conn)
{
  // Check pointers
  assert(nullptr != topitem);

  treeWidgetItemConn* childitem = new treeWidgetItemConn(topitem, conn);
  m_connTreeTable->clearSelection();
  topitem->addChild(childitem);
  // childitem->setSelected(true);
}

///////////////////////////////////////////////////////////////////////////////
// addLoadedConnections
//

void
MainWindow::addLoadedConnections(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QMap<std::string, json>::const_iterator it =
    pworks->m_mapConn.constBegin();
  while (it != pworks->m_mapConn.constEnd()) {

    json j = it.value();

    spdlog::trace(j.dump(4).c_str());

    if (j.contains("type") && j["type"].is_number()) {

      switch (
        static_cast<CVscpClient::connType>(j["type"].get<int>())) {

        case CVscpClient::connType::TCPIP: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_tcpip, j);
          m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
        } break;

        case CVscpClient::connType::CANAL: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_canal, j);
          m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
        } break;

#ifndef WIN32
        case CVscpClient::connType::SOCKETCAN: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_socketcan, j);
          m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
        } break;
#endif

        case CVscpClient::connType::WS1: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_ws1, j);
          m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
        } break;

        case CVscpClient::connType::WS2: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_ws2, j);
          m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
        } break;

        case CVscpClient::connType::MQTT: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_mqtt, j);
          m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
        } break;

        case CVscpClient::connType::UDP: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_udp, j);
          m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
        } break;

        case CVscpClient::connType::MULTICAST: {
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_multicast, j);
          m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
        } break;

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

void
MainWindow::closeEvent(QCloseEvent* event)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (maybeSave()) {
    pworks->writeSettings();
    event->accept();
  }
  else {
    event->ignore();
  }
}

///////////////////////////////////////////////////////////////////////////////
// openConnectionSettingsDialog
//

void
MainWindow::openConnectionSettingsDialog(CVscpClient::connType type)
{
  switch (type) {

    case CVscpClient::connType::TCPIP:
      newTcpipConnection();
      break;

    case CVscpClient::connType::CANAL:
      newCanalConnection();
      break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      newSocketCanConnection();
      break;
#endif

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

    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// newConnection
//

void
MainWindow::newConnection()
{
  if (maybeSave()) {
    // m_textEdit->clear();
    setCurrentFile(QString());
  }

  CDlgNewConnection dlg(this);

  if (QDialog::Accepted == dlg.exec()) {
    openConnectionSettingsDialog(dlg.getSelectedType());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConnectionItem  uint32_t connectionIndex
//

void
MainWindow::editConnectionItem(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested in top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object
      json* pconn = itemConn->getJson();

      switch ((*pconn)["type"].get<int>()) {

        case static_cast<int>(CVscpClient::connType::TCPIP):
          editTcpipConnection(itemConn);
          break;

        case static_cast<int>(CVscpClient::connType::CANAL):
          editCanalConnection(itemConn);
          break;

#ifndef WIN32
        case static_cast<int>(CVscpClient::connType::SOCKETCAN):
          editSocketCanConnection(itemConn);
          break;
#endif

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

        default:
          break;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// cloneConnectionItem  uint32_t connectionIndex
//

void
MainWindow::cloneConnectionItem(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested din top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object
      json* pconn = itemConn->getJson();

      // Make copy of connection config
      json conn_copy = *pconn;

      int type = 0;
      if ((*pconn).contains("type") && (*pconn)["type"].is_number()) {
        type = (*pconn)["type"].get<int>();
      }

      // New connection so new uuid (addConnection adds it)
      conn_copy["uuid"] = "";

      // Set new name
      conn_copy["name"] = conn_copy["name"].get<std::string>() + " (copy)";

      // Add to main table
      pworks->addConnection(conn_copy, true);

      switch (type) {

        case static_cast<int>(CVscpClient::connType::TCPIP):
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_tcpip, conn_copy);
          break;

        case static_cast<int>(CVscpClient::connType::CANAL):
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_canal, conn_copy);
          break;

#ifndef WIN32
        case static_cast<int>(CVscpClient::connType::SOCKETCAN):
          // Add connection to connection tree
          addChildItemToConnectionTree(m_topitem_socketcan, conn_copy);
          break;
#endif

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

        default:
          break;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// removeConnectionItem
//

void
MainWindow::removeConnectionItem(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  // Context Menu Creation
  // QModelIndex selected = m_connTreeTable->indexAt(pos);
  // QModelIndex parent = selected.parent();
  // QTreeWidgetItem *item = m_connTreeTable->selectedItems();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();
  foreach (QTreeWidgetItem* item, itemList) {
    treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;
    if (itemConn->isSelected()) {
      json* pconn = itemConn->getJson();

      QString str = tr("Are you sure that you want to delete [");
      str += (*pconn)["name"].get<std::string>();
      str += "]";

      if (QMessageBox::Yes ==
          QMessageBox::question(this,
                                tr(APPNAME),
                                str,
                                QMessageBox::Yes | QMessageBox::No)) {
        pworks->removeConnection((*pconn)["uuid"].get<std::string>().c_str(), true);
        QTreeWidgetItem* parent = item->parent();
        parent->removeChild(item);
        delete item;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void
MainWindow::onDoubleClicked(QTreeWidgetItem* item)
{
  // Get parent of item if any
  // QTreeWidgetItem *itemParent = item->parent();

  // If this is a top level item allow just new connection
  if (NULL != item->parent()) {

    // Get item
    treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

    // Get the connection object
    json* pconn = itemConn->getJson();

    switch ((*pconn)["type"].get<int>()) {

      case static_cast<int>(CVscpClient::connType::TCPIP):
        newSession();
        break;

      case static_cast<int>(CVscpClient::connType::CANAL):
        newSession();
        break;

#ifndef WIN32
      case static_cast<int>(CVscpClient::connType::SOCKETCAN):
        newSession();
        break;
#endif

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

      default:
        break;
    }
  }

  // This is a top level item
  else {

    switch (item->type()) {

      case static_cast<int>(CVscpClient::connType::TCPIP):
        newTcpipConnection();
        break;

      case static_cast<int>(CVscpClient::connType::CANAL):
        newCanalConnection();
        break;

#ifndef WIN32
      case static_cast<int>(CVscpClient::connType::SOCKETCAN):
        newSocketCanConnection();
        break;
#endif

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

      default:
        newConnection();
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// showConnectionContextMenu
//

void
MainWindow::showConnectionContextMenu(const QPoint& pos)
{
  // Context Menu Creation
  QModelIndex selected  = m_connTreeTable->indexAt(pos);
  QModelIndex parent    = selected.parent();
  QTreeWidgetItem* item = m_connTreeTable->itemAt(pos);

  if (nullptr != item) {
    statusBar()->showMessage(item->text(0));
  }

  int row = selected.row();

  QMenu* menu = new QMenu(this);

  // If this is a top level item allow just new connection
  if (QModelIndex() == parent) {
    switch (item->type()) {

      case static_cast<int>(CVscpClient::connType::TCPIP):
        menu->addAction(QString(tr("Add new tcp/ip connection...")),
                        this,
                        SLOT(newTcpipConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::CANAL):
        menu->addAction(QString(tr("Add new CANAL connection...")),
                        this,
                        SLOT(newCanalConnection()));
        break;

#ifndef WIN32
      case static_cast<int>(CVscpClient::connType::SOCKETCAN):
        menu->addAction(QString(tr("Add new Socketcan connection...")),
                        this,
                        SLOT(newSocketCanConnection()));
        break;
#endif

      case static_cast<int>(CVscpClient::connType::WS1):
        menu->addAction(QString(tr("Add new websocket WS1 connection...")),
                        this,
                        SLOT(newWs1Connection()));
        break;

      case static_cast<int>(CVscpClient::connType::WS2):
        menu->addAction(QString(tr("Add new websocket WS2 connection...")),
                        this,
                        SLOT(newWs2Connection()));
        break;

      case static_cast<int>(CVscpClient::connType::MQTT):
        menu->addAction(QString(tr("Add new MQTT connection...")),
                        this,
                        SLOT(newMqttConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::UDP):
        menu->addAction(QString(tr("Add new UDP connection...")),
                        this,
                        SLOT(newUdpConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::MULTICAST):
        menu->addAction(QString(tr("Add new multicast connection...")),
                        this,
                        SLOT(newMulticastConnection()));
        break;

      default:
        menu->addAction(QString(tr("Add new connection...")),
                        this,
                        SLOT(newConnection()));
        break;
    }
  }
  else {
    switch (item->parent()->type()) {

      case static_cast<int>(CVscpClient::connType::TCPIP):
        menu->addAction(QString(tr("Add new tcp/ip connection")),
                        this,
                        SLOT(newTcpipConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::CANAL):
        menu->addAction(QString(tr("Add new CANAL connection")),
                        this,
                        SLOT(newCanalConnection()));
        break;

#ifndef WIN32
      case static_cast<int>(CVscpClient::connType::SOCKETCAN):
        menu->addAction(QString(tr("Add new Socketcan connection")),
                        this,
                        SLOT(newSocketCanConnection()));
        break;
#endif

      case static_cast<int>(CVscpClient::connType::WS1):
        menu->addAction(QString(tr("Add new websocket WS1 connection")),
                        this,
                        SLOT(newWs1Connection()));
        break;

      case static_cast<int>(CVscpClient::connType::WS2):
        menu->addAction(QString(tr("Add new websocket WS2 connection")),
                        this,
                        SLOT(newWs2Connection()));
        break;

      case static_cast<int>(CVscpClient::connType::MQTT):
        menu->addAction(QString(tr("Add new MQTT connection")),
                        this,
                        SLOT(newMqttConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::UDP):
        menu->addAction(QString(tr("Add new UDP connection")),
                        this,
                        SLOT(newUdpConnection()));
        break;

      case static_cast<int>(CVscpClient::connType::MULTICAST):
        menu->addAction(QString(tr("Add new multicast connection")),
                        this,
                        SLOT(newMulticastConnection()));
        break;

      default:
        menu->addAction(QString(tr("Add new connection...")),
                        this,
                        SLOT(newConnection()));
        break;
    }

    // Connections are stored in a list and there position is the index.
    // item->type() is this index + 1000 therefore we need to subtract 1000
    // to get the correct index (uint32_t)(item->type()))

    menu->addAction(QString(tr("Edit this connection")),
                    this,
                    SLOT(editConnectionItem()));
    menu->addAction(QString(tr("Remove this connection")),
                    this,
                    SLOT(removeConnectionItem()));
    menu->addAction(QString(tr("Clone this connection")),
                    this,
                    SLOT(cloneConnectionItem()));
    menu->addAction(QString(tr("Add new connection")),
                    this,
                    SLOT(newConnection()));

    // Add a separator
    menu->addSeparator();

    // Add the choices for the different operation frames that need a connection
    const QIcon newSessionIcon = QIcon(":/page.png");
    menu->addAction(newSessionIcon,
                    QString(tr("Session")),
                    this,
                    SLOT(newSession()));

    const QIcon devConfigIcon = QIcon(":/page_process.png");
    menu->addAction(devConfigIcon,
                    QString(tr("Configuration")),
                    this,
                    SLOT(newNodeConfiguration()));

    const QIcon scanForNodeIcon = QIcon(":/page_search.png");
    menu->addAction(scanForNodeIcon,
                    QString(tr("Node scan")),
                    this,
                    SLOT(newNodeScan()));

    const QIcon bootloadIcon = QIcon(":/page_up.png");
    menu->addAction(bootloadIcon,
                    QString(tr("Bootloader wizard")),
                    this,
                    SLOT(newNodeBootload()));
  }

  menu->popup(m_connTreeTable->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// open
//

void
MainWindow::open()
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

bool
MainWindow::save()
{
  if (curFile.isEmpty()) {
    return saveAs();
  }
  else {
    return saveFile(curFile);
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveAs
//

bool
MainWindow::saveAs()
{
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) {
    return false;
  }
  return saveFile(dialog.selectedFiles().first());
}

///////////////////////////////////////////////////////////////////////////////
// chkUpdate
//

void
MainWindow::chkUpdate()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  std::string tempPath =
    QStandardPaths::writableLocation(QStandardPaths::TempLocation)
      .toStdString();
  tempPath += "/vscpworks-version.js";

  if (CURLE_OK != pworks->downLoadFromURL(pworks->URL_VSCPWORKS_VERSION.toStdString(), tempPath)) {
    // Failed to download version info file
    spdlog::error("Failed to download file {0} to {1}",
                  tempPath,
                  tempPath);
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Failed to download file"),
                          QMessageBox::Ok);
    return;
  }

  // Read the JSON file
  json j;
  try {
    std::ifstream ifs(tempPath);
    j = json::parse(ifs);
  }
  catch (...) {
    spdlog::error("Failed to read version info file");
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Failed to read version info file"),
                          QMessageBox::Ok);
    return;
  }

  if (!(j.contains("version") && j["version"].is_object())) {
    spdlog::error("Version file is in wrong format");
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Version file is in wrong format"),
                          QMessageBox::Ok);
    return;
  }

  uint16_t major   = 0;
  uint16_t minor   = 0;
  uint16_t release = 0;
  uint16_t build   = 0;

  json jj = j["version"];

  if (jj.contains("major") && jj["major"].is_number()) {
    major = jj["major"].get<uint16_t>();
  }
  else {
    spdlog::error("Version file (major) is in wrong format");
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Version file (major) is in wrong format"),
                          QMessageBox::Ok);
    return;
  }

  if (jj.contains("minor") && jj["minor"].is_number()) {
    minor = jj["minor"].get<uint16_t>();
  }
  else {
    spdlog::error("Version file (minor) is in wrong format");
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Version file (minor) is in wrong format"),
                          QMessageBox::Ok);
    return;
  }

  if (jj.contains("release") && jj["release"].is_number()) {
    release = jj["release"].get<uint16_t>();
  }
  else {
    spdlog::error("Version file (release) is in wrong format");
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("Version file (release) is in wrong format"),
                          QMessageBox::Ok);
    return;
  }

  if ((major > VSCPWORKS_MAJOR_VERSION) ||
      ((major == VSCPWORKS_MAJOR_VERSION) && (minor > VSCPWORKS_MINOR_VERSION)) ||
      (((major == VSCPWORKS_MAJOR_VERSION) && (minor == VSCPWORKS_MINOR_VERSION) && (release > VSCPWORKS_RELEASE_VERSION)))) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("A new version of VSCP Works+ is available."),
                             QMessageBox::Ok);
  }
  else {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("You are running the latest version of VSCP Works+"),
                             QMessageBox::Ok);
    return;
  }

  std::string downloadURL;
  std::string downloadPath =
    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
      .toStdString();
  downloadPath += "/";

  // messagebox that ask if file should be downloaded
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this,
                                tr(APPNAME),
                                tr("Do you want to download the new version?"),
                                QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
#ifdef WIN32
    if (j.contains("win_x64") && j["win_x64"].is_string()) {
      downloadURL = j["win_x64"].get<std::string>();
    }
    else {
      spdlog::error("There is no win-x64 version in version file");
      QMessageBox::critical(this,
                            tr(APPNAME),
                            tr("Sorry, no version for your OS is available at the moment."),
                            QMessageBox::Ok);
      return;
    }
#else
    if (j.contains("linux_x64") && j["linux_x64"].is_string()) {
      downloadURL = j["linux_x64"].get<std::string>();
    }
    else {
      spdlog::error("There is no linux-x64 version in version file");
      QMessageBox::critical(this,
                            tr(APPNAME),
                            tr("Sorry, no version for your OS is available at the moment."),
                            QMessageBox::Ok);
      return;
    }
#endif

    // Get the file name
    downloadPath += downloadURL.substr(downloadURL.find_last_of("/") + 1);

    try {
      // Remove old file
      if (QFile::exists(downloadPath.c_str())) {
        QFile::remove(downloadPath.c_str());
      }
    }
    catch (...) {
      spdlog::error("Failed to remove old file {0}", downloadPath);
      QMessageBox::critical(this,
                            tr(APPNAME),
                            tr("Failed to remove old file"),
                            QMessageBox::Ok);
      return;
    }

    if (CURLE_OK != pworks->downLoadFromURL(downloadURL, downloadPath)) {
      // Failed to download install file
      spdlog::error("Failed to download installation file {0} to {1}",
                    downloadURL,
                    downloadPath);
      QMessageBox::critical(this,
                            tr(APPNAME),
                            tr("Failed to download installation file"),
                            QMessageBox::Ok);
      return;
    }

    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Downloaded new version of VSCP Works+ %0.%1.%2 to %3\n Please install it manually.")
                               .arg(major)
                               .arg(minor)
                               .arg(release)
                               .arg(downloadPath.c_str()),
                             QMessageBox::Ok);

    // Start the downloaded file
    // std::string strCmd = "start ";
    // strCmd += downloadPath;
    // system(strCmd.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// about
//

void
MainWindow::about()
{
  QString str;
  QTextStream(&str) << tr("<b>VSCP Works+</b> is the second generation "
                          "toolbox for general VSCP configuration, "
                          "diagnostics and other VSCP related tasks.<br/>"
                          "<p><b>Version</b>: ")
                    << tr(VSCPWORKS_DISPLAY_VERSION) << "</p><p>"
                    << tr(VSCPWORKS_COPYRIGHT_HTML) << "</p>";
  QMessageBox::about(this, tr("About VSCP Works+"), str);
}

///////////////////////////////////////////////////////////////////////////////
// connectionsWasModified
//

void
MainWindow::connectionsWasModified()
{
  // setWindowModified(m_textEdit->document()->isModified());
}

///////////////////////////////////////////////////////////////////////////////
// createActions for Main menu
//

void
MainWindow::createActions()
{

  QMenu* fileMenu       = menuBar()->addMenu(tr("&File"));
  QToolBar* fileToolBar = addToolBar(tr("File"));

  // New connection
  const QIcon newIcon =
    QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
  QAction* newAct = new QAction(newIcon, tr("&New connection..."), this);
  newAct->setShortcut(Qt::Key_N | Qt::CTRL);
  newAct->setStatusTip(tr("New connection"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newConnection);
  fileMenu->addAction(newAct);
  fileToolBar->addAction(newAct);

  fileMenu->addSeparator();

  // New Session
  // const QIcon newSessionIcon = QIcon::fromTheme("emblem-symbolic-link");
  const QIcon newSessionIcon = QIcon(":/page.png");
  QAction* newSessionAct =
    new QAction(newSessionIcon, tr("&Session window..."), this);
  newSessionAct->setShortcut(Qt::Key_S | Qt::ALT);
  newSessionAct->setStatusTip(tr("Open a new VSCP Session window"));
  connect(newSessionAct, &QAction::triggered, this, &MainWindow::newSession);
  fileMenu->addAction(newSessionAct);
  fileToolBar->addAction(newSessionAct);

  // Node configuration
  const QIcon newDevConfigIcon = QIcon(":/page_process.png");
  QAction* newDevConfigAct =
    new QAction(newDevConfigIcon, tr("&Configuration..."), this);
  newDevConfigAct->setShortcut(Qt::Key_C | Qt::ALT);
  newDevConfigAct->setStatusTip(tr("Open a new VSCP configuration window"));
  connect(newDevConfigAct,
          &QAction::triggered,
          this,
          &MainWindow::newNodeConfiguration);
  fileMenu->addAction(newDevConfigAct);
  fileToolBar->addAction(newDevConfigAct);

  // Scan for nodes
  // const QIcon scanForNodeIcon = QIcon::fromTheme("edit-find");
  const QIcon scanForNodeIcon = QIcon(":/page_search.png");
  QAction* scanForNodeAct =
    new QAction(scanForNodeIcon, tr("Scan for &nodes..."), this);
  scanForNodeAct->setShortcut(Qt::Key_F | Qt::ALT);
  scanForNodeAct->setStatusTip(tr("Open a new node scan window."));
  connect(scanForNodeAct, &QAction::triggered, this, &MainWindow::newNodeScan);
  fileMenu->addAction(scanForNodeAct);
  fileToolBar->addAction(scanForNodeAct);

  // Bootloader wizard
  // const QIcon bootloaderIcon = QIcon::fromTheme("emblem-downloads");
  const QIcon bootloaderIcon = QIcon(":/page_up.png");
  QAction* bootloaderAct =
    new QAction(bootloaderIcon, tr("&Bootloader wizard..."), this);
  bootloaderAct->setShortcut(Qt::Key_B | Qt::ALT);
  bootloaderAct->setStatusTip(tr("Open a new VSCP bootloader wizard."));
  connect(bootloaderAct, &QAction::triggered, this, &MainWindow::newNodeBootload);
  fileMenu->addAction(bootloaderAct);
  fileToolBar->addAction(bootloaderAct);

  // MDF Editor
  // const QIcon newMdfEditorIcon = QIcon::fromTheme("emblem-documents"); //
  // applications-office
  const QIcon newMdfEditorIcon = QIcon(":/page_edit.png");
  QAction* newMdfEditorAct =
    new QAction(newMdfEditorIcon, tr("&MDF editor..."), this);
  newMdfEditorAct->setShortcut(Qt::Key_M | Qt::CTRL);
  newMdfEditorAct->setStatusTip(tr("Open a new MDF editor"));
  connect(newMdfEditorAct, &QAction::triggered, this, &MainWindow::mdfEdit);
  fileMenu->addAction(newMdfEditorAct);
  fileToolBar->addAction(newMdfEditorAct);

  fileMenu->addSeparator();

  const QIcon exitIcon = QIcon::fromTheme("application-exit");
  QAction* exitAct =
    fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
  exitAct->setShortcuts(QKeySequence::Quit);

  exitAct->setStatusTip(tr("Exit the application"));

  // ----------------------------------------------------------------

  QMenu* editMenu       = menuBar()->addMenu(tr("&Edit"));
  QToolBar* editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
  const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
  QAction* cutAct     = new QAction(cutIcon, tr("Cu&t"), this);

  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                          "clipboard"));
  editMenu->addAction(cutAct);
  editToolBar->addAction(cutAct);

  const QIcon copyIcon =
    QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
  QAction* copyAct = new QAction(copyIcon, tr("&Copy"), this);
  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                           "clipboard"));
  editMenu->addAction(copyAct);
  editToolBar->addAction(copyAct);

  const QIcon pasteIcon =
    QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
  QAction* pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                            "selection"));
  editMenu->addAction(pasteAct);
  editToolBar->addAction(pasteAct);

#endif // !QT_NO_CLIPBOARD

  menuBar()->addSeparator();
  editMenu->addSeparator();

  const QIcon preferenceIcon =
    QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
  QAction* preferenceAct = editMenu->addAction(tr("&Settings..."), this, &MainWindow::showMainsettings);
  preferenceAct->setShortcuts(QKeySequence::Paste);
  preferenceAct->setStatusTip(tr("Set preferences..."));
  // editToolBar->addAction(preferenceAct);

  // ----------------------------------------------------------------

  QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
  // QAction* settingsAct = toolsMenu->addAction(tr("&Settings"),
  //                                             this,
  //                                             &MainWindow::showMainsettings);
  // settingsAct->setStatusTip(tr("Open settings..."));
  QAction* knownGuidAct =
    toolsMenu->addAction(tr("&Known GUID's"), this, &MainWindow::knownGuids);
  knownGuidAct->setStatusTip(tr("Edit/Add known GUID's..."));
  QAction* sessionFilterAct = toolsMenu->addAction(tr("&Session filters..."),
                                                   this,
                                                   &MainWindow::sessionFilter);
  knownGuidAct->setStatusTip(tr("Edit/Add session filters..."));

  // ----------------------------------------------------------------

  // QMenu* configMenu = menuBar()->addMenu(tr("&Configuration"));
  // QAction* settingsAct =
  //   configMenu->addAction(tr("&Settings"), this, &MainWindow::showMainsettings);
  // settingsAct->setStatusTip(tr("Open settings..."));

  // ----------------------------------------------------------------

  QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

  QAction* checkUpdateAct =
    helpMenu->addAction(tr("Check for updates..."), this, &MainWindow::chkUpdate);
  checkUpdateAct->setStatusTip(tr("Check for program updates"));

  helpMenu->addSeparator();

  QAction* aboutAct =
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction* aboutQtAct =
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
  cutAct->setEnabled(false);
  copyAct->setEnabled(false);
  // connect(m_textEdit, &QPlainTextEdit::copyAvailable, cutAct,
  // &QAction::setEnabled); connect(m_textEdit, &QPlainTextEdit::copyAvailable,
  // copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// createStatusBar *
//

void
MainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

///////////////////////////////////////////////////////////////////////////////
// readSettings *
//

void
MainWindow::readSettings()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QScreen* screen = QGuiApplication::primaryScreen();
  // // QList <QScreen*> screens = QGuiApplication::screens();  // Multiscreen
  // QSettings settings(QCoreApplication::organizationName(),
  //                    QCoreApplication::applicationName());
  // const QByteArray geometry =
  //   settings.value("geometry", QByteArray()).toByteArray();
  // if (geometry.isEmpty()) {
  //   const QRect availableGeometry = screen->availableGeometry();
  //   resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  //   move((availableGeometry.width() - width()) / 2,
  //        (availableGeometry.height() - height()) / 2);
  // }
  // else {
  //   restoreGeometry(geometry);
  // }
  if (!pworks->m_mainWindowRect.isValid() && !pworks->m_mainWindowRect.isNull()) {
    setGeometry(pworks->m_mainWindowRect);
  }
  else {
    QScreen* screen               = QGuiApplication::primaryScreen();
    const QRect availableGeometry = screen->availableGeometry();
    resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
    move((availableGeometry.width() - width()) / 2,
         (availableGeometry.height() - height()) / 2);
  }

  // int size = settings.beginReadArray("hosts/connections");
  // for (int i = 0; i < size; ++i) {
  //   settings.setArrayIndex(i);
  //   std::string conn = settings.value("conn", "").toString().toStdString();
  // }
  // settings.endArray();

  // vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // int margin = settings.value("editor/wrapMargin").toInt();
  // int margin = settings.value("editor/wrapMargin", 80).toInt();

  // std::string source { "Hello, World!" };
  // QVariant destination;
  // destination = QString::fromStdString(source);

  // Configuration folder vscpworks
  // pworks->m_cfgfolder = settings.value("cfgfolder", "").toString();

  // Default numerical base
  // default_base_ numerical_base = settings.value("general/numerical-base",
  // 0).toInt();
}

///////////////////////////////////////////////////////////////////////////////
// maybeSave
//

bool
MainWindow::maybeSave()
{
  // if (!m_textEdit->document()->isModified()) {
  //     return true;
  // }

  // const QMessageBox::StandardButton ret
  //     = QMessageBox::warning(this, tr("Application"),
  //                            tr("The document has been modified.\n"
  //                               "Do you want to save your changes?"),
  //                            QMessageBox::Save | QMessageBox::Discard |
  //                            QMessageBox::Cancel);
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

void
MainWindow::loadConfiguration(const QString& fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(
      this,
      tr("Application"),
      tr("Cannot read file %1:\n%2.")
        .arg(QDir::toNativeSeparators(fileName), file.errorString()));
    return;
  }

  QTextStream in(&file);
#ifndef QT_NO_CURSOR
  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
#endif
  // m_textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
  QGuiApplication::restoreOverrideCursor();
#endif

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
}

///////////////////////////////////////////////////////////////////////////////
// saveFile
//

bool
MainWindow::saveFile(const QString& fileName)
{
  /*
  QString errorMessage;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents()
  QSaveFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << m_textEdit->toPlainText();
      if (!file.commit()) {
          errorMessage = tr("Cannot write file %1:\n%2.")
                         .arg(QDir::toNativeSeparators(fileName),
  file.errorString());
      }
  } else {
      errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                     .arg(QDir::toNativeSeparators(fileName),
  file.errorString());
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

void
MainWindow::setCurrentFile(const QString& fileName)
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

QString
MainWindow::strippedName(const QString& fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

///////////////////////////////////////////////////////////////////////////////
// commitData
//

#ifndef QT_NO_SESSIONMANAGER
void
MainWindow::commitData(QSessionManager& manager)
{
  if (manager.allowsInteraction()) {
    if (!maybeSave())
      manager.cancel();
  }
  else {
    // Non-interactive: save without asking
    // if (m_textEdit->document()->isModified()) {
    //     save();
    // }
  }
}

///////////////////////////////////////////////////////////////////////////////
// newSession
//

void
MainWindow::newSession()
{
  QList<QTreeWidgetItem*> itemList;
  itemList          = m_connTreeTable->selectedItems();
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested in top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object

      json* pconn    = itemConn->getJson();
      CFrmSession* w = new CFrmSession(this, pconn);
      if (!w->isConnected() && (pworks->m_session_bAutoConnect)) {
        delete w;
        return;
      }
      w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
      w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
      w->setWindowFlags(Qt::Window);
      w->show();
      w->raise();
      // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
      w->activateWindow();
      // Qt::WindowFlags eFlags = windowFlags();
      // eFlags |= Qt::WindowStaysOnTopHint;
      // setWindowFlags(eFlags);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// showMainsettings
//

void
MainWindow::showMainsettings(void)
{
  CDlgMainSettings* dlg = new CDlgMainSettings(this);
  dlg->show();
}

///////////////////////////////////////////////////////////////////////////////
// showMainsettings
//

void
MainWindow::knownGuids(void)
{
  CDlgKnownGuid* dlg = new CDlgKnownGuid(this);

  // place help dialog "top right" corner just inside the top right corner of
  // parent window
  // QRect rect = dlg->geometry();           // get current geometry of dialog
  // window QRect parentRect = this->geometry();    // get current geometry of
  // parent window rect.moveTo(mapToGlobal(QPoint(parentRect.x() +
  // parentRect.width() - rect.width(), parentRect.y())));
  // dlg->setGeometry(rect);

  dlg->show();
}

///////////////////////////////////////////////////////////////////////////////
// sessionFilters
//

void
MainWindow::sessionFilter(void)
{
  CDlgSessionFilter dlg;
  if (QDialog::Accepted == dlg.exec()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("filter set"),
                             QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newCanalConnection
//

void
MainWindow::newCanalConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsCanal dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {

    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientCanal *pClient = new vscpClientCanal();

    // pClient->setName(strName.toStdString());
    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->init(dlg.getPath().toStdString(),
    // dlg.getConfig().toStdString(), dlg.getFlags() );

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_canal, conn);
    m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editCanalConnection
//

void
MainWindow::editCanalConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsCanal dlg(this);
  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_canal->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newTcpipConnection
//

void
MainWindow::newTcpipConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsTcpip dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientTcp *pClient = new vscpClientTcp();
    // pClient->setName(strName.toStdString());

    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_tcpip, conn);
    m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editTcpipConnection
//

void
MainWindow::editTcpipConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsTcpip dlg(this);
  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {

    QString strName = dlg.getName();

    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_tcpip->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newSocketCanConnection
//

void
MainWindow::newSocketCanConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
#ifndef WIN32
  CDlgConnSettingsSocketCan dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientSocketCan *pClient = new vscpClientSocketCan();
    // pClient->setName(strName.toStdString());

    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_socketcan, conn);
    m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// editSocketCanConnection
//

void
MainWindow::editSocketCanConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
#ifndef WIN32
  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsSocketCan dlg(this);

  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_socketcan->sortChildren(0, Qt::AscendingOrder);
  }
#endif
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// newMqttConnection
//

void
MainWindow::newMqttConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsMqtt dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_mqtt, conn);
    m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editMqttConnection
//

void
MainWindow::editMqttConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsMqtt dlg(this);

  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_mqtt->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newWs1Connection
//

void
MainWindow::newWs1Connection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsWs1 dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientWs1 *pClient = new vscpClientWs1();

    // pClient->setName(strName.toStdString());
    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_ws1, conn);
    m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editWs1Connection
//

void
MainWindow::editWs1Connection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsWs1 dlg(this);

  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_ws1->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newWs2Connection
//

void
MainWindow::newWs2Connection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsWs2 dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientWs2 *pClient = new vscpClientWs2();

    // pClient->setName(strName.toStdString());
    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_ws2, conn);
    m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editWs2Connection
//

void
MainWindow::editWs2Connection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsWs2 dlg(this);

  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_ws2->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newUdpConnection
//

void
MainWindow::newUdpConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsUdp dlg(this);

  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientUdp *pClient = new vscpClientUdp();

    // pClient->setName(strName.toStdString());
    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_udp, conn);
    m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editUdpConnection
//

void
MainWindow::editUdpConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsUdp dlg(this);

  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_udp->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newMulticastConnection
//

void
MainWindow::newMulticastConnection()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CDlgConnSettingsMulticast dlg(this);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    json conn = dlg.getJson();
    if (!pworks->addConnection(conn, true)) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to add new connection."),
                               QMessageBox::Ok);
    }

    // Create a new local communication object
    // vscpClientMulticast *pClient = new vscpClientMulticast();

    // pClient->setName(strName.toStdString());
    // pClient->initFromJson(QJsonDocument(dlg.getJson()).getConfigAsJson(QJsonDocument::Compact).toStdString());
    // pClient->setPath(dlg.getPath());
    // m_mapConn.push_back(pClient);

    // Add connection to connection tree
    addChildItemToConnectionTree(m_topitem_multicast, conn);
    m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editMulticatsConnection
//

void
MainWindow::editMulticastConnection(treeWidgetItemConn* itemConn)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Get the connection object
  json* pconn = itemConn->getJson();

  CDlgConnSettingsMulticast dlg(this);
  dlg.setJson(pconn);
  dlg.setInitialFocus();

restart:

  if (QDialog::Accepted == dlg.exec()) {
    QString strName = dlg.getName();
    if (!strName.length()) {
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("A connection needs a description"),
                           QMessageBox::Ok);
      goto restart;
    }

    // Set the new connection
    json conn = dlg.getJson();

    // Update listTree data
    itemConn->setJson(conn);

    // Update main table data
    pworks->m_mapConn[conn["uuid"].get<std::string>().c_str()] = conn;
    pworks->writeSettings();

    // Replace any possible new name
    itemConn->setText(0, conn["name"].get<std::string>().c_str());

    m_topitem_multicast->sortChildren(0, Qt::AscendingOrder);
  }
}

///////////////////////////////////////////////////////////////////////////////
// newNodeConfiguration
//

void
MainWindow::newNodeConfiguration()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // CFrmNodeConfig dlg(this);
  // dlg.setInitialFocus();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  // If no item is selected then complain
  if (!itemList.size()) {
    int ret = QMessageBox::warning(this,
                                   tr("VSVP Works+"),
                                   tr("No connection selected.\n"
                                      "Please select a connection first."));
    return;
  }

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested in top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object
      json* pconn = itemConn->getJson();

      CFrmNodeConfig* w = new CFrmNodeConfig(this, pconn);
      w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
      w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
      w->setWindowFlags(Qt::Window);
      w->show();
      w->raise();
      // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
      w->activateWindow();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// newNodeScan
//

void
MainWindow::newNodeScan()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  // If no item is selected then complain
  if (!itemList.size()) {
    int ret = QMessageBox::warning(this,
                                   tr("VSVP Works+"),
                                   tr("No connection selected.\n"
                                      "Please select a connection first."));
    return;
  }

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested in top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object
      json* pconn = itemConn->getJson();

      CFrmNodeScan* w = new CFrmNodeScan(this, pconn);
      w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
      w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
      w->setWindowFlags(Qt::Window);
      w->show();
      w->raise();
      // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
      w->activateWindow();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// newNodeBootload
//

void
MainWindow::newNodeBootload()
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> itemList;
  itemList = m_connTreeTable->selectedItems();

  // If no item is selected then complain
  if (!itemList.size()) {
    int ret = QMessageBox::warning(this,
                                   tr("VSVP Works+"),
                                   tr("No connection selected.\n"
                                      "Please select a connection first."));
    return;
  }

  foreach (QTreeWidgetItem* item, itemList) {

    // Not interested in top level items
    if (NULL != item->parent()) {

      // Get item
      treeWidgetItemConn* itemConn = (treeWidgetItemConn*)item;

      // Get the connection object
      json* pconn = itemConn->getJson();

      CBootLoadWizard wiz(this, pconn);
      if (VSCP_ERROR_SUCCESS != (rv = wiz.initBootLoaderWizard())) {
        spdlog::error("Aborting bootloader wizard (initBootLoaderWizard) rv={}", rv);
        return;
      }
      wiz.exec();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// mdfEdit
//

void
MainWindow::mdfEdit()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CFrmMdf* w = new CFrmMdf(this, nullptr);
  w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
  w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  w->setWindowFlags(Qt::Window);
  w->show();
  w->raise();
  // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
  w->activateWindow();
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
MainWindow::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections.md";
  QDesktopServices::openUrl(QUrl(link));
}

#endif
