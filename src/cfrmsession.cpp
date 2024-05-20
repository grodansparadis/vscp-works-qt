// cfrmsession.cpp
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
#include <vscpworks.h>

#include <mustache.hpp>

#include <vscp-client-canal.h>
#include <vscp-client-tcp.h>
#include <vscp-client-udp.h>
#include <vscp-client-ws1.h>
#include <vscp-client-ws2.h>
#ifndef WIN32
#include <vscp-client-socketcan.h>
#endif
#include <vscp-client-mqtt.h>
#include <vscp-client-multicast.h>

#include "cdlgknownguid.h"
#include "cdlgselectmqtttopics.h"
#include "cdlgsessionfilter.h"
#include "cfrmsession.h"

#include "cdlgmainsettings.h"
#include "cdlgtxedit.h"

#include <QClipboard>
#include <QFile>
#include <QJSEngine>
#include <QKeySequence>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------

CTxWidgetItem::CTxWidgetItem(const QString& text)
  : QTableWidgetItem(text, QTableWidgetItem::UserType)
{
  ;
}

CTxWidgetItem::~CTxWidgetItem()
{
  ;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// vscp-client-ack
//

// void CVscpClientCallback::eventReceived(vscpEvent *pev)
// {
//     vscpEvent ev;
//     //emit CFrmSession::receiveRow(pev, true);
// }

// static void
// eventReceived(vscpEvent &ev, void* pobj)
// {
//   vscpEvent* pevnew = new vscpEvent;
//   pevnew->sizeData  = 0;
//   pevnew->pdata     = nullptr;
//   vscp_copyEvent(pevnew, &ev);

//   CFrmSession* pSession = (CFrmSession*)pobj;
//   pSession->threadReceive(pevnew);
// }

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CFrmSession
//

CFrmSession::CFrmSession(QWidget* parent, QJsonObject* pconn)
  : QDialog(parent)
{
  // No connection set yet
  m_vscpConnType = CVscpClient::connType::NONE;
  m_vscpClient   = NULL;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(std::string(tr("Session: Session module opended").toStdString()));

  if (nullptr == pconn) {
    spdlog::error(std::string(tr("Session: pconn is null").toStdString()));
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("Can't open session window - configuration data is missing"),
      QMessageBox::Ok);
    return;
  }

  // Save session configuration
  m_connObject = *pconn;

  // Must have a type
  if (m_connObject["type"].isNull()) {
    spdlog::error(std::string(tr("Session: Type is not define in JSON data").toStdString()));
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("Can't open session window - The connection type is unknown"),
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

  createMenu();
  createToolbar();
  createHorizontalGroupBox();
  createRxGroupBox();
  // createFormGroupBox();
  createTxGridGroup();

  // Handle clicks
  connect(m_rxTable,
          SIGNAL(cellClicked(int, int)),
          SLOT(rxCellClicked(int, int)));

  // Open pop up menu on right click on VSCP type listbox
  connect(m_rxTable,
          &QTableWidget::customContextMenuRequested,
          this,
          &CFrmSession::showRxContextMenu);

  // Handle selections
  connect(m_rxTable->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          SLOT(rxSelectionChange(const QItemSelection&, const QItemSelection&)));

  // Lay out things
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMenuBar(m_menuBar);
  mainLayout->addWidget(m_toolBar);
  mainLayout->addWidget(m_gridGroupBox, 6);
  mainLayout->addWidget(m_txGroupBox, 3);

  setLayout(mainLayout);

  /*!
    This is an old construct to call a method from a 
    worker thread
  */
  qDebug() << connect(this,
                      &CFrmSession::dataReceived,
                      this,
                      &CFrmSession::receiveRxRow,
                      Qt::ConnectionType::QueuedConnection);

  QJsonDocument doc(m_connObject);
  QString strJson(doc.toJson(QJsonDocument::Compact));

  using namespace std::placeholders;
  auto cb = std::bind(&CFrmSession::receiveCallback, this, _1, _2);
  // lambda version for reference
  //auto cb = [this](auto a, auto b) { this->receiveCallback(a, b); };

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      m_vscpClient = new vscpClientTcp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

    case CVscpClient::connType::CANAL:
      m_vscpClient = new vscpClientCanal();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this, tr("VSCP Works +"), tr("Failed to initialize CANAL driver. See log for more details."));
        return;
      }
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      m_vscpClient = new vscpClientSocketCan();
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this, tr("VSCP Works +"), tr("Failed to initialize SOCKETCAN driver. See log for more details."));
        return;
      }
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;
#endif

    case CVscpClient::connType::WS1:
      m_vscpClient = new vscpClientWs1();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

    case CVscpClient::connType::WS2:
      m_vscpClient = new vscpClientWs2();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

    case CVscpClient::connType::MQTT:
      m_vscpClient = new vscpClientMqtt();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      // m_connectActBar->setChecked(true);
      //  Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

    case CVscpClient::connType::UDP:
      m_vscpClient = new vscpClientUdp();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

    case CVscpClient::connType::MULTICAST:
      m_vscpClient = new vscpClientMulticast();
      m_vscpClient->initFromJson(strJson.toStdString());
      m_vscpClient->setCallbackEv(/*eventReceived*/cb, this);
      //m_connectActBar->setChecked(true);
      // Connect if autoconnect is enabled
      if (pworks->m_session_bAutoConnect) {
        connectToRemoteHost(true);
      }
      break;

  }

  // TX Table signales

  connect(
    m_txTable->selectionModel(),
    SIGNAL(QTableWidget::itemChanged(QTableWidgetItem * item)),
    SLOT(txItemChanged(QTableWidgetItem * item)));

  connect(
    m_txTable,
    &QTableWidget::cellDoubleClicked,
    this,
    &CFrmSession::txRowDoubleClicked);

  // Open pop up menu on right click on VSCP type listbox
  connect(m_txTable,
          &QTableWidget::customContextMenuRequested,
          this,
          &CFrmSession::showTxContextMenu);

  // Load events from last session
  loadTxOnStart();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmSession::~CFrmSession()
{
  // Autosave TX data
  saveTxOnExit();

  // Make sure we are disconnected
  doDisconnectFromRemoteHost();

  // Remove receive events
  while (m_rxEvents.size()) {
    vscpEvent* pev = m_rxEvents.front();
    m_rxEvents.pop_front();
    vscp_deleteEvent(pev);
    pev = nullptr;
  }

  // This should neo be needed
  // m_txTable->clear();
  // m_txTable->setRowCount(0);
}

///////////////////////////////////////////////////////////////////////////////
// createMenu
//

void
CFrmSession::createMenu()
{
  m_menuBar = new QMenuBar;

  // * * * File menu * * *

  m_fileMenu = new QMenu(tr("&File"), this);

  // Icons
  const QIcon windowCloseIcon = QIcon::fromTheme("window-close");

  const QIcon loadIcon = QIcon::fromTheme("file-open");
  const QIcon saveIcon = QIcon::fromTheme("file-save");

  // const QIcon connectIcon = QIcon::fromTheme("call-start");
  const QIcon disconnectIcon = QIcon::fromTheme("call-stop");
  // const QIcon filterIcon = QIcon::fromTheme("edit-find");
  const QIcon saveIcon2 = QIcon::fromTheme("file-save");

  // * * *  menu * * *

  m_loadEventsAct =
    m_fileMenu->addAction(QIcon::fromTheme("document-open"),
                          tr("Load RX rows from file..."),
                          this,
                          &CFrmSession::loadRxFromFile);

  m_saveEventsAct = m_fileMenu->addAction(QIcon::fromTheme("document-save-as"),
                                          tr("Write RX rows to file..."),
                                          this,
                                          &CFrmSession::saveRxToFile);

  m_loadTxAct =
    m_fileMenu->addAction(QIcon::fromTheme("document-open"),
                          tr("Load TX rows from file..."),
                          this,
                          &CFrmSession::loadTxEventsAct);

  m_saveTxAct = m_fileMenu->addAction(QIcon::fromTheme("document-save-as"),
                                      tr("Write TX rows to file..."),
                                      this,
                                      &CFrmSession::saveTxEventsAct);

  m_exitAct = m_fileMenu->addAction(windowCloseIcon,
                                    tr("Close window"),
                                    this,
                                    &CFrmSession::close);
  m_exitAct->setShortcut(QKeySequence(tr("Ctrl+X")));
  m_exitAct->setStatusTip(tr("Close session window"));

  m_menuBar->addMenu(m_fileMenu);

  // * * * Edit menu * * *

  m_editMenu  = new QMenu(tr("&Edit"), this);
  m_copyRxAct = m_editMenu->addAction(tr("Copy RX event to clipboard"),
                                      this,
                                      &CFrmSession::copyRxToClipboard);

  m_copyTxAct = m_editMenu->addAction(tr("Copy TX event to clipboard"),
                                      this,
                                      &CFrmSession::copyTxToClipboard);

  m_copyRxToTxAct = m_editMenu->addAction(tr("Copy RX event to TX"),
                                          this,
                                          &CFrmSession::copyRxToTx);
  m_editMenu->addSeparator();

  m_clrRxSelectionsAct = m_editMenu->addAction(tr("Clear RX selections"),
                                               this,
                                               &CFrmSession::menu_unselect_all_rxlist);
  m_clrRxSelectionsAct->setShortcut(QKeySequence(tr("Alt+U")));
  m_clrRxSelectionsAct->setStatusTip(tr("Clear selections in RX list"));

  m_clrRxListAct = m_editMenu->addAction(tr("Clear RX list"),
                                         this,
                                         &CFrmSession::menu_clear_rxlist);
  m_clrRxListAct->setShortcut(QKeySequence(tr("Ctrl+N")));
  m_clrRxListAct->setStatusTip(tr("Clear RX list"));

  m_clrTxListAct = m_editMenu->addAction(tr("Clear TX list"),
                                         this,
                                         &CFrmSession::menu_clear_txlist);
  m_clrTxListAct->setShortcut(QKeySequence(tr("Alt+2")));
  m_clrTxListAct->setStatusTip(tr("Clear TX list"));

  m_editMenu->addSeparator();

  m_toggleRxRowMarkAct = m_editMenu->addAction(tr("Toggle RX row mark"),
                                               this,
                                               &CFrmSession::setVscpRowMark);
  m_toggleRxRowMarkAct->setShortcut(QKeySequence(tr("Alt+M")));
  m_toggleRxRowMarkAct->setStatusTip(tr("Mark the receive row with special background color"));

  m_toggleRxClassMarkAct = m_editMenu->addAction(tr("Toggle RX Class mark"),
                                                 this,
                                                 &CFrmSession::setVscpClassMark);
  m_toggleRxClassMarkAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
  m_toggleRxClassMarkAct->setStatusTip(tr("Mark the receive row with special background color"));

  m_toggleRxTypeMarkAct = m_editMenu->addAction(tr("Toggle RX Type mark"),
                                                this,
                                                &CFrmSession::setVscpTypeMark);
  m_toggleRxTypeMarkAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_T));
  m_toggleRxTypeMarkAct->setStatusTip(tr("Mark the receive row with special background color"));

  m_addExCommentAct = m_editMenu->addAction(tr("Add/Edit RX comment"),
                                            this,
                                            &CFrmSession::addEventNote);
  m_toggleRxTypeMarkAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_T));
  m_toggleRxTypeMarkAct->setStatusTip(tr("Add a comment to selected RX row(s)"));

  m_deleteRxCommentAct = m_editMenu->addAction(tr("Remove RX comment"),
                                               this,
                                               &CFrmSession::removeEventNote);
  m_deleteRxCommentAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_T));
  m_deleteRxCommentAct->setStatusTip(tr("Remove a comment from selected RX row(s)"));

  m_menuBar->addMenu(m_editMenu);

  // * * * Connection menu * * *

  // m_connMenu = new QMenu(tr("&Connection"), this);

  // m_connectAct    = m_connMenu->addAction(tr("Connect to interface..."),
  //                                      this,
  //                                      &CFrmSession::menu_connect);
  // m_connectAct->setShortcut(QKeySequence(tr("Ctrl+1")));
  // m_disconnectAct = m_connMenu->addAction(tr("Disconnect from interface..."),
  //                                         this,
  //                                         &CFrmSession::menu_disconnect);
  // m_pauseAct = m_connMenu->addAction(tr("Pause host"));
  // m_addHostAct = m_connMenu->addAction(tr("Add host..."));

  // m_menuBar->addMenu(m_connMenu);

  // * * * VSCP menu * * *
  m_vscpMenu      = new QMenu(tr("&VSCP"), this);
  m_readRegAct    = m_vscpMenu->addAction(tr("Read register..."));
  m_writeRegAct   = m_vscpMenu->addAction(tr("Write register..."));
  m_readAllRegAct = m_vscpMenu->addAction(tr("Read (all) registers..."));
  m_readGuidAct   = m_vscpMenu->addAction(tr("Read GUID"));
  m_readMdfAct    = m_vscpMenu->addAction(tr("Read MDF..."));
  m_loadMdfAct    = m_vscpMenu->addAction(tr("Download MDF..."));
  m_menuBar->addMenu(m_vscpMenu);

  // * * * Settings menu * * *
  m_settingsMenu = new QMenu(tr("&Setting"), this);
  m_setFilterAct = m_settingsMenu->addAction(tr("Set/define filter..."));
  m_settingsAct  = m_settingsMenu->addAction(tr("Settings..."));
  m_menuBar->addMenu(m_settingsMenu);
  connect(m_settingsAct,
          &QAction::triggered,
          this,
          &CFrmSession::menu_open_main_settings);

  // * * * MQTT* * *
  if (CVscpClient::connType::MQTT == m_vscpConnType) {
    m_mqttMenu          = new QMenu(tr("&MQTT"), this);
    m_mqttSubscriptions = m_mqttMenu->addAction(tr("MQTT subscribe topics..."),
                                                this,
                                                &CFrmSession::openMqttSubscribeTopics);
    m_mqttPublishTopics = m_mqttMenu->addAction(tr("MQTT publish topics..."),
                                                this,
                                                &CFrmSession::openMqttPublishTopics);
    m_mqttPublishTopics = m_mqttMenu->addAction(tr("Clear MQTT retain topics..."),
                                                this,
                                                &CFrmSession::openClearMqttRetainPublishTopics);
    m_menuBar->addMenu(m_mqttMenu);
  }

  // * * * Tools menu * * *
  m_toolsMenu = new QMenu(tr("&Tools"), this);
  m_menuBar->addMenu(m_toolsMenu);

  // Connections

  connect(m_exitAct, &QAction::triggered, this, &QDialog::accept);

  // #ifndef QT_NO_CLIPBOARD
  //     cutAct->setEnabled(false);

  //     copyAct->setEnabled(false);
  //     connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct,
  //     &QAction::setEnabled); connect(textEdit,
  //     &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
  // #endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// createToolbar
//

void
CFrmSession::createToolbar()
{
  // We need a toolbar
  m_toolBar = new QToolBar();

  m_toolBar->addSeparator();

  // Numerical base
  m_baseComboBox = new QComboBox;
  m_baseComboBox->addItem("Hex");
  m_baseComboBox->addItem("Decimal");
  m_baseComboBox->addItem("Octal");
  m_baseComboBox->addItem("Binary");
  m_toolBar->addWidget(m_baseComboBox);

  m_toolBar->addSeparator();

  QIcon disconnectIcon(":/disconnect.png");
  QIcon connectIcon(":/connect.png");

  m_connect = new QToolButton(this);
  m_connect->setCheckable(true);
  m_connect->setIcon(disconnectIcon);
  m_connect->setShortcut(QKeySequence(tr("Ctrl+O")));
  m_connect->setStatusTip(
    tr("Connect/disconnect to/from remote host or interface"));
  m_toolBar->addWidget(m_connect);

  connect(m_connect,
          SIGNAL(clicked(bool)),
          this,
          SLOT(connectToRemoteHost(bool)));

  // m_connectActBar = m_toolBar->addAction(m_connect,
  //                                            this,
  //                                            &CFrmSession::menu_connect);

  // Connect

  // m_connectActToolBar = m_toolBar->addAction(connectIcon,
  //                                            tr("Connect"),
  //                                            this,
  //                                            &CFrmSession::menu_connect);
  // m_connectActToolBar->setShortcut(QKeySequence(tr("Ctrl+O")));
  // m_connectActToolBar->setStatusTip(
  //   tr("Connect/disconnect to/from remote host or interface"));
  // m_connectActToolBar->setCheckable(true);

  m_toolBar->addSeparator();

  // Filter
  // https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
  m_setFilterActToolBar = m_toolBar->addAction(QIcon(":/filter.png"),
                                               tr("Enable filter"),
                                               this,
                                               &CFrmSession::menu_filter_enable);
  m_setFilterActToolBar->setStatusTip(tr("Enable/disable filter"));
  m_setFilterActToolBar->setCheckable(true);

  m_filterComboBox = new QComboBox;
  m_filterComboBox->addItem("Filter 1");
  m_filterComboBox->addItem("Filter 2");
  m_filterComboBox->addItem("Filter 3");
  m_filterComboBox->addItem("Filter 4");
  m_toolBar->addWidget(m_filterComboBox);

  // Filter handling preferences-other
  m_setFilterActToolBar = m_toolBar->addAction(QIcon(":/process_accept.png"),
                                               tr("Filters..."),
                                               this,
                                               &CFrmSession::menu_filter_config);

  m_toolBar->addSeparator();

  m_lcdNumber = new QLCDNumber;
  m_lcdNumber->setDigitCount(7);
  m_lcdNumber->setSegmentStyle(QLCDNumber::Filled);
  m_toolBar->addWidget(m_lcdNumber);

  // Clear
  QIcon clearIcon(":/remove.png");
  m_setClearRcvListActToolBar =
    m_toolBar->addAction(clearIcon,
                         tr("Clear receive list"),
                         this,
                         &CFrmSession::menu_clear_rxlist);
  // m_btnClearRcvList = new QPushButton;
  // m_btnClearRcvList->setText("Clear");
  // m_toolBar->addWidget(m_btnClearRcvList);

  m_toolBar->addSeparator();

  // Clear
  QIcon unselectAllIcon(":/down.png");
  m_setUnselectAllActToolBar =
    m_toolBar->addAction(unselectAllIcon,
                         tr("Unselect all RX rows"),
                         this,
                         &CFrmSession::menu_unselect_all_rxlist);

  m_toolBar->addSeparator();

  // Connections

  // connect(m_connectActToolBar,
  //         SIGNAL(triggered(bool)),
  //         this,
  //         SLOT(connectToRemoteHost(bool)));

  // connect(m_connectAct,
  //         SIGNAL(triggered()),
  //         this,
  //         SLOT(menu_connect()));

  // connect(m_disconnectAct,
  //         SIGNAL(triggered()),
  //         this,
  //         SLOT(menu_disconnect()));
}

///////////////////////////////////////////////////////////////////////////////
// createHorizontalGroupBox
//

void
CFrmSession::createHorizontalGroupBox()
{
  m_horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
  QHBoxLayout* layout  = new QHBoxLayout;

  for (int i = 0; i < NumButtons; ++i) {
    m_buttons[i] = new QPushButton(tr("Button %1").arg(i + 1));
    layout->addWidget(m_buttons[i]);
  }
  m_horizontalGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createRxGridGroup
//

void
CFrmSession::createRxGroupBox()
{
  m_gridGroupBox      = new QGroupBox(tr("Receive Events "));
  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins(1, 1, 1, 1);

  QStringList headers(
    QString(tr("Dir, VSCP Class, VSCP Type, id, GUID")).split(','));
  m_rxTable = new QTableWidget;
  m_rxTable->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu
  m_rxTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_rxTable->setAlternatingRowColors(true);

  m_rxTable->setColumnCount(5);
  m_rxTable->setColumnWidth(0, 10);  // Dir
  m_rxTable->setColumnWidth(1, 200); // Class
  m_rxTable->setColumnWidth(2, 150); // Type
  m_rxTable->setColumnWidth(3, 50);  // Node id
  m_rxTable->setColumnWidth(4, 50);  // GUID
  m_rxTable->horizontalHeader()->setStretchLastSection(true);
  m_rxTable->setHorizontalHeaderLabels(headers);

  layout->addWidget(m_rxTable,
                    0,
                    0,
                    1,
                    4); //  fromRow, fromColumn, rowSpan, columnSpan

  m_rxTable->show();

  m_infoArea = new QTextBrowser;
  m_infoArea->setAcceptRichText(true);
  m_infoArea->setOpenLinks(true);
  m_infoArea->setOpenExternalLinks(true);
  QColor grey(Qt::red);
  m_infoArea->setTextBackgroundColor(grey);

  layout->addWidget(m_infoArea,
                    0,
                    4,
                    1,
                    2); //  fromRow, fromColumn, rowSpan, columnSpan
  layout->setColumnMinimumWidth(4, 350);

  // 30 10
  layout->setColumnStretch(0, 30);
  layout->setColumnStretch(1, 10);
  m_gridGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createTxGridGroup
//

void
CFrmSession::createTxGridGroup()
{
  m_txGroupBox        = new QGroupBox(tr("Transmit"));
  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins(1, 1, 1, 1);

  QStringList headers(QString(tr("x,Name,Period,Count,Event")).split(','));
  m_txTable = new QTableWidget;
  m_txTable->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu
  m_txTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_txTable->setAlternatingRowColors(true);
  m_txTable->setColumnCount(5);
  m_txTable->setColumnWidth(txrow_enable, 80); // x
  m_txTable->setColumnWidth(txrow_name, 200);  // Name
  m_txTable->setColumnWidth(txrow_period, 80); // Period
  m_txTable->setColumnWidth(txrow_count, 80);  // Count
  m_txTable->setColumnWidth(txrow_event, 50);  // Event
  m_txTable->horizontalHeader()->setStretchLastSection(true);
  m_txTable->setHorizontalHeaderLabels(headers);
  layout->addWidget(m_txTable,
                    0,
                    0,
                    1,
                    4); //  fromRow, fromColumn, rowSpan, columnSpan

  // Transmit/Add/Clone/Delete/Edit/Save/Load
  m_txToolBar = new QToolBar;
  m_txToolBar->setOrientation(Qt::Vertical);

  // Transmit
  const QIcon newIcon =
    QIcon::fromTheme("edit-undo"); // QIcon::fromTheme("document-new",
                                   // QIcon(":/images/new.png"));
  QAction* transmitAct = new QAction(newIcon, tr("&Transmit"), this);
  transmitAct->setShortcuts(QKeySequence::New);
  transmitAct->setStatusTip(tr("Transmit selected event(s)"));
  connect(transmitAct, &QAction::triggered, this, &CFrmSession::sendTxEvent);
  m_txToolBar->addAction(transmitAct);

  // Add tx row
  const QIcon addIcon =
    QIcon::fromTheme("document-new"); // QIcon::fromTheme("document-new",
                                      // QIcon(":/images/new.png"));
  QAction* addAct = new QAction(addIcon, tr("&Add"), this);
  addAct->setShortcuts(QKeySequence::New);
  addAct->setStatusTip(tr("Add transmit event"));
  connect(addAct, &QAction::triggered, this, &CFrmSession::addTxEvent);
  m_txToolBar->addAction(addAct);

  // Edit tx row
  const QIcon editIcon = QIcon::fromTheme("accessories-text-editor");
  QAction* editAct     = new QAction(editIcon, tr("&Edit"), this);
  editAct->setShortcuts(QKeySequence::New);
  editAct->setStatusTip(tr("Edit selected event"));
  connect(editAct, &QAction::triggered, this, &CFrmSession::editTxEvent);
  m_txToolBar->addAction(editAct);

  // Clone tx row
  const QIcon cloneIcon = QIcon::fromTheme("edit-copy");
  QAction* cloneAct     = new QAction(cloneIcon, tr("&Clone"), this);
  cloneAct->setShortcuts(QKeySequence::New);
  cloneAct->setStatusTip(tr("Clone selected event"));
  connect(cloneAct, &QAction::triggered, this, &CFrmSession::cloneTxEvent);
  m_txToolBar->addAction(cloneAct);

  // Delete tx row
  const QIcon deleteIcon = QIcon::fromTheme("edit-delete");
  QAction* deleteAct     = new QAction(deleteIcon, tr("&Delete"), this);
  deleteAct->setShortcuts(QKeySequence::New);
  deleteAct->setStatusTip(tr("Delete selected event"));
  connect(deleteAct, &QAction::triggered, this, &CFrmSession::deleteTxEvent);
  m_txToolBar->addAction(deleteAct);

  m_txToolBar->addSeparator();

  // Save tx rows
  const QIcon saveIcon = QIcon::fromTheme("document-save");
  QAction* saveAct     = new QAction(saveIcon, tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::New);
  saveAct->setStatusTip(tr("Save selected transmit event(s)"));
  connect(saveAct, &QAction::triggered, this, &CFrmSession::saveTxEventsAct);
  m_txToolBar->addAction(saveAct);

  // Load tx rows
  const QIcon loadIcon = QIcon::fromTheme("document-open");
  QAction* loadAct     = new QAction(loadIcon, tr("&Load"), this);
  loadAct->setShortcuts(QKeySequence::New);
  loadAct->setStatusTip(tr("Load transmit event(s)"));
  connect(loadAct, &QAction::triggered, this, &CFrmSession::loadTxEventsAct);
  m_txToolBar->addAction(loadAct);

  layout->addWidget(m_txToolBar, 0, 4, 1, 1);

  layout->setColumnStretch(0, 70);
  layout->setColumnStretch(1, 10);
  m_txGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// menu_connect
//

void
CFrmSession::menu_connect()
{
  // const QIcon connectIcon    = QIcon::fromTheme(":/connect.png");
  // const QIcon disconnectIcon = QIcon::fromTheme(":/disconnect.png");
  //  connectToRemoteHost(true);
  //   m_connectActToolBar->blockSignals(true);
  //   m_connectActToolBar->toggled(true);
  //   m_connectActToolBar->blockSignals(false);
  //  if (m_connectActToolBar->isChecked()) {
  //    m_connectActToolBar->setIcon(connectIcon);
  //  }
  //  else {
  //    m_connectActToolBar->setIcon(disconnectIcon);
  //  }
  //  m_connectAct->setIconVisibleInMenu(true);
}

///////////////////////////////////////////////////////////////////////////////
// menu_disconnect
//

// void
// CFrmSession::menu_disconnect()
// {
//   const QIcon connectIcon    = QIcon::fromTheme("call-start");
//   const QIcon disconnectIcon = QIcon::fromTheme("call-stop");
//   // connectToRemoteHost(false);
//   //  m_connectActToolBar->blockSignals(true);
//   //  m_connectActToolBar->toggled(false);
//   //  m_connectActToolBar->blockSignals(false);
//   //  if ( m_connectAct->isChecked()) {
//   //      m_connectAct->setIcon(connectIcon);
//   //  }
//   //  else {
//   //      m_connectAct->setIcon(disconnectIcon);
//   //  }
//   //  m_connectAct->setIconVisibleInMenu(true);
// }

///////////////////////////////////////////////////////////////////////////////
// menu_filter_config
//

void
CFrmSession::menu_filter_config()
{
  CDlgSessionFilter dlg;
  if (QDialog::Accepted == dlg.exec()) {
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("filter set"),
      QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// menu_filter_enable
//

void
CFrmSession::menu_filter_enable()
{
  QMessageBox::information(
    this,
    tr(APPNAME),
    tr("filter enable"),
    QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// menu_clear_rxlist
//

void
CFrmSession::menu_clear_rxlist(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  m_infoArea->clear();

  m_rxTable->setCurrentCell(-1, -1); // unselect all

  m_mutexRxList.lock();

  // Clear rx list
  while (m_rxTable->rowCount()) {
    m_rxTable->removeRow(0);
  }

  // Remove receive events
  while (m_rxEvents.size()) {
    vscpEvent* pev = m_rxEvents.front();
    m_rxEvents.pop_front();
    vscp_deleteEvent(pev);
    pev = nullptr;
  }

  // Clear the event counter
  m_mapRxEventToCount.clear();

  // Erase all comments
  m_mapRxEventComment.clear();

  // Display number of items
  m_lcdNumber->display(m_rxTable->rowCount());

  m_mutexRxList.unlock();

  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// menu_unselect_all_rxlist
//

void
CFrmSession::menu_unselect_all_rxlist(void)
{
  m_rxTable->clearSelection();
}

///////////////////////////////////////////////////////////////////////////////
// menu_open_main_settings
//

void
CFrmSession::menu_open_main_settings(void)
{
  CDlgMainSettings* dlg = new CDlgMainSettings(this);
  dlg->exec();
  // Update row in case info changed
  updateAllRows();
  updateCurrentRow();
}

///////////////////////////////////////////////////////////////////////////////
// txItemChanged
//

void
CFrmSession::txItemChanged(QTableWidgetItem* item)
{
  QMessageBox::about(this, tr("Item changed"), tr("Carpe Diem"));
}

///////////////////////////////////////////////////////////////////////////////
// txRowDoubleClicked
//

void
CFrmSession::txRowDoubleClicked(int row, int column)
{
  sendTxEvent();
}

///////////////////////////////////////////////////////////////////////////////
// showTxContextMenu
//

void
CFrmSession::showTxContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Send event")),
                  this,
                  SLOT(sendTxEvent()));

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

  menu->popup(m_txTable->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// addTxRow
//

bool
CFrmSession::addTxRow(bool bEnable,
                      const QString& name,
                      uint16_t count,
                      uint32_t period,
                      const QString& event)
{
  QTableWidgetItem* item;

  int row = m_txTable->rowCount();
  m_txTable->insertRow(row);

  // Enable
  item = new QTableWidgetItem;
  if (nullptr == item) {
    return false;
  }

  item->setText("Enable");
  item->setCheckState(bEnable ? Qt::Checked : Qt::Unchecked);
  m_txTable->setItem(m_txTable->rowCount() - 1, txrow_enable, item);

  // Name
  item = new QTableWidgetItem;
  if (nullptr == item) {
    return false;
  }
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setText(name);
  // Bluish
  item->setForeground(QBrush(QColor(0, 5, 180)));
  m_txTable->setItem(m_txTable->rowCount() - 1, txrow_name, item);

  // Count
  if (!count)
    count = 1; // Minvalue
  item = new QTableWidgetItem;
  if (nullptr == item) {
    return false;
  }
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setTextAlignment(Qt::AlignHCenter);
  item->setText(QString::number(count));
  m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

  // Period
  item = new QTableWidgetItem;
  if (nullptr == item) {
    return false;
  }
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setTextAlignment(Qt::AlignHCenter);
  item->setText(QString::number(period));
  m_txTable->setItem(m_txTable->rowCount() - 1, txrow_period, item);

  // Event
  CTxWidgetItem* itemEvent = new CTxWidgetItem("no name");
  if (nullptr == itemEvent) {
    return false;
  }

  itemEvent->m_tx.setEnable(bEnable);
  itemEvent->m_tx.setName(name);
  itemEvent->m_tx.setCount(count);
  itemEvent->m_tx.setPeriod(period);

  // Allocate new Event
  if (!itemEvent->m_tx.newEvent()) {
    delete itemEvent;
    return false;
  }

  vscpEvent* pev = itemEvent->m_tx.getEvent();
  if (!vscp_convertStringToEvent(pev, event.toStdString())) {
    delete itemEvent;
    return false;
  }

  itemEvent->setFlags(item->flags() & ~Qt::ItemIsEditable);
  itemEvent->setTextAlignment(Qt::AlignLeft);
  QString strEvent = getClassInfo(itemEvent->m_tx.getEvent());
  strEvent += "→";
  strEvent += getTypeInfo(itemEvent->m_tx.getEvent());
  strEvent += ": ";
  std::string str;
  vscp_writeDataWithSizeToString(str, pev->pdata, pev->sizeData);
  strEvent += str.c_str();
  itemEvent->setText(strEvent);
  itemEvent->setForeground(QBrush(QColor(0, 99, 0)));
  m_txTable->setItem(m_txTable->rowCount() - 1, txrow_event, itemEvent);

  m_txTable->setUpdatesEnabled(false);
  for (int i = 0; i < m_txTable->rowCount(); i++) {
    m_txTable->setRowHeight(i, 10);
  }

  m_txTable->setUpdatesEnabled(true);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
CFrmSession::isConnected(void)
{
  return m_vscpClient->isConnected();
}

///////////////////////////////////////////////////////////////////////////////
// menu_clear_txlist
//

void
CFrmSession::menu_clear_txlist(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  m_txTable->setCurrentCell(-1, -1); // unselect all

  m_mutexTxList.lock();

  // Clear TX list
  while (m_txTable->rowCount()) {
    m_txTable->removeRow(0);
  }

  m_mutexTxList.unlock();

  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// copyTxToClipboard
//

void
CFrmSession::copyTxToClipboard(void)
{
  QString str;
  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  // Must be at least one selected item
  if (!selection.size()) {
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("There is no TX row selected."),
      QMessageBox::Ok);
    return;
  }

  bool ok;
  int format = 0;
  QStringList items;
  items << tr("STRING") << tr("JSON") << tr("XML") << tr("HTML");
  QString item = QInputDialog::getItem(this, tr("Format"), tr("Season:"), items, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    if (item == "STRING") {
      format = 0;
    }
    else if (item == "JSON") {
      format = 1;
    }
    else if (item == "XML") {
      format = 2;
    }
    else if (item == "HTML") {
      format = 3;
    }
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {

    CTxWidgetItem* itemEvent = (CTxWidgetItem*)m_txTable->item(it->row(), txrow_event);
    vscpEvent* pev           = itemEvent->m_tx.getEvent();
    if (nullptr != pev) {

      bool rv;
      std::string strEvent;

      switch (format) {

        case 1:
          rv = vscp_convertEventToJSON(strEvent, pev);
          break;

        case 2:
          rv = vscp_convertEventToXML(strEvent, pev);
          break;

        case 3:
          rv = vscp_convertEventToHTML(strEvent, pev);
          break;

        case 0:
        default:
          rv = vscp_convertEventToString(strEvent, pev);
          break;
      }

      if (rv) {
        if (str.length())
          str += "\r\n";
        str += strEvent.c_str();
      }
      else {
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to convert event to requested format."),
          QMessageBox::Ok);
      }
    }
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// sendTxEvent
//

void
CFrmSession::sendTxEvent(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!isConnected()) {
    QMessageBox::warning(this,
                         tr(APPNAME),
                         tr("Connection must be open/active to be able to send events"),
                         QMessageBox::Ok);
    return;
  }

  if (!selection.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("You must select a TX row"),
                             QMessageBox::Ok);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {

    CTxWidgetItem* itemEvent = (CTxWidgetItem*)m_txTable->item(it->row(), txrow_event);
    vscpEvent* pev           = itemEvent->m_tx.getEvent();
    pev->timestamp           = vscp_makeTimeStamp(); // Set timestamp
    vscp_setEventToNow(pev);                         // Set time information to "now"

    setUpdatesEnabled(false);
    for (int i = 0; i < itemEvent->m_tx.getCount(); i++) {
      // Send Event
      if (VSCP_ERROR_SUCCESS != m_vscpClient->send(*pev)) {
        spdlog::error(std::string(tr("Session: Unable to send event").toStdString()));
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Unable to send event(s)"),
          QMessageBox::Ok);
        continue;
      }
      receiveTxRow(pev);
    }
    setUpdatesEnabled(true);
  }

  QApplication::restoreOverrideCursor();
  QApplication::processEvents();
}

///////////////////////////////////////////////////////////////////////////////
// addTxEvent
//

void
CFrmSession::addTxEvent(void)
{
  QTableWidgetItem* item;
  CDlgTxEdit dlg;

  if (QDialog::Accepted == dlg.exec()) {

    int row = m_txTable->rowCount();
    m_txTable->insertRow(row);

    // Enable
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }

    item->setText("Enable");
    item->setCheckState(dlg.getEnable() ? Qt::Checked : Qt::Unchecked);
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_enable, item);

    // Name
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setText(dlg.getName());
    // Bluish
    item->setForeground(QBrush(QColor(0, 5, 180)));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_name, item);

    // Count
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(dlg.getCount()));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

    // Period
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(dlg.getPeriod()));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_period, item);

    // Event
    CTxWidgetItem* itemEvent = new CTxWidgetItem("Test");
    if (nullptr == itemEvent) {
      return;
    }

    itemEvent->m_tx.setEnable(dlg.getEnable());
    itemEvent->m_tx.setName(dlg.getName());
    itemEvent->m_tx.setCount(dlg.getCount());
    itemEvent->m_tx.setPeriod(dlg.getPeriod());

    // Allocate new Event
    if (!itemEvent->m_tx.newEvent()) {
      delete itemEvent;
      return;
    }

    vscpEvent* pev = itemEvent->m_tx.getEvent();

    pev->vscp_class = dlg.getVscpClass();
    pev->vscp_type  = dlg.getVscpType();

    QString strGuid = dlg.getGuid();
    cguid guid(strGuid.toStdString());
    memcpy(pev->GUID, guid.getGUID(), 16);

    pev->head = dlg.getPriority() << 5;

    vscp_setEventDataFromString(pev, dlg.getData().toStdString());
    qDebug() << QString::number(pev->sizeData);
    for (int i = 0; i < pev->sizeData; i++) {
      qDebug() << QString::number(pev->pdata[i]);
    }

    itemEvent->setFlags(item->flags() & ~Qt::ItemIsEditable);
    itemEvent->setTextAlignment(Qt::AlignLeft);
    QString strEvent = getClassInfo(itemEvent->m_tx.getEvent());
    strEvent += "→";
    strEvent += getTypeInfo(itemEvent->m_tx.getEvent());
    strEvent += ": ";
    std::string str;
    vscp_writeDataWithSizeToString(str, pev->pdata, pev->sizeData);
    strEvent += str.c_str();
    itemEvent->setText(strEvent);
    itemEvent->setForeground(QBrush(QColor(0, 99, 0)));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_event, itemEvent);

    m_txTable->setUpdatesEnabled(false);
    for (int i = 0; i < m_txTable->rowCount(); i++) {
      m_txTable->setRowHeight(i, 10);
    }
    m_txTable->setUpdatesEnabled(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editTxEvent
//

void
CFrmSession::editTxEvent(void)
{
  std::string str;
  CDlgTxEdit dlg;

  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!selection.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("You must select a transmission row"),
                             QMessageBox::Ok);
    return;
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {

    CTxWidgetItem* itemEvent = (CTxWidgetItem*)m_txTable->item(it->row(), txrow_event);
    vscpEvent* pev           = itemEvent->m_tx.getEvent();

    dlg.setEnable(itemEvent->m_tx.getEnable());
    dlg.setName(itemEvent->m_tx.getName());
    dlg.setCount(itemEvent->m_tx.getCount());
    dlg.setPeriod(itemEvent->m_tx.getPeriod());

    dlg.setVscpClassType(pev->vscp_class, pev->vscp_type);
    cguid guid(pev->GUID);
    dlg.setGuid(guid.getAsString().c_str());

    vscp_writeDataWithSizeToString(str, pev->pdata, pev->sizeData);
    dlg.setData(str.c_str());

    dlg.setPriority((pev->head >> 5) & 7);

    if (QDialog::Accepted == dlg.exec()) {

      QTableWidgetItem* item;

      // Enable
      itemEvent->m_tx.setEnable(dlg.getEnable());
      item = m_txTable->item(it->row(), txrow_enable);
      item->setCheckState(dlg.getEnable() ? Qt::Checked : Qt::Unchecked);

      // Name
      itemEvent->m_tx.setName(dlg.getName());
      item = m_txTable->item(it->row(), txrow_name);
      item->setText(dlg.getName());

      // Count
      itemEvent->m_tx.setCount(dlg.getCount());
      item = m_txTable->item(it->row(), txrow_count);
      item->setText(QString::number(dlg.getCount()));

      // Period
      itemEvent->m_tx.setPeriod(dlg.getPeriod());
      item = m_txTable->item(it->row(), txrow_period);
      item->setText(QString::number(dlg.getPeriod()));

      pev->vscp_class = dlg.getVscpClass();
      pev->vscp_type  = dlg.getVscpType();

      QString strGuid = dlg.getGuid();
      cguid guid(strGuid.toStdString());
      memcpy(pev->GUID, guid.getGUID(), 16);

      pev->head = dlg.getPriority() << 5;

      // Delete old data (if any)
      if (nullptr != pev->pdata) {
        delete[] pev->pdata;
        pev->sizeData = 0;
        pev->pdata    = 0;
      }

      // Get data
      vscp_setEventDataFromString(pev, dlg.getData().toStdString());

      // Update TX table line
      QString strEvent = getClassInfo(itemEvent->m_tx.getEvent());
      strEvent += "→";
      strEvent += getTypeInfo(itemEvent->m_tx.getEvent());
      strEvent += ": ";
      std::string str;
      vscp_writeDataWithSizeToString(str, pev->pdata, pev->sizeData);
      strEvent += str.c_str();
      itemEvent->setText(strEvent);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// cloneTxEvent
//

void
CFrmSession::cloneTxEvent(void)
{
  vscpEvent* pev;
  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!selection.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("You must select a transmission row"),
                             QMessageBox::Ok);
    return;
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {

    QTableWidgetItem* item;
    CTxWidgetItem* itemSourceEvent = (CTxWidgetItem*)m_txTable->item(it->row(), txrow_event);
    if (nullptr == itemSourceEvent) {
      return;
    }
    CTxWidgetItem* itemTargetEvent = new CTxWidgetItem("new");
    if (nullptr == itemTargetEvent) {
      return;
    }

    itemTargetEvent->m_tx.newEvent();

    // Copy in event
    vscp_copyEvent(itemTargetEvent->m_tx.getEvent(), itemSourceEvent->m_tx.getEvent());

    // Save data
    // vscpEvent* pevSource = itemSourceEvent->m_tx.getEvent();
    bool bEnable = itemSourceEvent->m_tx.getEnable();
    QString name = itemSourceEvent->m_tx.getName();
    name += tr("_copy");
    uint16_t count  = itemSourceEvent->m_tx.getCount();
    uint32_t period = itemSourceEvent->m_tx.getPeriod();

    // Add new row
    int row = m_txTable->rowCount();
    m_txTable->insertRow(row);

    // Enable
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }

    item->setText("Enable");
    item->setCheckState(bEnable ? Qt::Checked : Qt::Unchecked);
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_enable, item);

    // Name
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setText(name);
    // Bluish
    item->setForeground(QBrush(QColor(0, 5, 180)));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_name, item);

    // Count
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(count));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

    // Period
    item = new QTableWidgetItem;
    if (nullptr == item) {
      return;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(period));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_period, item);

    // Event

    itemTargetEvent->m_tx.setEnable(bEnable);
    itemTargetEvent->m_tx.setName(name);
    itemTargetEvent->m_tx.setCount(count);
    itemTargetEvent->m_tx.setPeriod(period);

    pev = itemTargetEvent->m_tx.getEvent();

    itemTargetEvent->setFlags(item->flags() & ~Qt::ItemIsEditable);
    itemTargetEvent->setTextAlignment(Qt::AlignLeft);
    QString strEvent = getClassInfo(pev);
    strEvent += "→";
    strEvent += getTypeInfo(pev);
    strEvent += ": ";
    std::string str;
    vscp_writeDataWithSizeToString(str, pev->pdata, pev->sizeData);
    strEvent += str.c_str();
    itemTargetEvent->setText(strEvent);
    itemTargetEvent->setForeground(QBrush(QColor(0, 99, 0)));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_event, itemTargetEvent);

    m_txTable->setUpdatesEnabled(false);
    for (int i = 0; i < m_txTable->rowCount(); i++) {
      m_txTable->setRowHeight(i, 10);
    }
    m_txTable->setUpdatesEnabled(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteTxEvent
//

void
CFrmSession::deleteTxEvent(void)
{
  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!selection.size()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("You must select a transmission row"),
                             QMessageBox::Ok);
    return;
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    m_txTable->removeRow(it->row());
  }
}

///////////////////////////////////////////////////////////////////////////////
// clrSelectionsTxEvent
//

void
CFrmSession::clrSelectionsTxEvent(void)
{
  m_txTable->setCurrentCell(-1, 0);
}

///////////////////////////////////////////////////////////////////////////////
// loadTxEvents
//

void
CFrmSession::loadTxEvents(const QString& path)
{
  QString fileName = path;

  vscpworks* pworks         = (vscpworks*)QCoreApplication::instance();
  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!path.length()) {
    // QString initialPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString initialPath = pworks->m_shareFolder + "/txsets/txset.xml";
    fileName            = QFileDialog::getOpenFileName(this,
                                            tr("File to load transmitt events from"),
                                            initialPath,
                                            tr("TX files (*.xml *.*)"));
  }

  spdlog::debug("Loading tx events from {}", fileName.toStdString());

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    spdlog::error("Cannot read file - {}", file.errorString().toStdString());
    return;
  }

  QXmlStreamReader reader(&file);

  if (reader.readNextStartElement()) {
    if (reader.name() == "txrows") {
      qDebug() << reader.name();
      while (reader.readNextStartElement()) {

        bool bEnable    = false;
        QString name    = tr("no name");
        uint16_t count  = 1;
        uint32_t period = 0;
        QString event;

        if (reader.name() == "row") {

          // enable
          if (reader.attributes().hasAttribute("enable")) {
            QString enable =
              reader.attributes().value("enable").toString();
            if (enable.contains("true", Qt::CaseInsensitive))
              bEnable = true;
          }

          // name
          if (reader.attributes().hasAttribute("name")) {
            name =
              reader.attributes().value("name").toString();
          }

          // count
          if (reader.attributes().hasAttribute("count")) {
            count =
              reader.attributes().value("count").toUInt();
          }

          // period
          if (reader.attributes().hasAttribute("period")) {
            period =
              reader.attributes().value("period").toULong();
          }

          // event
          if (reader.attributes().hasAttribute("event")) {
            event =
              reader.attributes().value("event").toString();
          }
        }

        // qDebug() << bEnable;
        // qDebug() << name;
        // qDebug() << count;
        // qDebug() << period;
        // qDebug() << event;
        addTxRow(bEnable, name, count, period, event);

        reader.skipCurrentElement();
      }
    }
    else
      reader.raiseError(QObject::tr("Incorrect file"));
  }

  file.close();
}

///////////////////////////////////////////////////////////////////////////////
// loadTxOnStart
//

void
CFrmSession::loadTxOnStart(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  //  QStandardPaths::AppLocalDataLocation
  // QString loadPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QString loadPath = pworks->m_shareFolder + "/cache/txevents_";
  loadPath += m_connObject["uuid"].toString();
  loadPath += ".xml";
  qDebug() << loadPath;

  if (pworks->m_session_bAutoSaveTxRows) {
    loadTxEvents(loadPath);
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveTxEvents
//

void
CFrmSession::saveTxEvents(const QString& path)
{
  QString fileName = path;
  // vscpEvent* pev;

  vscpworks* pworks         = (vscpworks*)QCoreApplication::instance();
  QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

  if (!path.length()) {
    QString initialPath = pworks->m_shareFolder + "/txsets/txset.xml";
    fileName            = QFileDialog::getSaveFileName(this,
                                            tr("File to save transmition events to"),
                                            initialPath,
                                            tr("TX files (*.xml *.*)"));
  }

  spdlog::debug("Saving tx events to {}", fileName.toStdString());

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

    // We're going to streaming text to the file
    QXmlStreamWriter stream(&file);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("txrows");

    if (selection.size()) {

      // Save selected items
      QList<QModelIndex>::iterator it;
      for (it = selection.begin(); it != selection.end(); it++) {
        CTxWidgetItem* itemSourceEvent =
          (CTxWidgetItem*)m_txTable->item(it->row(),
                                          txrow_event);

        stream.writeStartElement("row");
        stream.writeAttribute("enable",
                              itemSourceEvent->m_tx.getEnable() ? "true" : "false");
        stream.writeAttribute("name",
                              itemSourceEvent->m_tx.getName());
        stream.writeAttribute("count",
                              QString::number(itemSourceEvent->m_tx.getCount()));
        stream.writeAttribute("period",
                              QString::number(itemSourceEvent->m_tx.getPeriod()));
        std::string str;
        vscp_convertEventToString(str, itemSourceEvent->m_tx.getEvent());
        stream.writeAttribute("event", str.c_str());
        // stream.writeTextElement("test", "This is a test");

        stream.writeEndElement(); // row
      }
    }
    else {

      // save all
      for (int i = 0; i < m_txTable->rowCount(); i++) {
        CTxWidgetItem* itemSourceEvent = (CTxWidgetItem*)m_txTable->item(i, txrow_event);

        stream.writeStartElement("row");
        stream.writeAttribute("enable",
                              itemSourceEvent->m_tx.getEnable() ? "true" : "false");
        stream.writeAttribute("name",
                              itemSourceEvent->m_tx.getName());
        stream.writeAttribute("count",
                              QString::number(itemSourceEvent->m_tx.getCount()));
        stream.writeAttribute("period",
                              QString::number(itemSourceEvent->m_tx.getPeriod()));
        std::string str;
        vscp_convertEventToString(str, itemSourceEvent->m_tx.getEvent());
        stream.writeAttribute("event", str.c_str());

        stream.writeEndElement(); // row
      }
    }

    stream.writeEndElement(); // txrows

    stream.writeEndDocument();
    file.close();
  }
  else {
    // Failed to open file
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveTxOnExit
//

void
CFrmSession::saveTxOnExit(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_txTable->setCurrentCell(-1, -1); // unselect all

  //  QStandardPaths::AppLocalDataLocation
  // QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QString savePath = pworks->m_shareFolder + "/cache/txevents_";
  savePath += m_connObject["uuid"].toString();
  savePath += ".xml";
  qDebug() << savePath;

  if (pworks->m_session_bAutoSaveTxRows) {
    saveTxEvents(savePath);
  }
}

///////////////////////////////////////////////////////////////////////////////
// createFormGroupBox
//

void
CFrmSession::createFormGroupBox()
{
  m_formGroupBox      = new QGroupBox(tr("Form layout"));
  QFormLayout* layout = new QFormLayout;
  layout->addRow(new QLabel(tr("Line 1:")), new QLineEdit);
  layout->addRow(new QLabel(tr("Line 2, long text:")), new QComboBox);
  layout->addRow(new QLabel(tr("Line 3:")), new QSpinBox);
  m_formGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// connectToHost
//

void
CFrmSession::connectToRemoteHost(bool checked)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (checked) {
    doConnectToRemoteHost();
    QIcon connectIcon(":/connect.png");
    m_connect->setIcon(connectIcon);
    m_connect->setChecked(true);
  }
  else {
    doDisconnectFromRemoteHost();
    QIcon disconnectIcon(":/disconnect.png");
    m_connect->setIcon(disconnectIcon);
    m_connect->setChecked(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// doConnectToRemoteHost
//

void
CFrmSession::doConnectToRemoteHost(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);
        spdlog::error(std::string(tr("Session: Unable to connect to remote host.").toStdString()));
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to open a connection to the remote host (see log for more info)."),
          QMessageBox::Ok);
      }
      else {
        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);
        spdlog::info(std::string(tr("Session: Successful connect to remote client.").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);
        QString str = tr("Session: Unable to connect to the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to open a connection to the CANAL driver (see log for more info)."),
          QMessageBox::Ok);
      }
      else {
        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);
        spdlog::info(std::string(tr("Session: Successful connected to the CANAL driver.").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);
        QString str = tr("Session: Unable to connect to the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to open a connection to SOCKETCAN (see log for more info)."),
          QMessageBox::Ok);
      }
      else {
        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);
        spdlog::info(std::string(tr("Session: Successful connected to SOCKETCAN.").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);
        spdlog::error(std::string(tr("Session: Unable to connect to remote host").toStdString()));
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to open a connection to the remote host (see log for more info)."),
          QMessageBox::Ok);
      }
      else {
        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);
        spdlog::info(std::string(tr("Session: Successful connect to remote host").toStdString()));
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
CFrmSession::doDisconnectFromRemoteHost(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (m_vscpConnType) {

    case CVscpClient::connType::NONE:
      break;

    case CVscpClient::connType::TCPIP:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {

        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);

        spdlog::error(std::string(tr("Session: Unable to disconnect tcp/ip remote client").toStdString()));
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to disconnect the connection to the txp/ip remote host"),
          QMessageBox::Ok);
      }
      else {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);

        spdlog::info(std::string(tr("Session: Successful disconnect from tcp/ip remote host").toStdString()));
      }
      break;

    case CVscpClient::connType::CANAL:

      // Remove callback

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {

        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);

        QString str = tr("Session: Unable to disconnect from the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the CANAL driver"),
                                 QMessageBox::Ok);
      }
      else {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);

        spdlog::info(std::string(tr("Session: Successful disconnect from CANAL driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::SOCKETCAN:

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {

        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);

        QString str = tr("Session: Unable to disconnect from the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to disconnect the connection to the SOCKETCAN driver"),
                                 QMessageBox::Ok);
      }
      else {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);

        spdlog::info(std::string(tr("Session: Successful disconnect from SOCKETCAN driver").toStdString()));
      }
      QApplication::restoreOverrideCursor();
      break;

    case CVscpClient::connType::WS1:
      break;

    case CVscpClient::connType::WS2:
      break;

    case CVscpClient::connType::MQTT:
      if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {

        QIcon connectIcon(":/connect.png");
        m_connect->setIcon(connectIcon);
        m_connect->setChecked(true);

        spdlog::error(std::string(tr("Session: Unable to disconnect from MQTT remote client").toStdString()));
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to disconnect the connection to the MQTT remote host"),
          QMessageBox::Ok);
      }
      else {
        QIcon disconnectIcon(":/disconnect.png");
        m_connect->setIcon(disconnectIcon);
        m_connect->setChecked(false);

        spdlog::info(std::string(tr("Session: Successful disconnect from the MQTT remote host").toStdString()));
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
// showRxContextMenu
//

void
CFrmSession::showRxContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Clear selections")),
                  this,
                  SLOT(clrAllRxSelections()));
  menu->addSeparator();
  menu->addAction(QString(tr("Clear receive list")),
                  this,
                  SLOT(menu_clear_rxlist()));

  menu->addSeparator();

  menu->addAction(QString(tr("Copy RX event to clipboard")),
                  this,
                  SLOT(copyRxToClipboard()));

  menu->addAction(QString(tr("Copy RX event to TX")),
                  this,
                  SLOT(copyRxToTx()));

  menu->addSeparator();

  menu->addAction(QString(tr("Write events to file...")),
                  this,
                  SLOT(saveRxToFile()));
  menu->addAction(QString(tr("Write marked events to file...")),
                  this,
                  SLOT(saveMarkRxToFile()));
  menu->addAction(QString(tr("Load events from file...")),
                  this,
                  SLOT(loadRxFromFile()));
  menu->addSeparator();
  menu->addAction(QString(tr("Set/edit GUID (sensor)")),
                  this,
                  SLOT(setGuid()));
  menu->addSeparator();
  menu->addAction(QString(tr("Add/Edit comment...")), this, SLOT(addEventNote()));
  menu->addAction(QString(tr("Remove comment")),
                  this,
                  SLOT(removeEventNote()));
  menu->addSeparator();
  menu->addAction(QString(tr("Toggle row mark")), this, SLOT(setVscpRowMark()));
  menu->addAction(QString(tr("Toggle VSCP class mark")),
                  this,
                  SLOT(setVscpClassMark()));
  menu->addAction(QString(tr("Toggle VSCP type mark")),
                  this,
                  SLOT(setVscpTypeMark()));

  menu->popup(m_rxTable->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllRxSelections
//

void
CFrmSession::clrAllRxSelections(void)
{
  m_rxTable->setCurrentCell(-1, -1);
}

///////////////////////////////////////////////////////////////////////////////
// setGuid
//

void
CFrmSession::setGuid(void)
{
  std::string guid;
  std::string name;
  CDlgKnownGuid* dlg = new CDlgKnownGuid();

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    // m_rxTable->item(it->row(), 0)->setIcon(icon);
    // m_mapEventComment[it->row()] = text;
    pworks->m_mutexGuidMap.lock();
    vscp_writeGuidArrayToString(guid, m_rxEvents[it->row()]->GUID);
    pworks->m_mutexGuidMap.unlock();
    if (!dlg->selectByGuid(guid.c_str())) {
      dlg->setAddGuid(guid.c_str());
      dlg->btnAdd();
    }
    // Show dialog
    dlg->exec();
  }

  // Update row in case info changed
  updateAllRows();
}

///////////////////////////////////////////////////////////////////////////////
// addEventNote
//

void
CFrmSession::addEventNote(void)
{
  bool ok;

  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    QString comment = m_mapRxEventComment[it->row()];
    QString text    = QInputDialog::getText(this,
                                         tr("Add comment"),
                                         tr("Comment:"),
                                         QLineEdit::Normal,
                                         comment,
                                         &ok);
    if (ok && !text.isEmpty()) {
      QIcon icon(":/comment.png");
      m_rxTable->item(it->row(), 0)->setIcon(icon);
      m_mapRxEventComment[it->row()] = text;
    }

    // Cludge to display comment directly
    if (1 == selection.size()) {
      m_rxTable->clearSelection();
      m_rxTable->selectRow(selection.first().row());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// removeEventNote
//

void
CFrmSession::removeEventNote(void)
{
  QIcon icon;
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    m_rxTable->item(it->row(), 0)->setIcon(icon);
    std::map<int, QString>::iterator itmap;
    itmap = m_mapRxEventComment.find(it->row());
    m_mapRxEventComment.erase(itmap);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setVscpRowMark
//

void
CFrmSession::setVscpRowMark(void)
{
  QList<QTableWidgetItem*> sellist = m_rxTable->selectedItems();
  QList<QTableWidgetItem*>::iterator it;
  for (it = sellist.begin(); it != sellist.end(); it++) {
    if (QBrush(Qt::cyan) == (*it)->background()) {
      (*it)->setBackground(Qt::white);
    }
    else {
      (*it)->setBackground(Qt::cyan);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// unsetVscpRowMark
//

void
CFrmSession::unsetVscpRowMark(void)
{
  QList<QTableWidgetItem*> sellist = m_rxTable->selectedItems();
  QList<QTableWidgetItem*>::iterator it;
  for (it = sellist.begin(); it != sellist.end(); it++) {
    (*it)->setBackground(Qt::white);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setVscpClassMark
//

void
CFrmSession::setVscpClassMark(void)
{
  QIcon icon(":/check-mark-red.png");
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    if (!m_rxTable->item(it->row(), 1)->icon().cacheKey()) {
      m_rxTable->item(it->row(), 1)->setIcon(icon);
    }
    else {
      m_rxTable->item(it->row(), 1)->setIcon(QIcon());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// unsetVscpClassMark
//

void
CFrmSession::unsetVscpClassMark(void)
{
  QIcon icon;
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    m_rxTable->item(it->row(), 1)->setIcon(icon);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setVscpTypeMark
//

void
CFrmSession::setVscpTypeMark(void)
{
  QIcon icon(":/check-mark-red.png");
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    if (!m_rxTable->item(it->row(), 2)->icon().cacheKey()) {
      m_rxTable->item(it->row(), 2)->setIcon(icon);
    }
    else {
      m_rxTable->item(it->row(), 2)->setIcon(QIcon());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// unsetVscpTypeMark
//

void
CFrmSession::unsetVscpTypeMark(void)
{
  QIcon icon;
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    m_rxTable->item(it->row(), 2)->setIcon(icon);
  }
}

///////////////////////////////////////////////////////////////////////////////
// copyRxToClipboard
//

void
CFrmSession::copyRxToClipboard(void)
{
  QString str;
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();

  // Must be at least one selected item
  if (!selection.size()) {
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("There is no RX row selected."),
      QMessageBox::Ok);
    return;
  }

  bool ok;
  int format = 0;
  QStringList items;
  items << tr("STRING") << tr("JSON") << tr("XML") << tr("HTML");
  QString item = QInputDialog::getItem(this, tr("Format"), tr("Season:"), items, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    if (item == "STRING") {
      format = 0;
    }
    else if (item == "JSON") {
      format = 1;
    }
    else if (item == "XML") {
      format = 2;
    }
    else if (item == "HTML") {
      format = 3;
    }
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    vscpEvent* pev = m_rxEvents[it->row()];
    if (nullptr != pev) {

      bool rv;
      std::string strEvent;

      switch (format) {

        case 1:
          rv = vscp_convertEventToJSON(strEvent, pev);
          break;

        case 2:
          rv = vscp_convertEventToXML(strEvent, pev);
          break;

        case 3:
          rv = vscp_convertEventToHTML(strEvent, pev);
          break;

        case 0:
        default:
          rv = vscp_convertEventToString(strEvent, pev);
          break;
      }

      if (rv) {
        if (str.length())
          str += "\r\n";
        str += strEvent.c_str();
      }
      else {
        QMessageBox::information(
          this,
          tr(APPNAME),
          tr("Failed to convert event to requested format."),
          QMessageBox::Ok);
      }
    }
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// copyRxToClipboard
//

void
CFrmSession::copyRxToTx(void)
{
  vscpworks* pworks         = (vscpworks*)QCoreApplication::instance();
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();

  // Must be at least one selected item
  if (!selection.size()) {
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("There is no RX row selected."),
      QMessageBox::Ok);
    return;
  }

  QList<QModelIndex>::iterator it;
  for (it = selection.begin(); it != selection.end(); it++) {
    // m_rxTable->item(it->row(), 2)->setIcon(icon);
    vscpEvent* pev = m_rxEvents[it->row()];
    if (nullptr != pev) {
      std::string strevent;
      if (vscp_convertEventToString(strevent, pev)) {
        addTxRow(true, tr("Copy from RX"), 1, 0, strevent.c_str());
      }
      else {
        spdlog::error(std::string(tr("Failed to convert RX event to string").toStdString()));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveMarkRxToFile
//

void
CFrmSession::saveMarkRxToFile(void)
{
  QString fileName;
  // vscpEvent* pev;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QString initialPath = pworks->m_shareFolder + "/rxsets/rxset.xml";
  qDebug() << initialPath;
  fileName = QFileDialog::getSaveFileName(this,
                                          tr("File to save receive events to"),
                                          initialPath,
                                          tr("RX files (*.xml *.*)"));

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

    // We're going to streaming text to the file
    QXmlStreamWriter stream(&file);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("rxrows");

    for (int i = 0; i < m_rxTable->rowCount(); i++) {

      QTableWidgetItem* itemDir = m_rxTable->item(i, rxrow_dir);
      if (QBrush(Qt::cyan) == itemDir->background()) {

        stream.writeStartElement("row");

        stream.writeAttribute("mark-row",
                              (QBrush(Qt::cyan) == itemDir->background()) ? "true" : "false");

        QTableWidgetItem* itemClass = m_rxTable->item(i, rxrow_class);
        stream.writeAttribute("mark-class",
                              (itemClass->icon().cacheKey()) ? "true" : "false");

        QTableWidgetItem* itemType = m_rxTable->item(i, rxrow_type);
        stream.writeAttribute("mark-type",
                              (itemType->icon().cacheKey()) ? "true" : "false");

        QString strVscpComment = m_mapRxEventComment[i];
        stream.writeAttribute("comment", strVscpComment);

        std::string str;
        vscpEvent* pev = m_rxEvents[i];
        if (nullptr != pev) {
          vscp_convertEventToString(str, pev);
          stream.writeAttribute("event", str.c_str());
        }

        stream.writeEndElement(); // row
      }
    }

    stream.writeEndElement(); // txrows
    stream.writeEndDocument();

    file.close();
  }
}

///////////////////////////////////////////////////////////////////////////////
// saveRxToFile
//

void
CFrmSession::saveRxToFile(void)
{
  QString fileName;
  // vscpEvent* pev;

  vscpworks* pworks         = (vscpworks*)QCoreApplication::instance();
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();

  QString initialPath = pworks->m_shareFolder + "/rxsets/rxset.xml";
  qDebug() << initialPath;
  fileName = QFileDialog::getSaveFileName(this,
                                          tr("File to save receive events to"),
                                          initialPath,
                                          tr("RX files (*.xml *.*)"));

  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

    // We're going to streaming text to the file
    QXmlStreamWriter stream(&file);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("rxrows");

    if (selection.size()) {

      // Save selected items
      QList<QModelIndex>::iterator it;
      for (it = selection.begin(); it != selection.end(); it++) {

        stream.writeStartElement("row");

        QTableWidgetItem* itemDir = m_rxTable->item(it->row(), rxrow_dir);

        uint32_t flags = itemDir->data(Qt::UserRole).toULongLong();
        stream.writeAttribute("flags", QString::number(flags));

        // Background color
        int r, g, b, a;
        QColor bc = itemDir->background().color();
        bc.getRgb(&r, &g, &b, &a);
        uint32_t rgba = (((uint32_t)r) << 24) + (((uint32_t)g) << 16) + (((uint32_t)b) << 8) + a;
        stream.writeAttribute("rgba", QString::number(rgba));
        // stream.writeAttribute("r", QString::number(itemDir->background().color().red()));
        // stream.writeAttribute("g", QString::number(itemDir->background().color().green()));
        // stream.writeAttribute("b", QString::number(itemDir->background().color().blue()));
        // stream.writeAttribute("a", QString::number(itemDir->background().color().alpha()));

        stream.writeAttribute("mark-row",
                              (QBrush(Qt::cyan) == itemDir->background()) ? "true" : "false");

        QTableWidgetItem* itemClass = m_rxTable->item(it->row(), rxrow_class);
        stream.writeAttribute("mark-class",
                              (itemClass->icon().cacheKey()) ? "true" : "false");

        qDebug() << itemClass->icon().cacheKey();
        QTableWidgetItem* itemType = m_rxTable->item(it->row(), rxrow_type);
        stream.writeAttribute("mark-type",
                              (itemType->icon().cacheKey()) ? "true" : "false");

        QString strVscpComment = m_mapRxEventComment[it->row()];
        stream.writeAttribute("comment", strVscpComment);

        std::string str;
        vscpEvent* pev = m_rxEvents[it->row()];
        if (nullptr != pev) {
          vscp_convertEventToString(str, pev);
          stream.writeAttribute("event", str.c_str());
        }

        stream.writeEndElement(); // row
      }
    }
    else {

      // save all
      for (int i = 0; i < m_rxTable->rowCount(); i++) {

        stream.writeStartElement("row");

        QTableWidgetItem* itemDir = m_rxTable->item(i, rxrow_dir);

        uint32_t flags = itemDir->data(Qt::UserRole).toULongLong();
        stream.writeAttribute("flags", QString::number(flags));

        // Background color
        int r, g, b, a;
        QColor bc = itemDir->background().color();
        bc.getRgb(&r, &g, &b, &a);
        uint32_t rgba = (((uint32_t)r) << 24) + (((uint32_t)g) << 16) + (((uint32_t)b) << 8) + a;
        stream.writeAttribute("rgba", QString::number(rgba));

        stream.writeAttribute("mark-row",
                              (QBrush(Qt::cyan) == itemDir->background()) ? "true" : "false");

        QTableWidgetItem* itemClass = m_rxTable->item(i, rxrow_class);
        stream.writeAttribute("mark-class",
                              (itemClass->icon().cacheKey()) ? "true" : "false");

        QTableWidgetItem* itemType = m_rxTable->item(i, rxrow_type);
        stream.writeAttribute("mark-type",
                              (itemType->icon().cacheKey()) ? "true" : "false");

        QString strVscpComment = m_mapRxEventComment[i];
        stream.writeAttribute("comment", strVscpComment);

        std::string str;
        vscpEvent* pev = m_rxEvents[i];
        if (nullptr != pev) {
          vscp_convertEventToString(str, pev);
          stream.writeAttribute("event", str.c_str());
        }

        stream.writeEndElement(); // row
      }
    }

    stream.writeEndElement(); // txrows

    stream.writeEndDocument();
    file.close();
  }
  else {
    // Failed to open file
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadRxFromFile
//

void
CFrmSession::loadRxFromFile(void)
{
  QString fileName;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QString initialPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QString initialPath = pworks->m_shareFolder + "/rxsets/rxset.xml";
  fileName            = QFileDialog::getOpenFileName(this,
                                          tr("File to load receive events from"),
                                          initialPath,
                                          tr("RX files (*.xml *.*)"));

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Cannot read file" << file.errorString();
    return;
  }

  QXmlStreamReader reader(&file);

  if (reader.readNextStartElement()) {
    if (reader.name() == "rxrows") {
      qDebug() << reader.name();
      while (reader.readNextStartElement()) {

        uint32_t flags  = 0;
        bool bMarkRow   = false;
        bool bMarkClass = false;
        bool bMarkType  = false;
        uint32_t rgba   = 0;
        QString comment = tr("");
        QString event;

        if (reader.name() == "row") {

          // flags
          if (reader.attributes().hasAttribute("flags")) {
            flags = reader.attributes().value("flags").toULong();
          }

          // rgba
          if (reader.attributes().hasAttribute("rgba")) {
            QString str =
              reader.attributes().value("rgba").toString();
            qDebug() << str;
            rgba = vscp_readStringValue(str.toStdString());
          }

          // mark-row
          if (reader.attributes().hasAttribute("mark-row")) {
            QString enable =
              reader.attributes().value("mark-row").toString();
            if (enable.contains("true", Qt::CaseInsensitive))
              bMarkRow = true;
          }

          // mark-class
          if (reader.attributes().hasAttribute("mark-class")) {
            QString enable =
              reader.attributes().value("mark-class").toString();
            if (enable.contains("true", Qt::CaseInsensitive))
              bMarkClass = true;
          }

          // mark-type
          if (reader.attributes().hasAttribute("mark-type")) {
            QString enable =
              reader.attributes().value("mark-type").toString();
            if (enable.contains("true", Qt::CaseInsensitive))
              bMarkType = true;
          }

          // comment
          if (reader.attributes().hasAttribute("comment")) {
            comment =
              reader.attributes().value("comment").toString();
          }

          // event
          if (reader.attributes().hasAttribute("event")) {
            event =
              reader.attributes().value("event").toString();
          }
        }

        qDebug() << bMarkRow;
        qDebug() << bMarkClass;
        qDebug() << bMarkType;
        qDebug() << comment;
        qDebug() << event;

        vscpEvent* pev;
        vscp_newEvent(&pev);
        vscp_convertStringToEvent(pev, event.toStdString());

        m_mutexRxList.lock();

        int row = m_rxTable->rowCount();
        m_rxTable->insertRow(row);

        // Save event
        m_rxEvents.push_back(pev);

        QColor bc((rgba >> 24) & 0xff,
                  (rgba >> 16) & 0xff,
                  (rgba >> 8) & 0xff,
                  rgba & 0xff);

        // * * * Direction * * *
        QTableWidgetItem* itemDir =
          new QTableWidgetItem((flags & RX_ROW_FLAG_TX) ? "◀" : "ᐅ"); // ➤ ➜ ➡ ➤ ᐅ ᐊ
        itemDir->setTextAlignment(Qt::AlignCenter);
        // itemDir->setBackground(QBrush(bc));

        itemDir->setData(rxrow_role_flags, flags);

        // Not editable
        itemDir->setFlags(itemDir->flags() & ~Qt::ItemIsEditable);

        // Bluish
        itemDir->setForeground(QBrush(QColor(0, 5, 180)));

        // Add
        m_rxTable->setItem(m_rxTable->rowCount() - 1, 0, itemDir);

        // If one or more rows are selected don't autoscroll
        if (!m_rxTable->selectedItems().size()) {
          m_rxTable->scrollToItem(itemDir);
        }

        // * * * Class * * *
        QTableWidgetItem* itemClass = new QTableWidgetItem();
        // itemClass->setBackground(QBrush(bc));
        setClassInfoForRow(itemClass, pev);
        m_rxTable->setItem(m_rxTable->rowCount() - 1, 1, itemClass);

        // * * * Type * * *
        QTableWidgetItem* itemType = new QTableWidgetItem();
        setTypeInfoForRow(itemType, pev);
        m_rxTable->setItem(m_rxTable->rowCount() - 1, 2, itemType);

        // * * * Node id * * *
        QTableWidgetItem* itemNodeId = new QTableWidgetItem();
        setNodeIdInfoForRow(itemNodeId, pev);
        m_rxTable->setItem(m_rxTable->rowCount() - 1, 3, itemNodeId);

        // * * * Guid * * *
        QTableWidgetItem* itemGuid = new QTableWidgetItem();
        setGuidInfoForRow(itemGuid, pev);
        m_rxTable->setItem(m_rxTable->rowCount() - 1, 4, itemGuid);

        m_rxTable->setUpdatesEnabled(false);
        for (int i = 0; i < m_rxTable->rowCount(); i++) {
          m_rxTable->setRowHeight(i, 10);
        }
        m_rxTable->setUpdatesEnabled(true);

        // Display number of items
        m_lcdNumber->display(m_rxTable->rowCount());

        // Count events
        uint32_t cnt =
          m_mapRxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] +
          1;
        m_mapRxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] =
          cnt;

        m_mutexRxList.unlock();

        // Fill unselected info
        fillReceiveEventCount();

        reader.skipCurrentElement();
      }
    }
    else
      reader.raiseError(QObject::tr("Incorrect file"));
  }

  file.close();
}

///////////////////////////////////////////////////////////////////////////////
// rxCellClicked
//

void
CFrmSession::rxCellClicked(int row, int column)
{
  vscpworks* pworks      = (vscpworks*)QCoreApplication::instance();
  QTableWidgetItem* item = m_rxTable->item(row, column);
  if (item->isSelected()) {
    qDebug() << "Selected";
  }
  else {
    qDebug() << "Not Selected";
  }
}

///////////////////////////////////////////////////////////////////////////////
// slotSelectionChange
//

void
CFrmSession::rxSelectionChange(const QItemSelection& selected,
                               const QItemSelection& eselected)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();

  // ------------------------------------------------------------------------

  if (0 == selection.size()) {
    fillReceiveEventCount();
  }
  else if (1 == selection.size()) {
    fillRxStatusInfo(selection.first().row());
  }
  else {
    fillReceiveEventDiff();
  }
}

#define MAX_RENDER_FUNCTIONS 30

struct renderFunc {
  QString name;
  QString func;
  QString value;
};

///////////////////////////////////////////////////////////////////////////////
// fillRxStatusInfo
//

void
CFrmSession::fillRxStatusInfo(int selectedRow)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  vscpEvent* pev = m_rxEvents[selectedRow];
  if (nullptr == pev) {
    return;
  }

  std::string strVscpTemplate =
    "<h3>VSCP Event</h3>"
    "<small><p style=\"color:#993399\">{{{dir}}} event</p></small>"
    "<b><a "
    "href=\"https://grodansparadis.github.io/vscp-doc-spec/#/./"
    "vscphead\">Head</>: </b><span style=\"color:rgb(0, 0, "
    "153);\">{{VscpHead}}</span><br><span "
    "style=\"color:#666699\">{{VscpHeadBits}}</span><br>"
    "<b>OBID: </b><span style=\"color:rgb(0, 0, "
    "153);\">{{VscpObid}}</span><br>"
    "<b>CRC: </b><span style=\"color:rgb(0, 0, "
    "153);\">{{VscpCrc}}</span><br><br>"
    "<b>UTC Time: </b><span style=\"color:rgb(0, 0, "
    "153);\">{{VscpYear}}-{{VscpMonth}}-{{VscpDay}}T{{VscpHour}}:{{"
    "VscpMinute}}:{{VscpSecond}}</span><br>"
    "<b>Timestamp: </b><span style=\" color:rgb(0, 0, "
    "153);\">{{VscpTimestamp}}</span><br>"
    "<br>"
    "<b>Class: </b><a "
    "href=\"{{VscpClassHelpUrl}}\">{{VscpClassToken}}</a><span "
    "style=\"color:rgb(0, 102, 0);\"> {{VscpClass}}</span><br>"
    "<b>Type: </b><a href=\"{{VscpTypeHelpUrl}}\">{{VscpTypeToken}}</a> "
    "<span "
    "style=\"color:rgb(0, 102, 0);\">{{VscpType}}</span><br>"
    "<br>"
    "<b>GUID: </b><br><small><span style=\"color:rgb(0, 102, "
    "0);\">{{VscpGuid}}</span></small><br><span "
    "style=\"color:#666699\"><small>{{VscpGuidSymbolic}}</small></"
    "span><br><br>"
    "<b>Data: </b><br><span style=\"color:rgb(0, 102, 0);\">"
    "{{{VscpData}}}</span><small>{{{VscpMeasurement}}}</small>"
    "{{{VscpComment}}}";

  // Set event render template
  mustache templ{ strVscpTemplate };

  // --------------------------------------------------------------------

  QString renderEventVariables;
  QString renderEventTemplate;
  QStringList lst =
    pworks->getVscpRenderData(pev->vscp_class, pev->vscp_type);
  if (1 == lst.size()) {
    qDebug() << "Templates = " << lst[0];
    renderEventTemplate = lst[0];
  }
  else if (lst.size() >= 2) {

    qDebug() << "Variables = " << lst[0];
    renderEventVariables = lst[0];

    qDebug() << "Templates = " << lst[1];
    renderEventTemplate = lst[1];
  }

  // Variables - If any defined
  std::list<renderFunc*> lstRenderFuncs;
  if (renderEventVariables.length()) {

    // Define the event in the JavaScript domain
    QJSEngine myEngine;

    std::string str;
    std::string strEvaluate;

    strEvaluate = "var e = {};e.vscpData = [";
    vscp_writeDataToString(str, pev);
    strEvaluate += str.c_str();
    strEvaluate += "];e.sizeData=";
    strEvaluate += vscp_str_format("%d", pev->sizeData);
    strEvaluate += ";e.guid = [";
    vscp_writeGuidArrayToString(str, pev->GUID, true);
    strEvaluate += str;
    strEvaluate += "];e.vscpHead=";
    strEvaluate += vscp_str_format("%d", pev->head);
    strEvaluate += ";e.vscpCrc=";
    strEvaluate += vscp_str_format("%d", pev->crc);
    strEvaluate += ";e.vscpObid=";
    strEvaluate += vscp_str_format("%lu", pev->obid);
    strEvaluate += ";e.vscpTimeStamp=";
    strEvaluate += vscp_str_format("%lu", pev->timestamp);
    strEvaluate += ";e.vscpClass=";
    strEvaluate += vscp_str_format("%d", pev->vscp_class);
    strEvaluate += ";e.vscpType=";
    strEvaluate += vscp_str_format("%d", pev->vscp_type);
    strEvaluate += ";e.vscpYear=";
    strEvaluate += vscp_str_format("%d", pev->year);
    strEvaluate += ";e.vscpMonth=";
    strEvaluate += vscp_str_format("%d", pev->month);
    strEvaluate += ";e.vscpDay=";
    strEvaluate += vscp_str_format("%d", pev->day);
    strEvaluate += ";e.vscpHour=";
    strEvaluate += vscp_str_format("%d", pev->hour);
    strEvaluate += ";e.vscpMinute=";
    strEvaluate += vscp_str_format("%d", pev->minute);
    strEvaluate += ";e.vscpSecond=";
    strEvaluate += vscp_str_format("%d", pev->second);
    strEvaluate += ";";

    qDebug() << strEvaluate.c_str();

    // myEngine.evaluate("var e = {};e.data = [11,22,33];");
    QJSValue result = myEngine.evaluate(strEvaluate.c_str());
    qDebug() << result.isError();

    mustache templVar{ renderEventVariables.toStdString() };
    qDebug() << renderEventVariables;
    kainjow::mustache::data _dataVar;
    _dataVar.set("newline", "\r\n");
    _dataVar.set("quote", "\"");
    _dataVar.set("singlequote", "'");
    QString outputVar = templVar.render(_dataVar).c_str();
    qDebug() << outputVar;

    QStringList strlstFunc = outputVar.split("\n");
    qDebug() << strlstFunc.size();

    QString name;
    QString func;
    foreach (QString str, strlstFunc) {
      qDebug() << str.trimmed();
      str = str.trimmed();
      if (!str.length())
        break;
      if (!str.contains("function()"))
        break;
      // We have a function  "id: function() {....}
      // can be one line or multiline
      int posFunc  = str.indexOf("function()");
      int posColon = str.indexOf(":");
      name         = str.left(posColon);
      func         = str.right(str.length() - posFunc);

      // When {} pairs are equal in func we are done
      int cnt = 0;
      foreach (QChar c, func) {
        if ('{' == c) {
          cnt++;
        }
        if ('}' == c) {
          cnt--;
        }
      }

      if (!cnt) {
        struct renderFunc* prf = new struct renderFunc;
        if (nullptr != prf) {
          prf->name = name;
          prf->func = func = "(" + func + ")";
          QJSValue fun     = myEngine.evaluate(func);
          QJSValue result  = fun.call();
          prf->value       = result.toString().trimmed();
          lstRenderFuncs.push_back(prf);
        }
      }
    }
  } // Variables

  // Template - If any defined
  std::string strRenderedData;
  std::string strVscpMeasurement;
  if (renderEventTemplate.length()) {

    // renderEventTemplate = "<small>{{{lbl-start}}}Unit: {{{lbl-end}}} =
    // {{{unitstr}}} [{{{unit}}}] {{{newline}}} {{{lbl-start}}}Sensorindex:
    // {{{lbl-end}}} = {{{sensorindex}}}{{{newline}}} {{{lbl-start}}}Value:
    // {{{lbl-end}}} = {{{val}}}{{{symbol}}} - [{{{datacodingstr}}}]
    // {{{newline}}}</small>";

    mustache templVar{ renderEventTemplate.toStdString() };
    qDebug() << "renderEventTemplate = " << renderEventTemplate;

    kainjow::mustache::data _data;
    _data.set("quote", "&quot;");
    _data.set("singlequote", "'");
    _data.set("ampersand", "&amp;");
    _data.set("lessthan", "&lt;");
    _data.set("greaterthan", "&gt;");
    _data.set("nbrspace", "&nbsp;"); // Non breaking space
    _data.set("newline", "<br>");
    _data.set("lbl-start",
              "<small><b>"); // Label start (for "label: value"  renderings)
    _data.set("lbl-end",
              "</b></small>"); // Label end (for "label: value"  renderings)
    _data.set("val-start",
              "<span style=\"color:#666699\">"); // Value start (for "label:
                                                 // value"  renderings)
    _data.set("val-end",
              "</span>"); // Value end (for "label: value"  renderings)

    // Set data from embedded function calculations std::list<renderFunc *>
    // m_renderFuncs;
    // for (std::list<struct renderFunc *>::iterator it =
    // m_renderFuncs.begin(); it != m_renderFuncs.end(); ++it){
    while (lstRenderFuncs.size()) {
      struct renderFunc* prf = lstRenderFuncs.front();
      // Render
      _data.set(prf->name.toStdString(), prf->value.toStdString());
      qDebug() << prf->name << " - " << prf->value;
      // Remove from list
      lstRenderFuncs.pop_front();
      // Unallocate
      delete prf;
    }

    if (vscp_isMeasurement(pev)) {

      CVscpUnit u = pworks->getUnitInfo(pev->vscp_class,
                                        pev->vscp_type,
                                        vscp_getMeasurementUnit(pev));
      int datacoding;
      _data.set("datacoding",
                vscp_str_format(
                  "0x%02X",
                  (datacoding = vscp_getMeasurementDataCoding(pev))));
      switch ((datacoding >> 5) & 7) {
        case 0:
          _data.set("datacodingstr", "Bits");
          break;
        case 1:
          _data.set("datacodingstr", "Byte");
          break;
        case 2:
          _data.set("datacodingstr", "String");
          break;
        case 3:
          _data.set("datacodingstr", "Integer");
          break;
        case 4:
          _data.set("datacodingstr", "Norm-integer");
          break;
        case 5:
          _data.set("datacodingstr", "Float");
          break;
        case 6:
          _data.set("datacodingstr", "Double");
          break;
        case 7:
          _data.set("datacodingstr", "Reserved");
          break;
      }
      _data.set("unitstr", u.m_name);
      _data.set("unit", vscp_str_format("%d", u.m_unit));
      _data.set(
        "sensorindex",
        vscp_str_format("%d", vscp_getMeasurementSensorIndex(pev)));
      double val;
      vscp_getMeasurementAsDouble(&val, pev);
      std::string strValue = vscp_str_format("%f", val);
      _data.set("val", strValue);
      _data.set("symbol", u.m_symbol_utf8);
    }

    strRenderedData = templVar.render(_data);
    qDebug() << strRenderedData.c_str();
  }

  // --------------------------------------------------------------------

  QTableWidgetItem* itemDir = m_rxTable->item(selectedRow, rxrow_dir);
  uint32_t flags            = itemDir->data(rxrow_role_flags).toUInt();
  std::string strDir        = "Received";
  if (flags & RX_ROW_FLAG_TX)
    strDir = "Transmitted";

  QTableWidgetItem* itemClass = m_rxTable->item(selectedRow, rxrow_class);
  QTableWidgetItem* itemType  = m_rxTable->item(selectedRow, rxrow_type);

  std::string strVscpHead     = vscp_str_format("0x%04X", pev->head);
  std::string strVscpHeadBits = vscp_str_format("\n[ri=%d ", pev->head & 7);
  strVscpHeadBits +=
    vscp_str_format("!c=%s ", (pev->head & 0x0004) ? "true" : "false");
  strVscpHeadBits +=
    vscp_str_format("h=%s ", (pev->head & 0x0010) ? "true" : "false");
  strVscpHeadBits += vscp_str_format("p=%d ", ((pev->head >> 5) & 3));
  strVscpHeadBits += vscp_str_format("g=%d ", ((pev->head >> 8) & 3));
  strVscpHeadBits +=
    vscp_str_format("d=%s ", (pev->head & 0x8000) ? "true" : "false");
  strVscpHeadBits += " ]";
  std::string strVscpObId      = vscp_str_format("0x%08X", pev->obid);
  std::string strVscpCrc       = vscp_str_format("0x%04X", pev->crc);
  std::string strVscpYear      = vscp_str_format("%d", pev->year);
  std::string strVscpMonth     = vscp_str_format("%02d", pev->month);
  std::string strVscpDay       = vscp_str_format("%02d", pev->second);
  std::string strVscpHour      = vscp_str_format("%02d", pev->hour);
  std::string strVscpMinute    = vscp_str_format("%02d", pev->minute);
  std::string strVscpSecond    = vscp_str_format("%02d", pev->second);
  std::string strVscpTimestamp = vscp_str_format("0x%08X", pev->timestamp);
  std::string strVscpClass =
    vscp_str_format("0x%04X, %d", pev->vscp_class, pev->vscp_class);
  std::string strVscpType =
    vscp_str_format("0x%04X, %d", pev->vscp_type, pev->vscp_type);

  std::string strVscpGuid;
  vscp_writeGuidArrayToString(strVscpGuid, pev->GUID);

  pworks->m_mutexGuidMap.lock();
  std::string strVscpGuidSymbolic =
    pworks->m_mapGuidToSymbolicName[strVscpGuid.c_str()].toStdString();
  pworks->m_mutexGuidMap.unlock();

  std::string strVscpData = "<small>";
  for (int i = 0; i < pev->sizeData; i++) {
    strVscpData += vscp_str_format("0x%02X ", pev->pdata[i]);
    if (!((i + 1) % 8))
      strVscpData += "</small><br><small>";
  }
  strVscpData += "</small><br>";

  strVscpMeasurement = strRenderedData;

  // If event is an measurement
  if (vscp_isMeasurement(pev)) {
    strVscpMeasurement = "<b>Measurement:</b><p>";
    QStringList qsl =
      pworks->getVscpRenderData(pev->vscp_class, pev->vscp_type);
    if (qsl.size()) {
      strVscpMeasurement = strRenderedData;
    }
    else {
      strVscpMeasurement += "Error: No definitions found in db";
    }

    strVscpMeasurement += "</p>";
  }

  // Add sensorindex symbolic id (if any)
  pworks->m_mutexSensorIndexMap.lock();
  std::string strSensorIndexSymbolic =
    pworks
      ->m_mapSensorIndexToSymbolicName
        [(pworks->getIdxForGuidRecord(strVscpGuid.c_str()) << 8) +
         vscp_getMeasurementSensorIndex(pev)]
      .toStdString();
  pworks->m_mutexSensorIndexMap.unlock();

  if (strSensorIndexSymbolic.length()) {
    strVscpGuidSymbolic += " - ";
    strVscpGuidSymbolic += strSensorIndexSymbolic;
  }

  // Add comment (if any)
  std::string strVscpComment = m_mapRxEventComment[selectedRow].toStdString();
  if (strVscpComment.length()) {
    strVscpComment =
      "<hr><b>Comment:</b><p style=\"color:rgb(0x80, 0x80, 0x80);\">" +
      strVscpComment;
    strVscpComment += "</p>";
  }

  kainjow::mustache::data _data;
  _data.set("dir", strDir);
  _data.set("VscpHead", strVscpHead);
  _data.set("VscpHeadBits", strVscpHeadBits);
  _data.set("VscpObid", strVscpObId);
  _data.set("VscpCrc", strVscpCrc);
  _data.set("VscpYear", strVscpYear);
  _data.set("VscpMonth", strVscpMonth);
  _data.set("VscpDay", strVscpDay);
  _data.set("VscpHour", strVscpHour);
  _data.set("VscpMinute", strVscpMinute);
  _data.set("VscpSecond", strVscpSecond);
  _data.set("VscpTimestamp", strVscpTimestamp);
  _data.set("VscpClass", strVscpClass);
  _data.set("VscpType", strVscpType);
  _data.set("VscpGuid", strVscpGuid);
  _data.set("VscpGuidSymbolic", strVscpGuidSymbolic);
  _data.set("VscpData", strVscpData);
  //_data.set("vscpRenderData", strRenderedData);
  _data.set("VscpClassToken", itemClass->text().toStdString());
  _data.set("VscpTypeToken", itemType->text().toStdString());
  _data.set("VscpClassHelpUrl",
            pworks->getHelpUrlForClass(pev->vscp_class).toStdString());
  _data.set(
    "VscpTypeHelpUrl",
    pworks->getHelpUrlForType(pev->vscp_class, pev->vscp_type).toStdString());
  _data.set("renderData", strRenderedData);
  _data.set("VscpMeasurement", strVscpMeasurement);
  _data.set("VscpComment", strVscpComment);

  std::string output = templ.render(_data);
  qDebug() << output.c_str();
  m_infoArea->setHtml(output.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// updateCurrentRow
//

void
CFrmSession::updateCurrentRow(void)
{
  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  // Must be a slected row to update
  if (!selection.size())
    return;

  // Update GUID info on row
  QTableWidgetItem* itemGuid =
    m_rxTable->item(selection.first().row(), rxrow_guid);
  setGuidInfoForRow(itemGuid, m_rxEvents[selection.first().row()]);

  // Update RX status
  fillRxStatusInfo(selection.first().row());
}

///////////////////////////////////////////////////////////////////////////////
// updateAllRows
//

void
CFrmSession::updateAllRows(void)
{
  m_mutexRxList.lock();
  // for (int i = 0; i < m_rxEvents.size(); i++) {
  //     std::cout << m_rxEvents[i] <<  " ";
  // }
  m_rxTable->setUpdatesEnabled(false);
  for (int i = 0; i < m_rxTable->rowCount(); i++) {
    // Update Class info on row
    QTableWidgetItem* itemVscpClass = m_rxTable->item(i, rxrow_class);
    setClassInfoForRow(itemVscpClass, m_rxEvents[i]);

    // Update Type info on row
    QTableWidgetItem* itemVscpType = m_rxTable->item(i, rxrow_type);
    setTypeInfoForRow(itemVscpType, m_rxEvents[i]);

    // Update node id info on row
    QTableWidgetItem* itemNodeId = m_rxTable->item(i, rxrow_nodeid);
    setNodeIdInfoForRow(itemNodeId, m_rxEvents[i]);

    // Update GUID info on row
    QTableWidgetItem* itemGuid = m_rxTable->item(i, rxrow_guid);
    setGuidInfoForRow(itemGuid, m_rxEvents[i]);
  }
  m_rxTable->setUpdatesEnabled(true);
  m_mutexRxList.unlock();

  updateCurrentRow();
}

///////////////////////////////////////////////////////////////////////////////
// openConnectionSettings
//

void
CFrmSession::openConnectionSettings(void)
{
  // printf("openConnectionSettings\n");
  //  parent()->editConnectionItem();
}

///////////////////////////////////////////////////////////////////////////////
// openMqttSubscribeTopics
// CVscpClient* m_vscpClient

void
CFrmSession::openMqttSubscribeTopics(void)
{
  CDlgSelectMqttTopics dlg;
  dlg.init(CDlgSelectMqttTopics::SUBSCRIBE, m_vscpClient);
  if (QDialog::Accepted == dlg.exec()) {
    // Disconnect
    connectToRemoteHost(false);
    // Reconnect
    connectToRemoteHost(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// openMqttPublishTopics
//

void
CFrmSession::openMqttPublishTopics(void)
{
  CDlgSelectMqttTopics dlg;
  dlg.init(CDlgSelectMqttTopics::PUBLISH, m_vscpClient);
  if (QDialog::Accepted == dlg.exec()) {
    // Disconnect
    connectToRemoteHost(false);
    // Reconnect
    connectToRemoteHost(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// openClearMqttRetainPublishTopics
//

void
CFrmSession::openClearMqttRetainPublishTopics(void)
{
  CDlgSelectMqttTopics dlg;
  dlg.init(CDlgSelectMqttTopics::CLRRETAIN, m_vscpClient);
  if (QDialog::Accepted == dlg.exec()) {
    // Noting to do
  }
}

///////////////////////////////////////////////////////////////////////////////
// getClassInfo
//

QString
CFrmSession::getClassInfo(const vscpEvent* pev)
{
  QString strClass;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (pworks->m_session_ClassDisplayFormat) {

    case classDisplayFormat::numerical_in_base:
      strClass =
        pworks->decimalToStringInBase(pev->vscp_class,
                                      m_baseComboBox->currentIndex());
      break;

    case classDisplayFormat::numerical_hex_dec:
      strClass = pworks->decimalToStringInBase(pev->vscp_class, 0);
      strClass += "/";
      strClass += pworks->decimalToStringInBase(pev->vscp_class, 1);
      break;

    case classDisplayFormat::symbolic_hex_dec:
      strClass = pworks->m_mapVscpClassToToken[pev->vscp_class];
      strClass += " - ";
      strClass += pworks->decimalToStringInBase(pev->vscp_class, 0);
      strClass += "/";
      strClass += pworks->decimalToStringInBase(pev->vscp_class, 1);
      break;

    case classDisplayFormat::symbolic:
    default:
      strClass = pworks->m_mapVscpClassToToken[pev->vscp_class];
      break;
  }

  return strClass;
}

///////////////////////////////////////////////////////////////////////////////
// setClassInfoForRow
//

void
CFrmSession::setClassInfoForRow(QTableWidgetItem* item, const vscpEvent* pev)
{
  QString strClass  = getClassInfo(pev);
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  item->setText(strClass);

  // Tooltip
  item->setToolTip(
    vscp_str_format(
      "%s\n0x%04X %d",
      pworks->m_mapVscpClassToToken[pev->vscp_class].toStdString().c_str(),
      pev->vscp_class,
      pev->vscp_class)
      .c_str());
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setForeground(QBrush(QColor(0, 99, 0)));
}

///////////////////////////////////////////////////////////////////////////////
// getTypeInfo
//

QString
CFrmSession::getTypeInfo(const vscpEvent* pev)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString strType;

  QString strShortTypeToken =
    pworks->getShortTypeToken(pev->vscp_class, pev->vscp_type);

  switch (pworks->m_session_TypeDisplayFormat) {

    case typeDisplayFormat::numerical_in_base:
      strType =
        pworks->decimalToStringInBase(pev->vscp_type,
                                      m_baseComboBox->currentIndex());
      break;

    case typeDisplayFormat::numerical_hex_dec:
      strType = pworks->decimalToStringInBase(pev->vscp_type, 0);
      strType += "/";
      strType += pworks->decimalToStringInBase(pev->vscp_type, 1);
      break;

    case typeDisplayFormat::symbolic_hex_dec:
      if (pworks->m_session_bShowFullTypeToken) {
        strType =
          pworks
            ->m_mapVscpTypeToToken[((uint32_t)pev->vscp_class << 16) +
                                   pev->vscp_type];
      }
      else {
        strType = strShortTypeToken;
      }
      strType += " - ";
      strType += pworks->decimalToStringInBase(pev->vscp_type, 0);
      strType += "/";
      strType += pworks->decimalToStringInBase(pev->vscp_type, 1);
      break;

    case typeDisplayFormat::symbolic:
    default:
      if (pworks->m_session_bShowFullTypeToken) {
        strType =
          pworks
            ->m_mapVscpTypeToToken[((uint32_t)pev->vscp_class << 16) +
                                   pev->vscp_type];
      }
      else {
        strType = strShortTypeToken;
      }
      break;
  }

  return strType;
}

///////////////////////////////////////////////////////////////////////////////
// setTypeInfoForRow
//

void
CFrmSession::setTypeInfoForRow(QTableWidgetItem* item, const vscpEvent* pev)
{
  QString strType   = getTypeInfo(pev);
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  item->setText(strType);

  // Tooltip
  item->setToolTip(
    vscp_str_format(
      "%s\n0x%04X %d",
      pworks
        ->m_mapVscpTypeToToken[((uint32_t)pev->vscp_class << 16) +
                               pev->vscp_type]
        .toStdString()
        .c_str(),
      pev->vscp_type,
      pev->vscp_type)
      .c_str());
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setForeground(QBrush(QColor(0, 5, 180)));
}

///////////////////////////////////////////////////////////////////////////////
// setNodeIdInfoForRow
//

void
CFrmSession::setNodeIdInfoForRow(QTableWidgetItem* item, const vscpEvent* pev)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QString strNodeId = pworks->decimalToStringInBase(
    ((uint16_t)pev->GUID[14] << 8) + pev->GUID[15],
    m_baseComboBox->currentIndex());

  item->setText(strNodeId);

  // Tooltip
  item->setToolTip(vscp_str_format("GUID[14]=0x%02X GUID[15]=0x%02X",
                                   pev->GUID[14],
                                   pev->GUID[15])
                     .c_str());
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  item->setTextAlignment(Qt::AlignCenter);
}

///////////////////////////////////////////////////////////////////////////////
// setGuidInfoForRow
//

void
CFrmSession::setGuidInfoForRow(QTableWidgetItem* item, const vscpEvent* pev)
{
  std::string strGuid;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  vscp_writeGuidArrayToString(strGuid, pev->GUID);

  pworks->m_mutexGuidMap.lock();
  QString guidSymbolicName = pworks->m_mapGuidToSymbolicName[strGuid.c_str()];
  pworks->m_mutexGuidMap.unlock();

  pworks->m_mutexSensorIndexMap.lock();
  QString strSensorIndexSymbolic =
    pworks->m_mapSensorIndexToSymbolicName
      [(pworks->getIdxForGuidRecord(strGuid.c_str()) << 8) +
       vscp_getMeasurementSensorIndex(pev)];
  pworks->m_mutexSensorIndexMap.unlock();

  QString strGuidDisplay;
  switch (pworks->m_session_GuidDisplayFormat) {

    case guidDisplayFormat::symbolic:
      if (guidSymbolicName.length()) {
        strGuidDisplay = guidSymbolicName;
      }
      else {
        strGuidDisplay = strGuid.c_str();
      }
      if (strSensorIndexSymbolic.length()) {
        if (strSensorIndexSymbolic.length())
          guidSymbolicName += " - ";
        guidSymbolicName += strSensorIndexSymbolic;
      }
      break;

    case guidDisplayFormat::symbolic_guid:
      if (strSensorIndexSymbolic.length()) {
        guidSymbolicName += " - ";
        guidSymbolicName += strSensorIndexSymbolic;
      }
      strGuidDisplay = guidSymbolicName;
      if (strGuidDisplay.length())
        strGuidDisplay += " - ";
      strGuidDisplay += strGuid.c_str();
      break;

    case guidDisplayFormat::guid_symbolic:
      if (strSensorIndexSymbolic.length()) {
        guidSymbolicName += " - ";
        guidSymbolicName += strSensorIndexSymbolic;
      }
      strGuidDisplay = strGuid.c_str();
      if (strGuidDisplay.length())
        strGuidDisplay += " - ";
      strGuidDisplay += guidSymbolicName;
      break;

    case guidDisplayFormat::guid:
    default:
      strGuidDisplay = strGuid.c_str();
      break;
  }

  item->setText(strGuidDisplay);

  // Tooltip
  if (strSensorIndexSymbolic.length()) {
    item->setToolTip(
      vscp_str_format("%s - %s\n%s",
                      guidSymbolicName.toStdString().c_str(),
                      strSensorIndexSymbolic.toStdString().c_str(),
                      strGuid.c_str())
        .c_str());
  }
  else {
    if (guidSymbolicName.length()) {
      item->setToolTip(
        vscp_str_format("%s\n%s",
                        guidSymbolicName.toStdString().c_str(),
                        strGuid.c_str())
          .c_str());
    }
    else {
      item->setToolTip(vscp_str_format("%s", strGuid.c_str()).c_str());
    }
  }

  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}

///////////////////////////////////////////////////////////////////////////////
// receiveRxRow
//

void
CFrmSession::receiveRxRow(vscpEvent* pev)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_mutexRxList.lock();

  int row = m_rxTable->rowCount();
  m_rxTable->insertRow(row);

  // Save event
  m_rxEvents.push_back(pev);

  // * * * Direction * * *
  QTableWidgetItem* itemDir = new QTableWidgetItem("ᐅ"); // ➤ ➜ ➡ ➤ ᐊ
  itemDir->setTextAlignment(Qt::AlignCenter);
  itemDir->setData(rxrow_role_flags, RX_ROW_FLAG_RX);

  // Not editable
  itemDir->setFlags(itemDir->flags() & ~Qt::ItemIsEditable);

  // Reset TX flag if set
  uint32_t flags = itemDir->data(rxrow_role_flags).toULongLong();
  itemDir->setData(rxrow_role_flags, flags & ~RX_ROW_FLAG_TX);

  // Bluish
  itemDir->setForeground(QBrush(QColor(0, 5, 180)));

  // Add
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 0, itemDir);

  // If one or more rows are selected don't autoscroll
  if (!m_rxTable->selectedItems().size()) {
    m_rxTable->scrollToItem(itemDir);
  }

  // * * * Class * * *
  QTableWidgetItem* itemClass = new QTableWidgetItem();
  setClassInfoForRow(itemClass, pev);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 1, itemClass);

  // * * * Type * * *
  QTableWidgetItem* itemType = new QTableWidgetItem();
  setTypeInfoForRow(itemType, pev);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 2, itemType);

  // * * * Node id * * *
  QTableWidgetItem* itemNodeId = new QTableWidgetItem();
  setNodeIdInfoForRow(itemNodeId, pev);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 3, itemNodeId);

  // * * * Guid * * *
  QTableWidgetItem* itemGuid = new QTableWidgetItem();
  setGuidInfoForRow(itemGuid, pev);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 4, itemGuid);

  m_rxTable->setUpdatesEnabled(false);
  for (int i = 0; i < m_rxTable->rowCount(); i++) {
    m_rxTable->setRowHeight(i, 10);
  }
  m_rxTable->setUpdatesEnabled(true);

  // Display number of items
  m_lcdNumber->display(m_rxTable->rowCount());

  // Count events
  uint32_t cnt =
    m_mapRxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] +
    1;
  m_mapRxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] =
    cnt;

  m_mutexRxList.unlock();

  // Fill unselected info
  fillReceiveEventCount();
}

///////////////////////////////////////////////////////////////////////////////
// receiveTxRow
//

void
CFrmSession::receiveTxRow(vscpEvent* pev)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_mutexRxList.lock();

  int row = m_rxTable->rowCount();
  m_rxTable->insertRow(row);

  // Make copy of event
  vscpEvent* pevnew = new vscpEvent;
  if (pevnew == nullptr) {
    // TODO log error
    return;
  }
  pevnew->sizeData = 0;
  pevnew->pdata    = nullptr;
  vscp_copyEvent(pevnew, pev);

  // Save event copy in rx list
  m_rxEvents.push_back(pevnew);

  // * * * Direction * * *
  QTableWidgetItem* itemDir = new QTableWidgetItem("◀"); // ➤ ➜ ➡ ➤ ᐊ
  itemDir->setTextAlignment(Qt::AlignCenter);
  itemDir->setData(rxrow_role_flags, RX_ROW_FLAG_TX);

  // Not editable
  itemDir->setFlags(itemDir->flags() & ~Qt::ItemIsEditable);

  // Bluish
  itemDir->setForeground(QBrush(QColor(0, 5, 180)));

  // Add
  m_rxTable->setItem(m_rxTable->rowCount() - 1, rxrow_dir, itemDir);

  // If one or more rows are selected don't autoscroll
  if (!m_rxTable->selectedItems().size()) {
    m_rxTable->scrollToItem(itemDir);
  }

  // * * * Class * * *
  QTableWidgetItem* itemClass = new QTableWidgetItem();
  setClassInfoForRow(itemClass, pevnew);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 1, itemClass);

  // * * * Type * * *
  QTableWidgetItem* itemType = new QTableWidgetItem();
  setTypeInfoForRow(itemType, pevnew);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 2, itemType);

  // * * * Node id * * *
  QTableWidgetItem* itemNodeId = new QTableWidgetItem();
  setNodeIdInfoForRow(itemNodeId, pevnew);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 3, itemNodeId);

  // * * * Guid * * *
  QTableWidgetItem* itemGuid = new QTableWidgetItem();
  setGuidInfoForRow(itemGuid, pevnew);
  m_rxTable->setItem(m_rxTable->rowCount() - 1, 4, itemGuid);

  m_rxTable->setUpdatesEnabled(false);
  for (int i = 0; i < m_rxTable->rowCount(); i++) {
    m_rxTable->setRowHeight(i, 10);
  }
  m_rxTable->setUpdatesEnabled(true);

  // Display number of items
  m_lcdNumber->display(m_rxTable->rowCount());

  // Count events
  uint32_t cnt =
    m_mapTxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] +
    1;
  m_mapTxEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] =
    cnt;

  m_mutexRxList.unlock();

  // Fill unselected info
  fillReceiveEventCount();
}

///////////////////////////////////////////////////////////////////////////////
// fillReceiveEventCount
//

void
CFrmSession::fillReceiveEventCount()
{
  // * * * Info area event count compilation * * *

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  // QList<QListWidgetItem *> sellist = m_rxTable->selectedItems();

  if (0 == selection.size()) {

    m_mutexRxList.lock();

    std::map<uint32_t, uint32_t>::iterator it;
    QString strOut = tr("<h3>VSCP Event count</h3>");

    strOut += "<br><b>RX</b><br>";
    for (it = m_mapRxEventToCount.begin(); it != m_mapRxEventToCount.end();
         it++) {
      strOut += "<small><span style=\"color:rgb(0, 0, 153);\">";
      strOut +=
        pworks->m_mapVscpClassToToken[it->first >> 16]; // class token
      strOut += "/";
      strOut +=
        pworks->getShortTypeToken(it->first >> 16, it->first & 0xffff);
      strOut += "</span></small> = ";
      strOut += QString::number(it->second); // count
      strOut += "<br>";
    }

    strOut += "<br><b>TX</b><br>";

    for (it = m_mapTxEventToCount.begin(); it != m_mapTxEventToCount.end();
         it++) {
      strOut += "<small><span style=\"color:rgb(0, 0, 153);\">";
      strOut +=
        pworks->m_mapVscpClassToToken[it->first >> 16]; // class token
      strOut += "/";
      strOut +=
        pworks->getShortTypeToken(it->first >> 16, it->first & 0xffff);
      strOut += "</span></small> = ";
      strOut += QString::number(it->second); // count
      strOut += "<br>";
    }
    strOut += "";

    m_infoArea->setHtml(strOut);

    m_mutexRxList.unlock();
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillReceiveEventDiff
//

void
CFrmSession::fillReceiveEventDiff()
{
  QString strOut;
  uint32_t lastTimestamp = 0;
  uint32_t diff          = 0;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
  // QList<QListWidgetItem *> sellist = m_rxTable->selectedItems();

  if (selection.size() > 1) {

    m_mutexRxList.lock();

    mustache templ{
      "<b>UTC Time: </b><span style=\"color:rgb(0xc0, 0xc0, "
      "0xc0);\">{{VscpYear}}-{{VscpMonth}}-{{VscpDay}}T{{VscpHour}}:{{"
      "VscpMinute}}:{{VscpSecond}}</span><br>"
    };

    std::map<int, vscpEvent*> mapRowEvent;

    QList<QModelIndex>::iterator it;
    for (it = selection.begin(); it != selection.end(); it++) {
      // QTableWidgetItem* itemClass = m_rxTable->item(it->row(), 1);
      mapRowEvent[it->row()] = m_rxEvents[it->row()];
    }

    std::map<int, vscpEvent*>::iterator itmap;
    for (itmap = mapRowEvent.begin(); itmap != mapRowEvent.end(); itmap++) {

      vscpEvent* pev = itmap->second;

      kainjow::mustache::data _data;
      _data.set("VscpYear", vscp_str_format("%d", pev->year));
      _data.set("VscpMonth", vscp_str_format("%02d", pev->month));
      _data.set("VscpDay", vscp_str_format("%02d", pev->second));
      _data.set("VscpHour", vscp_str_format("%02d", pev->hour));
      _data.set("VscpMinute", vscp_str_format("%02d", pev->minute));
      _data.set("VscpSecond", vscp_str_format("%02d", pev->second));
      std::string strDate = templ.render(_data);

      strOut += "<span style=\"color:rgb(0, 0, 153);\">";
      strOut +=
        pworks->m_mapVscpClassToToken[pev->vscp_class]; // class token
      strOut += "/";
      strOut +=
        pworks->getShortTypeToken(pev->vscp_class, pev->vscp_type);
      strOut += "</span><br>";
      strOut += strDate.c_str();
      strOut += "<b>Timestamp</b> = ";
      strOut += QString::number(pev->timestamp);
      strOut += "  µS<br><b>Diff</b> = ";
      diff = pev->timestamp - lastTimestamp;
      strOut += QString::number(diff);
      strOut += " µS ";
      strOut +=
        vscp_str_format("(%.3f seconds)", (double)diff / 1000000).c_str();
      strOut += "<br><br>";
      lastTimestamp = pev->timestamp;
    }
    strOut += "";
    m_infoArea->setHtml(strOut);

    m_mutexRxList.unlock();
  }
}

///////////////////////////////////////////////////////////////////////////////
// threadReceive
//

// void
// CFrmSession::threadReceive(vscpEvent* pev)
// {
//   emit dataReceived(pev);
// }

///////////////////////////////////////////////////////////////////////////////
// receiveCallback
//

void
CFrmSession::receiveCallback(vscpEvent& ev, void *pobj) 
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