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
#include <QProgressBar>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------

CRegisterWidgetItem::CRegisterWidgetItem(const QString& text)
  : QTreeWidgetItem(QTreeWidgetItem::UserType + 1)
{
  m_regPage = 0;
  m_regOffset = 0;
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

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_nUpdates           = 0;         // No update operations yet
  m_StandardRegTopPage = nullptr;   // No standard registers
  ui->treeWidgetRegisters->clear(); // Clear the tree
  m_mapRegTopPages.clear();         // Clear the page map

  int cnt         = ui->session_tabWidget->count();
  QTabBar* tabBar = ui->session_tabWidget->tabBar();
  // tabBar->addTab("Skogig tab");

  QHeaderView* regTreeViewHeader = ui->treeWidgetRegisters->header();
  regTreeViewHeader->setDefaultAlignment(Qt::AlignCenter);
  // Enable context menu
  ui->treeWidgetRegisters->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_POS, 200);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_ACCESS, 80);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_POS, 160);

  ui->treeWidgetRegisters->clear();
  ui->treeWidgetRegisters->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Numerical base from settings
  m_baseComboBox = new QComboBox;
  m_baseComboBox->addItem("Hex");
  m_baseComboBox->addItem("Decimal");
  m_baseComboBox->addItem("Octal");
  m_baseComboBox->addItem("Binary");
  ui->mainToolBar->addWidget(m_baseComboBox);
  m_baseComboBox->setCurrentIndex(static_cast<int>(pworks->m_config_base));
  ui->mainToolBar->addSeparator();

  // MDF colors from settings
  ui->actionDisableColors->setChecked(pworks->m_config_bDisableColors);

  // QStringList headers;
  // for (int i = 0; i < ui->treeView->model()->columnCount(); i++) {
  //     headers.append(ui->treeView->model()->headerData(i,
  //     Qt::Horizontal).toString());
  // }
  // void QTreeWidgetItem::setForeground(int column, const QBrush &brush)

  // Add page top item
  // QTreeWidgetItem* itemTopReg1 = new
  // QTreeWidgetItem(QTreeWidgetItem::UserType); itemTopReg1->setText(0, "Page
  // 0"); itemTopReg1->setFont(0, QFont("Arial", 12, QFont::Bold));
  // itemTopReg1->setTextAlignment(REG_COL_POS, Qt::AlignLeft);
  // itemTopReg1->get  setStyleSheet("background-color: red");
  // itemTopReg1->setForeground(0, QBrush(QColor("#0000FF")));
  // itemTopReg1->setForeground(0, QBrush(QColor("royalblue")));
  // itemTopReg1->setBackground(0, QBrush(QColor(Qt::black)));
  // QList<QTableWidgetItem*>::iterator it;
  // for (it = sellist.begin(); it != sellist.end(); it++) {
  //     if (QBrush(Qt::cyan) == (*it)->background()) {
  //         (*it)->setBackground(Qt::white);
  //     }
  //     else {
  //         (*it)->setBackground(Qt::cyan);
  //     }
  // }
  // ui->treeWidgetRegisters->addTopLevelItem(itemTopReg1);

  // // Add register sub item
  // CRegisterWidgetItem* itemReg = new CRegisterWidgetItem("trttttt");
  // itemReg->setText(0, "0000:0000");
  // itemReg->setTextAlignment(0, Qt::AlignCenter);
  // itemReg->setText(1, "rw");
  // itemReg->setTextAlignment(1, Qt::AlignCenter);
  // itemReg->setText(2, "0x55");
  // itemReg->setTextAlignment(2, Qt::AlignCenter);
  // itemReg->setText(3, "This is a test");
  // itemReg->setTextAlignment(0, Qt::AlignLeft);
  // itemTopReg1->addChild(itemReg);

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

  // vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
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
        foreach (const QJsonValue& value, json_if_array) {
          m_comboInterface->addItem(value.toObject().value("if-item").toString());
        }

        m_comboInterface->setCurrentText(interface.c_str());
        std::cout << "interface = " << interface << std::endl;

        // Nickname
        m_nodeidConfig = new QSpinBox();
        m_nodeidConfig->setRange(1, 0xfd);
        ui->mainToolBar->addWidget(new QLabel(" node id:"));
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

  // Connect has been clicked
  connect(ui->actionConnect,
          SIGNAL(triggered(bool)),
          this,
          SLOT(connectToRemoteHost(bool)));

  // Update has been clicked
  connect(ui->actionUpdate, SIGNAL(triggered()), this, SLOT(update()));

  connect(ui->actionDisableColors,
          SIGNAL(triggered(bool)),
          this,
          SLOT(disableColors(bool)));

  // Base change
  connect(m_baseComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(onBaseChange(int)));

  // Interface change
  connect(m_comboInterface,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(onInterfaceChange(int)));

  // Node id change
  connect(m_nodeidConfig,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(onNodeIdChange(int)));

  // Register row has been double clicked.
  connect(ui->treeWidgetRegisters,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmNodeConfig::onRegisterTreeWidgetItemDoubleClicked);

  // Register item value has changed.
  connect(ui->treeWidgetRegisters,
          &QTreeWidget::itemChanged,
          this,
          &CFrmNodeConfig::onRegisterTreeWidgetCellChanged);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->treeWidgetRegisters,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeConfig::showRegisterContextMenu);
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

  menu->addAction(QString(tr("Update")), this, SLOT(update()));

  menu->addAction(QString(tr("Full Update")), this, SLOT(updateFull()));

  menu->addSeparator();

  menu->addAction(QString(tr("Read value(s) for selected row(s)")),
                  this,
                  SLOT(readSelectedRegisterValues()));

  menu->addAction(QString(tr("Write value(s) for selected row(s)")),
                  this,
                  SLOT(writeSelectedRegisterValues()));

  menu->addAction(QString(tr("Write default value(s) for selected row(s)")),
                  this,
                  SLOT(defaultSelectedRegisterValues()));

  menu->addAction(QString(tr("Set default values for ALL rows")),
                  this,
                  SLOT(defaultRegisterAll()));

  menu->addAction(QString(tr("Undo value(s) for selected row(s)")),
                  this,
                  SLOT(undoSelectedRegisterValues()));

  menu->addAction(QString(tr("Redo value(s) for selected row(s)")),
                  this,
                  SLOT(redoSelectedRegisterValues()));

  menu->addSeparator();

  menu->addAction(
    QString(tr("Save register value(s) for selected row(s) to disk")),
    this,
    SLOT(saveSelectedRegisterValues()));

  menu->addAction(QString(tr("Save ALL register values to disk")),
                  this,
                  SLOT(saveAllRegisterValues()));

  menu->addAction(QString(tr("Load register values from disk")),
                  this,
                  SLOT(loadRegisterValues()));

  menu->addSeparator();

  menu->addAction(QString(tr("Goto register page...")),
                  this,
                  SLOT(gotoRegisterPageMenu()));

  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
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
// disableColors
//

void
CFrmNodeConfig::disableColors(bool bColors)
{
  vscpworks* pworks               = (vscpworks*)QCoreApplication::instance();
  pworks->m_config_bDisableColors = bColors;

  // Disable/Enable colors for standard registers
  if (bColors) {
    for (int i = 0; i < m_StandardRegTopPage->childCount(); i++) {
      CRegisterWidgetItem *child = (CRegisterWidgetItem *)m_StandardRegTopPage->child(i);
      for (int j = 0; j < 4; j++) {
        if ( child->type() == TREE_LIST_REGISTER_TYPE) {
          //child->setForeground(j, child->parent()->foreground(j));
          child->setBackground(j, child->parent()->background(j));  
        }
      }
    }
  }
  else {
    for (int i = 0; i < m_StandardRegTopPage->childCount(); i++){
      CRegisterWidgetItem *child = (CRegisterWidgetItem *)m_StandardRegTopPage->child(i);
      for (int j = 0; j < 4; j++) { 
        if ( child->type() == TREE_LIST_REGISTER_TYPE) {
          //child->setForeground(j, QBrush(QColor("black")));
          child->setBackground(j, QBrush(QColor(m_stdregs.m_vscp_standard_registers_defs[i].bgcolor)));  
        }
      }
    }
  }
  
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void
CFrmNodeConfig::onBaseChange(int index)
{
  int base = 10;
  QString qstr;
  QString prefix;
  numerical_base numbase = static_cast<numerical_base>(index);

  QApplication::setOverrideCursor(Qt::WaitCursor);

  switch (numbase) {
    case numerical_base::HEX:
      prefix = "0x";
      base   = 16;
      break;
    case numerical_base::DECIMAL:
    default:
      prefix = "";
      base   = 10;
      break;
    case numerical_base::OCTAL:
      prefix = "0o";
      base   = 8;
      break;
    case numerical_base::BINARY:
      prefix = "0b";
      base   = 2;
      break;
  }

  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// onNodeIdChange
//

void
CFrmNodeConfig::onInterfaceChange(int index)
{
  ui->treeWidgetRegisters->clear();
  m_nUpdates = 0;
}

///////////////////////////////////////////////////////////////////////////////
// onNodeIdChange
//

void
CFrmNodeConfig::onNodeIdChange(int nodeid)
{
  ui->treeWidgetRegisters->clear();
  m_nUpdates = 0;
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
      // updateChanged();
      updateFull();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRegisterTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onRegisterTreeWidgetItemDoubleClicked(QTreeWidgetItem* item,
                                                      int column)
{
  if (2 == column) {
    ui->treeWidgetRegisters->editItem(item, column);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRegisterTreeWidgetCellChanged
//

void
CFrmNodeConfig::onRegisterTreeWidgetCellChanged(QTreeWidgetItem* item,
                                                int column)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString strValue  = item->text(column);
  uint32_t value    = vscp_readStringValue(strValue.toStdString());

  strValue =
    pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex());

  // switch(static_cast<numerical_base>(m_baseComboBox->currentIndex()) ) {
  //   case numerical_base::HEX:
  //     strvalue = vscpworks::decimalToStringInBase(value,
  //     m_baseComboBox->currentIndex()); break;
  //   case numerical_base::DECIMAL:
  //     value = vscp_readStringValue(item->text(column).toStdString());
  //     break;
  //   case numerical_base::OCTAL:
  //     value = vscp_readStringValue(item->text(column).toStdString());
  //     break;
  //   case numerical_base::BINARY:
  //     value = vscp_readStringValue(item->text(column).toStdString());
  //     break;
  //   default:
  //     break;
  // }
  item->setForeground(column, QBrush(Qt::red));
  item->setText(column, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// renderStandardRegisters
//

void
CFrmNodeConfig::renderStandardRegisters(void)
{
  int rv;
  std::string str;
  uint8_t value;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QTreeWidgetItem* itemTopStdReg = new QTreeWidgetItem(QTreeWidgetItem::UserType);
  itemTopStdReg->setText(REG_COL_POS, "Standard registers");
  itemTopStdReg->setFont(REG_COL_POS, QFont("Arial", 12, QFont::Bold));
  itemTopStdReg->setTextAlignment(REG_COL_POS, Qt::AlignLeft);
  itemTopStdReg->setForeground(0, QBrush(QColor("royalblue")));
  ui->treeWidgetRegisters->addTopLevelItem(itemTopStdReg);
  // Save a pointer to the standard register top item
  m_StandardRegTopPage = itemTopStdReg;

  CRegisterWidgetItem* itemReg;

  for (int i = 0; 
        i < sizeof(m_stdregs.m_vscp_standard_registers_defs) /
                        sizeof(__struct_standard_register_defs);
       i++) {

    // Register
    itemReg = new CRegisterWidgetItem("Register");
    if (nullptr == itemReg) {
      spdlog::critical("Failed to create standard register widget item");
      return;
    }

    // Save register pos for later reference
    itemReg->m_regPage = 0;
    itemReg->m_regOffset = m_stdregs.m_vscp_standard_registers_defs[i].reg;

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

    // Set foreground and background colors from MDF
    for (int j = 0; j < 4; j++) {
      itemReg->setForeground(j, QBrush(QColor("black")));
      itemReg->setBackground(j, QBrush(QColor(m_stdregs.m_vscp_standard_registers_defs[i].bgcolor)));
    }

    // Register pospos
    uint8_t reg = m_stdregs.m_vscp_standard_registers_defs[i].reg;
    str = "0 : " + QString::number(reg, 10).toStdString();
    str += " / 0x" + QString::number(reg, 16).toStdString();
    itemReg->setText(REG_COL_POS, str.c_str());
    itemReg->setTextAlignment(REG_COL_POS, Qt::AlignCenter);

    // Access
    if (0 == m_stdregs.m_vscp_standard_registers_defs[i].access) {
      itemReg->setText(REG_COL_ACCESS, "r");
    }
    else if (1 == m_stdregs.m_vscp_standard_registers_defs[i].access) {
      itemReg->setText(REG_COL_ACCESS, "rw");
      itemFlags |= Qt::ItemIsEditable;
    }
    else if (2 == m_stdregs.m_vscp_standard_registers_defs[i].access) {
      itemReg->setText(REG_COL_ACCESS, "w");
      itemFlags |= Qt::ItemIsEditable;
    }
    else {
      itemReg->setText(REG_COL_ACCESS, "?");
    }
    itemReg->setTextAlignment(REG_COL_ACCESS, Qt::AlignCenter);

    // Value
    value = m_stdregs.getReg(m_stdregs.m_vscp_standard_registers_defs[i].reg);
    std::cout << "Value: " << QString::number(value, 10).toStdString() << std::endl;
    itemReg->setText(
      REG_COL_VALUE,
      pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
        .toStdString()
        .c_str());
    itemReg->setTextAlignment(REG_COL_VALUE, Qt::AlignCenter);

    str = m_stdregs.m_vscp_standard_registers_defs[i].name;
    // str +=
    // QString::number(sizeof(m_stdregs.m_vscp_standard_registers_defs)/sizeof(__struct_standard_register_defs)).toStdString();
    itemReg->setText(REG_COL_NAME, str.c_str());
    itemReg->setTextAlignment(REG_COL_NAME, Qt::AlignLeft);

    itemReg->setFlags(itemFlags);

    itemTopStdReg->addChild(itemReg);
  }
}

///////////////////////////////////////////////////////////////////////////////
// renderRegisters
//

void
CFrmNodeConfig::renderRegisters(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  ui->treeWidgetRegisters->clear(); // Clear the tree
  m_mapRegTopPages.clear();         // Clear the page map

  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;

  // node id
  guidNode.setLSB(m_nodeidConfig->value());

  // ----------------------------------------------------------
  // Fill status info
  // ----------------------------------------------------------

  renderStandardRegisters();

  // ----------------------------------------------------------
  // Fill register info
  // ----------------------------------------------------------

  // Att top level pages

  std::set<uint16_t> pages;
  uint32_t nPages = m_mdf.getPages(pages);
  spdlog::trace("MDF page count = {}", nPages);

  // Get user registers for all pages
  rv = m_userregs.init(*m_vscpClient, guidNode, guidInterface, pages);
  if (VSCP_ERROR_SUCCESS != rv) {
    std::cout << "Failed to read user regs: " << rv << std::endl;
    spdlog::error("Failed to init user registers");
    return;
  }

  for (auto page : pages) {

    spdlog::trace("MDF page = {}", page);
    //std::cout << "MDF page = " << page << std::endl;

    // CRegisterPage *preg = m_userregs.getPage(page);

    QTreeWidgetItem* itemTopReg1 = new QTreeWidgetItem(QTreeWidgetItem::UserType);
    itemTopReg1->setText(REG_COL_POS, "Page " + QString::number(page));
    itemTopReg1->setFont(REG_COL_POS, QFont("Arial", 12, QFont::Bold));
    itemTopReg1->setTextAlignment(REG_COL_POS, Qt::AlignLeft);
    itemTopReg1->setForeground(0, QBrush(QColor("royalblue")));
    ui->treeWidgetRegisters->addTopLevelItem(itemTopReg1);
    // Save a pointer to the register top item
    m_mapRegTopPages[page] = itemTopReg1;

    // Add registers
    // std::deque<CMDF_Register *> *pregs = m_mdf.getRegisterList();

    std::map<uint32_t, CMDF_Register*> mapRegs;
    m_mdf.getRegisterMap(page, mapRegs);

    for (auto item : mapRegs) {

      CMDF_Register* pregmdf = item.second;

      CRegisterWidgetItem* itemReg = new CRegisterWidgetItem("Register");
      if (nullptr == itemReg) {
        spdlog::critical("Failed to create register widget item");
        continue;
      }

      Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

      // Set foreground and background colors from MDF
      if (!pworks->m_config_bDisableColors) {
        for (int i = 0; i < 4; i++) {
          itemReg->setForeground(i, QBrush(QColor(pregmdf->getForegroundColor())));
          itemReg->setBackground(i, QBrush(QColor(pregmdf->getBackgroundColor())));
        }
      }

      // Save reister info so we can find info later
      itemReg->m_regPage = page;
      itemReg->m_regOffset = pregmdf->getOffset();

      // Save a pointer to register information
      //itemReg->m_pmdfreg = pregmdf;

      // page : offset in selected base (not binary)
      char buf[64];
      std::string str;
      sprintf(buf, "%u : %lu", page, (unsigned long)pregmdf->getOffset());
      itemReg->setText(REG_COL_POS, buf);
      itemReg->setTextAlignment(REG_COL_POS, Qt::AlignCenter);

      mdf_access_mode access = pregmdf->getAccess();
      if (MDF_REG_ACCESS_READ_ONLY == access) {
        // itemReg->setForeground(2, QBrush(QColor("gray")));
        itemReg->setText(REG_COL_ACCESS, "r");
      }
      else if (MDF_REG_ACCESS_WRITE_ONLY == access) {
        // itemReg->setForeground(2, QBrush(QColor("darkgreen")));
        itemReg->setText(REG_COL_ACCESS, "w");
        itemFlags |= Qt::ItemIsEditable;
      }
      else if (MDF_REG_ACCESS_READ_WRITE == access) {
        // itemReg->setForeground(2, QBrush(QColor("red")));
        itemReg->setText(REG_COL_ACCESS, "rw");
        itemFlags |= Qt::ItemIsEditable;
      }
      else {
        // itemReg->setForeground(2, QBrush(QColor("black")));
        itemReg->setText(REG_COL_ACCESS, "---");
      }
      itemReg->setTextAlignment(REG_COL_ACCESS, Qt::AlignCenter);

      // Value
      int value = m_userregs.getReg(page, pregmdf->getOffset());
      if (-1 == value) {
        itemReg->setText(REG_COL_VALUE, "---");
      }
      else {
        itemReg->setText(
          REG_COL_VALUE,
          pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
            .toStdString()
            .c_str());
      }
      itemReg->setTextAlignment(REG_COL_VALUE, Qt::AlignCenter);

      itemReg->setFlags(itemFlags);

      itemReg->setText(REG_COL_NAME, pregmdf->getName().c_str());
      itemReg->setTextAlignment(REG_COL_NAME, Qt::AlignLeft);
      itemTopReg1->addChild(itemReg);
    }
  }

  m_nUpdates++; // Another update
}

///////////////////////////////////////////////////////////////////////////////
// updateChanged
//

void
CFrmNodeConfig::updateChanged(void)
{
}

static void
delay(uint16_t n)
{
  QTime dieTime = QTime::currentTime().addSecs(n);
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
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Show a progress bar
  QProgressBar* pbar = new QProgressBar(this);
  // ui->statusBar->addWidget(pbar);
  pbar->setMaximum(100);

  // Must be connected to a remote host
  if (m_vscpConnType == CVscpClient::connType::NONE) {
    ui->statusBar->removeWidget(pbar);
    QApplication::restoreOverrideCursor();
    return;
  }

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  // node id
  guidNode.setLSB(m_nodeidConfig->value());
  // cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  // * * * Load standard registers * * *

  ui->statusBar->showMessage(tr("Reading standard registers from device..."));

  int rv = m_stdregs.init(*m_vscpClient, guidNode, guidInterface);
  if (VSCP_ERROR_SUCCESS != rv) {
    ui->statusBar->showMessage(
      tr("Failed to read standard registers from device."));
    spdlog::error("Failed to init standard registers {0}", rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  pbar->setValue(25);
  spdlog::trace("Standard register read");

  // * * * Download MDF * * *

  // Get GUID
  cguid guid;
  m_stdregs.getGUID(guid);
  spdlog::trace("Standard register getGUID = {}", guid.toString());

  std::string url = m_stdregs.getMDF();
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
  std::string tempPath =
    QStandardPaths::writableLocation(QStandardPaths::TempLocation)
      .toStdString();
  tempPath += "/";
  tempPath += tempMdfFileName;

  spdlog::debug("Temporary path: {}", tempPath);

  ui->statusBar->showMessage(tr("Downloading MDF file..."));

  CURLcode curl_rv;
  curl_rv = m_mdf.downLoadMDF(url, tempPath);
  if (CURLE_OK != curl_rv) {
    ui->statusBar->showMessage(tr("Failed to download MDF file for device."));
    spdlog::error("Failed to download MDF {0} curl rv={1}", url, curl_rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  pbar->setValue(75);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  spdlog::trace("MDF Downloader", tempPath);

  // * * * Parse  MDF * * *

  ui->statusBar->showMessage(tr("Parsing MDF file..."));
  rv = m_mdf.parseMDF(tempPath);
  if (VSCP_ERROR_SUCCESS != rv) {
    ui->statusBar->showMessage(tr("Failed to parse MDF file for device."));
    spdlog::error("Failed to parse MDF {0} rv={1}", tempPath, rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return;
  }

  pbar->setValue(80);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  ui->statusBar->showMessage(tr("MDF read from device and parsed OK"));
  spdlog::trace("Parsing MDF OK");

  // Fill register data
  renderRegisters();

  fillDeviceHtmlInfo();

  pbar->setValue(100);

  QApplication::restoreOverrideCursor();
  ui->statusBar->removeWidget(pbar);
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::readSelectedRegisterValues(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  // node id
  guidNode.setLSB(m_nodeidConfig->value());

  QList<QTreeWidgetItem *>listSelected = ui->treeWidgetRegisters->selectedItems();
  for (auto item: listSelected) {
    if (item->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem *)item;
      uint8_t value = vscp_readStringValue(item->text(REG_COL_VALUE).toStdString()); 
      if (VSCP_ERROR_SUCCESS == vscp_readLevel1Register(*m_vscpClient,
                                                          guidNode,
                                                          guidInterface,
                                                          itemReg->m_regPage,
                                                          itemReg->m_regOffset,
                                                          value )) {                                                           
        item->setText( REG_COL_VALUE,
                          pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
                          .toStdString().c_str()); 
        for (int i=0; i<4; i++) {
          item->setForeground(i, QBrush(Qt::black));
        }                            
      }
      else {
        spdlog::error("Failed to read register");
      }
    }  
  }
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::writeSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::defaultSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::defaultRegisterAll(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::undoSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::redoSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::gotoRegisterPageMenu(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::saveSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::saveAllRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// loadRegisterValues
//

void
CFrmNodeConfig::loadRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// fillDeviceHtmlInfo
//

void
CFrmNodeConfig::fillDeviceHtmlInfo(void)
{
  int idx;
  std::string html;
  std::string str;

  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body style=\"font-family:'Ubuntu'; font-size:11pt; "
          "font-weight:400; font-style:normal;\">";
  html += "<h1>";
  html += m_mdf.getModuleName();
  html += "</h1>";
  //html += "<font color=\"#009900\">";

  html += "<b>Node id</b>:<font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getNickname()).toStdString();
  html += " </font> addressed node id is:<font color=\"#009900\"> ";
  html += m_nodeidConfig->text().toStdString();
  html += "<br>";

  html += "</font><b>Interface</b>:<font color=\"#009900\"> ";
  html += m_comboInterface->currentText().toStdString();
  html += "<br>";

  html += "</font><b>Nickname</b>:<font color=\"#009900\"> ";
  html += m_nodeidConfig->text().toStdString();
  html += "<br>";

  // Device GUID
  html += "</font><b>Device GUID</b>:<font color=\"#009900\"> ";
  cguid guid;
  m_stdregs.getGUID(guid);
  html += guid.toString();
  html += " (real GUID)<br>";

  // Proxy GUID
  str = m_comboInterface->currentText().toStdString();
  cguid guidNode;
  guidNode.getFromString(str);
  // node id
  guidNode.setLSB(m_nodeidConfig->value());
  html += "</font><b>Proxy GUID</b>:<font color=\"#009900\"> ";
  html += guidNode.toString();
  html += "<br>";

  html += "</font><b>MDF URL</b>:<font color=\"#009900\"> ";
  html += "<a href=\"";
  html += m_stdregs.getMDF();
  html += "\" target=\"ext\">";
  html += m_stdregs.getMDF();;
  html += "</a>";
  html += "<br>";

  // Alarm
  html += "</font><b>Alarm:</b><font color=\"#009900\"> ";
  if (m_stdregs.getAlarm()) {
    html += "Yes";
  }
  else {
    html += "No";
  }
  html += "<br>";

  html += "</font><b>Device error counter:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getErrorCounter()).toStdString();
  html += "<br>";

  html += "</font><b>Firmware VSCP conformance:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getConformanceVersionMajor()).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getConformanceVersionMinor()).toStdString();
  html += "<br>";

  html += "</font><b>User Device ID:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 1)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 2)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 3)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 3)).toStdString();
  html += "<br>";

  html += "</font><b>Manufacturer Device ID:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getManufacturerDeviceID(),16).toStdString();
  html += " - ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 1)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 2)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 3)).toStdString();
  html += "<br>";

  html += "</font><b>Manufacturer sub device ID:</b><font color=\"#009900\"> ";
  html += "0x";
  html += QString::number(m_stdregs.getManufacturerSubDeviceID(),16).toStdString();
  html += " - ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 1)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 2)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 3)).toStdString();
  html += "<br>";

  html += "</font><b>Page select:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getRegisterPage()).toStdString();
  html += " MSB=";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_PAGE_SELECT_MSB)).toStdString();
  html += " LSB= ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_PAGE_SELECT_LSB)).toStdString();
  html += "<br>";

  html += "</font><b>Firmware version:</b><font color=\"#009900\"> ";
  html += m_stdregs.getFirmwareVersionString();
  html += "<br>";

  html += "</font><b>Boot loader algorithm:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getBootloaderAlgorithm()).toStdString();
  html += " - ";
  switch (m_stdregs.getBootloaderAlgorithm()) {

    case 0x00:
      html += "VSCP universal algorithm";
      break;

    case 0x01:
      html += "Microchip PIC algorithm 0";
      break;

    case 0x10:
      html += "Atmel AVR algorithm 0";
      break;

    case 0x20:
      html += "NXP ARM algorithm 0";
      break;

    case 0x30:
      html += "ST ARM algorithm 0";
      break;

    case 0xFF:
      html += "No bootloader implemented.";
      break;

    default:
      html += "Unknown algorithm.";
      break;
  }

  html += "<br>";

  html += "</font><b>Buffer size:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getBufferSize()).toStdString();
  html += " bytes.";
  if (!m_stdregs.getBufferSize()) {
    html += " ( == default size (8 or 487 bytes) )";
  }
  html += "<br>";
   
  CMDF_DecisionMatrix *pdm = m_mdf.getDM();
  if ((nullptr == pdm) || !pdm->getRowCount() ) {
    html += "No Decision Matrix is available on this device.";
    html += "<br>";
  }
  else {
    html += "</font><b>Decision Matrix:</b><font color=\"#009900\"> Rows=";
    html += QString::number(pdm->getRowCount()).toStdString();
    html += " Startoffset=";
    html += QString::number(pdm->getStartOffset()).toStdString();
    html += " (0x";
    html += QString::number(pdm->getStartOffset(),16).toStdString();
    html += ") Startpage=";
    html += QString::number(pdm->getStartPage()).toStdString();
    html += " (0x";
    html += QString::number(pdm->getStartPage(),16).toStdString();
    html += ") Row size=";
    html += QString::number(pdm->getRowSize()).toStdString();
    html += ")<br>";
  }

  html += "</font><br>";


  // MDF Info
  html += "<b>MDF Information</b>";

  // Manufacturer data
  html += "<b>Module name :</b><font color=\"#009900\"> ";
  html += m_mdf.getModuleName();
  html += "<br>";

  html += "</font><b>Module model:</b><font color=\"#009900\"> ";
  html += m_mdf.getModuleModel();
  html += "<br>";

  html += "</font><b>Module version:</b><font color=\"#009900\"> ";
  html += m_mdf.getModuleVersion();
  html += "<br>";

  html += "</font><b>Module last change:</b><font color=\"#009900\"> ";
  html += m_mdf.getModuleChangeDate();
  html += "<br>";

  html += "</font><b>Module description:</b><font color=\"#009900\"> ";
  html += m_mdf.getModuleDescription();
  html += "<br>";

  html += "</font><b>Module URL</b><font color=\"#009900\"> : ";
  html += "<a href=\"";
  html += m_mdf.getModuleHelpUrl();
  html += "\">";
  html += m_mdf.getModuleHelpUrl();
  html += "</a>";
  html += "<br><br>";



  html += "</font><b>Manufacturer:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerName();
  //html += "<br>";
  html += "<br>";
  html += "</font><b>Street:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerStreetAddress();
  html += "<br>";
  html += "</font><b>Town:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerTownAddress();
  html += "<br>";
  html += "</font><b>City:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerCityAddress();
  html += "<br>";
  html += "</font><b>Post Code:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerPostCodeAddress();
  html += "<br>";
  html += "</font><b>State:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerStateAddress();
  html += "<br>";
  html += "</font><b>Region:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerRegionAddress();
  html += "<br>";
  html += "</font><b>Country:</b><font color=\"#009900\"> ";
  html += m_mdf.getManufacturerCountryAddress();
  html += "<br><br>";

  idx = 0;
  CMDF_Item *phone;
  while(nullptr != (phone = m_mdf.getManufacturer()->getPhoneObj(idx))) {
    html += "</font><b>Phone:</b><font color=\"#009900\"> ";
    html += phone->getName();
    html += "</font> - ";
    html += phone->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item *fax;
  while(nullptr != (fax = m_mdf.getManufacturer()->getFaxObj(idx))) {
    html += "</font><b>Fax:</b><font color=\"#009900\"> ";
    html += fax->getName();
    html += "</font> - ";
    html += fax->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item *email;
  while(nullptr != (email = m_mdf.getManufacturer()->getEmailObj(idx))) {
    html += "</font><b>Email:</b><font color=\"#009900\"> ";
    html += email->getName();
    html += "</font> - ";
    html += email->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item *web;
  while(nullptr != (web = m_mdf.getManufacturer()->getWebObj(idx))) {
    html += "</font><b>Web:</b><font color=\"#009900\"> ";
    html += web->getName();
    html += "</font> - ";
    html += web->getDescription();
    html += "<br>";
    idx++;
  }
     
  html += "</font>";
  html += "</body></html>";

  // Set the HTML
  ui->infoArea->setHtml(html.c_str());
}