// cfrmnodescan.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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
#include <vscpworks.h>

#include <mustache.hpp>

#include <vscp_client_canal.h>
#include <vscp_client_mqtt.h>
#include <vscp_client_multicast.h>
#include <vscp_client_rawcan.h>
#include <vscp_client_rawmqtt.h>
#include <vscp_client_rest.h>
#include <vscp_client_rs232.h>
#include <vscp_client_rs485.h>
#include <vscp_client_socketcan.h>
#include <vscp_client_tcp.h>
#include <vscp_client_udp.h>
#include <vscp_client_ws1.h>
#include <vscp_client_ws2.h>

#include "cfrmnodescan.h"
#include "ui_cfrmnodescan.h"
//#include "cdlgmainsettings.h"
//#include "cdlgtxedit.h"


#include <QClipboard>
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

///////////////////////////////////////////////////////////////////////////////
// vscp_client_ack
//

// void CVscpClientCallback::eventReceived(vscpEvent *pev)
// {
//     vscpEvent ev;
//     //emit CFrmSession::receiveRow(pev, true);
// }

static void
eventReceived(vscpEvent* pev, void* pobj)
{
  vscpEvent* pevnew = new vscpEvent;
  pevnew->sizeData  = 0;
  pevnew->pdata     = nullptr;
  vscp_copyEvent(pevnew, pev);

  CFrmSession* pSession = (CFrmSession*)pobj;
  pSession->threadReceive(pevnew);
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

  // No connection set yet
  m_vscpConnType = CVscpClient::connType::NONE;
  m_vscpClient   = NULL;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(
    std::string(tr("Node configuration module opended").toStdString()));

  if (nullptr == pconn) {
    spdlog::error(std::string(tr("pconn is null").toStdString()));
    QMessageBox::information(this,
                             tr("vscpworks+"),
                             tr("Can't open node configuration window - "
                                "application configuration data is missing"),
                             QMessageBox::Ok);
    return;
  }

  // Save session configuration
  m_connObject = *pconn;

  // Must have a type
  if (m_connObject["type"].isNull()) {
    spdlog::error(
      std::string(tr("Type is not define in JSON data").toStdString()));
    QMessageBox::information(this,
                             tr("vscpworks+"),
                             tr("Can't open node configuration  window - The "
                                "connection type is unknown"),
                             QMessageBox::Ok);
    return;
  }

  m_vscpConnType =
    static_cast<CVscpClient::connType>(m_connObject["type"].toInt());

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
  int nWidth  = 1200;
  int nHeight = 800;

  if (parent != NULL) {
    setGeometry(parent->x() + parent->width() / 2 - nWidth / 2,
                parent->y() + parent->height() / 2 - nHeight / 2,
                nWidth,
                nHeight);
  }
  else {
    resize(nWidth, nHeight);
  }

  QJsonDocument doc(m_connObject);
  QString strJson(doc.toJson(QJsonDocument::Compact));

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::LOCAL:
      break;

    case CVscpClient::connType::TCPIP:
      m_vscpClient = new vscpClientTcp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::CANAL:
      m_vscpClient = new vscpClientCanal();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(
          this,
          tr("VSCP Works +"),
          tr("Failed to initialize CANAL driver. See log for more details."));
        return;
      }
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::SOCKETCAN:
      m_vscpClient = new vscpClientSocketCan();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this,
                             tr("VSCP Works +"),
                             tr("Failed to initialize SOCKETCAN driver. See "
                                "log for more details."));
        return;
      }
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::WS1:
      m_vscpClient = new vscpClientWs1();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::WS2:
      m_vscpClient = new vscpClientWs2();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MQTT:
      m_vscpClient = new vscpClientMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::UDP:
      m_vscpClient = new vscpClientUdp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MULTICAST:
      m_vscpClient = new vscpClientMulticast();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::REST:
      m_vscpClient = new vscpClientRest();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RS232:
      m_vscpClient = new vscpClientRs232();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RS485:
      m_vscpClient = new vscpClientRs485();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWCAN:
      m_vscpClient = new vscpClientRawCan();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWMQTT:
      m_vscpClient = new vscpClientRawMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      // m_connectActToolBar->setChecked(true);
      connectToRemoteHost(true);
      break;
  }
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

void CFrmNodeScan::done(int rv)
{
    if (QDialog::Accepted == rv) { // ok was pressed
        
        vscpworks *pworks = (vscpworks *)QCoreApplication::instance();


        // Session window
        //pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
        
    }
    //QMainWindow::done(rv);
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
// showRegisterContextMenu
//

void
CFrmNodeScan::showRegisterContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Send event")), this, SLOT(sendTxEvent()));

  menu->addSeparator();

  menu->addAction(QString(tr("Copy TX event to clipboard")),
                  this,
                  SLOT(copyTxToClipboard()));

  menu->addSeparator();

  menu->addAction(QString(tr("Add transmission row...")),
                  this,
                  SLOT(addTxEvent()));

  menu->addAction(QString(tr("Edit selected transmission row...")),
                  this,
                  SLOT(editTxEvent()));

  menu->addAction(QString(tr("Clone selected transmission row...")),
                  this,
                  SLOT(cloneTxEvent()));

  menu->addAction(QString(tr("Delete selected transmission row...")),
                  this,
                  SLOT(deleteTxEvent()));

  menu->addSeparator();

  menu->addAction(QString(tr("Save transmission rows...")),
                  this,
                  SLOT(saveTxEvents()));

  menu->addAction(QString(tr("Load transmission rows...")),
                  this,
                  SLOT(loadTxEvents()));

  menu->addSeparator();

  menu->addAction(QString(tr("Clear selections...")),
                  this,
                  SLOT(clrSelectionsTxEvent()));

  // menu->popup(m_txTable->viewport()->mapToGlobal(pos));
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

    case CVscpClient::connType::LOCAL:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(
          tr("Session: Unable to connect to remote host.").toStdString()));
        QMessageBox::information(this,
                                 tr("vscpworks+"),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::error(std::string(
          tr("Session: Successful connect to remote client.").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str = tr("Session: Unable to connect to the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr("vscpworks+"),
                                 tr("Failed to open a connection to the CANAL "
                                    "driver (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::error(
          std::string(tr("Session: Successful connected to the CANAL driver.")
                        .toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str =
          tr("Session: Unable to connect to the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr("vscpworks+"),
                                 tr("Failed to open a connection to SOCKETCAN "
                                    "(see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::error(std::string(
          tr("Session: Successful connected to SOCKETCAN.").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(
          tr("Session: Unable to connect to remote host").toStdString()));
        QMessageBox::information(this,
                                 tr("vscpworks+"),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::error(std::string(
          tr("Session: Successful connect to remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::UDP:
      break;

    case CVscpClient::connType::MULTICAST:
      break;

    case CVscpClient::connType::REST:
      break;

    case CVscpClient::connType::RS232:
      break;

    case CVscpClient::connType::RS485:
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

    case CVscpClient::connType::LOCAL:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
        spdlog::error(
          std::string(tr("Session: Unable to disconnect tcp/ip remote client")
                        .toStdString()));
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Failed to disconnect the connection to the txp/ip remote "
             "host"),
          QMessageBox::Ok);
      }
      else {
        spdlog::error(std::string(
          tr("Session: Successful disconnect from tcp/ip remote host")
            .toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:

      // Remove callback

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QString str =
          tr("Session: Unable to disconnect from the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Failed to disconnect the connection to the CANAL driver"),
          QMessageBox::Ok);
      }
      else {
        spdlog::error(
          std::string(tr("Session: Successful disconnect from CANAL driver")
                        .toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QString str = tr("Session: Unable to disconnect from the "
                         "SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Failed to disconnect the connection to the SOCKETCAN "
             "driver"),
          QMessageBox::Ok);
      }
      else {
        spdlog::error(
          std::string(tr("Session: Successful disconnect from SOCKETCAN driver")
                        .toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
        spdlog::error(std::string(
          tr("Session: Unable to disconnect from MQTT remote client")
            .toStdString()));
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Failed to disconnect the connection to the MQTT remote "
             "host"),
          QMessageBox::Ok);
      }
      else {
        spdlog::error(std::string(
          tr("Session: Successful disconnect from the MQTT remote host")
            .toStdString()));
      }
      break;

    case CVscpClient::connType::UDP:
      break;

    case CVscpClient::connType::MULTICAST:
      break;

    case CVscpClient::connType::REST:
      break;

    case CVscpClient::connType::RS232:
      break;

    case CVscpClient::connType::RS485:
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
