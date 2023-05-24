// cfrmnodeconfig.cpp
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
#ifndef WIN32
#include <vscp_client_socketcan.h>
#endif
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

#include <string>
#include <fstream>
#include <iostream>

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
#include <QInputDialog>

#include <expat.h>
#include <json.hpp>         // Needs C++11  -std=c++11
#include <mustache.hpp>
#include <maddy/parser.h>   // Markdown -> HTML

#define XML_BUFF_SIZE 0xffff

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

CMdfFileWidgetItem::CMdfFileWidgetItem(const QString& text, int type)
  : QTreeWidgetItem(type)
{
  m_mdfFileType = mdf_file_type_none;

  m_picture_obj = nullptr;
  m_video_obj = nullptr;             
  m_firmware_obj = nullptr;
  m_driver_obj = nullptr;              
  m_manual_obj = nullptr;             
  m_setup_obj = nullptr; 
}

CMdfFileWidgetItem::~CMdfFileWidgetItem()
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
  pworks->newChildWindow(this);

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
    QApplication::beep();
    spdlog::error(std::string(tr("Type is not define in JSON data").toStdString()));
    QMessageBox::information(this,
                             tr(APPNAME),
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
  std::string interface = m_connObject["selected-interface"].toString().toStdString();

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

        std::string _str;
        size_t sz = json_if_array.size();
        foreach (const QJsonValue& value, json_if_array) {
          m_comboInterface->addItem(value.toObject().value("if-item").toString());
        }

        m_comboInterface->setCurrentText(interface.c_str());
        //std::cout << "interface = " << interface << std::endl;

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

#ifndef WIN32
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
#endif

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

  // Load registers
  connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadRegisterValues()));

  // Save registers
  connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveAllRegisterValues()));

  // Save selected registers
  connect(ui->actionSelectedSave, SIGNAL(triggered()), this, SLOT(saveSelectedRegisterValues()));

  // Load defaults to all registers
  connect(ui->actionLoadDefaultsAll, SIGNAL(triggered()), this, SLOT(loadDefaults()));

  // Load defaults to selected registers
  connect(ui->actionLoadDefaultsSelected, SIGNAL(triggered()), this, SLOT(loadDefaultsSelected()));

  // Navigation: Goto register page
  connect(ui->actionGoto_register_page,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage()));

  // Navigation: Goto register on register page
  connect(ui->actionGoto_page_register,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterOnPage()));  

  // Navigation: Goto selected DM or remote variable register
  connect(ui->actionGoto_related_register,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterForSelected()));

  // Navigation: Goto register page 0
  connect(ui->actionGoto_page_0,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage0()));

  // Navigation: Goto register page 1
  connect(ui->actionGoto_page_1,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage1()));

  // Navigation: Goto register page 2
  connect(ui->actionGoto_page_2,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage2()));                    

  // Navigation: Goto register page 3
  connect(ui->actionGoto_page_3,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage3()));        

  // Navigation: Goto register page 4
  connect(ui->actionGoto_page_4,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage4()));

  // Navigation: Goto register page 5
  connect(ui->actionGoto_page_5,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage5()));

  // Navigation: Goto register page 6
  connect(ui->actionGoto_page_6,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage6()));

  // Navigation: Goto register page 7
  connect(ui->actionGoto_page_7,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage7()));

  // Navigation: Goto register page 8
  connect(ui->actionGoto_page_8,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage8()));

  // Navigation: Goto register page 9
  connect(ui->actionGoto_page_9,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPage9()));

  // Edit selected DM row
  QAction *actEditDM = new QAction(tr("Edit DM row"), this);  
  actEditDM->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));                                                                         
  connect(actEditDM, &QAction::triggered, this, &CFrmNodeConfig::editDMRow);
  ui->treeWidgetDecisionMatrix->addAction(actEditDM); 

  // Toggle active state for selected DM row
  QAction *actToggleDM = new QAction(tr("Toggle active state for DM row"), this);  
  actToggleDM->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));                                                                         
  connect(actToggleDM, &QAction::triggered, this, &CFrmNodeConfig::toggleDMRow);
  ui->treeWidgetDecisionMatrix->addAction(actToggleDM); 

  // Navigation: Goto standard registers register page
  connect(ui->actionGoto_standard_registers,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPageStdReg()));       

  // Navigation: Goto DM register page
  connect(ui->actionGoto_decsion_matrix,
          SIGNAL(triggered()),
          this,
          SLOT(gotoRegisterPageDM()));      

  // Navigation: Collapse all register top levels
  connect(ui->actionCollapse_registers,
          SIGNAL(triggered()),
          this,
          SLOT(collapseAllRegisterTopItems()));  

  // Navigation: Collapse all file top levels
  connect(ui->actionCollapse_files,
          SIGNAL(triggered()),
          this,
          SLOT(collapseAllFileTopItems()));                         

  // Go to page
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

  // Register row has been clicked.
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
          &CFrmNodeConfig::showMdfFilesContextMenu);

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

  // DM row has been clicked.
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::itemClicked,
          this,
          &CFrmNodeConfig::onDMTreeWidgetItemClicked);

  // DM row has been double clicked.
  connect(ui->treeWidgetDecisionMatrix,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmNodeConfig::onDMTreeWidgetItemDoubleClicked);

  // MDF Files

  // MDF file item has been clicked.
  connect(ui->treeWidgetMdfFiles,
          &QTreeWidget::itemClicked,
          this,
          &CFrmNodeConfig::onMdfFileTreeWidgetItemClicked);  

  // MDF file item has been double clicked.
  connect(ui->treeWidgetMdfFiles,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmNodeConfig::onMdfFileTreeWidgetItemDoubleClicked);                

  // m_shortcut_info = new QShortcut(QKeySequence(tr("Ctrl+I")), this);
  // connect(m_shortcut_info,
  //           &QShortcut::activated,
  //           this,
  //           &CFrmNodeConfig::fillDeviceHtmlInfo);

  // MDF file item has been clicked
  connect(ui->actionShowMdfInfo,
          &QAction::triggered,
          this,
          &CFrmNodeConfig::fillDeviceHtmlInfo);

  // MDF file item has been double clicked          
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmNodeConfig::~CFrmNodeConfig()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Make sure we are disconnected
  doDisconnectFromRemoteHost();
 
  pworks->clearChildWindow(this);
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
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        QApplication::beep();
        spdlog::error(std::string(tr("Session: Unable to connect to remote host.").toStdString()));
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
      }
      else {
        spdlog::info(std::string(tr("Session: Successful connect to remote client.").toStdString()));
        ui->actionConnect->setChecked(true);
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to connect to the CANAL driver. rv=");
        str += rv;        
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
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
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to connect to the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
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
                                 tr(APPNAME),
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
                                  tr(APPNAME),
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
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to disconnect from the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                  tr(APPNAME),
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
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
        QApplication::beep();
        QString str = tr("Session: Unable to disconnect from the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(
          this,
          tr(APPNAME),
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
                                  tr(APPNAME),
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

      // First read: Read in and render all registers
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
  QApplication::processEvents();

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
  QApplication::processEvents();

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Show a progress bar
  QProgressBar* pbar = new QProgressBar(this);
  ui->statusBar->addWidget(pbar);
  pbar->setMaximum(100);
  QApplication::processEvents();

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

  pbar->setValue(25);
  QApplication::processEvents();

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
  QApplication::processEvents();

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
  QApplication::processEvents();

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
  QApplication::processEvents();

  QApplication::restoreOverrideCursor();
  ui->statusBar->removeWidget(pbar);
  QApplication::processEvents();

  return VSCP_ERROR_SUCCESS;
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



// ****************************************************************************
//                   * * *  Register handling * * *
// ****************************************************************************



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
  QApplication::processEvents();

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
  QApplication::processEvents();

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
// collapseAllRegisterTopItems
//

void
CFrmNodeConfig::collapseAllRegisterTopItems(void)
{
  ui->treeWidgetRegisters->collapseAll();
}

///////////////////////////////////////////////////////////////////////////////
// collapseAllFileTopItems
//

void
CFrmNodeConfig::collapseAllFileTopItems(void)
{
  ui->treeWidgetMdfFiles->collapseAll();
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage
//

void
CFrmNodeConfig::gotoRegisterPage(int page)
{
  // Collapse all
  collapseAllRegisterTopItems();

  // Unselect all
  ui->treeWidgetRegisters->selectionModel()->clearSelection();

  if (!m_stdregs.getMDF().size()) {
    QMessageBox::information(this, tr(APPNAME), tr("No register pages"), QMessageBox::Ok);  
    return;
  }

  // if page is -1 open choice dialog
  if (-1 == page) {

    bool ok;

    QStringList items;
    items << tr("Standard register page"); 
    std::set<long> *ppages = m_userregs.getPages();
    for (auto it = ppages->begin(); it != ppages->end(); ++it) {
      items << tr("Register page ") + QString::number(*it);
    }
    
    QString strpage = QInputDialog::getItem(this, tr("Select register page"), tr("Register page:"), 
                                              items, 0, false, &ok);
    if (ok && !items.isEmpty()) {

      // Go to register tab
      ui->session_tabWidget->setCurrentIndex(0);

      if (strpage == tr("Standard register page")) {
        m_StandardRegTopPage->setSelected(true);
        m_StandardRegTopPage->setExpanded(true);  
      }
      else {
        std::string str = strpage.right(strpage.size()-13)  .toStdString();
        page = vscp_readStringValue(str);
        m_mapRegTopPages[page]->setSelected(true);
        m_mapRegTopPages[page]->setExpanded(true);
        ui->treeWidgetRegisters->scrollToItem(m_mapRegTopPages[page], QAbstractItemView::PositionAtTop); // QAbstractItemView::EnsureVisible
      }
    }
    else {
      return;
    }
  }
  else {
    // Must be a valid page
    std::set<long> *ppages = m_userregs.getPages();
    auto search = ppages->find(page);
    if (search == ppages->end()) {
      QMessageBox::information(this, tr(APPNAME), tr("Register page is not valid"), QMessageBox::Ok); 
      return;
    }

    // Go to register tab
    ui->session_tabWidget->setCurrentIndex(0);
    m_mapRegTopPages[page]->setSelected(true);
    m_mapRegTopPages[page]->setExpanded(true);
    ui->treeWidgetRegisters->scrollToItem(m_mapRegTopPages[page], QAbstractItemView::PositionAtTop); // QAbstractItemView::EnsureVisible
  }
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage1
//

void
CFrmNodeConfig::gotoRegisterPage1(void)
{
  gotoRegisterPage(1);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage2
//

void
CFrmNodeConfig::gotoRegisterPage2(void)
{
  gotoRegisterPage(2);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage3
//

void
CFrmNodeConfig::gotoRegisterPage3(void)
{
  gotoRegisterPage(3);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage4
//

void
CFrmNodeConfig::gotoRegisterPage4(void)
{
  gotoRegisterPage(4);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage5
//

void
CFrmNodeConfig::gotoRegisterPage5(void)
{
  gotoRegisterPage(5);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage6
//

void
CFrmNodeConfig::gotoRegisterPage6(void)
{
  gotoRegisterPage(6);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage7
//

void
CFrmNodeConfig::gotoRegisterPage7(void)
{
  gotoRegisterPage(7);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage8
//

void
CFrmNodeConfig::gotoRegisterPage8(void)
{
  gotoRegisterPage(8);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage9
//

void
CFrmNodeConfig::gotoRegisterPage9(void)
{
  gotoRegisterPage(9);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterOnPage
//

void
CFrmNodeConfig::gotoRegisterOnPage(int page, int reg)
{
  // Collapse all
  collapseAllRegisterTopItems();

  // Unselect all
  ui->treeWidgetRegisters->selectionModel()->clearSelection();

  if (!m_stdregs.getMDF().size()) {
    QMessageBox::information(this, tr(APPNAME), tr("No register pages"), QMessageBox::Ok);  
    return;
  }

  // Must be a valid page
  if (-1 != page) {
    std::set<long> *ppages = m_userregs.getPages();
    auto search = ppages->find(page);
    if (search == ppages->end()) {
      QMessageBox::information(this, tr(APPNAME), tr("Register page is not valid"), QMessageBox::Ok); 
      return;
    }
  }

  gotoRegisterPage(page);

  if (-1 == reg) {
    bool ok;
    reg = QInputDialog::getInt(this, tr("Select register"),
                                          tr("Register:"),
                                          0,0,2147483647,1, &ok);
    if (!ok) {                                          
      return;
    }
  }

  QList<QTreeWidgetItem *> selected =	ui->treeWidgetRegisters->selectedItems();
  if (!selected.size() ) {
    QMessageBox::information(this, tr(APPNAME), tr("No register page item selected (it should be!)"), QMessageBox::Ok); 
    return;
  }

  for (int i=0; i<selected[0]->childCount(); i++) {
    QTreeWidgetItem* item = selected[0]->child(i);
    if (item->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
      if (itemReg->m_regOffset == reg) {
        itemReg->setSelected(true);
        itemReg->setExpanded(true);
        ui->treeWidgetRegisters->scrollToItem(item, QAbstractItemView::PositionAtTop); // QAbstractItemView::EnsureVisible
        onRegisterTreeWidgetItemClicked(itemReg, 0);
        break;
      }
    }    
  }
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterForSelected
//

void
CFrmNodeConfig::gotoRegisterForSelected(void)
{
  switch (ui->session_tabWidget->currentIndex()) {
    
    case TABBAR_INDEX_REMOTEVARS:
      gotoRemoteVarRegisterPos();
      break;

    case TABBAR_INDEX_DM:
      gotoDMRegisterPos();
      break; 
       
  }
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPage0
//

void
CFrmNodeConfig::gotoRegisterPage0(void)
{
  // Collapse all
  collapseAllRegisterTopItems();

  gotoRegisterPage(0);
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPageStdReg
//

void
CFrmNodeConfig::gotoRegisterPageStdReg(void)
{
  // Collapse all
  collapseAllRegisterTopItems();

  ui->session_tabWidget->setCurrentIndex(0);
  if (nullptr != m_StandardRegTopPage) {
    m_StandardRegTopPage->setSelected(true);
    m_StandardRegTopPage->setExpanded(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// gotoRegisterPageDM
//

void
CFrmNodeConfig::gotoRegisterPageDM(int row)
{
  // Collapse all
  collapseAllRegisterTopItems();

  QTreeWidgetItemIterator it(ui->treeWidgetDecisionMatrix);
  while (*it) {
    CDMWidgetItem *pdmItem = (CDMWidgetItem *)*it;
    if ((nullptr != pdmItem) && (nullptr != pdmItem->m_pDM) && (pdmItem->m_row == row)) {
       gotoRegisterOnPage(pdmItem->m_pDM->getStartPage(), pdmItem->m_pDM->getStartOffset());
       break;
    }
    **it;
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveSelectedRegisterValues
//

void
CFrmNodeConfig::saveSelectedRegisterValues(void)
{
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
  saveRegisterValues(pworks->m_bSaveAlwaysJSON, false);
}

///////////////////////////////////////////////////////////////////////////////
// saveAllRegisterValues
//

void
CFrmNodeConfig::saveAllRegisterValues(void)
{
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
  saveRegisterValues(pworks->m_bSaveAlwaysJSON, true);
}

///////////////////////////////////////////////////////////////////////////////
// writeRegisterRecord
//

void
CFrmNodeConfig::writeRegisterRecord(QFile &file, const CRegisterWidgetItem* itemReg, bool bJSON)
{
  if (nullptr == itemReg) {
    return;
  }

  uint8_t value                = vscp_readStringValue(itemReg->text(REG_COL_VALUE).toStdString());
  QString strName              = itemReg->text(REG_COL_NAME);

  if (bJSON) {
    file.write(QString("\t\t{\n").toUtf8());
    file.write(QString("\t\t\t\"page\": ").toUtf8());
    file.write(QString::number(itemReg->m_regPage).toUtf8());
    file.write(QString(",\n\t\t\t\"offset\": ").toUtf8());
    file.write(QString::number(itemReg->m_regOffset).toUtf8());
    file.write(QString(",\n\t\t\t\"value\": ").toUtf8());
    file.write(QString::number(value).toUtf8());
    file.write(QString(",\n\t\t\t\"name\": \"").toUtf8());
    file.write(strName.toUtf8());
    file.write(QString("\"\n\t\t}").toUtf8());
  }
  else {
    
    file.write(QString("<reg page=\"").toUtf8());
    file.write(QString::number(itemReg->m_regPage).toUtf8());
    file.write(QString("\" offset=\"").toUtf8());
    file.write(QString::number(itemReg->m_regOffset).toUtf8());
    file.write(QString("\" value=\"").toUtf8());
    file.write(QString::number(value).toUtf8());
    file.write(QString("\" name=\"").toUtf8());
    file.write(strName.toUtf8());
    file.write(QString("\" />\n").toUtf8());    
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveRegisterValues
//

void
CFrmNodeConfig::saveRegisterValues(bool bJSON, bool bAll)
{
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

  // Check if we should save on JSON format
  if (!bJSON) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr(APPNAME), "Save in JSON format?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      bJSON = true;
    }   
  }

  QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save registers to file"),
                                    /*"~/.vscpworks/device-registers.reg"*/"/tmp/device-registers.reg",
                                    tr("Register Files (*.reg);;XML Files (*.xml);;JSON Files (*.json);;All Files (*.*)"));
  //std::cout << "Filename: |" << fileName.toStdString() << "|" << std::endl;
  if (fileName.isEmpty()) {
    return;
  }

  // Open the file
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly)) {
    QMessageBox::information(this, tr(APPNAME),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
    return;
  }

  // Check if we should save on JSON format
  if (bJSON) {

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();    

    file.write(QString("{\n").toUtf8());

    // Write identification data for module

    file.write(QString("\t\"module-name\":\"").toUtf8());
    file.write(QString(m_mdf.getModuleName().c_str()).toUtf8());
    file.write(QString("\",\n").toUtf8());

    file.write(QString("\t\"module-model\":\"").toUtf8());
    file.write(QString(m_mdf.getModuleModel().c_str()).toUtf8());
    file.write(QString("\",\n").toUtf8());

    file.write(QString("\t\"module-version\":\"").toUtf8());
    file.write(QString(m_mdf.getModuleVersion().c_str()).toUtf8());
    file.write(QString("\",\n").toUtf8());

    file.write(QString("\t\"module-date\":\"").toUtf8());
    file.write(QString(m_mdf.getModuleChangeDate().c_str()).toUtf8());
    file.write(QString("\",\n").toUtf8());

    // Write registers

    file.write(QString("\t\"registers\": [\n").toUtf8());

    if (bAll) {
      // Write all registers
      QTreeWidgetItemIterator item(ui->treeWidgetRegisters);
      bool bFirst = true;
      while (*item) {
        if ((*item)->type() == TREE_LIST_REGISTER_TYPE) {
          CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*item);
          if (bFirst) {
            bFirst = false;
          }
          else {  
            file.write(QString(",\n").toUtf8());
          }
          writeRegisterRecord(file, itemReg, true);
        }
        ++item;
      }
    }
    else {

      // Write selected registers
      QList<QTreeWidgetItem*> listSelected = ui->treeWidgetRegisters->selectedItems();
      // TODO: sort option

      bool bFirst = true;
      for (auto item : listSelected) {
        if (item->type() == TREE_LIST_REGISTER_TYPE) {
          CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
          if (bFirst) {
            bFirst = false;
          }
          else {  
            file.write(QString(",\n").toUtf8());
          }
          writeRegisterRecord(file, itemReg, true);
        }
        else {
          // Save all children on page
          for (int i=0; i<item->childCount(); i++) {
            CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item->child(i);
            writeRegisterRecord(file, itemReg, true);
          }
        }
      }
    }

    file.write(QString("\n\t]\n").toUtf8());
    file.write(QString("}\n").toUtf8()); 

    QApplication::restoreOverrideCursor();  
    QApplication::processEvents(); 

    ui->statusBar->showMessage(tr("Saved registers."));
  }
  else {

    // XML format

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents(); 

    file.write(QString("<?xml version = \"1.0\" encoding =  \"UTF-8\" ?>\n").toUtf8());


    file.write(QString("<registerset ").toUtf8());

    // Save module info
    file.write(QString("module-name=\"").toUtf8());
    file.write(QString(m_mdf.getModuleName().c_str()).toUtf8());
    file.write(QString("\" ").toUtf8());

    file.write(QString("module-model=\"").toUtf8());
    file.write(QString(m_mdf.getModuleModel().c_str()).toUtf8());
    file.write(QString("\" ").toUtf8());

    file.write(QString("module-version=\"").toUtf8());
    file.write(QString(m_mdf.getModuleVersion().c_str()).toUtf8());
    file.write(QString("\" ").toUtf8());

    file.write(QString("module-date=\"").toUtf8());
    file.write(QString(m_mdf.getModuleChangeDate().c_str()).toUtf8());
    file.write(QString("\" ").toUtf8());


    file.write(QString(">\n").toUtf8());

    if (bAll) {
      // Write all registers
      QTreeWidgetItemIterator item(ui->treeWidgetRegisters);
      while (*item) {
        if ((*item)->type() == TREE_LIST_REGISTER_TYPE) {
          CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*item);
          writeRegisterRecord(file, itemReg, false);
        }
        ++item;
      }
    }
    else {
      // Write selected registers
      QList<QTreeWidgetItem*> listSelected = ui->treeWidgetRegisters->selectedItems();
      // TODO: sort option

      for (auto item : listSelected) {
        if (item->type() == TREE_LIST_REGISTER_TYPE) {
          CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
          writeRegisterRecord(file, itemReg, false);
        }
        else {
          // Save all children on page
          for (int i=0; i<item->childCount(); i++) {
            CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item->child(i);
            writeRegisterRecord(file, itemReg, false);
          }
        }
      }
    }

    file.write(QString("</registerset>\n").toUtf8());

    QApplication::restoreOverrideCursor();  
    QApplication::processEvents();

    ui->statusBar->showMessage(tr("Saved registers."));
  }
}

// ----------------------------------------------------------------------------
//                             XML register parser 
// ----------------------------------------------------------------------------

struct __xml_register_struct__ {
  uint8_t page;
  uint8_t offset;
  uint8_t value;
};

struct __xml_parser_struct__ {
  
  // Parser variables
  int depth_xml_parser;
  std::deque<std::string> tokenList;

  // Parser errors
  uint16_t errors;
  std::string errorStr;

  // Holds found register writes for later handling
  std::deque<__xml_register_struct__ *> registerList;

  // Temporary values
  uint8_t page;
  uint8_t offset;
  uint8_t value;

  // Module information
  std::string moduleName;
  std::string moduleModel;
  std::string moduleVersion;
  std::string moduleChangeDate;
};

static void
__startSetupRegisterParser(void *data, const char *name, const char **attr)
{
  __xml_parser_struct__ *parsestruct = (__xml_parser_struct__ *) data;
  if (nullptr == parsestruct) {
    spdlog::trace("Parse-XML: ---> startSetupMDFParser: Data object is invalid");
    return;
  }

  spdlog::trace("Parse-XML: <--- startSetupMDFParser: Tag: {0} Depth: {1}", name, parsestruct->depth_xml_parser);

  // Save token
  std::string currentToken = name;
  vscp_trim(currentToken);
  vscp_makeLower(currentToken);
  parsestruct->tokenList.push_front(currentToken);

  switch (parsestruct->depth_xml_parser) {
    
    case 0:
      if (currentToken == "registerset") {
        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          if (0 == strcasecmp(attr[i], "module-name")) {
            if (!attribute.empty()) {
              parsestruct->moduleName = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "module-model")) {
            if (!attribute.empty()) {
              parsestruct->moduleModel = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "module-version")) {
            if (!attribute.empty()) {
              parsestruct->moduleVersion = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "module-date")) {
            if (!attribute.empty()) {
              parsestruct->moduleChangeDate = attribute;
            }
          }
        }
      }
      break;

    case 1:
      if (((currentToken == "reg") || (currentToken == "register")) && 
            (parsestruct->tokenList.back() == "registerset")) {

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);
          if (0 == strcasecmp(attr[i], "page")) {
            if (!attribute.empty()) {
              parsestruct->page = vscp_readStringValue(attribute);
            }
          }
          else if (0 == strcasecmp(attr[i], "offset")) {
            if (!attribute.empty()) {
              parsestruct->offset = vscp_readStringValue(attribute);
            }
          }
          else if (0 == strcasecmp(attr[i], "value")) {
            if (!attribute.empty()) {
              parsestruct->value = vscp_readStringValue(attribute);
            }
          }
        }
      }
      break;

    // Old form  
    // <registerset>
    // <reg page="0" offset="2" >
    //   <value>0</value>
    //   <description>sub zone i/o 0</name>
    // </registerset>
    case 2: // value or description
      break; 

  }

  parsestruct->depth_xml_parser++;
}


static void
__handleRegisterParserData(void *data, const XML_Char *content, int length)
{
  // Get the pointer to the CMDF object
  __xml_parser_struct__ *parsestruct = (__xml_parser_struct__ *) data;
  if (nullptr == parsestruct) {
    spdlog::error("Parse-XML: ---> handleMDFParserData: Data object is invalid");
    return;
  }

  // Must be some content to work on
  if (!content) {
    spdlog::error("Parse-RegisterXML: ---> handleMDFParserData: No content");
    parsestruct->errors++;
    parsestruct->errorStr += "Null length data content in XML file\n";
    return;
  }

  std::string strContent = std::string(content, length);
  vscp_trim(strContent);
  if (strContent.empty()) {
    return;
  }

  // No use to work without the <registerset> tag
  if (!(parsestruct->tokenList.back() == "registerset")) {
    spdlog::error("Parse-RegisterXML: ---> handleMDFParserData: No registerset tag");
    parsestruct->errors++;
    parsestruct->errorStr += "Syntax error in XML file (no <registerset> tag)\n";
    return;
  }

  // Old form has value and description here
  if (3 == parsestruct->depth_xml_parser) {
    // Get value
    if (parsestruct->tokenList.front() == "value") {
      parsestruct->value = vscp_readStringValue(strContent);
    }
  }

}
 
 static void
__endSetupRegisterParser(void *data, const char *name)
{
  // Get the pointer to the CMDF object
  __xml_parser_struct__ *parsestruct = (__xml_parser_struct__ *) data;
  if (nullptr == parsestruct) {
    spdlog::trace("Parse-RegisterXML: ---> endSetupMDFParser: Data object is invalid");
    return;
  }

  spdlog::trace("Parse-RegisterXML: ---> End: Tag: {0} Depth: {1}", name, parsestruct->depth_xml_parser);

  std::string currentToken = name;
  vscp_trim(currentToken);
  vscp_makeLower(currentToken);

  switch (parsestruct->depth_xml_parser) {

    case 2:  // reg/register
      if ((currentToken == "reg") || (currentToken == "register")) {
        // Save for later handling
        __xml_register_struct__ *preg = new __xml_register_struct__;
        if (nullptr == preg) {
          spdlog::error("Parse-RegisterXML: ---> endSetupMDFParser: Failed to allocate memory");
          parsestruct->errors++;
          parsestruct->errorStr += "Failed to allocate memory for register structure\n";
          return;
        }
        preg->page = parsestruct->page;
        preg->offset = parsestruct->offset;
        preg->value = parsestruct->value;
        parsestruct->registerList.push_back(preg);
      }
      break;
  }

  parsestruct->depth_xml_parser--;
}


///////////////////////////////////////////////////////////////////////////////
// loadXMLRegs
//

int 
CFrmNodeConfig::loadXMLRegs(const std::string &path)
{
  int rv = VSCP_ERROR_SUCCESS;
  std::ifstream ifs;
  __xml_parser_struct__ parsestruct;
  parsestruct.errors = 0;
  parsestruct.depth_xml_parser = 0;

  try {
    ifs.open(path, std::ifstream::in);
  }
  catch (...) {
    spdlog::error("Parse-RegisterXML: Failed to open register XML file.");
    ui->statusBar->showMessage(tr("Failed to open register XML file %1.").arg(path.c_str()));
    return VSCP_ERROR_PARSING;
  }

  XML_Parser xmlParser = XML_ParserCreate("UTF-8");
  XML_SetUserData(xmlParser, &parsestruct);
  XML_SetElementHandler(xmlParser, __startSetupRegisterParser, __endSetupRegisterParser);
  XML_SetCharacterDataHandler(xmlParser, __handleRegisterParserData);

  int bytes_read;
  void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

  while (ifs.good()) {
    ifs.read((char *) buf, XML_BUFF_SIZE);
    bytes_read = ifs.gcount();
    if (bytes_read > 0) {
      if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
        spdlog::error("Parse-RegisterXML: Failed parse register XML file at line {0} [{1}].",
                      XML_GetCurrentLineNumber(xmlParser),
                      XML_ErrorString(XML_GetErrorCode(xmlParser)));
        ui->statusBar->showMessage(tr("Loaded registers failed due to parser error."));              
        rv = VSCP_ERROR_PARSING;
        break;
      }
    }
  }

  XML_ParserFree(xmlParser);

  if (parsestruct.errors) {
    if (QMessageBox::Yes != QMessageBox::question(this, "XML Parser register load", "There was errors during load, continue anyway?")) {
      ui->statusBar->showMessage(tr("Aborted loaded %1 registers due to errors (errors = %1).").arg(parsestruct.errors));
      return VSCP_ERROR_SUCCESS;
    }
  }

  if (parsestruct.moduleName.size() && (parsestruct.moduleName != m_mdf.getModuleName())) {
    int rv = QMessageBox::warning(this, 
                tr(APPNAME), tr("Module name does not match. Continue anyway?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (rv == QMessageBox::No) {
      return false;
    }
  }

  if (parsestruct.moduleModel.size() && (parsestruct.moduleModel != m_mdf.getModuleModel())) {
    int rv = QMessageBox::warning(this, 
                tr(APPNAME), tr("Module model does not match. Continue anyway?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (rv == QMessageBox::No) {
      return false;
    }
  }

  if (parsestruct.moduleVersion.size() && (parsestruct.moduleVersion != m_mdf.getModuleVersion())) {
    int rv = QMessageBox::warning(this, 
                tr(APPNAME), tr("Module version does not match. Continue anyway?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (rv == QMessageBox::No) {
      return false;
    }
  }

  // If model information is missing warn about it
  if (!parsestruct.moduleName.size() && !parsestruct.moduleModel.size() && !parsestruct.moduleVersion.size()) {
    int rv = QMessageBox::warning(this, 
                  tr(APPNAME), tr("There is no module information in the register file. Continue anyway?"),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (rv == QMessageBox::No) {
        return false;
      }
  }

  // Write to registers
  size_t regcnt = parsestruct.registerList.size();
  uint16_t regskipped = 0;  // Regs skipped
  for (auto &reg : parsestruct.registerList) {

    // Invalid level I register offsets (standard registers)
    if (reg->offset >= 0x80) {
      regskipped++;
      spdlog::info("Parse-JSON registers: Offset ({0}:{1}) is out of range for a level I device. Skipped", reg->page, reg->offset);
      continue;
    }

    if (m_mdf.isRegisterWriteable(reg->offset, reg->page)) {
      parsestruct.errors++;
      spdlog::error("Parse-JSON registers: Register is not writeable. {0}:{1}", reg->page, reg->offset);
      parsestruct.errorStr += tr("Register is not writeable  %1:%2 = %3\n").arg(reg->page).arg(reg->offset).arg(reg->value).toStdString();
      continue;
    }

    if (!m_userregs.putReg(reg->offset, reg->page, reg->value)) {
      spdlog::error("Parse-RegisterXML: Failed to write register {0}:{1}.", reg->page, reg->offset);
      rv = VSCP_ERROR_PARSING;
      parsestruct.errors++;
      parsestruct.errorStr += tr("Failed to write register  %1:%2 = %3\n").arg(reg->page).arg(reg->offset).arg(reg->value).toStdString();
    }
    delete reg;
  }

  ui->statusBar->showMessage(tr("Loaded %1 registers, %2 skipped (errors = %3).").arg(regcnt).arg(regskipped).arg(parsestruct.errors));
  updateVisualRegisters();

  if (parsestruct.errors) {
    spdlog::info(parsestruct.errorStr);
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// loadJSONRegs
//

int 
CFrmNodeConfig::loadJSONRegs(const std::string &path)
{
  int rv = VSCP_ERROR_SUCCESS;
  json j;
  uint16_t regcnt = 0;      // Regs written
  uint16_t regskipped = 0;  // Regs skipped
  // Module information
  std::string moduleName;
  std::string moduleModel;
  std::string moduleVersion;
  std::string moduleChangeDate;

  try {
    std::ifstream ifs(path, std::ifstream::in);
    ifs >> j;
    ifs.close();
  }
  catch (...) {
    spdlog::error("Parse-JSON: Failed to load and  parse JSON register file.");
    QMessageBox::warning(this, 
                  tr(APPNAME), tr("Failed to load and parse JSON register file %1.").arg(path.c_str()),
                  QMessageBox::Yes);
    return false;
  }

  if (j.contains("module-name") && j["module-name"].is_string()) {
    moduleName = j["module-name"];
    if (moduleName.size() && (moduleName != m_mdf.getModuleName())) {
      int rv = QMessageBox::warning(this, 
                  tr(APPNAME), tr("Module name does not match. Continue anyway?"),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (rv == QMessageBox::No) {
        return false;
      }
    }
  }

  if (j.contains("module-model") && j["module-model"].is_string()) {
    moduleModel = j["module-model"];
    if (moduleModel.size() && (moduleModel != m_mdf.getModuleModel())) {
      int rv = QMessageBox::warning(this, 
                  tr(APPNAME), tr("Module model does not match. Continue anyway?"),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (rv == QMessageBox::No) {
        return false;
      }
    }
  }

  if (j.contains("module-version") && j["module-version"].is_string()) {
    moduleVersion = j["module-version"];
    if (moduleVersion.size() && (moduleVersion != m_mdf.getModuleVersion())) {
      int rv = QMessageBox::warning(this, 
                  tr(APPNAME), tr("Module version does not match. Continue anyway?"),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (rv == QMessageBox::No) {
        return false;
      }
    }
  }

  if (j.contains("module-date") && j["module-date"].is_string()) {
    moduleChangeDate = j["module-date"];
  }

  // If model information is missing warn about it
  if (!moduleName.size() && !moduleModel.size() && !moduleVersion.size()) {
    int rv = QMessageBox::warning(this, 
                  tr(APPNAME), tr("There is no module information in the register file. Continue anyway?"),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (rv == QMessageBox::No) {
        return false;
      }
  }

  if (!(j.contains("registers") && j["registers"].is_array())) {
    spdlog::error("Parse-JSON registers: module info is not found. <<{}>>", j.dump());
    ui->statusBar->showMessage(tr("JSON register load aborted: Parse-JSON registers: module info is not found.").arg(path.c_str()));
    return VSCP_ERROR_PARSING;
  }
  else {
    for (const auto &item : j["registers"].items()) {

      if (item.value().is_object()) {

        json jreg(item.value());

        // name is optional, page, offset, value must be present
        if (!(jreg.contains("page") && jreg.contains("offset") && jreg.contains("value"))) {
          spdlog::error("Parse-JSON registers: module info is not found. <<{}>>", jreg.dump());
          ui->statusBar->showMessage(tr("JSON register load aborted: Format of JSON file is not correct."));
          return VSCP_ERROR_PARSING;
        }
        else {

          int page = jreg["page"];
          int offset = jreg["offset"];
          int value = jreg["value"];

          if (offset >= 128) {
            regskipped++;
            spdlog::info("Parse-JSON registers: Offset ({0}:{1}) is out of range for a level I device. Skipped", page, offset);
            continue;
          }

          regcnt++;          

          if (page < 0 || page > 0xffff) {
            spdlog::error("Parse-JSON registers: page is out of range. <<{}>>", jreg.dump());
            ui->statusBar->showMessage(tr("JSON register load aborted: 'page' parameter is out of range."));
            return VSCP_ERROR_PARSING;
          }
          if (offset < 0 || offset > 255) {
            spdlog::error("Parse-JSON registers: register is out of range. <<{}>>", jreg.dump());
            ui->statusBar->showMessage(tr("JSON register load aborted: 'offset' parameter is out of range."));
            return VSCP_ERROR_PARSING;
          }
          if (value < 0 || value > 255) {
            spdlog::error("Parse-JSON registers: value is out of range. <<{}>>", jreg.dump());
            ui->statusBar->showMessage(tr("JSON register load aborted: 'value' parameter is out of range."));
            return VSCP_ERROR_PARSING;
          }
          
          if (m_mdf.isRegisterWriteable(offset,page)) {
            regskipped++;
            spdlog::error("Parse-JSON registers: Register is not writeable. {0}:{1}", page, offset);
            continue;
          }

          if (!m_userregs.putReg(offset, page, value)) {
            spdlog::error("Parse-JSON registers: Failed to write register.");
            ui->statusBar->showMessage(tr("JSON register load aborted: Unable to write register data."));
            rv = VSCP_ERROR_PARSING;
          }

          ui->statusBar->showMessage(tr("Loaded %1 registers. %2 skipped").arg(regcnt).arg(regskipped));
          updateVisualRegisters();

        }
      }
      else {
        spdlog::error("Parse-JSON registers: Register object invalid. <<{}>>", item.value().dump());
        ui->statusBar->showMessage(tr("JSON register load aborted: Format of JSON file is not correct. Register object invalid."));
        return VSCP_ERROR_PARSING;
      }
    }
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// loadRegisterValues
//

void
CFrmNodeConfig::loadRegisterValues(void)
{
  int rv = VSCP_ERROR_SUCCESS;
  std::ifstream ifs;
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

  QString path = QFileDialog::getOpenFileName(this,
                                    tr("Load registers from file"),
                                    /*"~/.vscpworks/device-registers.reg"*/"/tmp/device-registers.reg",
                                    tr("Register Files (*.reg);;XML Files (*.xml);;JSON Files (*.json);;All Files (*.*)"));
  //std::cout << "Filename: |" << fileName.toStdString() << "|" << std::endl;
  if (path.isEmpty()) {
    return;
  }

  // Check format
  // ------------
  // If the file is a JSON file we will parse it
  // as JSON else we will parse it as XML. The first
  // character determines the type "{" for JSON or "<"
  // for XML. Whitespace is ignored.

  try {
    ifs.open(path.toStdString(), std::ifstream::in);
  }
  catch (...) {
    spdlog::error("Load registers: Failed to open file {}", path.toStdString());
    return;
  }

  size_t pos;
  std::string str;

  while (std::getline(ifs, str)) {

    vscp_trim(str);
    if ((pos = str.find('{')) != std::string::npos) {
      spdlog::debug("Load registers: Register file format is JSON");
      ifs.close();
      rv = loadJSONRegs(path.toStdString());
      if (VSCP_ERROR_SUCCESS != rv) {
        spdlog::error("Load registers: Failed to load registers from file {}", path.toStdString());
        QMessageBox::information(this, tr(APPNAME),
                             tr("Failed to parse JSON file %1:\n%2.")
                             .arg(path)
                             .arg(rv));
      }
      break;
    }
    else if ((pos = str.find('<')) != std::string::npos) {
      spdlog::debug("Load registers: MDF file format is XML");
      ifs.close();
      rv = loadXMLRegs(path.toStdString());
      if (VSCP_ERROR_SUCCESS != rv) {
        spdlog::error("Load registers: Failed to load registers from file {}", path.toStdString());
        QMessageBox::information(this, tr(APPNAME),
                             tr("Failed to parse XML file %1:\n%2.")
                             .arg(path)
                             .arg(rv));
      }
      break;
    }
    else {
      ifs.close();
      rv = VSCP_ERROR_INVALID_SYNTAX;
      spdlog::error("Load registers: Failed to load registers from file {} - Invalid syntax", path.toStdString());
      QMessageBox::information(this, tr(APPNAME),
                             tr("Syntax error in register file %1:\n%2.")
                             .arg(path)
                             .arg(rv));
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadDefaults
//

void
CFrmNodeConfig::loadDefaults(void)
{
  int rv = VSCP_ERROR_SUCCESS;
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

  // Write all registers
  QTreeWidgetItemIterator item(ui->treeWidgetRegisters);
  while (*item) {
    if ((*item)->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)(*item);
      if (m_mdf.isRegisterWriteable(itemReg->m_regOffset, itemReg->m_regPage)) {
        int val = m_mdf.getDefaultRegisterValue(itemReg->m_regOffset, itemReg->m_regPage);
        if (-1 == val) {
          spdlog::error("Load defaults: Failed to get default register value. {%1}:{%2}", itemReg->m_regPage, itemReg->m_regOffset);
          continue;;
        }
	      if (!m_userregs.putReg(itemReg->m_regOffset, itemReg->m_regPage, val)) {
          spdlog::error("Load defaults: Failed to write register {%1}:{%2}.", itemReg->m_regPage, itemReg->m_regOffset);
        }
      } 
    }
    ++item;
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadDefaultsSelected
//

void
CFrmNodeConfig::loadDefaultsSelected(void)
{
  int rv = VSCP_ERROR_SUCCESS;
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

  // Write selected registers
  QList<QTreeWidgetItem*> listSelected = ui->treeWidgetRegisters->selectedItems();
  // TODO: sort option

  for (auto item : listSelected) {
    if (item->type() == TREE_LIST_REGISTER_TYPE) {
      CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item;
      if (m_mdf.isRegisterWriteable(itemReg->m_regOffset, itemReg->m_regPage)) {
        int val = m_mdf.getDefaultRegisterValue(itemReg->m_regOffset, itemReg->m_regPage);
        if (-1 == val) {
          spdlog::error("Load defaults: Failed to get default register value. {%1}:{%2}", itemReg->m_regPage, itemReg->m_regOffset);
          continue;;
        }
	      if (!m_userregs.putReg(itemReg->m_regOffset, itemReg->m_regPage, val)) {
          spdlog::error("Load defaults: Failed to write register {%1}:{%2}.", itemReg->m_regPage, itemReg->m_regOffset);
        }
      }
    }
    else {
      // Save all children on page
      for (int i=0; i<item->childCount(); i++) {
        CRegisterWidgetItem* itemReg = (CRegisterWidgetItem*)item->child(i);
        if (m_mdf.isRegisterWriteable(itemReg->m_regOffset, itemReg->m_regPage)) {
          int val = m_mdf.getDefaultRegisterValue(itemReg->m_regOffset, itemReg->m_regPage);
          if (-1 == val) {
            spdlog::error("Load defaults: Failed to get default register value. {%1}:{%2}", itemReg->m_regPage, itemReg->m_regOffset);
            continue;
          }
          if (!m_userregs.putReg(itemReg->m_regOffset, itemReg->m_regPage, val)) {
            spdlog::error("Load defaults: Failed to write register {%1}:{%2}.", itemReg->m_regPage, itemReg->m_regOffset);
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// onRegisterTreeWidgetItemClicked
//

void
CFrmNodeConfig::onRegisterTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
  if ((item->type() != TREE_LIST_REGISTER_TYPE)) {
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
CFrmNodeConfig::onRegisterTreeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  if (2 == column) {
    ui->treeWidgetRegisters->editItem(item, column);
  }
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
  menu->addAction(QString(tr("Full update with local MDF")), this, SLOT(updateLocal()));
  menu->addSeparator();
  menu->addAction(QString(tr("Read value(s) for selected row(s)")), this, SLOT(readSelectedRegisterValues()));
  menu->addAction(QString(tr("Write value(s) for selected row(s)")), this, SLOT(writeSelectedRegisterValues()));
  menu->addSeparator();
  menu->addAction(QString(tr("Write default value(s) for selected row(s)")), this, SLOT(defaultSelectedRegisterValues()));
  menu->addAction(QString(tr("Set default values for ALL rows")), this, SLOT(defaultRegisterAll()));
  // TODO
  //menu->addSeparator();
  //menu->addAction(QString(tr("Undo value(s) for selected row(s)")), this, SLOT(undoSelectedRegisterValues()));
  //menu->addAction(QString(tr("Redo value(s) for selected row(s)")), this, SLOT(redoSelectedRegisterValues()));
  menu->addSeparator();
  menu->addAction(QString(tr("Save selected registers")), this, SLOT(saveSelectedRegisterValues()));
  menu->addAction(QString(tr("Save ALL registers")), this, SLOT(saveAllRegisterValues()));
  menu->addAction(QString(tr("Load registers")), this, SLOT(loadRegisterValues()));
  menu->addSeparator();
  menu->addAction(QString(tr("Goto register page...")), this, SLOT(gotoRegisterPage()));

  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
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

      Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

      // Set foreground and background colors from MDF
      if (!pworks->m_config_bDisableColors) {
        for (int i = 0; i < 4; i++) {
          itemReg->setForeground(i, QBrush(QColor(pregmdf->getForegroundColor())));
          itemReg->setBackground(i, QBrush(QColor(pregmdf->getBackgroundColor())));
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

// ****************************************************************************
//                   * * *  Remote variable handling * * *
// ****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// onRemoteVariableTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onRemoteVariableTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
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
CFrmNodeConfig::onRemoteVariableTreeWidgetItemDoubleClicked( QTreeWidgetItem* item, int column)
{
  if (0 == column) {
    ui->treeWidgetRemoteVariables->editItem(item, column);
  }
}

///////////////////////////////////////////////////////////////////////////////
// gotoRemoteVarRegisterPos
//

void
CFrmNodeConfig::gotoRemoteVarRegisterPos(void)
{
  CRemoteVariableWidgetItem *item = (CRemoteVariableWidgetItem*)ui->treeWidgetRemoteVariables->currentItem();
  if (nullptr == item) {
    return;
  }

  CMDF_RemoteVariable *prv = item->m_pRemoteVariable;
  if (nullptr == item) {
    spdlog::error("gotoRemoteVarRegisterPos: No remote variable stored in item");
    return;
  }

  gotoRegisterOnPage(prv->getPage(), prv->getOffset());
}

///////////////////////////////////////////////////////////////////////////////
// showRemoteVariableContextMenu
//

void
CFrmNodeConfig::showRemoteVariableContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);
  menu->addAction(QString(tr("Go to register(s) for this remote variable")), this, SLOT(gotoRemoteVarRegisterPos()));
  menu->addSeparator();
  menu->addAction(QString(tr("Read value(s) for selected row(s)")), this, SLOT(readSelectedRegisterValues()));
  menu->addAction(QString(tr("Write value(s) for selected row(s)")), this, SLOT(writeSelectedRegisterValues()));
  menu->addAction(QString(tr("Write default value(s) for selected row(s)")), this, SLOT(defaultSelectedRegisterValues()));
  menu->addAction(QString(tr("Set default values for ALL rows")), this, SLOT(defaultRegisterAll()));
  // TODO
  // menu->addAction(QString(tr("Undo value(s) for selected row(s)")), this, SLOT(undoSelectedRegisterValues()));
  // menu->addAction(QString(tr("Redo value(s) for selected row(s)")), this, SLOT(redoSelectedRegisterValues()));
  // TODO go to register
  menu->addSeparator();
  menu->addAction(QString(tr("Save register value(s) for selected row(s) to disk")), this, SLOT(saveSelectedRegisterValues()));
  menu->addAction(QString(tr("Save ALL register values to disk")), this, SLOT(saveAllRegisterValues()));
  menu->addAction(QString(tr("Load register values from disk")), this, SLOT(loadRegisterValues()));

  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
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




// ****************************************************************************
//                       * * *  DM handling * * *
// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// onDMTreeWidgetItemClicked
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
// onDMTreeWidgetItemDoubleClicked
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
// showDMContextMenu
//

void
CFrmNodeConfig::showDMContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);
  menu->addAction(QString(tr("Edit row")), this, SLOT(editDMRow()));
  menu->addAction(QString(tr("Go to register position for row")), this, SLOT(gotoDMRegisterPos()));
  menu->addSeparator();
  menu->addAction(QString(tr("Enable/Disable selected row(s)")), this, SLOT(toggleDMRow()));
  menu->addAction(QString(tr("Read selected DM row(s)")), this, SLOT(readSelectedDMRow()));
  menu->addAction(QString(tr("Write selected DM row(s)")), this, SLOT(writeSelectedDMRow()));
  //menu->addSeparator();   TODO
  //menu->addAction(QString(tr("Undo value(s) for selected row(s)")), this, SLOT(undoSelectedRegisterValues()));
  //menu->addAction(QString(tr("Redo value(s) for selected row(s)")), this, SLOT(redoSelectedRegisterValues()));
  // TODO Defaults
  // TODO go to register
  menu->addSeparator();
  menu->addAction(QString(tr("Save DM values for selected row(s) to disk")), this, SLOT(saveSelectedRegisterValues()));
  menu->addAction(QString(tr("Save DM registers to disk")), this, SLOT(saveAllRegisterValues()));
  menu->addAction(QString(tr("Load DM values from disk")), this, SLOT(loadRegisterValues()));

  menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
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
  if (nullptr == item) {
    spdlog::error("editDMRow: Item for DM register row is null");
    return;
  }

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
// toggleDMRow
//

void
CFrmNodeConfig::toggleDMRow(void)
{
  int reg;
  vscpworks* pworks                = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* item              = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->currentItem();
  if (nullptr == item) {
    spdlog::error("editDMRow: Item for DM register row is null");
    return;
  }

  if (item->m_pDM == nullptr) {
    spdlog::error("editDMRow: Item for DM register row has no DM");
    return;
  }

  uint8_t value = m_userregs.getReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_FLAGS,
                      item->m_pDM->getStartPage());

  if (value & 0x80) {
    value &= 0x7f;
  }
  else {
    value |= 0x80;
  }                      

  m_userregs.putReg(item->m_pDM->getStartOffset() +
                        (item->m_row * item->m_pDM->getRowSize()) +
                        CMDF_DecisionMatrix::IDX_ADDRESS_FLAGS,
                      item->m_pDM->getStartPage(),
                      value );

  updateVisualRegisters();
  fillDMHtmlInfo(item, 0);
}

///////////////////////////////////////////////////////////////////////////////
// readSelectedDMRow
//

void
CFrmNodeConfig::readSelectedDMRow(void)
{
  int rv;
  int reg;
  vscpworks* pworks                = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* item              = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->currentItem();
  if (nullptr == item) {
    spdlog::error("readSelectedDMRow: Item for DM register row is null");
    return;
  }

  if (item->m_pDM == nullptr) {
    spdlog::error("readSelectedDMRow: Item for DM register row has no DM");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;

  // node id
  guidNode.setLSB(m_nodeidConfig->value());

  std::map<uint8_t,uint8_t> values;
  if (VSCP_ERROR_SUCCESS != (rv = vscp_readLevel1RegisterBlock(*m_vscpClient,
                                      guidNode,
                                      guidInterface,
                                      item->m_pDM->getStartPage(), 
                                      item->m_pDM->getStartOffset() + (item->m_row * item->m_pDM->getRowSize()),
                                      item->m_pDM->getRowSize(),
                                      values,
                                      pworks->m_config_timeout))) {
    
    QApplication::restoreOverrideCursor();  
    QApplication::processEvents();

    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to read DM row from node.\n"
                                "Error code: %1").arg(rv),
                             QMessageBox::Ok);   
    return;                                                              
  }

  for (int i=0; i<item->m_pDM->getRowSize(); i++) {
    uint8_t pos = item->m_pDM->getStartOffset() +
                    (item->m_row * item->m_pDM->getRowSize()) +
                    i;
    m_userregs.putReg( pos,
                        item->m_pDM->getStartPage(),
                        values[pos] );
  }

  updateVisualRegisters();
  fillDMHtmlInfo(item, 0);

  QApplication::restoreOverrideCursor();
  QApplication::processEvents();
}

///////////////////////////////////////////////////////////////////////////////
// writeSelectedDMRow
//

void
CFrmNodeConfig::writeSelectedDMRow(void)
{
  int rv;
  int reg;
  vscpworks* pworks                = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* item              = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->currentItem();
  if (nullptr == item) {
    spdlog::error("writeSelectedDMRow: Item for DM register row is null");
    return;
  }

  if (item->m_pDM == nullptr) {
    spdlog::error("writeSelectedDMRow: Item for DM register row has no DM");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // CAN4VSCP interface
  std::string str = m_comboInterface->currentText().toStdString();
  cguid guidInterface;
  cguid guidNode;
  guidInterface.getFromString(str);
  guidNode = guidInterface;
  
  // node id
  guidNode.setLSB(m_nodeidConfig->value());

  std::map<uint8_t,uint8_t> values;
  for (int i=0; i<item->m_pDM->getRowSize(); i++) {
    uint8_t pos = item->m_pDM->getStartOffset() +
                    (item->m_row * item->m_pDM->getRowSize()) +
                    i;
    values[pos] = m_userregs.getReg(pos, item->m_pDM->getStartPage());
  }

  if (VSCP_ERROR_SUCCESS != (rv = vscp_writeLevel1RegisterBlock(*m_vscpClient,
                                    guidNode,
                                    guidInterface,
                                    item->m_pDM->getStartPage(),                                    
                                    values,
                                    pworks->m_config_timeout))) {
    
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to write DM row from node.\n"
                                "Error code: %1").arg(rv),
                             QMessageBox::Ok);  
    return;                                                              
  }

  for (int i=0; i<item->m_pDM->getRowSize(); i++) {
    uint8_t pos = item->m_pDM->getStartOffset() +
                    (item->m_row * item->m_pDM->getRowSize()) +
                    i;
    // Mark register as written                
    m_userregs.setChangedState(pos, item->m_pDM->getStartPage(), false);
    updateChangeDM(pos, item->m_pDM->getStartPage());
  }
                  
  updateVisualRegisters();
  updateVisualDM();
  fillDMHtmlInfo(item, 0);
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// gotoDMRegisterPos
//

void
CFrmNodeConfig::gotoDMRegisterPos(void)
{
  vscpworks* pworks                = (vscpworks*)QCoreApplication::instance();
  CDMWidgetItem* item              = (CDMWidgetItem*)ui->treeWidgetDecisionMatrix->currentItem();
  if (nullptr == item) {
    spdlog::error("gotoDMRegisterPos: Item for DM register row is null");
    return;
  }

  uint16_t page                    = item->m_pDM->getStartPage();
  gotoRegisterOnPage(item->m_pDM->getStartPage(), item->m_pDM->getStartOffset() +
                                                    (item->m_row * item->m_pDM->getRowSize()));
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



// ****************************************************************************
//                       * * *  MDF files handling * * *
// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// onMdfFileTreeWidgetItemClicked
//

void
CFrmNodeConfig::onMdfFileTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
  fillMdfFileHtmlInfo(item, column);
  m_bMainInfo = false;
}

///////////////////////////////////////////////////////////////////////////////
// onMdfFileTreeWidgetItemDoubleClicked
//

void
CFrmNodeConfig::onMdfFileTreeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  CMdfFileWidgetItem* pitem = (CMdfFileWidgetItem*)item;
  if (nullptr == item) {
    spdlog::critical("Doubleclick: MDF file item is NULL {0}", pitem->text(0).toStdString());
    ui->infoArea->setHtml(tr("Doubleclick: MDF file item is NULL"));
    return;    
  }
  //QDesktopServices::openUrl(QUrl("file:///C:/Documents and Settings/All Users/Desktop", QUrl::TolerantMode));
  //QDesktopServices::openUrl(QUrl("https://www.vscp.org/presentations/vscpbtdetect.mp4", QUrl::TolerantMode));

  switch (/*pitem->m_mdfFileType*/pitem->type()) {

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_picture)):
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_picture)):
      if (nullptr != pitem->m_picture_obj) {      
        QDesktopServices::openUrl(QUrl(pitem->m_picture_obj->getUrl().c_str()));
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open picture!"),
                             QMessageBox::Ok);  
      }      
      break;  
    
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_video)):
      break;
    
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_video)):
      if (nullptr != pitem->m_video_obj) {      
        QDesktopServices::openUrl(QUrl(pitem->m_video_obj->getUrl().c_str()));
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open video!"),
                             QMessageBox::Ok);  
      }      
      break;

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_firmware)):
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_firmware)):
      if (nullptr != pitem->m_firmware_obj) {      
        // TODO
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open firmware file!"),
                             QMessageBox::Ok);  
      }      
      break;  
    
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_manual)):
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_manual)):
      if (nullptr != pitem->m_manual_obj) {      
        QDesktopServices::openUrl(QUrl(pitem->m_manual_obj->getUrl().c_str()));
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open manual!"),
                             QMessageBox::Ok);  
      }      
      break;

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_driver)):      
      break;
    
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_driver)):
      if (nullptr != pitem->m_manual_obj) {    
        // TODO  
        QDesktopServices::openUrl(QUrl(pitem->m_manual_obj->getUrl().c_str()));
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open manual!"),
                             QMessageBox::Ok);  
      }      
      break;

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_setup)):
      break;
    
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_setup)):
      if (nullptr != pitem->m_setup_obj) {      
        // TODO
      }
      else {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to open setup!"),
                             QMessageBox::Ok);  
      }      
      break;

    default:
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_none)):
      break;          
  }
}

///////////////////////////////////////////////////////////////////////////////
// showMdfFilesContextMenu
//

void
CFrmNodeConfig::showMdfFilesContextMenu(const QPoint& pos)
{
  // QMenu* menu = new QMenu(this);
  // menu->addAction(QString(tr("Update")), this, SLOT(update()));
  // menu->addAction(QString(tr("Full ipdate")), this, SLOT(updateFull()));
  // menu->addAction(QString(tr("Full update with local MDF")), this, SLOT(updateLocal()));
  // menu->popup(ui->treeWidgetRegisters->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// renderMdfFiles
//

bool
CFrmNodeConfig::renderMdfFiles(void)
{
  int rv;
  CMdfFileWidgetItem* topItemWidget;
  CMdfFileWidgetItem* itemWidget;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  ui->treeWidgetMdfFiles->clear(); // Clear the tree

  // Pictures
  if (m_mdf.getPictureCount()) {

    topItemWidget = new CMdfFileWidgetItem("Pictures", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_picture));
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for pictures");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Pictures");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getPictureCount(); i++) {

      itemWidget = new CMdfFileWidgetItem("Pictures", 
                        QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_picture));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for pictures");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to view picture"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_picture;
      itemWidget->m_picture_obj = m_mdf.getPictureObj(i);

      QString name(m_mdf.getPictureObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
        name += m_mdf.getPictureObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add picture child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Video
  if (m_mdf.getVideoCount()) {

    topItemWidget = new CMdfFileWidgetItem("Video", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_video));
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for videos");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Video");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getVideoCount(); i++) {
      
      itemWidget = new CMdfFileWidgetItem("Video", 
                        QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_video));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for videos");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to view video"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_video;
      itemWidget->m_video_obj = m_mdf.getVideoObj(i);

      QString name(m_mdf.getVideoObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
        name += m_mdf.getVideoObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add picture child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Firmware
  if (m_mdf.getFirmwareCount()) {

    topItemWidget = new CMdfFileWidgetItem("Firmware", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_firmware));
    if (nullptr == topItemWidget) {
      spdlog::critical(
        "Failed to create MDF file widget top item for firmware");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Firmware");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getFirmwareCount(); i++) {

      itemWidget = new CMdfFileWidgetItem("Firmware", 
                        QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_firmware));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for firmware");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to load selected firmware to remote device"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_firmware;
      itemWidget->m_firmware_obj = m_mdf.getFirmwareObj(i);

      QString name(m_mdf.getFirmwareObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
        name += m_mdf.getFirmwareObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add firmware child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Driver
  if (m_mdf.getDriverCount()) {

    topItemWidget = new CMdfFileWidgetItem("Driver", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_driver));
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for driver");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Driver");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getDriverCount(); i++) {

      itemWidget = new CMdfFileWidgetItem("Driver", 
                        QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_driver));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for driver");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to download and install driver"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_driver;
      itemWidget->m_driver_obj = m_mdf.getDriverObj(i);

      QString name(m_mdf.getDriverObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
        name += m_mdf.getDriverObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add driver child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Manual
  if (m_mdf.getManualCount()) {

    topItemWidget = new CMdfFileWidgetItem("Manual", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_manual));
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for manual");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Manual");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getManualCount(); i++) {

      itemWidget = new CMdfFileWidgetItem("Manual", 
                        QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_manual));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for Manual");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to open manual"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_manual;
      itemWidget->m_manual_obj = m_mdf.getManualObj(i);

      QString name(m_mdf.getManualObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
        name += m_mdf.getManualObj(i)->getDescription().c_str();
      }
      itemWidget->setText(0, name);

      // Add driver child
      topItemWidget->addChild(itemWidget);
    }
  }

  // Setup
  if (m_mdf.getSetupCount()) {

    topItemWidget = new CMdfFileWidgetItem("Setup", QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_setup));
    if (nullptr == topItemWidget) {
      spdlog::critical("Failed to create MDF file widget top item for setup");
      return false;
    }
    // Header is typeless
    topItemWidget->m_mdfFileType = mdf_file_type_none;

    topItemWidget->setFont(0, QFont("Arial", 12, QFont::Bold));
    topItemWidget->setText(0, "Setup");

    // Add item
    ui->treeWidgetMdfFiles->addTopLevelItem(topItemWidget);

    for (int i = 0; i < m_mdf.getSetupCount(); i++) {

      itemWidget = new CMdfFileWidgetItem("Setup", QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_setup));
      if (nullptr == itemWidget) {
        spdlog::critical("Failed to create MDF file widget item for setup");
        return false;
      }

      itemWidget->setToolTip(0, tr("Double click to start setup wizard"));
      itemWidget->setForeground(0, QBrush(QColor(Qt::darkGreen)));
      itemWidget->m_mdfFileType = mdf_file_type_setup;
      itemWidget->m_setup_obj = m_mdf.getSetupObj(i);

      QString name(m_mdf.getSetupObj(i)->getName().c_str());
      if (!name.length()) {
        //name = QString::number(i);
        name = "";
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
// fillMdfFileHtmlInfo
//

void 
CFrmNodeConfig::fillMdfFileHtmlInfo(QTreeWidgetItem *item, int column)
{
  int idx;
  std::string html;
  std::string str;
  CMdfFileWidgetItem* pitem = (CMdfFileWidgetItem*)item;
  if (nullptr == item) {
    spdlog::critical("MDF file item is NULL {0}", pitem->text(0).toStdString());
    ui->infoArea->setHtml(tr("MDF file item is NULL"));
    return;    
  }

  // if (/*pitem->m_mdfFileType == mdf_file_type_none*/) {
  //   spdlog::critical("MDF file has no type {0}", pitem->text(0).toStdString());
  //   ui->infoArea->setHtml(tr("MDF file has no type"));
  //   return; 
  // }
  
  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body>";

  str += pitem->text(0).toStdString();
  switch (/*pitem->m_mdfFileType*/pitem->type()) {

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_picture)):
      html += "<h4>";
      html += tr("Pictures related to device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_picture)):
      html += "<h4>";
      if (nullptr != pitem->m_picture_obj) {
        html += pitem->m_picture_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_picture_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_picture_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_picture_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_picture_obj->getFormat();
        html += "</p><hr><p>";
        html += "<img src=\"";
        html += pitem->m_picture_obj->getUrl().c_str();
        html += "\" alt=\"";
        html += pitem->m_picture_obj->getName().c_str();
        html += "\" /></p><hr>";
      }
      else {
        html += tr("Invalid picture item").toStdString();
        html += "</h4>";
      }      
      break;  
    
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_video)):
      html += "<h4>";
      html += tr("Videos related to the device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_video)):
      html += "<h4>";
      if (nullptr != pitem->m_video_obj) {
        html += pitem->m_video_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_video_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_video_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_video_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_video_obj->getFormat();
        html += "</p><hr><p>";
        html += "<img src=\"";
        html += pitem->m_video_obj->getUrl().c_str();
        html += "\" alt=\"";
        html += pitem->m_video_obj->getName().c_str();
        html += "\" /></p><hr>";
      }
      else {
        html += tr("Invalid video item").toStdString();
        html += "</h4>";
      }      
      break;    
    
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_firmware)):
      html += "<h4>";
      html += tr("Firmware for device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;
    
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_firmware)):
      html += "<h4>";
      if (nullptr != pitem->m_firmware_obj) {
        html += pitem->m_firmware_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_firmware_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_firmware_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_firmware_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_firmware_obj->getFormat();
        html += "<br><b>Version:</b> ";
        html += QString::number(pitem->m_firmware_obj->getVersionMajor()).toStdString();
        html += ".";
        html += QString::number(pitem->m_firmware_obj->getVersionMinor()).toStdString();
        html += ".";
        html += QString::number(pitem->m_firmware_obj->getVersionPatch()).toStdString();
        html += "<br><b>Date:</b> ";
        html += pitem->m_firmware_obj->getDate().c_str();
        html += "<br><b>Size:</b> ";
        html += QString::number(pitem->m_firmware_obj->getSize()).toStdString();
        html += "<br><b>MD5:</b> ";
        html += pitem->m_firmware_obj->getMd5().c_str();
        html += "<br><b>Target:</b> ";
        html += pitem->m_firmware_obj->getTarget().c_str();
        html += "<br><b>Target code:</b> ";
        html += QString::number(pitem->m_firmware_obj->getTargetCode()).toStdString();
        html += "</p>";
        html += "<hr>";
      }
      else {
        html += tr("Invalid firmware item").toStdString();
        html += "</h4>";
      }      
      break;

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_manual)):
      html += "<h4>";
      html += tr("Manuals for device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;

    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_manual)):
      html += "<h4>";
      if (nullptr != pitem->m_manual_obj) {
        html += pitem->m_manual_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_manual_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_manual_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_manual_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_manual_obj->getFormat();
        html += "</p><hr><p>";
        html += "<img src=\"";
        html += pitem->m_manual_obj->getUrl().c_str();
        html += "\" alt=\"";
        html += pitem->m_manual_obj->getName().c_str();
        html += "\" /></p><hr>";
      }
      else {
        html += tr("Invalid manual item").toStdString();
        html += "</h4>";
      }      
      break;   
    
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_driver)):
      html += "<h4>";
      html += tr("Drivers for device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;
      
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_driver)):
      html += "<h4>";
      if (nullptr != pitem->m_driver_obj) {
        html += pitem->m_driver_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_driver_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_driver_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_driver_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_driver_obj->getType();
        html += "<br><b>Version:</b> ";
        html += QString::number(pitem->m_driver_obj->getVersionMajor()).toStdString();
        html += ".";
        html += QString::number(pitem->m_driver_obj->getVersionMinor()).toStdString();
        html += ".";
        html += QString::number(pitem->m_driver_obj->getVersionPatch()).toStdString();
        html += "<br><b>Date:</b> ";
        html += pitem->m_driver_obj->getDate().c_str();
        html += "<br><b>MD5:</b> ";
        html += pitem->m_driver_obj->getMd5().c_str();
        html += "</a><br><b>OS:</b> ";
        html += pitem->m_driver_obj->getOS();
        html += "</a><br><b>OS Version:</b> ";
        html += pitem->m_driver_obj->getOSVer();
        
        html += "</p>";
        html += "<hr>";
      }
      else {
        html += tr("Invalid manual item").toStdString();
        html += "</h4>";
      }      
      break; 

    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_setup)):
      html += "<h4>";
      html += tr("Setups for device").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;
    
    case (QTreeWidgetItem::UserType + CMdfFileWidgetItem::ITEM_OFFSET + static_cast<int>(mdf_file_type_setup)):
      html += "<h4>";
      if (nullptr != pitem->m_setup_obj) {
        html += pitem->m_setup_obj->getName().c_str();
        html += "</h4>";
        html += "<p>";
        html += pitem->m_setup_obj->getDescription().c_str();
        html += "</p><p><b>url:</b> <a href=\"";        
        html += pitem->m_setup_obj->getUrl().c_str();
        html += "\">";
        html += pitem->m_setup_obj->getUrl().c_str();
        html += "</a><br><b>Format:</b> ";
        html += pitem->m_setup_obj->getFormat();
        html += "</p><hr><p>";
        html += "<img src=\"";
        html += pitem->m_setup_obj->getUrl().c_str();
        html += "\" alt=\"";
        html += pitem->m_setup_obj->getName().c_str();
        html += "\" /></p><hr>";
      }
      else {
        html += tr("Invalid manual item").toStdString();
        html += "</h4>";
      }      
      break; 

    default:
    case (QTreeWidgetItem::UserType + static_cast<int>(mdf_file_type_none)):
      html += "<h4>";
      html += tr("Invalid entry").toStdString();
      html += "</h4>";
      html += tr("Select item below to get information about ir.").toStdString();
      break;          
  }

  // html += "<h4>";
  // html += str;
  // html += "</h4>";
  // html += "<p><b>";
  // //html += pitem->text(REG_COL_POS).toStdString();
  // html += "</b> [";
  // //html += pitem->text(REG_COL_ACCESS).toStdString();
  // html += "] ";
  // html += "</p>";
  // html += "<p>";
  // html += "<p>";
  html += "</font>";
  html += "</body></html>";

  // Set the HTML
  ui->infoArea->setHtml(html.c_str());  
}


// ---------------------------------------------------------------------------- 


