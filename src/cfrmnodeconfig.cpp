// cfrmnodeconfig.cpp
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

//#include <curl/curl.h>

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

#include "cdlgknownguid.h"

#include "cfrmnodeconfig.h"
#include "ui_cfrmnodeconfig.h"
//#include "cdlgmainsettings.h"
//#include "cdlgtxedit.h"

#include <mdf.h>

#include <QByteArray>
#include <QClipboard>
#include <QFile>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>
#include <QProgressBar>

// ----------------------------------------------------------------------------

CRegisterWidgetItem::CRegisterWidgetItem(const QString& text)
  : QTreeWidgetItem(QTreeWidgetItem::UserType + 1)
{
    ;
}

CRegisterWidgetItem::~CRegisterWidgetItem()
{
    ;
}

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
// CFrmNodeConfig
//

CFrmNodeConfig::CFrmNodeConfig(QWidget* parent, QJsonObject* pconn)
  : QMainWindow(parent)
  , ui(new Ui::CFrmNodeConfig)
{
  ui->setupUi(this);

  m_nUpdates = 0; // No update operations yet

  int cnt         = ui->session_tabWidget->count();
  QTabBar* tabBar = ui->session_tabWidget->tabBar();
  //tabBar->addTab("Skogig tab");

  QHeaderView* regTreeViewHeader = ui->treeWidgetRegisters->header();
  regTreeViewHeader->setDefaultAlignment(Qt::AlignCenter);
  ui->treeWidgetRegisters->setColumnWidth(0, 150);
  ui->treeWidgetRegisters->setColumnWidth(1, 80);
  ui->treeWidgetRegisters->setColumnWidth(2, 80);

  ui->treeWidgetRegisters->clear();

  // QStringList headers;
  // for (int i = 0; i < ui->treeView->model()->columnCount(); i++) {
  //     headers.append(ui->treeView->model()->headerData(i, Qt::Horizontal).toString());
  // }
  //void QTreeWidgetItem::setForeground(int column, const QBrush &brush)

  // Add page top item
  QTreeWidgetItem* itemTopReg1 = new QTreeWidgetItem(QTreeWidgetItem::UserType);
  itemTopReg1->setText(0, "Page 0");
  itemTopReg1->setFont(0, QFont("Arial", 12, QFont::Bold));
  itemTopReg1->setTextAlignment(REG_COL_POS, Qt::AlignLeft);
  //itemTopReg1->get  setStyleSheet("background-color: red");
  //itemTopReg1->setForeground(0, QBrush(QColor("#0000FF")));
  itemTopReg1->setForeground(0, QBrush(QColor("royalblue")));
  //itemTopReg1->setBackground(0, QBrush(QColor(Qt::black)));
  // QList<QTableWidgetItem*>::iterator it;
  // for (it = sellist.begin(); it != sellist.end(); it++) {
  //     if (QBrush(Qt::cyan) == (*it)->background()) {
  //         (*it)->setBackground(Qt::white);
  //     }
  //     else {
  //         (*it)->setBackground(Qt::cyan);
  //     }
  // }
  ui->treeWidgetRegisters->addTopLevelItem(itemTopReg1);

  // Add register sub item
  CRegisterWidgetItem* itemReg = new CRegisterWidgetItem("trttttt");
  itemReg->setText(0, "0000:0000");
  itemReg->setTextAlignment(0, Qt::AlignCenter);
  itemReg->setText(1, "rw");
  itemReg->setTextAlignment(1, Qt::AlignCenter);
  itemReg->setText(2, "0x55");
  itemReg->setTextAlignment(2, Qt::AlignCenter);
  itemReg->setText(3, "This is a test");
  itemReg->setTextAlignment(0, Qt::AlignLeft);
  itemTopReg1->addChild(itemReg);

  m_nodeidConfig   = nullptr;
  m_guidConfig     = nullptr;
  m_comboInterface = nullptr;

  /*
    m_nodeidConfig = new QSpinBox();
    m_nodeidConfig->setRange(0, 0xff);
    ui->mainToolBar->addWidget(new QLabel("node id:"));
    ui->mainToolBar->addWidget(m_nodeidConfig);

    m_guidConfig = new QLineEdit();
    ui->mainToolBar->addWidget(new QLabel("GUID: "));
    ui->mainToolBar->addWidget(m_guidConfig);

    m_comboInterface = new QComboBox();
    ui->mainToolBar->addWidget(new QLabel("Interface: "));
    ui->mainToolBar->addWidget(m_comboInterface);
  */

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

  QString str;
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

  /*!
    The selected interface is the interface we should select in the combo box
    when we open the window.
  */
  std::string interface =
    m_connObject["selected-interface"].toString().toStdString();

  /*!
    Interfaced at the time of configuration.
  */
  QJsonArray json_if_array = m_connObject["interfaces"].toArray();

  /*!
   *  If bFullLevel2 is true only GUID textbox is shown.
   *  If false the interface combo plus nickname spinnbox is shown.
   */
  bool bFullLevel2 = m_connObject["bfull-l2"].toBool();

  /*
  std::string _str;
  size_t sz = json_if_array.size();
  std::string sss = json_if_array.at(0)["if-item"].toString().toStdString();
  foreach (const QJsonValue &value, json_if_array) {
    qDebug() << value.toObject().value("if-item").toString();
    std::string sss =
  value.toObject().value("if-item").toString().toStdString(); std::cout << "if =
  " << sss << std::endl;
    //_str = value.toObject()["if-item"].toString().toStdString();
    // if (value.toObject()["name"].toString() == interface) {
    //   _str = value.toObject();
    //   //break;
    // }
  }
  */

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::LOCAL:
      // GUID
      break;

    case CVscpClient::connType::TCPIP: {
      std::string interface =
        m_connObject["selected-interface"].toString().toStdString();
    }

      if (bFullLevel2) {
        // GUID
        m_guidConfig = new QLineEdit();
        ui->mainToolBar->addWidget(new QLabel(" GUID: "));
        ui->mainToolBar->addWidget(m_guidConfig);

        m_btnSetGUID = new QPushButton(tr(" Set GUID"));
        ui->mainToolBar->addWidget(m_btnSetGUID);
        connect(m_btnSetGUID, SIGNAL(clicked()), this, SLOT(selectGuid()));
      }
      else {
        // Interface
        m_comboInterface = new QComboBox();
        ui->mainToolBar->addWidget(new QLabel(" If: "));
        ui->mainToolBar->addWidget(m_comboInterface);
        m_comboInterface->addItem(tr("---"));

        QJsonArray json_if_array = m_connObject["interfaces"].toArray();
        std::string _str;
        size_t sz = json_if_array.size();
        std::string sss =
          json_if_array.at(0)["if-item"].toString().toStdString();
        foreach (const QJsonValue& value, json_if_array) {
          // qDebug() << value.toObject().value("if-item").toString();
          // std::string if =
          // value.toObject().value("if-item").toString().toStdString();
          // std::cout << "if = " << if << std::endl;
          m_comboInterface->addItem(
            value.toObject().value("if-item").toString());
          //_str = value.toObject()["if-item"].toString().toStdString();
          // if (value.toObject()["name"].toString() == interface) {
          //   _str = value.toObject();
          //   //break;
          // }
          m_comboInterface->setCurrentText(interface.c_str());
        }

        // Nickname
        m_nodeidConfig = new QSpinBox();
        m_nodeidConfig->setRange(0, 0xff);
        ui->mainToolBar->addWidget(new QLabel("node id:"));
        ui->mainToolBar->addWidget(m_nodeidConfig);
      }

      m_vscpClient = new vscpClientTcp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::CANAL:
      // nodeid
      m_vscpClient = new vscpClientCanal();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(
          this,
          tr("VSCP Works +"),
          tr("Failed to initialize CANAL driver. See log for more details."));
        return;
      }

      m_nodeidConfig = new QSpinBox();
      m_nodeidConfig->setRange(0, 0xff);
      ui->mainToolBar->addWidget(new QLabel(" node id:"));
      ui->mainToolBar->addWidget(m_nodeidConfig);

      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::SOCKETCAN:
      // nodeid
      m_vscpClient = new vscpClientSocketCan();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this,
                             tr("VSCP Works +"),
                             tr("Failed to initialize SOCKETCAN driver. See "
                                "log for more details."));
        return;
      }

      m_nodeidConfig = new QSpinBox();
      m_nodeidConfig->setRange(0, 0xff);
      ui->mainToolBar->addWidget(new QLabel("node id:"));
      ui->mainToolBar->addWidget(m_nodeidConfig);

      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::WS1:
      // GUID
      m_vscpClient = new vscpClientWs1();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::WS2:
      // GUID
      m_vscpClient = new vscpClientWs2();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MQTT:
      // GUID
      m_vscpClient = new vscpClientMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::UDP:
      // GUID
      m_vscpClient = new vscpClientUdp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::MULTICAST:
      // GUID
      m_vscpClient = new vscpClientMulticast();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::REST:
      // GUID
      m_vscpClient = new vscpClientRest();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RS232:
      // nodeid
      m_vscpClient = new vscpClientRs232();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RS485:
      // nodeid
      m_vscpClient = new vscpClientRs485();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWCAN:
      m_vscpClient = new vscpClientRawCan();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;

    case CVscpClient::connType::RAWMQTT:
      m_vscpClient = new vscpClientRawMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallback(eventReceived, this);
      ui->actionConnect->setChecked(true);
      connectToRemoteHost(true);
      break;
  }

  // Menu and toolbar commands

  connect(ui->actionConnect,
          SIGNAL(triggered(bool)),
          this,
          SLOT(connectToRemoteHost(bool)));

  connect(ui->actionUpdate, 
          SIGNAL(triggered()),           
          this, 
          SLOT(update()));       
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmNodeConfig::~CFrmNodeConfig()
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
CFrmNodeConfig::done(int rv)
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
CFrmNodeConfig::menu_open_main_settings(void)
{
  // CDlgMainSettings* dlg = new CDlgMainSettings(this);
  // dlg->exec();
}

///////////////////////////////////////////////////////////////////////////////
// showRegisterContextMenu
//

void
CFrmNodeConfig::showRegisterContextMenu(const QPoint& pos)
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
CFrmNodeConfig::connectToRemoteHost(bool checked)
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
CFrmNodeConfig::doConnectToRemoteHost(void)
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
        spdlog::info(std::string(
          tr("Session: Successful connect to remote client.").toStdString()));
        ui->actionConnect->setChecked(true);
      }
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
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
        spdlog::info(
          std::string(tr("Session: Successful connected to the CANAL driver.")
                        .toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:
      QApplication::setOverrideCursor(Qt::WaitCursor);
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
        spdlog::info(std::string(
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
        spdlog::info(std::string(
          tr("Session: Unable to connect to remote host").toStdString()));
        QMessageBox::information(this,
                                 tr("vscpworks+"),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(
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
CFrmNodeConfig::doDisconnectFromRemoteHost(void)
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
CFrmNodeConfig::receiveRxRow(vscpEvent* pev)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// threadReceive
//

void
CFrmNodeConfig::threadReceive(vscpEvent* pev)
{
  emit dataReceived(pev);
}

///////////////////////////////////////////////////////////////////////////////
// selectGuid
//

void
CFrmNodeConfig::selectGuid(void)
{
  cguid guid(m_guidConfig->text().toStdString());
  CDlgKnownGuid* dlg = new CDlgKnownGuid(this);
  dlg->selectByGuid(guid.getAsString().c_str());
  dlg->setModal(true);
  dlg->exec();
  dlg->getSelectedGuid(guid);
  m_guidConfig->setText(guid.toString().c_str());
}

///////////////////////////////////////////////////////////////////////////////
// update
//

void
CFrmNodeConfig::update(void)
{
  if (m_connObject["bfull-l2"].toBool()) {

  }
  else {
    if (!m_nUpdates) {
      // Update all registers
      updateFull();
    }
    else {
      // update changed registers
      updateChanged();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateChanged
//

void
CFrmNodeConfig::updateChanged(void)
{

}

static void delay(uint16_t n)
{
  QTime dieTime= QTime::currentTime().addSecs(n);
  while (QTime::currentTime() < dieTime) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateFull
//

void
CFrmNodeConfig::updateFull(void)
{
  m_nUpdates = 0;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QProgressBar* pbar = new QProgressBar(this);
  ui->statusBar->addWidget(pbar);
  pbar->setMaximum(100);
       

  if (m_vscpConnType == CVscpClient::connType::NONE) {
    ui->statusBar->removeWidget(pbar); 
    QApplication::restoreOverrideCursor();
    return;    
  }

  // Must be connected to a remote host
  if (m_vscpConnType == CVscpClient::connType::NONE) {
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar); 
    return;
  }

  cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  // CAN4VSCP interface
  cguid guidInterface("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  // std::cout << "GUID " << guid.getAsString() << std::endl;

  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  ui->statusBar->showMessage(tr("Reading standard registers from device..."));
  CVscpClient& obj = *m_vscpClient;
  int rv = m_stdregs.init(obj, guidNode, guidInterface);
  if (VSCP_ERROR_SUCCESS != rv) {
    std::cout << "Failed to read standard regs: " << rv << std::endl;
    spdlog::error("Failed to init standard registers");
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  pbar->setValue(25);
  spdlog::trace("Standard register read");

  // Get GUID
  cguid guid;
  m_stdregs.getGUID(guid);
  spdlog::trace("Standard register getGUID = {}", guid.toString());

  std::string url = m_stdregs.getMDF();
  spdlog::trace("Standard register getMDF = {}", url);

  // create a temporary file name for remote MDF
  std::string tempMdfFileName;
  for (int i=0; i<url.length(); i++ ) {
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

  spdlog::debug("Temporary path: {}", tempPath);

  ui->statusBar->showMessage(tr("Downloading MDF file..."));

  CURLcode curl_rv;
  curl_rv = m_mdf.downLoadMDF(url, tempPath);
  if (CURLE_OK != curl_rv) {
    spdlog::error("Failed to download MDF {0} curl rv={1}", url, curl_rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  pbar->setValue(75);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  spdlog::trace("MDF Downloader", tempPath);

  ui->statusBar->showMessage(tr("Parsing MDF file..."));
  rv = m_mdf.parseMDF(tempPath);
  if (VSCP_ERROR_SUCCESS != rv) {
    spdlog::error("Failed to parse MDF {0} rv={1}", tempPath, rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  pbar->setValue(100);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  ui->statusBar->showMessage(tr("MDF read from device and parsed OK"));
  spdlog::trace("Parsing MDF OK");
  //delay(5);
  QApplication::restoreOverrideCursor();
  ui->statusBar->removeWidget(pbar);

  m_nUpdates++; // Another update - well the first

  // ==========================================================
  
  ui->treeWidgetRegisters->clear(); // Clear the tree
  m_mapRegTopPages.clear();         // Clear the page map

  // ----------------------------------------------------------
  // Fill status info
  // ----------------------------------------------------------

  // ----------------------------------------------------------
  // Fill register info
  // ----------------------------------------------------------

  // Att top level pages

  std::set<long> pages;
  uint32_t nPages = m_mdf.getPages(pages);
  spdlog::trace("MDF page count = {}", nPages);
  for (auto page : pages) {
    spdlog::trace("MDF page = {}", page);
    std::cout << "MDF page = " << page << std::endl;
    QTreeWidgetItem* itemTopReg1 = new QTreeWidgetItem(QTreeWidgetItem::UserType);
    itemTopReg1->setText(0, "Page " + QString::number(page));
    itemTopReg1->setFont(0, QFont("Arial", 12, QFont::Bold));
    itemTopReg1->setTextAlignment(REG_COL_POS, Qt::AlignLeft);
    itemTopReg1->setForeground(0, QBrush(QColor("royalblue")));
    ui->treeWidgetRegisters->addTopLevelItem(itemTopReg1);
    // Save a pointer to the register top item
    m_mapRegTopPages[page] = itemTopReg1;

    // Add registers
    //std::deque<CMDF_Register *> *pregs = m_mdf.getRegisterList();

    std::map<uint32_t, CMDF_Register *> mapRegs;
    m_mdf.getRegisterMap(page, mapRegs);

    for (auto reg : mapRegs) {
      CRegisterWidgetItem* itemReg = new CRegisterWidgetItem("trttttt");
      char buf[64];
      std::string str;
      sprintf(buf, "%X : %X", page, reg.second->getOffset());
      itemReg->setText(0, buf);
      itemReg->setTextAlignment(0, Qt::AlignCenter);
      itemReg->setText(1, "rw");
      itemReg->setTextAlignment(1, Qt::AlignCenter);
      itemReg->setText(2, "0x55");
      itemReg->setTextAlignment(2, Qt::AlignCenter);
      itemReg->setText(3, reg.second->getName().c_str());
      itemReg->setTextAlignment(0, Qt::AlignLeft);
      itemTopReg1->addChild(itemReg);
    }

  }

}