// cfrmnodescan.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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
// tableWidget->resizeRowsToContents();

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <stdlib.h>

#include <vscp.h>
#include <vscphelper.h>
#include <vscpworks.h>

#include <mdf.h>

#include <mustache.hpp>

#include <vscp-client-canal.h>
#include <vscp-client-mqtt.h>
#include <vscp-client-multicast.h>
#include <vscp-client-rawcan.h>
#include <vscp-client-rawmqtt.h>
#ifndef WIN32
#include <vscp-client-socketcan.h>
#endif
#include <vscp-client-tcp.h>
#include <vscp-client-udp.h>
#include <vscp-client-ws1.h>
#include <vscp-client-ws2.h>

#include "cfrmnodeconfig.h"

#include "cfrmnodescan.h"
#include "ui_cfrmnodescan.h"
// #include "cdlgmainsettings.h"
// #include "cdlgtxedit.h"

#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QJSEngine>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------

CFoundNodeWidgetItem::CFoundNodeWidgetItem(QTreeWidget* parent)
  : QTreeWidgetItem(parent, TREE_LIST_FOUND_NODE_TYPE)
{
  m_nodeid   = 0;
  m_bStdRegs = false;
  m_bMdf     = false;
}

CFoundNodeWidgetItem::~CFoundNodeWidgetItem()
{
  // if (nullptr != m_pmdf) {
  //   delete m_pmdf;
  //   m_pmdf = nullptr;
  // }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// vscp-client-ack
//

// void CVscpClientCallback::eventReceived(vscpEvent *pev)
// {
//     vscpEvent ev;
//     //emit CFrmNode Scan::receiveRow(pev, true);
// }

static void
eventReceived(vscpEvent& ev, void* pobj)
{
  // printf("Scan event: %X:%X\n", pev->vscp_class, pev->vscp_type);

  vscpEvent* pevnew = new vscpEvent;
  pevnew->sizeData  = 0;
  pevnew->pdata     = nullptr;
  vscp_copyEvent(pevnew, &ev);

  CFrmNodeScan* pNodeScan = (CFrmNodeScan*)pobj;
  pNodeScan->threadReceive(pevnew);
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CFrmNodeScan
//

CFrmNodeScan::CFrmNodeScan(QWidget* parent, QJsonObject* pconn)
  : QMainWindow(parent)
  , ui(new Ui::CFrmNodeScan)
{
  ui->setupUi(this);

  ui->treeFound->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeFound->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // No connection set yet
  m_vscpConnType = CVscpClient::connType::NONE;
  m_vscpClient   = NULL;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(std::string(tr("Node configuration module opended").toStdString()));

  if (nullptr == pconn) {
    spdlog::error(std::string(tr("pconn is null").toStdString()));
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Can't open node configuration window - "
                                "application configuration data is missing"),
                             QMessageBox::Ok);
    return;
  }

  // Save session configuration
  m_connObject = *pconn;

  // Must have a type
  if (m_connObject["type"].isNull()) {
    spdlog::error(std::string(tr("Type is not defined in JSON data").toStdString()));
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Can't open node configuration  window - The "
                                "connection type is unknown"),
                             QMessageBox::Ok);
    return;
  }

  m_vscpConnType = static_cast<CVscpClient::connType>(m_connObject["type"].toInt());

  QString str; // = tr("VSCP Client Session - ");
  str += pworks->getConnectionName(m_vscpConnType);
  str += tr(" - ");
  if (!m_connObject["name"].isNull()) {
    str += m_connObject["name"].toString();
  }
  else {
    str += tr("Unknown");
  }

  setWindowTitle(str);

  // Initial default size of window
  int nWidth  = ui->centralwidget->width();
  int nHeight = ui->centralwidget->height();

  /*
    if (parent != NULL) {
      setGeometry(parent->x() + parent->width() / 2 - nWidth / 2,
                  parent->y() + parent->height() / 2 - nHeight / 2,
                  nWidth,
                  nHeight);
    }
    else {
      resize(nWidth, nHeight);
    }
  */

  QJsonDocument doc(m_connObject);
  QString strJson(doc.toJson(QJsonDocument::Compact));

  using namespace std::placeholders;
  auto cb = std::bind(&CFrmNodeScan::receiveCallback, this, _1, _2);
  //  lambda version for reference
  // auto cb = [this](auto a, auto b) { this->receiveCallback(a, b); };

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      m_vscpClient = new vscpClientTcp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::CANAL:
      m_vscpClient = new vscpClientCanal();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(
          this,
          tr(APPNAME),
          tr("Failed to initialize CANAL driver. See log for more details."));
        return;
      }
      // m_connObject["selected-interface"] = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
      connectToRemoteHost(true);
      break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      m_vscpClient = new vscpClientSocketCan();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to initialize SOCKETCAN driver. See "
                                "log for more details."));
        return;
      }
      connectToRemoteHost(true);
      break;
#endif

    case CVscpClient::connType::WS1:
      m_vscpClient = new vscpClientWs1();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::WS2:
      m_vscpClient = new vscpClientWs2();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MQTT:
      m_vscpClient = new vscpClientMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::UDP:
      m_vscpClient = new vscpClientUdp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(eventReceived, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MULTICAST:
      m_vscpClient = new vscpClientMulticast();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWCAN:
      m_vscpClient = new vscpClientRawCan();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWMQTT:
      m_vscpClient = new vscpClientRawMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      connectToRemoteHost(true);
      break;
  }

  ui->actionConnect->setDisabled(true);
  ui->actionConnect->setVisible(false);

  // Connect has been clicked
  connect(ui->actionConnect,
          SIGNAL(triggered(bool)),
          this,
          SLOT(connectToRemoteHost(bool)));

  // Scan has been selected in the menu
  connect(ui->actionScan, SIGNAL(triggered()), this, SLOT(doScan()));

  // Load MDF has been selected in the menu
  connect(ui->actionLoadMdf, SIGNAL(triggered()), this, SLOT(loadSelectedMdf()));

  // Load MDF has been selected in the menu
  connect(ui->actionLoadMdfAll, SIGNAL(triggered()), this, SLOT(loadAllMdf()));

  // Session has been selected in the menu
  connect(ui->actionSession, SIGNAL(triggered()), this, SLOT(goSession()));

  // Config has been selected in the menu
  connect(ui->actionConfigure, SIGNAL(triggered()), this, SLOT(goConfig()));

  // Enable/disable slow scan
  connect(ui->chkSlowScan, SIGNAL(stateChanged(int)), this, SLOT(slowScanStateChange(int)));

  // Open pop up menu on right click on register list
  connect(ui->treeFound,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeScan::showFindNodesContextMenu);

  // Register row has been clicked.
  connect(ui->treeFound,
          &QTreeWidget::itemClicked,
          this,
          &CFrmNodeScan::onFindNodesTreeWidgetItemClicked);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmNodeScan::~CFrmNodeScan()
{
  // Make sure we are disconnected
  doDisconnectFromRemoteHost();

  // Remove receive events
  while (m_rxEvents.size()) {
    vscpEvent* pev = m_rxEvents.front();
    m_rxEvents.pop_front();
    vscp_deleteEvent(pev);
    pev = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void
CFrmNodeScan::done(int rv)
{
  if (QDialog::Accepted == rv) { // ok was pressed
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    // Session window
    // pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
  }
  // QMainWindow::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// menu_open_main_settings
//

void
CFrmNodeScan::menu_open_main_settings(void)
{
  // CDlgMainSettings* dlg = new CDlgMainSettings(this);
  // dlg->exec();
}

///////////////////////////////////////////////////////////////////////////////
// connectToHost
//

void
CFrmNodeScan::connectToRemoteHost(bool checked)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (checked) {
    if (pworks->m_session_bAutoConnect) {
      doConnectToRemoteHost();
    }
  }
  else {
    doDisconnectFromRemoteHost();
  }
}

///////////////////////////////////////////////////////////////////////////////
// doConnectToRemoteHost
//

void
CFrmNodeScan::doConnectToRemoteHost(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(tr("Node Scan: Unable to connect to remote host.").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful connect to remote client.").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str = tr("Node Scan: Unable to connect to the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the CANAL "
                                    "driver (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful connected to the CANAL driver.").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str = tr("Node Scan: Unable to connect to the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to SOCKETCAN "
                                    "(see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful connected to SOCKETCAN.").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(tr("Node Scan: Unable to connect to remote host").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful connect to remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::UDP:
      break;

    case CVscpClient::connType::MULTICAST:
      break;

    case CVscpClient::connType::RAWCAN:
      break;

    case CVscpClient::connType::RAWMQTT:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// doDisconnectFromRemoteHost
//

void
CFrmNodeScan::doDisconnectFromRemoteHost(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
        spdlog::error(std::string(tr("Node Scan: Unable to disconnect tcp/ip remote client").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the txp/ip remote host"),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful disconnect from tcp/ip remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QString str = tr("Node Scan: Unable to disconnect from the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the CANAL driver"),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful disconnect from CANAL driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QString str = tr("Node Scan: Unable to disconnect from the "
                         "SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the SOCKETCAN driver"),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful disconnect from SOCKETCAN driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
        spdlog::error(std::string(tr("Node Scan: Unable to disconnect from MQTT remote client").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the MQTT remote host"),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Node Scan: Successful disconnect from the MQTT remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::UDP:
      break;

    case CVscpClient::connType::MULTICAST:
      break;

    case CVscpClient::connType::RAWCAN:
      break;

    case CVscpClient::connType::RAWMQTT:
      break;
  }
}

void
CFrmNodeScan::receiveRxRow(vscpEvent* pev)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// threadReceive
//

void
CFrmNodeScan::threadReceive(vscpEvent* pev)
{
  emit dataReceived(pev);
}

///////////////////////////////////////////////////////////////////////////////
// receiveCallback
//

void
CFrmNodeScan::receiveCallback(vscpEvent& ev, void *pobj) 
{
  vscpEvent* pevnew = new vscpEvent;
  pevnew->sizeData  = 0;
  pevnew->pdata     = nullptr;
  vscp_copyEvent(pevnew, &ev);

  emit dataReceived(pevnew);

  // Alternative method for reference
  //CFrmSession* pSession = (CFrmSession*)pobj;
  //pSession->threadReceive(pevnew);
}

///////////////////////////////////////////////////////////////////////////////
// parseNodes
//

bool
CFrmNodeScan::parseNodes(std::set<uint16_t>& nodelist)
{
  std::string str = ui->editSearchNodes->text().toStdString();
  vscp_trim(str);
  if (str.empty()) {
    return false;
  }

  std::deque<std::string> tokens;
  vscp_split(tokens, str, ",");
  if (tokens.empty()) {
    return false;
  }

  nodelist.clear();

  // Go through the tokens
  for (auto const& item : tokens) {
    std::string str = item;
    vscp_trim(str);
    if (str.empty()) {
      continue;
    }

    // Check if it is a range (x-y)
    std::deque<std::string> tokens2;
    vscp_split(tokens2, str, "-");
    if (tokens2.size() == 2) {
      std::string str1 = tokens2[0];
      std::string str2 = tokens2[1];
      vscp_trim(str1);
      vscp_trim(str2);
      if (!str1.empty() && !str2.empty()) {
        uint16_t nodeid1 = vscp_readStringValue(str1);
        uint16_t nodeid2 = vscp_readStringValue(str2);
        if ((nodeid1 > 0) && (nodeid2 > 0) && (nodeid1 < nodeid2) && (nodeid2 < 0xffff)) {
          for (uint16_t nodeid = nodeid1; nodeid <= nodeid2; nodeid++) {
            nodelist.insert(nodeid);
          }
        }
        else {
          spdlog::error(std::string(tr("Node Scan: Invalid node range").toStdString()));
        }
      }
    }
    else {
      uint16_t nodeid = vscp_readStringValue(str);
      if ((nodeid > 0) && (nodeid < 0xffff)) {
        nodelist.insert(nodeid);
      }
      else {
        spdlog::error(std::string(tr("Node Scan: Invalid node id").toStdString()));
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// doScan
//

void
CFrmNodeScan::doScan(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // ui->btnScan->setEnabled(false);
  ui->actionScan->setEnabled(false);

  // This is a sorted list with the nodeid's to search for.
  std::set<uint16_t> nodelist;

  // Get nodes to scan
  if (!parseNodes(nodelist)) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to parse nodes"),
                             QMessageBox::Ok);

    ui->actionScan->setEnabled(true);
    return;
  }

  ui->progressBarScan->setValue(0);
  ui->treeFound->clear();
  ui->infoArea->clear();

  if (ui->chkSlowScan->isChecked()) {
    std::string str = "Searching nodes : ";
    for (auto const& item : nodelist) {
      str += QString::number(item).toStdString();
      str += " ";
    }
    ui->infoArea->setText(QString::fromStdString(str));
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  std::string interface = m_connObject["selected-interface"].toString().toStdString();
  cguid guidInterface(interface);

  ui->progressBarScan->setValue(30);

  std::set<uint16_t> found;

  // SLOW SCAN
  if (ui->chkSlowScan->isChecked()) {

    // Slow scan

    uint32_t delay   = vscp_readStringValue(ui->editDelay->text().toStdString());
    uint32_t timeout = vscp_readStringValue(ui->editTimeout->text().toStdString());

    if (VSCP_ERROR_SUCCESS != vscp_scanSlowForDevices(*m_vscpClient,
                                                      guidInterface,
                                                      nodelist,
                                                      found,
                                                      nullptr,
                                                      delay,
                                                      timeout)) {
      ui->progressBarScan->setValue(0);
      spdlog::error(std::string(tr("Node Slow Scan: Failed to scan for devices").toStdString()));
      QApplication::restoreOverrideCursor();
      ui->infoArea->setText("Scan failed...");
      ui->infoArea->repaint();

      // QMessageBox::information(this,
      //                         tr(APPNAME),
      //                         tr("Failed to scan nodes"),
      //                         QMessageBox::Ok);

      ui->actionScan->setEnabled(true);
      return;
    }
  }
  // NORMAL SCAN
  else {
    // Normal scan

    ui->infoArea->setText("Scan in progress...");
    ui->infoArea->repaint();

    if (VSCP_ERROR_SUCCESS != vscp_scanForDevices(*m_vscpClient,
                                                  guidInterface,
                                                  found,
                                                  nullptr,
                                                  pworks->m_config_timeout)) {
      ui->progressBarScan->setValue(0);
      spdlog::error(std::string(tr("Node Fast Scan: Failed to scan for devices").toStdString()));
      QApplication::restoreOverrideCursor();
      QMessageBox::information(this,
                               APPNAME,
                               tr("Failed to scan nodes"),
                               QMessageBox::Ok);
      ui->actionScan->setEnabled(true);
      return;
    }
  }

  QString str = QString("Found %1 nodes").arg(found.size());
  ui->infoArea->setText(str);
  ui->infoArea->repaint();
  // printf("found node count = %zu\n", found.size());

  size_t additem = 70 / (found.size() + 1); // Add for the progress bar for each mdf file
  if (!ui->chkFetchInfo->isChecked()) {
    ui->progressBarScan->setValue(90);
  }

  for (auto const& item : found) {
    QString str               = "node " + QString::number(item);
    CFoundNodeWidgetItem* top = new CFoundNodeWidgetItem(ui->treeFound);
    top->setText(0, tr("Node with id = ") + QString::number(item));
    top->m_nodeid = item; // Save nodeid
    // Load mdf and standard registers if requested to do so
    if (ui->chkFetchInfo->isChecked()) {
      doLoadMdf(item);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      ui->progressBarScan->setValue(ui->progressBarScan->value() + (int)additem);
    }
  }

  if (ui->chkSlowScan->isChecked()) {
    std::string str = "Found nodes : ";
    str += QString::number(found.size()).toStdString();
    str += "\n";
    str += ui->infoArea->toPlainText().toStdString();
    ui->infoArea->setText(QString::fromStdString(str));
    ui->infoArea->repaint();
  }

  ui->progressBarScan->setValue(100);

  QApplication::restoreOverrideCursor();
  QApplication::processEvents();

  ui->actionScan->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// slowScanStateChange
//

void
CFrmNodeScan::slowScanStateChange(int state)
{
  if (state == Qt::Checked) {
    ui->editSearchNodes->setEnabled(true);
    ui->editDelay->setEnabled(true);
    ui->editTimeout->setEnabled(true);
  }
  else {
    ui->editSearchNodes->setEnabled(false);
    ui->editDelay->setEnabled(false);
    ui->editTimeout->setEnabled(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// showFindNodesContextMenu
//

void
CFrmNodeScan::showFindNodesContextMenu(const QPoint& pos)
{

  QMenu* menu = new QMenu(this);
  menu->addAction(QString(tr("Fetch MDF")), this, SLOT(loadSelectedMdf()));
  menu->addAction(QString(tr("Fetch ALL MDF")), this, SLOT(loadAllMdf()));
  menu->addSeparator();
  menu->addAction(QString(tr("Rescan")), this, SLOT(doScan()));
  menu->addSeparator();
  menu->addAction(QString(tr("Configure")), this, SLOT(goConfig()));
  menu->addAction(QString(tr("Session")), this, SLOT(goSession()));
  menu->popup(ui->treeFound->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// loadSelectedMdf
//

void
CFrmNodeScan::loadSelectedMdf(void)
{
  ui->actionScan->setEnabled(false);
  QList<QTreeWidgetItem*> selected = ui->treeFound->selectedItems();

  // Must be selected items
  if (!selected.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Must select a node to use operation."),
                             QMessageBox::Ok);
    ui->actionScan->setEnabled(true);
    return;
  }

  CFoundNodeWidgetItem* pitem = (CFoundNodeWidgetItem*)selected.at(0);
  if (nullptr == pitem) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to get node."),
                             QMessageBox::Ok);
    ui->actionScan->setEnabled(true);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  doLoadMdf(pitem->m_nodeid);

  QApplication::restoreOverrideCursor();
  QApplication::processEvents();

  ui->actionScan->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// loadMdf
//

void
CFrmNodeScan::loadAllMdf(void)
{
  // ui->btnScan->setEnabled(false);
  ui->actionScan->setEnabled(false);
  CFoundNodeWidgetItem* pItem;
  QTreeWidgetItemIterator it(ui->treeFound);
  while (*it) {
    pItem = (CFoundNodeWidgetItem*)(*it);
    doLoadMdf(pItem->m_nodeid);
    ++it;
  }

  ui->actionScan->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// loadMdf
//

void
CFrmNodeScan::doLoadMdf(uint16_t nodeid)
{

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CFoundNodeWidgetItem* pItem;
  QTreeWidgetItemIterator it(ui->treeFound);
  while (*it) {
    pItem = (CFoundNodeWidgetItem*)(*it);
    if (pItem->m_nodeid == nodeid) {
      break;
    }
    pItem = nullptr;
    ++it;
  }

  // Check if item found
  if (nullptr == pItem) {
    return;
  }

  pItem->m_bStdRegs = false;
  pItem->m_bMdf     = false;

  std::string interface = m_connObject["selected-interface"].toString().toStdString();
  cguid guidInterface(interface);

  cguid guidNode;
  guidNode = guidInterface;
  guidNode.setLSB(pItem->m_nodeid); // Set node id

  // Get MDF
  std::string str = "Fetching MDF for node " + QString::number(pItem->m_nodeid).toStdString();
  ui->infoArea->setText(QString::fromStdString(str));
  ui->infoArea->repaint();
  QApplication::processEvents();

  ui->statusBar->showMessage(tr("Reading standard registers from device..."));

  // Get standard registers
  int rv = pItem->m_stdregs.init(*m_vscpClient, guidNode, guidInterface, nullptr, pworks->m_config_timeout);
  if (VSCP_ERROR_SUCCESS != rv) {
    ui->statusBar->showMessage(tr("Failed to read standard registers from device. rv=") + QString::number(rv));
    spdlog::error("Failed to init standard registers {0}", rv);
    return;
  }

  // Standard registers downloaded
  pItem->m_bStdRegs = false;

  // Get GUID
  cguid guid;
  pItem->m_stdregs.getGUID(guid);
  spdlog::trace("Standard register getGUID = {}", guid.toString());

  std::string url = pItem->m_stdregs.getMDF();
  spdlog::trace("Standard register getMDF = {}", url);

  // create a temporary file name for remote MDF
  std::string tempMdfFileName;
  for (int i = 0; i < url.length(); i++) {
    if ((url[i] == '/') || (url[i] == '\\')) {
      tempMdfFileName += "_";
    }
    else {
      tempMdfFileName += url[i];
    }
  }

  // mkstemp()
  std::string tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString();
  tempPath += "/";
  tempPath += tempMdfFileName;
  pItem->m_tempMdfFile = tempPath;

  spdlog::debug("Temporary path: {}", tempPath);

  ui->statusBar->showMessage(tr("Downloading MDF file..."));

  CURLcode curl_rv;
  curl_rv = pItem->m_mdf.downLoadMDF(url, tempPath);
  if (CURLE_OK != curl_rv) {
    ui->statusBar->showMessage(tr("Failed to download MDF file for device."));
    spdlog::error("Failed to download MDF {0} curl rv={1}", url, (int)curl_rv);
    return;
  }

  // * * * Parse  MDF * * *

  ui->statusBar->showMessage(tr("Parsing MDF file..."));
  rv = pItem->m_mdf.parseMDF(tempPath);
  if (VSCP_ERROR_SUCCESS != rv) {
    ui->statusBar->showMessage(tr("Failed to parse MDF file for device."));
    spdlog::error("Failed to parse MDF {0} rv={1}", tempPath, rv);
    return;
  }

  // MDF downloaded & parsed
  pItem->m_bMdf = true;

  // Replace item string
  std::string strItem = tr("Node: ").toStdString();
  strItem += QString::number(pItem->m_nodeid).toStdString();
  strItem += " - ";
  strItem += pItem->m_mdf.getModuleName();
  strItem += ", Ver: ";
  strItem += pItem->m_mdf.getModuleVersion();
  pItem->setText(0, QString::fromStdString(strItem));

  // Set the HTML
  std::string html = vscp_getDeviceInfoHtml(pItem->m_mdf, pItem->m_stdregs);
  ui->infoArea->setHtml(html.c_str());
  ui->infoArea->repaint();
}

///////////////////////////////////////////////////////////////////////////////
// onFindNodesTreeWidgetItemClicked
//

void
CFrmNodeScan::onFindNodesTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
  CFoundNodeWidgetItem* pItem = (CFoundNodeWidgetItem*)item;
  if (nullptr == pItem) {
    return;
  }

  if ((pItem->type() == TREE_LIST_FOUND_NODE_TYPE) && pItem->m_bMdf && !pItem->m_bStdRegs) {

    // Set the HTML
    std::string html = vscp_getDeviceInfoHtml(pItem->m_mdf, pItem->m_stdregs);
    ui->infoArea->setHtml(html.c_str());
  }
  else {
    ui->infoArea->setText(tr("MDF info should be loaded before device info can be viewed"));
  }
}

///////////////////////////////////////////////////////////////////////////////
// goSession
//

void
CFrmNodeScan::goSession(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  CFrmSession* w = new CFrmSession(parentWidget(), &m_connObject);
  w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
  w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  w->setWindowFlags(Qt::Window);
  w->show();
  w->raise();
  // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
  w->activateWindow();
}

///////////////////////////////////////////////////////////////////////////////
// goConfig
//

void
CFrmNodeScan::goConfig(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QList<QTreeWidgetItem*> selected = ui->treeFound->selectedItems();
  // Must be selected items
  if (!selected.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Must select a node to use operation."),
                             QMessageBox::Ok);
    return;
  }

  CFoundNodeWidgetItem* pitem = (CFoundNodeWidgetItem*)selected.at(0);
  if (nullptr == pitem) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to get node."),
                             QMessageBox::Ok);
    return;
  }

  CFrmNodeConfig* w = new CFrmNodeConfig(parentWidget(), &m_connObject);
  w->setAttribute(Qt::WA_DeleteOnClose, true); // Make window close on exit
  w->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  w->setWindowFlags(Qt::Window);
  w->show();
  w->setNodeId(pitem->m_nodeid);
  w->update();
  w->raise();
  // https://wiki.qt.io/Technical_FAQ#QWidget_::activateWindow.28.29_-_behavior_under_windows
  w->activateWindow();
}