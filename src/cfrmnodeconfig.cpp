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

#include "cdlgeditdm.h"
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
#include <QTreeWidgetItemIterator>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------

CRegisterWidgetItem::CRegisterWidgetItem(const QString& text)
  : QTreeWidgetItem(TREE_LIST_REGISTER_TYPE)
{
  m_regPage   = 0;
  m_regOffset = 0;
}

CRegisterWidgetItem::~CRegisterWidgetItem()
{
  ;
}

// ----------------------------------------------------------------------------

CRemoteVariableWidgetItem::CRemoteVariableWidgetItem(const QString& text)
  : QTreeWidgetItem(TREE_LIST_REMOTEVAR_TYPE)
{
  m_pRemoteVariable = nullptr;
}

CRemoteVariableWidgetItem::~CRemoteVariableWidgetItem()
{
  ;
}

// ----------------------------------------------------------------------------

CDMWidgetItem::CDMWidgetItem(const QString& text)
  : QTreeWidgetItem(TREE_LIST_DM_TYPE)
{
  m_pDM = nullptr;
}

CDMWidgetItem::~CDMWidgetItem()
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
  m_bInternalChange = false;        // Cell update works as normal
  m_bMainInfo = false;              // No main MDF info written yet to the info area

  int cnt         = ui->session_tabWidget->count();
  QTabBar* tabBar = ui->session_tabWidget->tabBar();

  // Select register tab
  tabBar->setCurrentIndex(0);

  // Setup register tab
  QHeaderView* treeViewHeaderRegisters = ui->treeWidgetRegisters->header();
  ui->treeWidgetRegisters->clear();
  ui->treeWidgetRegisters->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeWidgetRegisters->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_POS, 200);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_ACCESS, 80);
  ui->treeWidgetRegisters->setColumnWidth(REG_COL_POS, 160);

  // Setup remote variable tab
  QHeaderView* treeViewHeaderRemoteVariables = ui->treeWidgetRemoteVariables->header();
  ui->treeWidgetRemoteVariables->clear();
  ui->treeWidgetRemoteVariables->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeWidgetRemoteVariables->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->treeWidgetRemoteVariables->setColumnWidth(REMOTEVAR_COL_VALUE, 200);
  ui->treeWidgetRemoteVariables->setColumnWidth(REMOTEVAR_COL_ACCESS, 80);
  ui->treeWidgetRemoteVariables->setColumnWidth(REMOTEVAR_COL_TYPE, 170);

  // Setup decision matrix tab
  QHeaderView* treeViewHeaderDecisionMatrix = ui->treeWidgetDecisionMatrix->header();
  treeViewHeaderDecisionMatrix->setDefaultAlignment(Qt::AlignCenter);
  ui->treeWidgetDecisionMatrix->clear();
  ui->treeWidgetDecisionMatrix->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeWidgetDecisionMatrix->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_ORIGIN, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_FLAGS, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_CLASS_MASK, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_CLASS_FILTER, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_TYPE_MASK, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_TYPE_FILTER, 120);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_ACTION, 200);
  ui->treeWidgetDecisionMatrix->setColumnWidth(DM_LEVEL1_COL_PARAMETER, 200);

  QHeaderView* treeViewHeaderMdfFiles = ui->treeWidgetMdfFiles->header();
  ui->treeWidgetMdfFiles->clear();
  ui->treeWidgetMdfFiles->setContextMenuPolicy(Qt::CustomContextMenu);

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

  m_nodeidConfig   = nullptr;
  m_guidConfig     = nullptr;
  m_comboInterface = nullptr;

  // No connection set yet
  m_vscpConnType = CVscpClient::connType::NONE;
  m_vscpClient   = NULL;

  spdlog::debug(std::string(tr("Node configuration module opended").toStdString()));

  if (nullptr == pconn) {
    QApplication::beep();
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
    QApplication::beep();
    spdlog::error(std::string(tr("Type is not define in JSON data").toStdString()));
    QMessageBox::information(this,
                             tr("vscpworks+"),
                             tr("Can't open node configuration  window - The "
                                "connection type is unknown"),
                             QMessageBox::Ok);
    return;
  }

  m_vscpConnType = static_cast<CVscpClient::connType>(m_connObject["type"].toInt());

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

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::LOCAL:
      // GUID
      break;

    case CVscpClient::connType::TCPIP: 
      // {
      //   std::string interface = m_connObject["selected-interface"].toString().toStdString();
      // }

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
          m_comboInterface->addItem(
            value.toObject().value("if-item").toString());
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
        QMessageBox::warning(this,
                              tr(APPNAME),
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
                              tr(APPNAME),
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

  // CONNECTIONS
  // Menu and toolbar commands

  // Connect has been clicked
  connect(ui->actionConnect,
          SIGNAL(triggered(bool)),
          this,
          SLOT(connectToRemoteHost(bool)));

  // Update has been clicked
  connect(ui->actionUpdate, SIGNAL(triggered()), this, SLOT(update()));

  // Full update has been clicked
  connect(ui->actionUpdateFull, SIGNAL(triggered()), this, SLOT(updateFull()));

  // Full update has been clicked
  connect(ui->actionUpdateLocal,
          SIGNAL(triggered()),
          this,
          SLOT(updateLocal()));

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

  // Tab bar clicked
  connect(ui->session_tabWidget,
            SIGNAL(tabBarClicked(int)),
            this,
            SLOT(onMainTabBarClicked(int)));        

  // Tab bar changed
  connect(ui->session_tabWidget,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(onMainTabBarChanged(int)));

  // Register row has been double clicked.
  connect(ui->treeWidgetRegisters,
            &QTreeWidget::itemClicked,
            this,
            &CFrmNodeConfig::onRegisterTreeWidgetItemClicked);

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

  // Open pop up menu on right click on register list
  connect(ui->treeWidgetRegisters,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeConfig::showRegisterContextMenu);

  // Remote variable item value has changed.
  connect(ui->treeWidgetRemoteVariables,
          &QTreeWidget::itemChanged,
          this,
          &CFrmNodeConfig::onRemoteVarTreeWidgetCellChanged);

  // Open pop up menu on right click on remote variable list
  connect(ui->treeWidgetRemoteVariables,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeConfig::showRemoteVariableContextMenu);

  // DM item value has changed.
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::itemChanged,
          this,
          &CFrmNodeConfig::onDMTreeWidgetCellChanged);          

  // Open pop up menu on right click on DM list
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeConfig::showDMContextMenu);

  // Open pop up menu on right click on files list
  connect(ui->treeWidgetMdfFiles,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmNodeConfig::showFilesContextMenu);

  // Remote variables

  // Register row has been double clicked.
  connect(ui->treeWidgetRemoteVariables,
          &QTreeWidget::itemClicked,
          this,
          &CFrmNodeConfig::onRemoteVariableTreeWidgetItemClicked);

  // Register row has been double clicked.
  connect(ui->treeWidgetRemoteVariables,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmNodeConfig::onRemoteVariableTreeWidgetItemDoubleClicked);

  // Decision matrix

  // DM row has been double clicked.
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::itemClicked,
          this,
          &CFrmNodeConfig::onDMTreeWidgetItemClicked);

  // DM row has been double clicked.
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmNodeConfig::onDMTreeWidgetItemDoubleClicked);

  // m_shortcut_info = new QShortcut(QKeySequence(tr("Ctrl+I")), this);
  // connect(m_shortcut_info,
  //           &QShortcut::activated,
  //           this,
  //           &CFrmNodeConfig::fillDeviceHtmlInfo);
  connect(ui->actionShowMdfInfo,
          &QAction::triggered,
          this,
          &CFrmNodeConfig::fillDeviceHtmlInfo);
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

  menu->addAction(QString(tr("Full update")), this, SLOT(updateFull()));

  menu->addAction(QString(tr("Full update with local MDF")),
                  this,
                  SLOT(updateLocal()));

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

  menu->addAction(QString(tr("Save register value(s) for selected row(s) to disk")),
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
// showRemoteVariableContextMenu
//

void
CFrmNodeConfig::showRemoteVariableContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

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

  menu->addAction(QString(tr("Full update with local MDF")),
                  this,
                  SLOT(updateLocal()));
  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// showDMContextMenu
//

void
CFrmNodeConfig::showDMContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Edit row")), this, SLOT(editDMRow()));

  menu->addSeparator();

  menu->addAction(QString(tr("Enable/Disable selected row(s)")),
                  this,
                  SLOT(update()));

  menu->addAction(QString(tr("Read selected DM row(s)")), this, SLOT(update()));

  menu->addAction(QString(tr("Write selected DM row(s)")),
                  this,
                  SLOT(updateFull()));

  menu->addAction(QString(tr("Set action for selected row(s)")),
                  this,
                  SLOT(updateLocal()));

  menu->addAction(QString(tr("Set action parameter for selected row(s)")),
                  this,
                  SLOT(updateLocal()));

  menu->addSeparator();

  menu->addAction(QString(tr("Undo value(s) for selected row(s)")),
                  this,
                  SLOT(undoSelectedRegisterValues()));
  menu->addAction(QString(tr("Redo value(s) for selected row(s)")),
                  this,
                  SLOT(redoSelectedRegisterValues()));
  menu->addSeparator();
  menu->addAction(QString(tr("Save DM value(s) for selected row(s) to disk")),
                  this,
                  SLOT(saveSelectedRegisterValues()));

  menu->addAction(QString(tr("Save ALL DM values to disk")),
                  this,
                  SLOT(saveAllRegisterValues()));

  menu->addAction(QString(tr("Load DM values from disk")),
                  this,
                  SLOT(loadRegisterValues()));

  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// showFilesContextMenu
//

void
CFrmNodeConfig::showFilesContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Update")), this, SLOT(update()));

  menu->addAction(QString(tr("Full ipdate")), this, SLOT(updateFull()));

  menu->addAction(QString(tr("Full update with local MDF")),
                  this,
                  SLOT(updateLocal()));
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
      QApplication::setOverrideCursor(Qt::WaitCursor);
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        QApplication::beep();
        spdlog::error(std::string(tr("Session: Unable to connect to remote host.").toStdString()));
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
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QApplication::beep();
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
        QApplication::beep();
        QString str = tr("Session: Unable to connect to the SOCKETCAN driver. rv=");
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
        QApplication::beep();
        spdlog::error(std::string(tr("Session: Unable to disconnect tcp/ip remote client").toStdString()));
        QMessageBox::information(this,
                                  tr("vscpworks+"),
                                  tr("Failed to disconnect the connection to the txp/ip remote host"),
          QMessageBox::Ok);
      }
      else {
        spdlog::trace(std::string(tr("Session: Successful disconnect from tcp/ip remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:

      // Remove callback

      QApplication::setOverrideCursor(Qt::WaitCursor);

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to disconnect from the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                  tr("vscpworks+"),
                                  tr("Failed to disconnect the connection to the CANAL driver"),
                                  QMessageBox::Ok);
      }
      else {
        spdlog::trace(std::string(tr("Session: Successful disconnect from CANAL driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:
      QApplication::setOverrideCursor(Qt::WaitCursor);

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to disconnect from the SOCKETCAN driver. rv=");
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
        spdlog::trace(std::string(tr("Session: Successful disconnect from SOCKETCAN driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
        QApplication::beep();
        spdlog::error(std::string(tr("Session: Unable to disconnect from MQTT remote client").toStdString()));
        QMessageBox::information(this,
                                  tr("vscpworks+"),
                                  tr("Failed to disconnect the connection to the MQTT remote "
                                    "host"),
                                  QMessageBox::Ok);
                              }
      else {
        spdlog::trace(std::string(tr("Session: Successful disconnect from the MQTT remote host").toStdString()));
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
      CRegisterWidgetItem* child =
        (CRegisterWidgetItem*)m_StandardRegTopPage->child(i);
      for (int j = 0; j < 4; j++) {
        if (child->type() == TREE_LIST_REGISTER_TYPE) {
          // child->setForeground(j, child->parent()->foreground(j));
          child->setBackground(j, child->parent()->background(j));
        }
      }
    }
  }
  else {
    for (int i = 0; i < m_StandardRegTopPage->childCount(); i++) {
      CRegisterWidgetItem* child =
        (CRegisterWidgetItem*)m_StandardRegTopPage->child(i);
      for (int j = 0; j < 4; j++) {
        if (child->type() == TREE_LIST_REGISTER_TYPE) {
          // child->setForeground(j, QBrush(QColor("black")));
          child->setBackground(
            j,
            QBrush(
              QColor(m_stdregs.m_vscp_standard_registers_defs[i].bgcolor)));
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

}

///////////////////////////////////////////////////////////////////////////////
// onNodeIdChange
//

void
CFrmNodeConfig::onInterfaceChange(int index)
{
  ui->treeWidgetRegisters->clear();
  ui->treeWidgetRemoteVariables->clear();
  ui->treeWidgetDecisionMatrix->clear();
  ui->treeWidgetMdfFiles->clear();
  for (int i=0; i<NUMBER_OF_TABS; i++) {
    m_saveInfoArea[i].clear();
  }
  m_nUpdates = 0;
}

///////////////////////////////////////////////////////////////////////////////
// onNodeIdChange
//

void
CFrmNodeConfig::onNodeIdChange(int nodeid)
{
  ui->treeWidgetRegisters->clear();
  ui->treeWidgetRemoteVariables->clear();
  ui->treeWidgetDecisionMatrix->clear();
  ui->treeWidgetMdfFiles->clear();
  for (int i=0; i<NUMBER_OF_TABS; i++) {
    m_saveInfoArea[i].clear();
  }
  m_nUpdates = 0;
}

///////////////////////////////////////////////////////////////////////////////
// update
//

void
CFrmNodeConfig::update(void)
{
  if (m_connObject["bfull-l2"].toBool()) {
    ;
  }
  else {
    if (!m_nUpdates) {

      // Read in and render all registers
      if (VSCP_ERROR_SUCCESS != doUpdate("")) {
        QApplication::beep();
        spdlog::error("Update: Failed to read and render registers from the "
                      "remote device.");
        QMessageBox::information(this,
                                  tr(APPNAME),
                                  tr("Failed to read and render registers from the remote device."),
                                  QMessageBox::Ok);
      }

      renderRemoteVariables();
      renderDecisionMatrix();
      renderMdfFiles();
    }
    else {
      // Write changes
      if (VSCP_ERROR_SUCCESS != writeChanges()) {
        QApplication::beep();
        spdlog::error("Update: Failed to write changes to remote device.");
        int ret = QMessageBox::warning(this,
                                       tr(APPNAME),
                                       tr("Failed to write changes to remote "
                                          "device. Please retry operation."),
                                       QMessageBox::Ok);

        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateFull
//

void
CFrmNodeConfig::updateFull(void)
{
  m_nUpdates = 0;
  for (int i=0; i<NUMBER_OF_TABS; i++) {
    m_saveInfoArea[i].clear();
  }
  update();
}

///////////////////////////////////////////////////////////////////////////////
// updateLocal
//

void
CFrmNodeConfig::updateLocal(void)
{
  m_nUpdates = 0;
  for (int i=0; i<NUMBER_OF_TABS; i++) {
    m_saveInfoArea[i].clear();
  }
  update();
}

///////////////////////////////////////////////////////////////////////////////
// onRegisterTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onRegisterTreeWidgetItemClicked(QTreeWidgetItem* item,
                                                int column)
{
  if ((item->type() != TREE_LIST_REGISTER_TYPE) /*&& item->isSelected()*/) {
    fillDeviceHtmlInfo();
    m_bMainInfo = true;
  }
  else {
    fillRegisterHtmlInfo(item, column);
    m_bMainInfo = false;
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
// onRemoteVariableTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onRemoteVariableTreeWidgetItemClicked(QTreeWidgetItem* item,
                                                      int column)
{
  if ((item->type() != TREE_LIST_REMOTEVAR_TYPE)) {
    fillDeviceHtmlInfo();
    m_bMainInfo = true;
  }
  else {
    fillRemoteVariableHtmlInfo(item, column);
    m_bMainInfo = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRemoteVariableTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onRemoteVariableTreeWidgetItemDoubleClicked(
  QTreeWidgetItem* item,
  int column)
{
  if (0 == column) {
    ui->treeWidgetRemoteVariables->editItem(item, column);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRemoteVariableTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onDMTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
  if ((item->type() != TREE_LIST_DM_TYPE)) {
    fillDeviceHtmlInfo();
    m_bMainInfo = true;
  }
  else {
    fillDMHtmlInfo(item, column);
    m_bMainInfo = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRemoteVariableTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onDMTreeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  // SHIFT    = Qt::ShiftModifier
  // CTRL     = Qt::ControlModifier
  // ALT      = Qt::AltModifier
  Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
  if (modifiers.testFlag(Qt::ControlModifier)) {
    // CTRL was hold when this function was called
    ui->treeWidgetDecisionMatrix->editItem(item, column);
  }
  else {
    // CTRL wasn't hold
    editDMRow();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRegisterTreeWidgetCellChanged
//

void
CFrmNodeConfig::onRegisterTreeWidgetCellChanged(QTreeWidgetItem* item, int column)
{
  vscpworks* pworks            = (vscpworks*)QCoreApplication::instance();
  CRegisterWidgetItem* regItem = (CRegisterWidgetItem*)item;
  QString strValue             = item->text(column);
  uint8_t value                = vscp_readStringValue(strValue.toStdString());

  strValue = pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex());
  item->setText(column, strValue);

  // Write new register value
  m_userregs.putReg(regItem->m_regOffset, regItem->m_regPage, value);

  // If not an internal change we should mark as red to
  // indicate a changed value
  if (m_nUpdates) {
    if (m_userregs.isChanged(regItem->m_regOffset, regItem->m_regPage)) {
      item->setForeground(column, QBrush(Qt::red));
      m_bInternalChange = true;
      updateChangeDM(regItem->m_regOffset, regItem->m_regPage, true);
      updateChangeRemoteVariable(regItem->m_regOffset, regItem->m_regPage, true);
      m_bInternalChange = false;
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
// renderStandardRegisters
//

bool
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

  for (int i = 0; i < sizeof(m_stdregs.m_vscp_standard_registers_defs) /
                        sizeof(__struct_standard_register_defs);
       i++) {

    // Register
    itemReg = new CRegisterWidgetItem("Register");
    if (nullptr == itemReg) {
      spdlog::critical("Failed to create standard register widget item");
      return false;
    }

    // Save register pos for later reference
    itemReg->m_regPage   = 0;
    itemReg->m_regOffset = m_stdregs.m_vscp_standard_registers_defs[i].reg;

    Qt::ItemFlags itemFlags =
      Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

    // Set foreground and background colors from MDF
    for (int j = 0; j < 4; j++) {
      itemReg->setForeground(j, QBrush(QColor("black")));
      itemReg->setBackground(
        j,
        QBrush(QColor(m_stdregs.m_vscp_standard_registers_defs[i].bgcolor)));
    }

    // Register pospos
    uint8_t reg = m_stdregs.m_vscp_standard_registers_defs[i].reg;
    str         = "0 : " + QString::number(reg, 10).toStdString();
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
    // std::cout << "Value: " << QString::number(value, 10).toStdString() <<
    // std::endl;
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

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// renderRegisters
//

bool
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

  if (!renderStandardRegisters()) {
    spdlog::critical("Failed to render standard registers");
    return false;
  }

  // ----------------------------------------------------------
  // Fill register info
  // ----------------------------------------------------------

  // Att top level pages

  std::set<uint16_t> pages;
  uint32_t nPages = m_mdf.getPages(pages);
  spdlog::trace("MDF page count = {}", nPages);

  // Get user registers for all pages
  rv = m_userregs.init(*m_vscpClient, guidNode, guidInterface, pages, pworks->m_config_timeout);
  if (VSCP_ERROR_SUCCESS != rv) {
    std::cout << "Failed to read user regs: " << rv << std::endl;
    QApplication::beep();
    spdlog::error("Failed to init user registers");
    return false;
  }

  for (auto page : pages) {

    spdlog::trace("MDF page = {}", page);
    // std::cout << "MDF page = " << page << std::endl;

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
      if (NULL == pregmdf) {
        spdlog::critical("MDF register definition is missing");
        continue;
      }

      CRegisterWidgetItem* itemReg = new CRegisterWidgetItem("Register");
      if (nullptr == itemReg) {
        spdlog::critical("Failed to create register widget item");
        continue;
      }

      Qt::ItemFlags itemFlags =
        Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

      // Set foreground and background colors from MDF
      if (!pworks->m_config_bDisableColors) {
        for (int i = 0; i < 4; i++) {
          itemReg->setForeground(i,
                                 QBrush(QColor(pregmdf->getForegroundColor())));
          itemReg->setBackground(i,
                                 QBrush(QColor(pregmdf->getBackgroundColor())));
        }
      }

      // Save reister info so we can find info later
      itemReg->m_regPage   = page;
      itemReg->m_regOffset = pregmdf->getOffset();

      // Save a pointer to register information
      // itemReg->m_pmdfreg = pregmdf;

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
      int value = m_userregs.getReg(pregmdf->getOffset(), page);
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

  // Clear changes and history as this is the first load.
  m_userregs.clearChanges();
  m_userregs.clearHistory();

  m_nUpdates++; // Another update

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// renderRemoteVariables
//

bool
CFrmNodeConfig::renderRemoteVariables(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  ui->treeWidgetRemoteVariables->clear(); // Clear the tree
  std::deque<CMDF_RemoteVariable*>* listRemoteVariables = m_mdf.getRemoteVariableList();
  for (auto const& item : *listRemoteVariables) {

    CMDF_RemoteVariable* prvmdf = item;

    CRemoteVariableWidgetItem* itemWidget = new CRemoteVariableWidgetItem("Remote Variable");
    if (nullptr == itemWidget) {
      spdlog::critical("Failed to create remote variable widget item");
      continue;
    }

    // Save a pointer to the MDF remote variable item
    itemWidget->m_pRemoteVariable = item;

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

    // Set foreground and background colors from MDF
    if (!pworks->m_config_bDisableColors) {
      for (int i = 0; i < 4; i++) {
        itemWidget->setForeground(i, QBrush(QColor(prvmdf->getForegroundColor())));
        itemWidget->setBackground(i, QBrush(QColor(prvmdf->getBackgroundColor())));
      }
    }

    // Save register info so we can find info later
    itemWidget->m_pRemoteVariable = item;
    // itemWidget->m_regPage = prvmdf->getPage();
    // itemWidget->m_regOffset = prvmdf->getOffset();

    mdf_access_mode access = prvmdf->getAccess();
    if (MDF_REG_ACCESS_READ_ONLY == access) {
      // itemReg->setForeground(2, QBrush(QColor("gray")));
      itemWidget->setText(REMOTEVAR_COL_ACCESS, "r");
    }
    else if (MDF_REG_ACCESS_WRITE_ONLY == access) {
      // itemReg->setForeground(2, QBrush(QColor("darkgreen")));
      itemWidget->setText(REMOTEVAR_COL_ACCESS, "w");
      itemFlags |= Qt::ItemIsEditable;
    }
    else if (MDF_REG_ACCESS_READ_WRITE == access) {
      // itemReg->setForeground(2, QBrush(QColor("red")));
      itemWidget->setText(REMOTEVAR_COL_ACCESS, "rw");
      itemFlags |= Qt::ItemIsEditable;
    }
    else {
      // itemReg->setForeground(2, QBrush(QColor("black")));
      itemWidget->setText(REMOTEVAR_COL_ACCESS, "---");
    }
    itemWidget->setTextAlignment(REMOTEVAR_COL_ACCESS, Qt::AlignCenter);
    itemWidget->setFlags(itemFlags);

    itemWidget->setTextAlignment(REMOTEVAR_COL_TYPE, Qt::AlignCenter);
    itemWidget->setText(REMOTEVAR_COL_TYPE, prvmdf->getTypeString().c_str());

    itemWidget->setTextAlignment(REMOTEVAR_COL_NAME, Qt::AlignLeft);
    itemWidget->setText(REMOTEVAR_COL_NAME, prvmdf->getName().c_str());

    std::string str;
    uint8_t format = FORMAT_REMOTEVAR_DECIMAL;

    if (0 == m_baseComboBox->currentIndex()) {
      format = FORMAT_REMOTEVAR_HEX;
    }

    if (VSCP_ERROR_SUCCESS != m_userregs.remoteVarFromRegToString(*prvmdf, str, format)) {
      str = "ERROR";
      itemWidget->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("red")));
    }
    itemWidget->setTextAlignment(REMOTEVAR_COL_VALUE, Qt::AlignCenter);
    itemWidget->setText(REMOTEVAR_COL_VALUE, str.c_str());

    bool bChanged = false;
    for (int pos=prvmdf->getOffset(); pos<prvmdf->getOffset()+prvmdf->getTypeByteCount(); pos++) {
      if (m_userregs.isChanged(pos)) {
        bChanged = true;
        break;
      }
    }

    if (bChanged) {
      itemWidget->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("red")));
    }
    else {
      itemWidget->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("black")));
    }

    // Add item
    ui->treeWidgetRemoteVariables->addTopLevelItem(itemWidget);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// renderDecisionMatrix
//

bool
CFrmNodeConfig::renderDecisionMatrix(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  ui->treeWidgetDecisionMatrix->clear(); // Clear the tree

  CMDF_DecisionMatrix* pdm = m_mdf.getDM();
  uint16_t startPage       = pdm->getStartPage();
  uint32_t startOffset     = pdm->getStartOffset();

  for (int row = 0; row < pdm->getRowCount(); row++) {
    CDMWidgetItem* itemWidget = new CDMWidgetItem("DM");
    if (nullptr == itemWidget) {
      spdlog::critical("Failed to create DM widget item");
      continue;
    }

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled |
                              Qt::ItemIsSelectable |
                              Qt::ItemNeverHasChildren;
    itemFlags |= Qt::ItemIsEditable;

    // Save helper info
    itemWidget->m_row = row;
    itemWidget->m_pDM = pdm;

    if (row % 2) {
      for (int i = 0; i < 8; i++) {
        itemWidget->setBackground(i, QBrush(QColor("#c0c0c0")));
      }
    }
    else {
      for (int i = 0; i < 8; i++) {
        itemWidget->setBackground(i, QBrush(QColor("#e0e0e0")));
      }
    }

    // Fill in data
    for (int pos = 0; pos < 8; pos++) {
      int value     = m_userregs.getReg(startOffset + row * 8 + pos, startPage);
      bool bChanged = m_userregs.isChanged(startOffset + row * 8 + pos,
                                           startPage);
      if (-1 == value) {
        itemWidget->setText(REG_COL_VALUE, "---");
        itemWidget->setTextAlignment(DM_LEVEL1_COL_ORIGIN + pos,
                                     Qt::AlignCenter);
      }
      else {
        // std::string str = pworks->decimalToStringInBase(value,
        // m_baseComboBox->currentIndex()).toStdString();
        itemWidget->setTextAlignment(DM_LEVEL1_COL_ORIGIN + pos,
                                     Qt::AlignCenter);
        itemWidget->setText(
          DM_LEVEL1_COL_ORIGIN + pos,
          pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex()));
        if (bChanged) {
          itemWidget->setForeground(DM_LEVEL1_COL_ORIGIN + pos, QBrush(QColor("red")));
        }
        else {
          itemWidget->setForeground(DM_LEVEL1_COL_ORIGIN + pos, QBrush(QColor("black")));
        }
      }
    }

    QComboBox* pcombo = new QComboBox();
    pcombo->addItems(QStringList() << "item1"
                                   << "item2");
    ui->treeWidgetDecisionMatrix->setItemWidget(itemWidget, 6, pcombo);

    itemWidget->setFlags(itemFlags);

    // Add item
    ui->treeWidgetDecisionMatrix->addTopLevelItem(itemWidget);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// renderMdfFiles
//

bool
CFrmNodeConfig::renderMdfFiles(void)
{
  int rv;
  CDMWidgetItem* topItemWidget;
  CDMWidgetItem* itemWidget;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  ui->treeWidgetMdfFiles->clear(); // Clear the tree

  // Pictures
  if (m_mdf.getPictureCount()) {

    topItemWidget = new CDMWidgetItem("Pictures");
    if (nullptr == topItemWidget) {
      spdlog::critical(
        "Failed to create MDF file widget top item for pictures");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Pictures");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getPictureCount(); i++) {

      itemWidget = new CDMWidgetItem("Pictures");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for pictures");
        return false;
      }

      QString name(m_mdf.getPictureObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getPictureObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add picture child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Video
  if (m_mdf.getVideoCount()) {

    topItemWidget = new CDMWidgetItem("Video");
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for videos");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Video");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getVideoCount(); i++) {
      itemWidget = new CDMWidgetItem("Video");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for videos");
        return false;
      }

      QString name(m_mdf.getVideoObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getVideoObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add picture child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Firmware
  if (m_mdf.getFirmwareCount()) {

    topItemWidget = new CDMWidgetItem("Firmware");
    if (nullptr == topItemWidget) {
      spdlog::critical(
        "Failed to create MDF file widget top item for firmware");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Firmware");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getFirmwareCount(); i++) {
      itemWidget = new CDMWidgetItem("Firmware");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for firmware");
        return false;
      }

      QString name(m_mdf.getFirmwareObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getFirmwareObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add firmware child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Driver
  if (m_mdf.getDriverCount()) {

    topItemWidget = new CDMWidgetItem("Driver");
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for driver");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Driver");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getDriverCount(); i++) {
      itemWidget = new CDMWidgetItem("Driver");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for driver");
        return false;
      }

      QString name(m_mdf.getDriverObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getDriverObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add driver child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Manual
  if (m_mdf.getDriverCount()) {

    topItemWidget = new CDMWidgetItem("Manual");
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for manual");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Manual");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getManualCount(); i++) {
      itemWidget = new CDMWidgetItem("Manual");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for Manual");
        return false;
      }

      QString name(m_mdf.getManualObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getManualObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add driver child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Setup
  if (m_mdf.getSetupCount()) {

    topItemWidget = new CDMWidgetItem("Setup");
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for setup");
      return false;
    }

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Setup");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getSetupCount(); i++) {
      itemWidget = new CDMWidgetItem("Setup");
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for setup");
        return false;
      }

      QString name(m_mdf.getSetupObj(i)->getName().c_str());
      if (!name.length()) {
        name = QString::number(i);
        name += " - ";
        name += m_mdf.getSetupObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add driver child
      topItemWidget->addChild(itemWidget);
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeChanges
//

int
CFrmNodeConfig::writeChanges(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (!m_vscpClient->isConnected()) {
    QApplication::beep();
    spdlog::error("Aborted write changed register(s) due to no connection.");
    return VSCP_ERROR_CONNECTION;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  guidNode.setLSB(m_nodeidConfig->value()); // Set node id

  QTreeWidgetItemIterator item(ui->treeWidgetRegisters);
  while (*item) {

    if ((*item)->type() == TREE_LIST_REGISTER_TYPE) {

      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*item);

      // Only interested in changed registers
      if (!m_userregs.isChanged(itemReg->m_regOffset, itemReg->m_regPage)) {
        ++item;
        continue;
      }

      uint8_t value = vscp_readStringValue((*item)->text(REG_COL_VALUE).toStdString());
      if (VSCP_ERROR_SUCCESS == (rv = vscp_writeLevel1Register(*m_vscpClient,
                                                               guidNode,
                                                               guidInterface,
                                                               itemReg->m_regPage,
                                                               itemReg->m_regOffset,
                                                               value,
                                                               pworks->m_config_timeout))) {
        
        m_userregs.setChangedState(itemReg->m_regOffset, itemReg->m_regPage, false);
        
        (*item)->setText(REG_COL_VALUE,
                         pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
                           .toStdString()
                           .c_str());
        (*item)->setForeground(REG_COL_VALUE, QBrush(QColor("royalblue")));

        updateChangeDM(itemReg->m_regOffset, itemReg->m_regPage);
        updateChangeRemoteVariable(itemReg->m_regOffset, itemReg->m_regPage);
      }
      else {
        QApplication::beep();
        spdlog::error("Failed to write register(s) rv = {}", rv);
        QString str = tr("Failed to write register(s) rv = ") + QString::number(rv);
        ui->statusBar->showMessage(str);
        QApplication::restoreOverrideCursor();
        return VSCP_ERROR_COMMUNICATION;
      }
    }
    ++item;
  }

  ui->statusBar->showMessage(tr("Changed registers written OK"));
  QApplication::restoreOverrideCursor();

  return VSCP_ERROR_SUCCESS;
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
// doUpdate
//

int
CFrmNodeConfig::doUpdate(std::string mdfpath)
{
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
    return VSCP_ERROR_CONNECTION;
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

  int rv = m_stdregs.init(*m_vscpClient, guidNode, guidInterface, pworks->m_config_timeout);
  if (VSCP_ERROR_SUCCESS != rv) {
    QApplication::beep();
    ui->statusBar->showMessage(tr("Failed to read standard registers from device. rv=") +
                               QString::number(rv));
    spdlog::error("Failed to init standard registers {0}", rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return VSCP_ERROR_COMMUNICATION;
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
    QApplication::beep();
    ui->statusBar->showMessage(tr("Failed to download MDF file for device."));
    spdlog::error("Failed to download MDF {0} curl rv={1}", url, curl_rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return VSCP_ERROR_COMMUNICATION;
  }

  pbar->setValue(75);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  spdlog::trace("MDF Downloader", tempPath);

  // * * * Parse  MDF * * *

  ui->statusBar->showMessage(tr("Parsing MDF file..."));
  rv = m_mdf.parseMDF(tempPath);
  if (VSCP_ERROR_SUCCESS != rv) {
    QApplication::beep();
    ui->statusBar->showMessage(tr("Failed to parse MDF file for device."));
    spdlog::error("Failed to parse MDF {0} rv={1}", tempPath, rv);
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return VSCP_ERROR_PARSING;
  }

  pbar->setValue(80);
  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  ui->statusBar->showMessage(tr("MDF read from device and parsed OK"));
  spdlog::trace("Parsing MDF OK");

  // Fill register data
  if (!renderRegisters()) {
    QApplication::beep();
    ui->statusBar->showMessage(tr("Failed to render registers"));
    spdlog::error("Failed to render registers");
    QApplication::restoreOverrideCursor();
    ui->statusBar->removeWidget(pbar);
    return VSCP_ERROR_PARSING;
  }
  fillDeviceHtmlInfo();
  m_bMainInfo = true;
  pbar->setValue(100);

  QApplication::restoreOverrideCursor();
  ui->statusBar->removeWidget(pbar);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedRegisterValues
//

void
CFrmNodeConfig::readSelectedRegisterValues(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (!m_vscpClient->isConnected()) {
    int ret = QMessageBox::warning(this,
                                    tr(APPNAME),
                                    tr("Need to be connected to perform this operation."),
                                    QMessageBox::Ok);
    spdlog::error("Aborted read register(s) due to no connection.");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  guidNode.setLSB(m_nodeidConfig->value()); // Set node id

  QList<QTreeWidgetItem*> listSelected =
    ui->treeWidgetRegisters->selectedItems();
  for (auto item : listSelected) {
    if (item->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
      uint8_t value = vscp_readStringValue(item->text(REG_COL_VALUE).toStdString());
      if (VSCP_ERROR_SUCCESS == (rv = vscp_readLevel1Register(*m_vscpClient,
                                                                guidNode,
                                                                guidInterface,
                                                                itemReg->m_regPage,
                                                                itemReg->m_regOffset,
                                                                value,
                                                                pworks->m_config_timeout))) {
        item->setText(REG_COL_VALUE, pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
                                      .toStdString()
                                      .c_str());
        item->setForeground(REG_COL_VALUE, QBrush(QColor("black")));
      }
      else {
        QApplication::beep();
        spdlog::error("Failed to read register(s) rv = {}", rv);
        QString str = tr("Failed to read register(s) rv = ") + QString::number(rv);
        ui->statusBar->showMessage(str);
        QApplication::restoreOverrideCursor();        
        return;
      }
    }
  }

  ui->statusBar->showMessage(tr("Register(s) read OK"));
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// writeSelectedRegisterValues
//

void
CFrmNodeConfig::writeSelectedRegisterValues(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (!m_vscpClient->isConnected()) {
    QApplication::beep();
    int ret = QMessageBox::warning(this,
                                    tr(APPNAME),
                                    tr("Need to be connected to perform this operation."),
                                    QMessageBox::Ok);
    spdlog::error("Aborted read register(s) due to no connection.");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  guidNode.setLSB(m_nodeidConfig->value()); // Set node id

  // Walk through selected items
  QList<QTreeWidgetItem*> listSelected = ui->treeWidgetRegisters->selectedItems();
  for (auto item : listSelected) {
    if (item->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
      uint8_t value                = vscp_readStringValue(item->text(REG_COL_VALUE).toStdString());
      if (VSCP_ERROR_SUCCESS == (rv = vscp_writeLevel1Register(*m_vscpClient,
                                                                  guidNode,
                                                                  guidInterface,
                                                                  itemReg->m_regPage,
                                                                  itemReg->m_regOffset,
                                                                  value,
                                                                  pworks->m_config_timeout))) {
        // Certain that read value is the same as the one we just wrote here
        m_userregs.setChangedState(itemReg->m_regOffset, itemReg->m_regPage, false);
        m_userregs.putReg(itemReg->m_regOffset,
                          itemReg->m_regPage,
                          value);
        item->setText(REG_COL_VALUE,
                      pworks->decimalToStringInBase(value, m_baseComboBox->currentIndex())
                        .toStdString()
                        .c_str());
        item->setForeground(REG_COL_VALUE, QBrush(QColor("royalblue")));

        updateChangeDM(itemReg->m_regOffset, itemReg->m_regPage);
        updateChangeRemoteVariable(itemReg->m_regOffset, itemReg->m_regPage);
      }
      else {
        QApplication::beep();
        spdlog::error("Failed to write register(s) rv = {}", rv);
        QString str = tr("Failed to write register(s) rv = ") + QString::number(rv);
        ui->statusBar->showMessage(str);
        QApplication::restoreOverrideCursor();
        return;
      }
    }
  }

  ui->statusBar->showMessage(tr("Register(s) written OK"));
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// defaultSelectedRegisterValues
//

void
CFrmNodeConfig::defaultSelectedRegisterValues(void)
{
  writeChanges();
}

///////////////////////////////////////////////////////////////////////////////
// defaultRegisterAll
//

void
CFrmNodeConfig::defaultRegisterAll(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// undoSelectedRegisterValues
//

void
CFrmNodeConfig::undoSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// redoSelectedRegisterValues
//

void
CFrmNodeConfig::redoSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPageMenu
//

void
CFrmNodeConfig::gotoRegisterPageMenu(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// saveSelectedRegisterValues
//

void
CFrmNodeConfig::saveSelectedRegisterValues(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// saveAllRegisterValues
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
// fillRegisterHtmlInfo
//

void
CFrmNodeConfig::fillRegisterHtmlInfo(QTreeWidgetItem* item, int column)
{
  int idx;
  std::string html;
  std::string str;
  CRegisterWidgetItem* pitem = (CRegisterWidgetItem*)item;

  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body style=\"font-family:'Ubuntu'; font-size:11pt; "
          "font-weight:400; font-style:normal;\">";
  str += pitem->text(REG_COL_NAME).toStdString();
  html += "<h1>";
  html += str;
  html += "</h1>";
  html += "<p><b>";
  html += pitem->text(REG_COL_POS).toStdString();
  html += "</b> [";
  html += pitem->text(REG_COL_ACCESS).toStdString();
  html += "] ";
  html += "</p>";
  html += "<p>";
  CMDF_Register* preg = m_mdf.getRegister(pitem->m_regOffset, pitem->m_regPage);
  if (nullptr == preg) {
    html += tr("Register not found in MDF").toStdString();
  }
  else {
    str = preg->getDescription();
    html += m_mdf.format(str);
    // str = "# test\n";
    // str += "This _is_ som **test** text\n\nAnd some more text\n\n";
    // str += "1. First item\n";
    // str += "2. Second item\n";
    // str += "3. Third item\n";
    // str += "4. Fourth item\n\n";
    // str += "* One\n";
    // str += "* Two\n";
    // str += "* Three\n";
    // str += "* Four\n\n";
    // html = m_mdf.format(str);
  }
  html += "<p>";
  html += "</font>";
  html += "</body></html>";

  // Set the HTML
  ui->infoArea->setHtml(html.c_str());
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
  // html += "<font color=\"#009900\">";

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

  std::string prefix = "http://";
  if (std::string::npos != m_stdregs.getMDF().find("http://")) {
    prefix = "";
  }

  html += "</font><b>MDF URL</b>:<font color=\"#009900\"> ";
  html += "<a href=\"";
  html += prefix;
  html += m_stdregs.getMDF();
  html += "\" target=\"ext\">";
  html += prefix;
  html += m_stdregs.getMDF();
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
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID)).toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 1))
            .toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 2))
            .toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 3))
            .toStdString();
  html += ".";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_ID + 3))
            .toStdString();
  html += "<br>";

  html += "</font><b>Manufacturer Device ID:</b><font color=\"#009900\"> ";
  html +=
    QString::number(m_stdregs.getManufacturerDeviceID(), 16).toStdString();
  html += " - ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID))
            .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 1))
      .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 2))
      .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 3))
      .toStdString();
  html += "<br>";

  html += "</font><b>Manufacturer sub device ID:</b><font color=\"#009900\"> ";
  html += "0x";
  html +=
    QString::number(m_stdregs.getManufacturerSubDeviceID(), 16).toStdString();
  html += " - ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID))
            .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 1))
      .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 2))
      .toStdString();
  html += ".";
  html +=
    QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 3))
      .toStdString();
  html += "<br>";

  html += "</font><b>Page select:</b><font color=\"#009900\"> ";
  html += QString::number(m_stdregs.getRegisterPage()).toStdString();
  html += " MSB=";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_PAGE_SELECT_MSB))
            .toStdString();
  html += " LSB= ";
  html += QString::number(m_stdregs.getReg(VSCP_STD_REGISTER_PAGE_SELECT_LSB))
            .toStdString();
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

  CMDF_DecisionMatrix* pdm = m_mdf.getDM();
  if ((nullptr == pdm) || !pdm->getRowCount()) {
    html += "No Decision Matrix is available on this device.";
    html += "<br>";
  }
  else {
    html += "</font><b>Decision Matrix:</b><font color=\"#009900\"> Rows=";
    html += QString::number(pdm->getRowCount()).toStdString();
    html += " Startoffset=";
    html += QString::number(pdm->getStartOffset()).toStdString();
    html += " (0x";
    html += QString::number(pdm->getStartOffset(), 16).toStdString();
    html += ") Startpage=";
    html += QString::number(pdm->getStartPage()).toStdString();
    html += " (0x";
    html += QString::number(pdm->getStartPage(), 16).toStdString();
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
  // html += "<br>";
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
  CMDF_Item* phone;
  while (nullptr != (phone = m_mdf.getManufacturer()->getPhoneObj(idx))) {
    html += "</font><b>Phone:</b><font color=\"#009900\"> ";
    html += phone->getName();
    html += "</font> - ";
    html += phone->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item* fax;
  while (nullptr != (fax = m_mdf.getManufacturer()->getFaxObj(idx))) {
    html += "</font><b>Fax:</b><font color=\"#009900\"> ";
    html += fax->getName();
    html += "</font> - ";
    html += fax->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item* email;
  while (nullptr != (email = m_mdf.getManufacturer()->getEmailObj(idx))) {
    html += "</font><b>Email:</b><font color=\"#009900\"> ";
    html += email->getName();
    html += "</font> - ";
    html += email->getDescription();
    html += "<br><font color=\"#009900\">";
    idx++;
  }

  idx = 0;
  CMDF_Item* web;
  while (nullptr != (web = m_mdf.getManufacturer()->getWebObj(idx))) {
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

///////////////////////////////////////////////////////////////////////////////
// fillRemoteVariableHtmlInfo
//

void
CFrmNodeConfig::fillRemoteVariableHtmlInfo(QTreeWidgetItem* item, int column)
{
  int idx;
  std::string html;
  std::string str;
  CRemoteVariableWidgetItem* pitem = (CRemoteVariableWidgetItem*)item;

  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body style=\"font-family:'Ubuntu'; font-size:11pt; "
          "font-weight:400; font-style:normal;\">";
  str += pitem->text(REMOTEVAR_COL_NAME).toStdString();
  html += "<h1>";
  html += str;
  html += "</h1>";
  html += "<p><b>";
  html += pitem->text(REG_COL_POS).toStdString();
  html += "</b> [";
  html += pitem->text(REMOTEVAR_COL_ACCESS).toStdString();
  html += "] ";
  html += pitem->text(REMOTEVAR_COL_TYPE).toStdString();
  html += "</p>";
  html += "<p>";
  CMDF_RemoteVariable* prv = pitem->m_pRemoteVariable;
  if (nullptr == prv) {
    html += tr("Remote variable not found in MDF").toStdString();
  }
  else {
    std::string desc = prv->getDescription();
    html += m_mdf.format(desc);
  }
  html += "<p>";
  html += "</font>";
  html += "</body></html>";

  // Set the HTML
  ui->infoArea->setHtml(html.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// fillDMHtmlInfo
//

void
CFrmNodeConfig::fillDMHtmlInfo(QTreeWidgetItem* item, int column)
{
  int idx;
  std::string html;
  std::string str;
  CDMWidgetItem* pitem     = (CDMWidgetItem*)item;
  CMDF_DecisionMatrix* pDM = pitem->m_pDM;

  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body style=\"font-family:'Ubuntu'; font-size:11pt; "
          "font-weight:400; font-style:normal;\">";
  str = "DM Row ";
  str += QString::number(pitem->m_row + 1).toStdString();
  str += "/";
  str += QString::number(pitem->m_pDM->getRowCount()).toStdString();
  html += "<font color=\"#009999\"><b>";
  html += str;
  html += "</b></font>";
  html += "<hr>";
  html += "<b>Origin:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_ORIGIN).toStdString();
  html += "</font><br>";
  html += "<b>Flags:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_FLAGS).toStdString();
  html += "</font> -- ";
  uint8_t flags =
    vscp_readStringValue(pitem->text(DM_LEVEL1_COL_FLAGS).toStdString());
  if (flags & 0x80) {
    html += "enabled ";
  }
  else {
    html += "disabled ";
  }
  if (flags & 0x40) {
    html += ", oaddr should match nickname ";
  }
  else {
    html += ", oaddr is don't care ";
  }
  if (flags & 0x20) {
    html += ", oaddr should be hard-coded ";
  }
  if (flags & 0x10) {
    html += ", Match zone ";
  }
  if (flags & 0x08) {
    html += ", Match sub-zone ";
  }
  html += ", Class-mask bit 9=";
  if (flags & 0x02) {
    html += " 1 ";
  }
  else {
    html += " 0 ";
  }
  html += ", Type-mask bit 9=";
  if (flags & 0x01) {
    html += " 1<br>";
  }
  else {
    html += " 0<br>";
  }

  html += "<b>Class mask:</b><font color=\"#009900\"> 0x";
  html +=
    QString::number((flags & 0x02) * 512 +
                      vscp_readStringValue(
                      pitem->text(DM_LEVEL1_COL_CLASS_MASK).toStdString()),
                      16)
      .toStdString();
  html += "</font><br>";

  html += "<b>Class filter:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_CLASS_FILTER).toStdString();
  html += "</font><br>";

  html += "<b>Type mask:</b><font color=\"#009900\"> 0x";
  html +=
    QString::number((flags & 0x01) * 512 +
                      vscp_readStringValue(
                      pitem->text(DM_LEVEL1_COL_TYPE_MASK).toStdString()),
                      16)
      .toStdString();
  html += "</font><br>";

  html += "<b>Type filter:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_TYPE_FILTER).toStdString();
  html += "</font><br>";

  html += "<b>Action:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_ACTION).toStdString();
  std::deque<CMDF_Action*>* actionlList = pDM->getActionList();
  CMDF_Action* pAction =
    actionlList->at(/*pitem->m_row*8 + DM_LEVEL1_COL_ACTION*/ 1);
  if (nullptr == pAction) {
    // TODO;
  }
  else {
    html += "</font>";
    html += "<br> <b>Name:</b><font color=\"#000099\"> ";
    str = pAction->getName();
    html += m_mdf.format(str);
    html += "</font><br> <b>Description:</b><font color=\"#000099\"> ";
    str = pAction->getDescription();
    html += m_mdf.format(str);
    html += "</font>";
  }
  html += "<br>";

  html += "<b>Action Parameter:</b><font color=\"#009900\"> ";
  html += pitem->text(DM_LEVEL1_COL_PARAMETER).toStdString();
  html += "</font><br>";

  html += "<p>";

  if (nullptr == pDM) {
    html += tr("DM not found in MDF").toStdString();
  }
  else {
    std::string desc = QString::number(pitem->m_row).toStdString();
    html += m_mdf.format(desc);
  }
  html += "<p>";
  html += "</font>";
  html += "</body></html>";

  // Set the HTML
  ui->infoArea->setHtml(html.c_str());
}



///////////////////////////////////////////////////////////////////////////////
// onRemoteVarTreeWidgetCellChanged
//

void
CFrmNodeConfig::onRemoteVarTreeWidgetCellChanged(QTreeWidgetItem* item, int column)
{
  // Don't do anything if updated from register cell edit
  if (m_bInternalChange) {
    return;
  }

  int rv;
  vscpworks* pworks                  = (vscpworks*)QCoreApplication::instance();
  CRemoteVariableWidgetItem* rvItem = (CRemoteVariableWidgetItem*)item;
  std::string strValue                   = item->text(column).toStdString();
  if (VSCP_ERROR_SUCCESS != (rv = m_userregs.remoteVarFromStringToReg(*rvItem->m_pRemoteVariable, strValue))) {
    QApplication::beep();
    std::string strError = tr("Failed to write remote variable to registers. rv=").toStdString();
    strError.append(QString::number(rv).toStdString());
    spdlog::error(strError);
    ui->statusBar->showMessage(strError.c_str());
    QMessageBox::information(this,
                             tr(APPNAME),
                             strError.c_str(),
                             QMessageBox::Ok);
  }
  updateVisualRegisters();
}

///////////////////////////////////////////////////////////////////////////////
// onDMTreeWidgetCellChanged
//

void
CFrmNodeConfig::onDMTreeWidgetCellChanged(QTreeWidgetItem* item, int column)
{
  // Don't do anything if updated from register cell edit
  if (m_bInternalChange) {
    return;
  }

  vscpworks* pworks   = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* itemDM = (CDMWidgetItem*)item;
  QString strValue    = itemDM->text(column);
  uint8_t value       = vscp_readStringValue(strValue.toStdString());
  m_userregs.putReg((itemDM->m_row * itemDM->m_pDM->getRowSize()) + column,
                        itemDM->m_pDM->getStartPage(),
                        value);
  updateVisualRegisters();
}

///////////////////////////////////////////////////////////////////////////////
// editDMRow
//

void
CFrmNodeConfig::editDMRow()
{
  int reg;
  vscpworks* pworks                = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* item              = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->currentItem();
  uint16_t page                    = item->m_pDM->getStartPage();
  //std::map<uint32_t, uint8_t>* map = m_userregs.getRegisterMap(item->m_pDM->getStartPage());

  CDlgEditDm* pDlg = new CDlgEditDm(this);
  pDlg->setWindowTitle(tr("Edit Decision Matrix Row"));
  pDlg->setMDF(&m_mdf);

  // Address origin
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) + 
                              CMDF_DecisionMatrix::IDX_ADDRESS_ORIGIN,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmAddressOrigin(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Flags
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_FLAGS,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmFlags(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Class mask
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_CLASS_MASK,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmClassMask(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Class filter
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_CLASS_FILTER,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmClassFilter(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Type mask
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_TYPE_MASK,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmTypeMask(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Type filter
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() +
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_TYPE_FILTER,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmTypeFilter(pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex()).toStdString());

  // Action
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() + 
                              (item->m_row * item->m_pDM->getRowSize()) +
                              CMDF_DecisionMatrix::IDX_ADDRESS_ACTION,
                            item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmActionFromCode(reg);

  // Action Parameter
  reg = m_userregs.getReg(item->m_pDM->getStartOffset() + 
                            (item->m_row * item->m_pDM->getRowSize()) +
                            CMDF_DecisionMatrix::IDX_ADDRESS_ACTION_PARAMETER,
                          item->m_pDM->getStartPage());
  if (reg < 0) {
    return;
  }
  pDlg->setDmActionParameter(
    pworks->decimalToStringInBase(reg, m_baseComboBox->currentIndex())
      .toStdString());

  // Show the dialog
  if (pDlg->exec() == QDialog::Accepted) {

    // Save data

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_ORIGIN,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmAddressOrigin());
    // If changed mark as changed in visual interface
    // if (m_userregs.isChanged( (item->m_row * item->m_pDM->getRowSize()) +
    //                       CMDF_DecisionMatrix::IDX_ADDRESS_ORIGIN,
    //                       item->m_pDM->getStartPage()) {

    // }

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_FLAGS,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmFlags());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_CLASS_MASK,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmClassMask());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_CLASS_FILTER,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmClassFilter());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_TYPE_MASK,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmTypeMask());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_TYPE_FILTER,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmTypeFilter());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_ACTION,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmAction());

    m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_ACTION_PARAMETER,
                      item->m_pDM->getStartPage(),
                      pDlg->getDmActionParameter());

    updateVisualRegisters();
    fillDMHtmlInfo(item, 0);  
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateVisualRegisters
//

void
CFrmNodeConfig::updateVisualRegisters(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Mark all changed registers
  QTreeWidgetItemIterator it(ui->treeWidgetRegisters);
  while (*it) {
    
    CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*it);
    //std::cout << "Item: " << itemReg->m_regPage << " " << itemReg->m_regOffset << std::endl;

    if (itemReg->type() != TREE_LIST_REGISTER_TYPE) {
      ++it;
      continue;
    }

    // No update of standard registers
    if ((0 == itemReg->m_regPage) && (itemReg->m_regOffset >= 128)) {
      ++it;
      continue;
    }

    // Write value
    itemReg->setText(
      REG_COL_VALUE,
      pworks
        ->decimalToStringInBase(
          m_userregs.getReg(itemReg->m_regOffset, itemReg->m_regPage),
          m_baseComboBox->currentIndex())
        .toStdString()
        .c_str());

    // Set forecolor
    if (m_userregs.isChanged(itemReg->m_regOffset, itemReg->m_regPage)) {
      itemReg->setForeground(REG_COL_VALUE, QBrush(QColor("red")));
    }
    // Blue if changed some time but written
    else if (m_userregs.hasWrittenChange(itemReg->m_regOffset, itemReg->m_regPage)) {
      itemReg->setForeground(REG_COL_VALUE, QBrush(QColor("royalblue")));
    }
    // Black if never changed
    else {
      itemReg->setForeground(REG_COL_VALUE, QBrush(QColor("black")));
    }
    ++it;
    // CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*item);
    // uint8_t value =
    // vscp_readStringValue((*item)->text(REG_COL_VALUE).toStdString()); if
    // (VSCP_ERROR_SUCCESS == vscp_writeLevel1Register(*m_vscpClient,
    //                                                    guidNode,
    //                                                    guidInterface,
    //                                                    itemReg->m_regPage,
    //                                                    itemReg->m_regOffset,
    //                                                    value,
    //                                                    pworks->m_config_timeout)) {
    //  QBrush(QColor("red")) == (*item)->foreground(REG_COL_VALUE)
    // m_userregs.markChanged(item->m_pDM->getStartPage(),
    //                         (item->m_row * item->m_pDM->getRowSize()) + i);
  }
  updateVisualRemoteVariables();
  updateVisualDM();
}

///////////////////////////////////////////////////////////////////////////////
// updateVisualRemoteVariables
//

void
CFrmNodeConfig::updateVisualRemoteVariables(void)
{
  std::string str;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  uint8_t format = FORMAT_REMOTEVAR_DECIMAL;
  if (0 == m_baseComboBox->currentIndex()) {
    format = FORMAT_REMOTEVAR_HEX;
  }

  QTreeWidgetItemIterator it(ui->treeWidgetRemoteVariables);
  while (*it) {
    CRemoteVariableWidgetItem* itemRV = (CRemoteVariableWidgetItem*)(*it);
    if ((nullptr == itemRV) || (nullptr == itemRV->m_pRemoteVariable) ) {
      ++it;
      continue;
    }
    int pos = itemRV->m_pRemoteVariable->getOffset();
    for (int i = pos; i < (pos + itemRV->m_pRemoteVariable->getTypeByteCount()); i++) {
      if (m_userregs.isChanged(i, itemRV->m_pRemoteVariable->getPage())) {
        if (VSCP_ERROR_SUCCESS != m_userregs.remoteVarFromRegToString(*itemRV->m_pRemoteVariable, str, format)) {
          itemRV->setText(REMOTEVAR_COL_VALUE, pworks->decimalToStringInBase(m_userregs.getReg(i, itemRV->m_pRemoteVariable->getPage()), m_baseComboBox->currentIndex()).toStdString().c_str());
          itemRV->setForeground(i % 8, QBrush(QColor("red")));
        }
      }
    }
    ++it;
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateVisualDM
//

void
CFrmNodeConfig::updateVisualDM(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QTreeWidgetItemIterator it(ui->treeWidgetDecisionMatrix);
  while (*it) {
    CDMWidgetItem* itemDM = (CDMWidgetItem*)(*it);
    int pos               = itemDM->m_pDM->getStartOffset() + itemDM->m_row * itemDM->m_pDM->getRowSize();
    for (int i = pos; i < (pos + 8); i++) {
      if (m_userregs.isChanged(i, itemDM->m_pDM->getStartPage())) {
        itemDM->setText(i % 8, pworks->decimalToStringInBase(m_userregs.getReg(i, itemDM->m_pDM->getStartPage()), m_baseComboBox->currentIndex()).toStdString().c_str());
        itemDM->setForeground(i % 8, QBrush(QColor("red")));
      }
    }
    ++it;
  }

  // renderDecisionMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// updateChangeRemoteVariable
//

void
CFrmNodeConfig::updateChangeRemoteVariable(uint32_t offset, uint16_t page, bool bFromRegUpdate)
{
  // Check if register is use by the DM
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QTreeWidgetItemIterator it(ui->treeWidgetRemoteVariables);
  while (*it) {

    // Get row item for remote variable
    CRemoteVariableWidgetItem* itemRV = (CRemoteVariableWidgetItem*)(*it);
    if (nullptr == itemRV) {
      ++it;
      continue;
    }

    // Get remote variable definition
    CMDF_RemoteVariable *pRemoteVariable = itemRV->m_pRemoteVariable;
    if (nullptr == pRemoteVariable) {
      ++it;
      continue;
    }

    // Must be on the correct page
    if (pRemoteVariable->getPage() != page) {
      ++it;
      continue;
    }

    // Register part of this remote variable
    if ((offset >= pRemoteVariable->getOffset()) && 
        (offset < (pRemoteVariable->getOffset()+ pRemoteVariable->getTypeByteCount()))) {

      std::string str;
      uint8_t format = FORMAT_REMOTEVAR_DECIMAL;
      if (0 == m_baseComboBox->currentIndex()) {
        format = FORMAT_REMOTEVAR_HEX;
      }
      if (VSCP_ERROR_SUCCESS != m_userregs.remoteVarFromRegToString(*pRemoteVariable, str, format)) {
        str = "ERROR";
        itemRV->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("red")));
      }
      
      m_bInternalChange = true;
      itemRV->setText(REMOTEVAR_COL_VALUE, str.c_str());                        
      m_bInternalChange = false;

      // Set forecolor
      if (m_userregs.isChanged(offset, page)) {
        itemRV->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("red")));
      }
      // Blue if changed some time but written
      else if (m_userregs.hasWrittenChange(offset, page)) {
        itemRV->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("royalblue")));
      }
      // Black if never changed
      else {
        itemRV->setForeground(REMOTEVAR_COL_VALUE, QBrush(QColor("black")));
      }
    }
    ++it;
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateChangeDM
//

void
CFrmNodeConfig::updateChangeDM(uint32_t offset, uint16_t page, bool bFromRegUpdate)
{
  // Check if register is use by the DM
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Should be the same page
  if (page != m_mdf.getDM()->getStartPage()) {
    return;
  }

  // Find the change positions in the DM
  int row = (offset - m_mdf.getDM()->getStartOffset()) / m_mdf.getDM()->getRowSize();
  int pos = ((offset - m_mdf.getDM()->getStartOffset()) + row * m_mdf.getDM()->getRowSize()) % m_mdf.getDM()->getRowSize();
  //std::cout << "Change DM: " << offset << " " << page << " " << row << " " << pos << std::endl;

  // offset is in the range of the DM
  if ((offset < m_mdf.getDM()->getStartOffset()) ||
      (offset > (m_mdf.getDM()->getStartOffset() + row * m_mdf.getDM()->getRowSize() + 8))) {
    return;
  }

  CDMWidgetItem* itemDM = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->topLevelItem(row);
  if (NULL == itemDM) {
    return;
  }

  // Write value
  itemDM->setText(pos,
                  pworks->decimalToStringInBase(m_userregs.getReg(offset, page),
                                                m_baseComboBox->currentIndex())
                    .toStdString()
                    .c_str());

  // If marked as changed: red
  if (m_userregs.isChanged(offset, page)) {
    itemDM->setForeground(pos, QBrush(QColor("red")));
  }
  // If changed but already written: blue
  else if (m_userregs.hasWrittenChange(offset, page)) {
    itemDM->setForeground(pos, QBrush(QColor("royalblue")));
  }
  // black
  else {
    itemDM->setForeground(pos, QBrush(QColor("black")));
  }
}


///////////////////////////////////////////////////////////////////////////////
// onMainTabBarChanged
//

void
CFrmNodeConfig::onMainTabBarChanged(int index)
{
  // save previous info area
  if (m_saveInfoArea[index].length()) {
    // Set new info areatext
    ui->infoArea->setHtml(m_saveInfoArea[index].c_str());
  }
  else {
    fillDeviceHtmlInfo();
    // switch (index) {
    //   case TABBAR_INDEX_REMOTEVARS:
    //     ;
    //     break;
    //   case TABBAR_INDEX_DM:
    //     ;
    //     break;
    //   case TABBAR_INDEX_FILES:
    //     ;
    //     break;
    //   case TABBAR_INDEX_REGISTERS:
    //   default:
    //     ;
    //     break;
    // }
  }
}

///////////////////////////////////////////////////////////////////////////////
// onMainTabBarClicked
//

void
CFrmNodeConfig::onMainTabBarClicked(int index)
{
  // Get current index
  int current_index = ui->session_tabWidget->currentIndex();
  if (-1 == current_index) return;
  
  // Save content of info area
  m_saveInfoArea[current_index] = ui->infoArea->toHtml().toStdString();
}