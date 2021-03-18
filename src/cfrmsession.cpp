// cfrmsession.cpp
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
// tableWidget->resizeRowsToContents();

#include <stdlib.h>

#include <vscp.h>
#include <vscpworks.h>

#include <mustache.hpp>

#include <vscp_client_tcp.h>

#include "cdlgknownguid.h"
#include "cfrmsession.h"

#include "cdlgmainsettings.h"
#include "cdlgtxedit.h"

#include <QJSEngine>
#include <QSqlTableModel>
#include <QTableView>
#include <QTableWidgetItem>
#include <QtSql>
#include <QtWidgets>
#include <QXmlStreamReader>
#include <QFile>
#include <QStandardPaths>

using namespace kainjow::mustache;

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
// vscp_client_callback
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
// CFrmSession
//

CFrmSession::CFrmSession(QWidget* parent, QJsonObject* pconn)
  : QDialog(parent)
{
    // No connection set yet
    m_vscpConnType = CVscpClient::connType::NONE;
    m_vscpClient   = NULL;

    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    pworks->log(pworks->LOG_LEVEL_DEBUG, "Session: Session module opended");

    if (nullptr == pconn) {
        pworks->log(pworks->LOG_LEVEL_ERROR, "Session: pconn is null");
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Can't open session window - configuration data is missing"),
          QMessageBox::Ok);
        return;
    }

    // Save session configuration
    m_connObject = *pconn;

    // Must have a type
    if (m_connObject["type"].isNull()) {
        pworks->log(pworks->LOG_LEVEL_ERROR,
                    "Session: Type is not define in JSON data");
        QMessageBox::information(
          this,
          tr("vscpworks+"),
          tr("Can't open session window - The connection type is unknown"),
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

    // We need a toolbar
    m_toolBar = new QToolBar();

    createMenu();
    createHorizontalGroupBox();
    createRxGroupBox();
    // createFormGroupBox();
    createTxGridGroup();

    m_buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

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
    connect(
      m_rxTable->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      SLOT(rxSelectionChange(const QItemSelection&, const QItemSelection&)));

    // Lay out things
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(m_menuBar);
    mainLayout->addWidget(m_toolBar);
    mainLayout->addWidget(m_gridGroupBox, 6);
    mainLayout->addWidget(m_txGroupBox, 3);

    setLayout(mainLayout);

    qDebug() << connect(this,
                        &CFrmSession::dataReceived,
                        this,
                        &CFrmSession::receiveRow,
                        Qt::ConnectionType::QueuedConnection);

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
            m_connectActToolBar->setChecked(true);
            connectToRemoteHost(true);
            break;

        case CVscpClient::connType::CANAL:
            break;

        case CVscpClient::connType::SOCKETCAN:
            break;

        case CVscpClient::connType::WS1:
            break;

        case CVscpClient::connType::WS2:
            break;

        case CVscpClient::connType::MQTT:
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

    // TX Table signales

    connect(
      m_txTable->selectionModel(),
      SIGNAL(QTableWidget::itemChanged(QTableWidgetItem *item)),
      SLOT(txItemChanged(QTableWidgetItem *item)));

    connect(
      m_txTable,
      &QTableWidget::cellDoubleClicked,
      this,
      &CFrmSession::txRowDoubleClicked ); 

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

    // File menu
    m_fileMenu = new QMenu(tr("&File"), this);

    // Icons
    const QIcon windowCloseIcon = QIcon::fromTheme("window-close");

    const QIcon loadIcon = QIcon::fromTheme("window-close");
    const QIcon saveIcon = QIcon::fromTheme("window-close");

    // const QIcon connectIcon = QIcon::fromTheme("call-start");
    const QIcon disconnectIcon = QIcon::fromTheme("call-stop");
    // const QIcon filterIcon = QIcon::fromTheme("edit-find");
    // const QIcon filterIcon = QIcon::fromTheme("edit-find");

    m_loadEventsAct =
      m_fileMenu->addAction(tr("Load VSCP events from file..."));
    m_saveEventsAct = m_fileMenu->addAction(tr("Save VSCP events to file..."));
    m_loadTxAct =
      m_fileMenu->addAction(tr("Load transmission set from file..."));
    m_saveTxAct = m_fileMenu->addAction(tr("Save transmission set to file"));

    m_exitAct = m_fileMenu->addAction(windowCloseIcon,
                                      tr("Close session window"),
                                      this,
                                      &CFrmSession::close);
    m_exitAct->setStatusTip(tr("Close session window"));

    m_menuBar->addMenu(m_fileMenu);

    m_toolBar->addSeparator();

    // Numerical base
    m_baseComboBox = new QComboBox;
    m_baseComboBox->addItem("Hex");
    m_baseComboBox->addItem("Decimal");
    m_baseComboBox->addItem("Octal");
    m_baseComboBox->addItem("Binary");
    m_toolBar->addWidget(m_baseComboBox);

    m_toolBar->addSeparator();

    // Connect
    QIcon connectIcon(":/connect.png");
    m_connectActToolBar = m_toolBar->addAction(connectIcon,
                                               tr("Connect"),
                                               this,
                                               &CFrmSession::menu_connect);
    m_connectActToolBar->setStatusTip(
      tr("Connect/disconnect from remote host"));
    m_connectActToolBar->setCheckable(true);

    m_toolBar->addSeparator();

    // Filter
    QIcon filterIcon(":/filter.png");
    m_setFilterActToolBar = m_toolBar->addAction(filterIcon,
                                                 tr("Enable filter"),
                                                 this,
                                                 &CFrmSession::menu_filter);
    m_setFilterActToolBar->setStatusTip(tr("Enable/disable filter"));
    m_setFilterActToolBar->setCheckable(true);

    m_filterComboBox = new QComboBox;
    m_filterComboBox->addItem("This is the magnifićant Filter 1");
    m_filterComboBox->addItem("Filter 2");
    m_filterComboBox->addItem("Filter 3");
    m_filterComboBox->addItem("Filter 4");
    m_toolBar->addWidget(m_filterComboBox);

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

    // ------------------------------------------------------------------------

    // Edit menu
    m_editMenu       = new QMenu(tr("&Edit"), this);
    m_cutAct         = m_editMenu->addAction(tr("Cut"));
    m_copyAct        = m_editMenu->addAction(tr("Copy"));
    m_pasteBeforeAct = m_editMenu->addAction(tr("Paste before"));
    m_pasteAfterAct  = m_editMenu->addAction(tr("Paste after"));
    m_menuBar->addMenu(m_editMenu);

    // Host menu
    m_hostMenu      = new QMenu(tr("&Host"), this);
    m_connectAct    = m_hostMenu->addAction(tr("Connect to host..."));
    m_disconnectAct = m_hostMenu->addAction(tr("Disconnect from host..."));
    // m_pauseAct = m_hostMenu->addAction(tr("Pause host"));
    // m_addHostAct = m_hostMenu->addAction(tr("Add host..."));
    m_editHostAct = m_hostMenu->addAction(tr("Edit host settings..."));
    m_menuBar->addMenu(m_hostMenu);

    // View menu
    m_viewMenu       = new QMenu(tr("&View"), this);
    m_viewMessageAct = m_viewMenu->addAction(tr("Message flow"));
    m_viewCountAct   = m_viewMenu->addAction(tr("Message count"));
    m_viewMenu->addSeparator();
    m_viewClrRxListAct = m_viewMenu->addAction(tr("Clear receive list..."));
    m_viewClrTxListAct =
      m_viewMenu->addAction(tr("Clear transmission list..."));
    m_menuBar->addMenu(m_viewMenu);

    // VSCP menu
    m_vscpMenu      = new QMenu(tr("&vscp"), this);
    m_readRegAct    = m_vscpMenu->addAction(tr("Read register..."));
    m_writeRegAct   = m_vscpMenu->addAction(tr("Write register..."));
    m_readAllRegAct = m_vscpMenu->addAction(tr("Read (all) registers..."));
    m_readGuidAct   = m_vscpMenu->addAction(tr("Read GUID"));
    m_readMdfAct    = m_vscpMenu->addAction(tr("Read MDF..."));
    m_loadMdfAct    = m_vscpMenu->addAction(tr("Download MDF..."));
    m_menuBar->addMenu(m_vscpMenu);

    // Settings menu
    m_settingsMenu = new QMenu(tr("&Setting"), this);
    m_setFilterAct = m_settingsMenu->addAction(tr("Set/define filter..."));
    m_settingsAct  = m_settingsMenu->addAction(tr("Settings..."));
    m_menuBar->addMenu(m_settingsMenu);
    connect(m_settingsAct,
            &QAction::triggered,
            this,
            &CFrmSession::menu_open_main_settings);

    // Tools menu
    m_toolsMenu = new QMenu(tr("&Tools"), this);
    m_menuBar->addMenu(m_toolsMenu);

    connect(m_exitAct, &QAction::triggered, this, &QDialog::accept);

    // connect(m_connectAct, SIGNAL(triggered(bool)), this,
    // SLOT(connectToRemoteHost(bool))); // &QAction::
    connect(m_connectActToolBar,
            SIGNAL(triggered(bool)),
            this,
            SLOT(connectToRemoteHost(bool)));
    connect(m_disconnectAct,
            SIGNAL(toggled(bool)),
            this,
            SLOT(connectToRemoteHost(bool)));

    // #ifndef QT_NO_CLIPBOARD
    //     cutAct->setEnabled(false);

    //     copyAct->setEnabled(false);
    //     connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct,
    //     &QAction::setEnabled); connect(textEdit,
    //     &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
    // #endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// close
//

// void CFrmSession::close()
// {
//     //accept();
//     close();
// }

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

    // for (int i = 0; i < NumGridRows; ++i) {
    //     labels[i] = new QLabel(tr("Line %1:").arg(i + 1));
    //     lineEdits[i] = new QLineEdit;
    //     layout->addWidget(labels[i], i + 1, 0);
    //     layout->addWidget(lineEdits[i], i + 1, 1);
    // }
    QStringList headers(
      QString(tr("Dir, VSCP Class, VSCP Type, id, GUID")).split(','));
    m_rxTable = new QTableWidget;
    m_rxTable->setContextMenuPolicy(
      Qt::CustomContextMenu); // Enable context menu
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

    // QTableWidgetItem* item = new QTableWidgetItem("ᐊ ᐅ"); // ➤ ➜ ➡ ➤
    // item->setTextAlignment(Qt::AlignHCenter);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    // int row = m_rxTable->rowCount();
    // m_rxTable->insertRow(row);

    // QTableWidgetItem *item = m_rxTable->item(1, 1);
    // item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    // item->setForeground(QBrush(QColor(0, 5, 180)));

    // m_rxTable->setItem(m_rxTable->rowCount() - 1, 0, item);
    // QTableWidgetItem *item;

    // QTableWidgetItem* icon_item = new QTableWidgetItem;
    // const QIcon loadIcon        = QIcon::fromTheme("window-close");
    // icon_item->setIcon(loadIcon);
    // m_rxTable->insertRow(row);
    // m_rxTable->setItem(m_rxTable->rowCount() - 1, 1, icon_item);

    // item = m_rxTable->item(row, 1);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    // item = m_rxTable->item(row, 2);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    // item = m_rxTable->item(row, 3);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);

    // m_rxTable->setUpdatesEnabled(false);
    // for (int i = 0; i < m_rxTable->rowCount(); i++) {
    //     m_rxTable->setRowHeight(i, 10);
    // }
    // m_rxTable->setUpdatesEnabled(true);

    // <---
    // QSqlTableModel *
    // m_rxmodel = new QSqlTableModel;
    // m_rxmodel->setTable("events");
    // m_rxmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // m_rxmodel->select();
    // m_rxmodel->setHeaderData(0, Qt::Horizontal, tr("class"));
    // m_rxmodel->setHeaderData(1, Qt::Horizontal, tr("type"));

    // m_rxTable->setModel(m_rxmodel);
    // m_rxTable->hideColumn(0); // don't show the ID

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
    // m_infoArea->insertHtml(tr(
    //   "<h3>VSCP Event</h3>"
    //   "<small><p style=\"color:#993399\">Received event</p></small>"
    //   "<b>Head: </b><span style=\"color:rgb(0, 0, 153);\">0x0100</span><br>"
    //   "<b>Time: </b><span style=\"color:rgb(0, 0, "
    //   "153);\">2021-09-12T12:10:29</span><br>"
    //   "<b>Timestamp: </b><span style=\" color:rgb(0, 0, "
    //   "153);\">0x1213140f</span><br>"
    //   "<br>"
    //   "<b>Class: </b><a href=\"https://www.vscp.org\">CLASS1_DATA</a><span "
    //   "style=\"color:rgb(0, 102, 0);\"> 0x000F, 15</span><br>"
    //   "<b>Type: </b><a href=\"https://www.vscp.org\">IO-VALUE</a> <span "
    //   "style=\"color:rgb(0, 102, 0);\">0x0001, 1</span><br>"
    //   "<br>"
    //   "<b>GUID: </b><small><span style=\"color:rgb(0, 102, "
    //   "0);\">FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF</span></small><br>"

    //   "<br><br>This <b>widget takes</b> up all the remaining space "
    //   "in the top-level layout ddddd."
    //   "in the top-level layout ddddd."
    //   "<h1>This is a test</h1> <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "This is a test <br>"
    //   "Carpe Diem <br>"));

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
    m_txTable->setContextMenuPolicy(
      Qt::CustomContextMenu); // Enable context menu
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
    const QIcon editIcon = QIcon::fromTheme("format-justify-center");
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
    const QIcon connectIcon    = QIcon::fromTheme("call-start");
    const QIcon disconnectIcon = QIcon::fromTheme("call-stop");
    // if ( m_connectAct->isChecked()) {
    //     m_connectAct->setIcon(connectIcon);
    // }
    // else {
    //     m_connectAct->setIcon(disconnectIcon);
    // }
    // m_connectAct->setIconVisibleInMenu(true);
}

///////////////////////////////////////////////////////////////////////////////
// menu_filter
//

void
CFrmSession::menu_filter()
{
    // insert into events (class,type) values (11,22);
    // QSqlQuery query = QSqlQuery( m_evdb );
    // query.exec("insert into events (class,type) values (11,22);");
    // for (int i = 0; i<9999; i++) {
    //     QSqlField field_idx("idx", QVariant::Int, "events");
    //     field_idx.setAutoValue(true);
    //     QSqlField field_class("class", QVariant::Int, "events");
    //     field_class.setValue(10);
    //     QSqlField field_type("type", QVariant::Int, "events");
    //     field_type.setValue(i);
    //     QSqlRecord rec;
    //     rec.append(field_idx);
    //     rec.append(field_class);
    //     rec.append(field_type);
    //     //tableModels->value((*registryMap)[type])
    //     m_rxmodel->insertRecord(-1, rec);
    // }
    // m_rxmodel->submitAll();
}

///////////////////////////////////////////////////////////////////////////////
// menu_clear_rxlist
//

void
CFrmSession::menu_clear_rxlist(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

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
    }

    // Clear the event counter
    m_mapEventToCount.clear();

    // Erase all comments
    m_mapEventComment.clear();

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
CFrmSession::txItemChanged(QTableWidgetItem *item)
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

bool CFrmSession::addTxRow(bool bEnable, 
                            const QString& name, 
                            uint8_t count, 
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

    // Period
    item = new QTableWidgetItem;
    if (nullptr == item) {
        return false;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(period));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

    // Count
    if (!count) count = 1;  // Minvalue
    item = new QTableWidgetItem;
    if (nullptr == item) {
        return false;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignHCenter);
    item->setText(QString::number(count));
    m_txTable->setItem(m_txTable->rowCount() - 1, txrow_period, item);

    // Event
    CTxWidgetItem* itemEvent = new CTxWidgetItem("Test");
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
// sendTxEvent
//

void
CFrmSession::sendTxEvent(void)
{
    QMessageBox::about(this, tr("Send Event"), tr("Carpe Diem"));
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

        // Period
        item = new QTableWidgetItem;
        if (nullptr == item) {
            return;
        }
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setText(QString::number(dlg.getPeriod()));
        m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

        // Count
        item = new QTableWidgetItem;
        if (nullptr == item) {
            return;
        }
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setText(QString::number(dlg.getCount()));
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
                                tr("vscpworks+"),
                                tr("You must select a transmission row"),
                                QMessageBox::Ok);
        return;
    }

    QList<QModelIndex>::iterator it;
    for (it = selection.begin(); it != selection.end(); it++) {

        CTxWidgetItem* itemEvent = (CTxWidgetItem *)m_txTable->item(it->row(), txrow_event);        
        vscpEvent* pev = itemEvent->m_tx.getEvent();

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

            QTableWidgetItem *item;

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
                delete [] pev->pdata;
                pev->sizeData = 0;
                pev->pdata = 0;
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
                                tr("vscpworks+"),
                                tr("You must select a transmission row"),
                                QMessageBox::Ok);
        return;
    }

    QList<QModelIndex>::iterator it;
    for (it = selection.begin(); it != selection.end(); it++) {

        QTableWidgetItem *item;
        CTxWidgetItem* itemSourceEvent = (CTxWidgetItem *)m_txTable->item(it->row(), txrow_event);
        if (nullptr == itemSourceEvent) {
            return;
        }
        CTxWidgetItem* itemTargetEvent = new CTxWidgetItem("Test");
        if (nullptr == itemTargetEvent) {
            return;
        }

        itemTargetEvent->m_tx.newEvent();

        // Copy in event
        vscp_copyEvent(itemTargetEvent->m_tx.getEvent(), itemSourceEvent->m_tx.getEvent());

        // Save data
        //vscpEvent* pevSource = itemSourceEvent->m_tx.getEvent();
        bool bEnable = itemSourceEvent->m_tx.getEnable();
        QString name = itemSourceEvent->m_tx.getName();
        name += tr("_copy");
        uint8_t count = itemSourceEvent->m_tx.getCount();
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

        // Period
        item = new QTableWidgetItem;
        if (nullptr == item) {
            return;
        }
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setText(QString::number(period));
        m_txTable->setItem(m_txTable->rowCount() - 1, txrow_count, item);

        // Count
        item = new QTableWidgetItem;
        if (nullptr == item) {
            return;
        }
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setText(QString::number(count));
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
                                tr("vscpworks+"),
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
    QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

    if (!path.length()) {
        QString initialPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        initialPath += "/txsets/txset.xml";
        qDebug() << initialPath;                                                    
        fileName = QFileDialog::getOpenFileName(this,
                                    tr("File to load transmition events from"),  
                                    initialPath, 
                                    tr("TX files (*.xml *.*)"));
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cannot read file" << file.errorString();
        return;
    }

    QXmlStreamReader reader(&file);

    if (reader.readNextStartElement()) {
        if (reader.name() == "txrows"){
            qDebug() << reader.name();
            while(reader.readNextStartElement()){
                
                bool bEnable = false;
                QString name = tr("no name");
                uint8_t count = 1;
                uint32_t period = 0;
                QString event;

                if (reader.name() == "row"){

                    // enable
                    if (reader.attributes().hasAttribute("enable")) {
                        QString enable =
                            reader.attributes().value("enable").toString();  
                        if (enable.contains("true", Qt::CaseInsensitive)) bEnable = true;                            
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
                            reader.attributes().value("period").toUInt();    
                    }

                    // event
                    if (reader.attributes().hasAttribute("event")) {
                        event =
                            reader.attributes().value("event").toString();  
                    }
                   
                }

                qDebug() << bEnable;
                qDebug() << name;
                qDebug() << count;
                qDebug() << period;
                qDebug() << event;
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

void CFrmSession::loadTxOnStart(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    //  QStandardPaths::AppLocalDataLocation
    QString loadPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    loadPath += "/cache/txevents_";
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
    vscpEvent* pev;
    QModelIndexList selection = m_txTable->selectionModel()->selectedRows();

    if (!path.length()) {
        QString initialPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        initialPath += "/txsets/txset.xml";
        qDebug() << initialPath;                                                    
        fileName = QFileDialog::getSaveFileName(this,
                                    tr("File to save transmition events to"),  
                                    initialPath, 
                                    tr("TX files (*.xml *.*)"));
    }

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
                CTxWidgetItem* itemSourceEvent = (CTxWidgetItem *)m_txTable->item(it->row(), txrow_event);

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
                //stream.writeTextElement("test", "This is a test");
                
                stream.writeEndElement(); // row

            }
        }
        else {

            // save all
            for (int i=0; i<m_txTable->rowCount(); i++) {
                CTxWidgetItem* itemSourceEvent = (CTxWidgetItem *)m_txTable->item(i, txrow_event);
                                
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
                    //stream.writeTextElement("test", "This is a test");

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

void CFrmSession::saveTxOnExit(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    //  QStandardPaths::AppLocalDataLocation
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    savePath += "/cache/txevents_";
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
        if (pworks->m_session_bAutoConnect) {
            if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
                pworks->log(pworks->LOG_LEVEL_ERROR,
                            "Session: Unable to connect to remote client");
                QMessageBox::information(
                  this,
                  tr("vscpworks+"),
                  tr("Failed to open a connection to the remote host"),
                  QMessageBox::Ok);
            }
            else {
            }
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
CFrmSession::doConnectToRemoteHost(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    switch (m_vscpConnType) {

        case CVscpClient::connType::NONE:
            break;

        case CVscpClient::connType::LOCAL:
            break;

        case CVscpClient::connType::TCPIP:
            if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
                pworks->log(pworks->LOG_LEVEL_ERROR,
                            "Session: Unable to connect to remote client");
                QMessageBox::information(
                  this,
                  tr("vscpworks+"),
                  tr("Failed to open a connection to the remote host"),
                  QMessageBox::Ok);
            }
            else {
                pworks->log(pworks->LOG_LEVEL_ERROR,
                            "Session: Successful connect to remote client");
            }
            break;

        case CVscpClient::connType::CANAL:
            break;

        case CVscpClient::connType::SOCKETCAN:
            break;

        case CVscpClient::connType::WS1:
            break;

        case CVscpClient::connType::WS2:
            break;

        case CVscpClient::connType::MQTT:
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
CFrmSession::doDisconnectFromRemoteHost(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    switch (m_vscpConnType) {

        case CVscpClient::connType::NONE:
            break;

        case CVscpClient::connType::LOCAL:
            break;

        case CVscpClient::connType::TCPIP:
            if (VSCP_ERROR_SUCCESS != m_vscpClient->disconnect()) {
                pworks->log(pworks->LOG_LEVEL_ERROR,
                            "Session: Unable to disconnect remote client");
                QMessageBox::information(
                  this,
                  tr("vscpworks+"),
                  tr("Failed to disconnect the connection to the remote host"),
                  QMessageBox::Ok);
            }
            break;

        case CVscpClient::connType::CANAL:
            break;

        case CVscpClient::connType::SOCKETCAN:
            break;

        case CVscpClient::connType::WS1:
            break;

        case CVscpClient::connType::WS2:
            break;

        case CVscpClient::connType::MQTT:
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
    menu->addAction(QString(tr("Save events to file...")),
                    this,
                    SLOT(saveRxToFile()));
    menu->addAction(QString(tr("Save marked event rows to file...")),
                    this,
                    SLOT(saveRxToFile()));
    menu->addAction(QString(tr("Save marked class event rows to file...")),
                    this,
                    SLOT(saveRxToFile()));
    menu->addAction(QString(tr("Save marked type event rows to file...")),
                    this,
                    SLOT(saveRxToFile()));
    menu->addAction(QString(tr("Load events from file...")),
                    this,
                    SLOT(loadRxFromFile()));
    menu->addSeparator();
    menu->addAction(QString(tr("Set/edit GUID (sensor)")),
                    this,
                    SLOT(setGuid()));
    menu->addSeparator();
    menu->addAction(QString(tr("Add comment...")), this, SLOT(addEventNote()));
    menu->addAction(QString(tr("Remove comment")),
                    this,
                    SLOT(removeEventNote()));
    menu->addSeparator();
    menu->addAction(QString(tr("Mark row")), this, SLOT(setVscpRowMark()));
    menu->addAction(QString(tr("Unmark row")), this, SLOT(unsetVscpRowMark()));
    menu->addAction(QString(tr("Mark VSCP class")),
                    this,
                    SLOT(setVscpClassMark()));
    menu->addAction(QString(tr("Unmark VSCP class")),
                    this,
                    SLOT(unsetVscpClassMark()));
    menu->addAction(QString(tr("Mark VSCP type")),
                    this,
                    SLOT(setVscpTypeMark()));
    menu->addAction(QString(tr("Unmark VSCP type")),
                    this,
                    SLOT(unsetVscpTypeMark()));

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
    QString text = QInputDialog::getText(this,
                                         tr("QInputDialog::getText()"),
                                         tr("Comment:"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);
    if (ok && !text.isEmpty()) {
        QIcon icon(":/comment.png");
        QModelIndexList selection = m_rxTable->selectionModel()->selectedRows();
        QList<QModelIndex>::iterator it;
        for (it = selection.begin(); it != selection.end(); it++) {
            m_rxTable->item(it->row(), 0)->setIcon(icon);
            m_mapEventComment[it->row()] = text;
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
        itmap = m_mapEventComment.find(it->row());
        m_mapEventComment.erase(itmap);
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
        (*it)->setBackground(Qt::cyan);
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
        m_rxTable->item(it->row(), 1)->setIcon(icon);
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
        m_rxTable->item(it->row(), 2)->setIcon(icon);
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
// saveRxToFile
//

void
CFrmSession::saveRxToFile(void)
{
    int i = 8;
}

///////////////////////////////////////////////////////////////////////////////
// loadRxFromFile
//

void
CFrmSession::loadRxFromFile(void)
{
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
    if (nullptr == pev)
        return;

    std::string strVscpTemplate =
      "<h3>VSCP Event</h3>"
      "<small><p style=\"color:#993399\">Received event</p></small>"
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
    std::list<renderFunc*> m_renderFuncs;
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
                    m_renderFuncs.push_back(prf);
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
        while (m_renderFuncs.size()) {
            struct renderFunc* prf = m_renderFuncs.front();
            // Render
            _data.set(prf->name.toStdString(), prf->value.toStdString());
            qDebug() << prf->name << " - " << prf->value;
            // Remove from list
            m_renderFuncs.pop_front();
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

    QTableWidgetItem* itemClass = m_rxTable->item(selectedRow, 1);
    QTableWidgetItem* itemType  = m_rxTable->item(selectedRow, 2);

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
    std::string strVscpComment = m_mapEventComment[selectedRow].toStdString();
    if (strVscpComment.length()) {
        strVscpComment =
          "<hr><b>Comment:</b><p style=\"color:rgb(0x80, 0x80, 0x80);\">" +
          strVscpComment;
        strVscpComment += "</p>";
    }

    kainjow::mustache::data _data;
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
// receiveRow
//

void
CFrmSession::receiveRow(vscpEvent* pev)
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
      m_mapEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] +
      1;
    m_mapEventToCount[((uint32_t)(pev->vscp_class) << 16) + pev->vscp_type] =
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
        for (it = m_mapEventToCount.begin(); it != m_mapEventToCount.end();
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

void
CFrmSession::threadReceive(vscpEvent* pev)
{
    emit dataReceived(pev);
}