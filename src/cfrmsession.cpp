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


#include "cfrmsession.h"

#include <stdlib.h>

#include <QtWidgets>
#include <QSqlTableModel>
#include <QTableView>
#include <QtSql>


///////////////////////////////////////////////////////////////////////////////
// vscp_client_callback
//


void CVscpClientCallback::eventReceived(vscpEvent *pev)
{
    vscpEvent ev;
    emit addRow(&ev, true);
}

// ----------------------------------------------------------------------------







///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmSession::~CFrmSession()
{
    // Close the event database3
    //m_evdb.close();
    //delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// CFrmSession
//

CFrmSession::CFrmSession(QWidget *parent) :
    QDialog(parent)
{
    //QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Set up database
    // https://wiki.qt.io/How_to_Store_and_Retrieve_Image_on_SQLite
    // QString uuid = QUuid::createUuid().toString();
    // uuid.remove('{');
    // uuid.remove('}');
    // QString eventdbname = "/tmp/vscpevents-" + uuid + ".sqlite3";
    // QString dbName(eventdbname);
    // QFile::remove( dbName ); // delete sqlite file if it exists from a previous run
    // m_evdb = QSqlDatabase::addDatabase( "QSQLITE" );
    // m_evdb.setDatabaseName( dbName );
    // m_evdb.open();
    // QSqlQuery query = QSqlQuery( m_evdb );
    // query.exec("CREATE TABLE IF NOT EXISTS events ("
	//             "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
	//             "head	INTEGER,"
	//             "class	INTEGER,"
	//             "type	INTEGER,"
	//             "sizedata	INTEGER,"
	//             "data	BLOB,"
	//             "obid	INTEGER,"
	//             "guid	BLOB,"
	//             "datetime	TEXT);");

    // No connection set yet
    m_vscpConnType = CVscpClient::connType::NONE;
    m_vscpClient = NULL;

    // Initial default size of window
    int nWidth = 1200;
    int nHeight = 800;

    if (parent != NULL) {
        setGeometry(parent->x() + parent->width()/2 - nWidth/2,
                    parent->y() + parent->height()/2 - nHeight/2,
                    nWidth, nHeight);
    }
    else {
        resize(nWidth, nHeight);
    }

    // We need a toolbar
    m_toolBar = new QToolBar();

    createMenu();
    createHorizontalGroupBox();
    createRxGroupBox();
    //createFormGroupBox();
    createTxGridGroup();    

    m_bigEditor = new QTextBrowser;
    m_bigEditor->setPlainText(tr("<h1>RRRRRubrik</h1>This <b>widget takes</b> up all the remaining space "
                               "in the top-level layout ddddd."
                               "<h1>This is a test</h1> <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                                "This is a test <br>"
                               "This is a test <br>"
                               "Carpe Diem <br>"
                               ));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Lay out things
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(m_menuBar);
    mainLayout->addWidget(m_toolBar);
    //mainLayout->addWidget(horizontalGroupBox);
    mainLayout->addWidget(m_gridGroupBox,6);
    mainLayout->addWidget(m_txGroupBox,3);
    
    //mainLayout->addWidget(formGroupBox);
    //mainLayout->addWidget(bigEditor);
    //mainLayout->addStretch(80);
    //mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
    setWindowTitle(tr("VSCP Client Session"));        
}

///////////////////////////////////////////////////////////////////////////////
// createMenu
//

void CFrmSession::createMenu()
{
    m_menuBar = new QMenuBar;

    // File menu
    m_fileMenu = new QMenu(tr("&File"), this);

    // Icons
    const QIcon windowCloseIcon = QIcon::fromTheme("window-close"); 

    const QIcon loadIcon = QIcon::fromTheme("window-close");
    const QIcon saveIcon = QIcon::fromTheme("window-close");

    //const QIcon connectIcon = QIcon::fromTheme("call-start"); 
    const QIcon disconnectIcon = QIcon::fromTheme("call-stop"); 
    //const QIcon filterIcon = QIcon::fromTheme("edit-find");
    //const QIcon filterIcon = QIcon::fromTheme("edit-find");
    
    m_loadEventsAct = m_fileMenu->addAction(tr("Load VSCP events from file..."));
    m_saveEventsAct = m_fileMenu->addAction(tr("Save VSCP events to file..."));
    m_loadTxAct = m_fileMenu->addAction(tr("Load transmission set from file..."));
    m_saveTxAct = m_fileMenu->addAction(tr("Save transmission set to file"));
    
    m_exitAct = m_fileMenu->addAction(windowCloseIcon, tr("Close session window"), this, &QWidget::close);
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
    m_connectAct = m_toolBar->addAction(connectIcon, tr("Connect"), this, &CFrmSession::menu_connect);
    m_connectAct->setStatusTip(tr("Connect/disconnect from remote host"));
    m_connectAct->setCheckable(true);

    m_toolBar->addSeparator();

    // Filter
    QIcon filterIcon(":/filter.png");
    m_setFilterAct = m_toolBar->addAction(filterIcon, tr("Enable filter"), this, &CFrmSession::menu_filter);
    m_setFilterAct->setStatusTip(tr("Enable/disable filter"));
    m_setFilterAct->setCheckable(true);

    m_filterComboBox = new QComboBox;
    m_filterComboBox->addItem("This is the magnifićant Filter 1");
    m_filterComboBox->addItem("Filter 2");
    m_filterComboBox->addItem("Filter 3");
    m_filterComboBox->addItem("Filter 4");
    m_toolBar->addWidget(m_filterComboBox);

    m_toolBar->addSeparator();

    

    //toolBar->addAction(connectAct);
    
    // Edit menu
    m_editMenu = new QMenu(tr("&Edit"), this);
    m_cutAct = m_editMenu->addAction(tr("Cut"));
    m_copyAct = m_editMenu->addAction(tr("Copy"));
    m_pasteBeforeAct = m_editMenu->addAction(tr("Paste before"));
    m_pasteAfterAct = m_editMenu->addAction(tr("Paste after"));
    m_menuBar->addMenu(m_editMenu);

    // Host menu
    m_hostMenu = new QMenu(tr("&Host"), this);
    m_connectAct = m_hostMenu->addAction(tr("Connect to host..."));
    m_disconnectAct = m_hostMenu->addAction(tr("Disconnect from host..."));
    m_pauseAct = m_hostMenu->addAction(tr("Pause host"));
    m_addHostAct = m_hostMenu->addAction(tr("Add host..."));
    m_editHostAct = m_hostMenu->addAction(tr("Edit host settings..."));
    m_menuBar->addMenu(m_hostMenu);

    // View menu
    m_viewMenu = new QMenu(tr("&View"), this);
    m_viewMessageAct = m_viewMenu->addAction(tr("Message flow"));
    m_viewCountAct = m_viewMenu->addAction(tr("Message count"));
    m_viewMenu->addSeparator();
    m_viewClrRxListAct = m_viewMenu->addAction(tr("Clear receive list..."));
    m_viewClrTxListAct = m_viewMenu->addAction(tr("Clear transmission list..."));
    m_menuBar->addMenu(m_viewMenu);

    // VSCP menu
    m_vscpMenu = new QMenu(tr("&vscp"), this);
    m_readRegAct = m_vscpMenu->addAction(tr("Read register..."));
    m_writeRegAct = m_vscpMenu->addAction(tr("Write register..."));
    m_readAllRegAct = m_vscpMenu->addAction(tr("Read (all) registers..."));
    m_readGuidAct = m_vscpMenu->addAction(tr("Read GUID"));
    m_readMdfAct = m_vscpMenu->addAction(tr("Read MDF..."));
    m_loadMdfAct = m_vscpMenu->addAction(tr("Download MDF..."));
    m_menuBar->addMenu(m_vscpMenu);

    // Settings menu
    m_settingsMenu = new QMenu(tr("&Setting"), this);
    m_setFilterAct = m_settingsMenu->addAction(tr("Set/define filter..."));
    m_settingsAct = m_settingsMenu->addAction(tr("Settings..."));
    m_menuBar->addMenu(m_settingsMenu);

    // Tools menu
    m_toolsMenu = new QMenu(tr("&Tools"), this);
    m_menuBar->addMenu(m_toolsMenu);

    connect(m_exitAct, &QAction::triggered, this, &QDialog::accept);

// #ifndef QT_NO_CLIPBOARD
//     cutAct->setEnabled(false);

//     copyAct->setEnabled(false);
//     connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
//     connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
// #endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// createHorizontalGroupBox
//

void CFrmSession::createHorizontalGroupBox()
{
    m_horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
    QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i < NumButtons; ++i) {
        m_buttons[i] = new QPushButton(tr("Button %1").arg(i + 1));
        layout->addWidget(m_buttons[i]);
    }
    m_horizontalGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createRxGridGroup
//

void CFrmSession::createRxGroupBox()
{
    m_gridGroupBox = new QGroupBox(tr("Receive Events "));
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(1,1,1,1);

    // for (int i = 0; i < NumGridRows; ++i) {
    //     labels[i] = new QLabel(tr("Line %1:").arg(i + 1));
    //     lineEdits[i] = new QLineEdit;
    //     layout->addWidget(labels[i], i + 1, 0);
    //     layout->addWidget(lineEdits[i], i + 1, 1);
    // }
    QStringList headers(QString(tr("Dir, VSCP Class, VSCP Type, id, GUID")).split(','));
    m_rxTable = new QTableWidget;
    m_rxTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    //QTableWidget::horizontalHeader().setStretchLastSection(true);
    m_rxTable->setRowCount(4);
    //m_rxTable->setVerticalHeaderLabels(headers);
    m_rxTable->setColumnCount(5);
    m_rxTable->setColumnWidth(0, 10);       // Dir
    m_rxTable->setColumnWidth(1, 200);      // Class
    m_rxTable->setColumnWidth(2, 200);      // Type
    m_rxTable->setColumnWidth(3, 50);       // Node id
    m_rxTable->setColumnWidth(4, 50);       // GUID
    m_rxTable->horizontalHeader()->setStretchLastSection(true);
    m_rxTable->setHorizontalHeaderLabels(headers);

    QTableWidgetItem *item = new QTableWidgetItem("ᐊ ᐅ"); // ➤ ➜ ➡ ➤
    item->setTextAlignment(Qt::AlignHCenter);
    //item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    int row  = m_rxTable->rowCount();
    m_rxTable->insertRow(row);
    
    //QTableWidgetItem *item = m_rxTable->item(1, 1);
    item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
    item->setForeground(QBrush(QColor(0, 5, 180)));

    m_rxTable->setItem( m_rxTable->rowCount()-1, 0, item);
    //QTableWidgetItem *item; 

    QTableWidgetItem *icon_item = new QTableWidgetItem;
    const QIcon loadIcon = QIcon::fromTheme("window-close");
    icon_item->setIcon(loadIcon);
    m_rxTable->insertRow(row);
    m_rxTable->setItem( m_rxTable->rowCount()-1, 1, icon_item);

    // item = m_rxTable->item(row, 1);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    // item = m_rxTable->item(row, 2);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    // item = m_rxTable->item(row, 3);
    // item->setFlags(item->flags() ^ Qt::ItemIsEditable);

    m_rxTable->setUpdatesEnabled(false);
    for(int i =0; i<m_rxTable->rowCount(); i++) {               
        m_rxTable->setRowHeight(i, 10); 
    }
    m_rxTable->setUpdatesEnabled(true);

    // <---
    //QSqlTableModel *
    // m_rxmodel = new QSqlTableModel;
    // m_rxmodel->setTable("events");
    // m_rxmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // m_rxmodel->select();
    // m_rxmodel->setHeaderData(0, Qt::Horizontal, tr("class"));
    // m_rxmodel->setHeaderData(1, Qt::Horizontal, tr("type"));

    // m_rxTable->setModel(m_rxmodel);
    // m_rxTable->hideColumn(0); // don't show the ID
    
    layout->addWidget(m_rxTable, 0, 0, 1, 4); //  fromRow, fromColumn, rowSpan, columnSpan 

    m_rxTable->show(); 

    m_infoArea = new QTextBrowser;
    m_infoArea->setAcceptRichText(true);
    m_infoArea->setOpenLinks(true);
    m_infoArea->setOpenExternalLinks(true); 
    QColor grey(Qt::red);
    m_infoArea->setTextBackgroundColor(grey);
    m_infoArea->insertHtml(tr("<h3>VSCP Event</h3>"
                               "<small><p style=\"color:#993399\">Received event</p></small>"
                               "<b>Head: </b><span style=\"color:rgb(0, 0, 153);\">0x0100</span><br>"
                               "<b>Time: </b><span style=\"color:rgb(0, 0, 153);\">2021-09-12T12:10:29</span><br>"
                               "<b>Timestamp: </b><span style=\" color:rgb(0, 0, 153);\">0x1213140f</span><br>"
                               "<br>"
                               "<b>Class: </b><a href=\"https://www.vscp.org\">CLASS1_DATA</a><span style=\"color:rgb(0, 102, 0);\"> 0x000F, 15</span><br>"
                               "<b>Type: </b><a href=\"https://www.vscp.org\">IO-VALUE</a> <span style=\"color:rgb(0, 102, 0);\">0x0001, 1</span><br>"
                               "<br>"
                               "<b>GUID: </b><small><span style=\"color:rgb(0, 102, 0);\">FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF</span></small><br>"
                               
                               "<br><br>This <b>widget takes</b> up all the remaining space "
                               "in the top-level layout ddddd."
                               "in the top-level layout ddddd."
                               "<h1>This is a test</h1> <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                               "This is a test <br>"
                                "This is a test <br>"
                               "This is a test <br>"
                               "Carpe Diem <br>"
                               ));
                              
    layout->addWidget(m_infoArea, 0, 4, 1, 2); //  fromRow, fromColumn, rowSpan, columnSpan
    layout->setColumnMinimumWidth(4,350);

    // 30 10
    layout->setColumnStretch(0, 30);
    layout->setColumnStretch(1, 10);
    m_gridGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createTxGridGroup
//

void CFrmSession::createTxGridGroup()
{
    m_txGroupBox = new QGroupBox(tr("Transmit"));
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(1,1,1,1);

    QStringList headers(QString(tr("x,Name,Period,Count,Trigger,Event")).split(','));
    m_txTable = new QTableWidget;
    m_txTable->setColumnCount(6);
    m_txTable->setColumnWidth(0, 20);     // x
    m_txTable->setColumnWidth(1, 300);    // Name
    m_txTable->setColumnWidth(2, 80);     // Period
    m_txTable->setColumnWidth(3, 80);     // Count
    m_txTable->setColumnWidth(4, 80);     // Trigger
    m_txTable->setColumnWidth(5, 50);     // Event
    m_txTable->horizontalHeader()->setStretchLastSection(true);
    m_txTable->setHorizontalHeaderLabels(headers);
    layout->addWidget(m_txTable, 0, 0, 1, 4); //  fromRow, fromColumn, rowSpan, columnSpan

    // Transmit/Add/Clone/Delete/Edit/Save/Load
    m_txToolBar = new QToolBar;
    m_txToolBar->setOrientation(Qt::Vertical);

    // Transmit
    const QIcon newIcon = QIcon::fromTheme("edit-undo"); //QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *transmitAct = new QAction(newIcon, tr("&Transmit"), this);
    transmitAct->setShortcuts(QKeySequence::New);
    transmitAct->setStatusTip(tr("Transmit selected event(s)"));
    connect(transmitAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(transmitAct);

    // Add tx row
    const QIcon addIcon = QIcon::fromTheme("document-new"); // QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *addAct = new QAction(addIcon, tr("&Add"), this);
    addAct->setShortcuts(QKeySequence::New);
    addAct->setStatusTip(tr("Add transmit event"));
    connect(addAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(addAct);

    // Edit tx row
    const QIcon editIcon = QIcon::fromTheme("format-justify-center");
    QAction *editAct = new QAction(editIcon, tr("&Edit"), this);
    editAct->setShortcuts(QKeySequence::New);
    editAct->setStatusTip(tr("Edit selected event"));
    connect(editAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(editAct);

    // Delete tx row
    const QIcon deleteIcon = QIcon::fromTheme("edit-delete");
    QAction *deleteAct = new QAction(deleteIcon, tr("&Delete"), this);
    deleteAct->setShortcuts(QKeySequence::New);
    deleteAct->setStatusTip(tr("Delete selected event"));
    connect(deleteAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(deleteAct);

    // Clone tx row
    const QIcon cloneIcon = QIcon::fromTheme("edit-copy");
    QAction *cloneAct = new QAction(cloneIcon, tr("&Clone"), this);
    cloneAct->setShortcuts(QKeySequence::New);
    cloneAct->setStatusTip(tr("Clone selected event"));
    connect(cloneAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(cloneAct);

    m_txToolBar->addSeparator();

    // Save tx rows
    const QIcon saveIcon = QIcon::fromTheme("document-save");
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::New);
    saveAct->setStatusTip(tr("Save selected transmit event(s)"));
    connect(saveAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(saveAct);

    // Load tx rows
    const QIcon loadIcon = QIcon::fromTheme("document-open");
    QAction *loadAct = new QAction(loadIcon, tr("&Load"), this);
    loadAct->setShortcuts(QKeySequence::New);
    loadAct->setStatusTip(tr("Load transmit event(s)"));
    connect(loadAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    m_txToolBar->addAction(loadAct);

    layout->addWidget(m_txToolBar, 0, 4, 1, 1);

    layout->setColumnStretch(0, 70);
    layout->setColumnStretch(1, 10);
    m_txGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// menu_connect
//

void CFrmSession::menu_connect()
{
    const QIcon connectIcon = QIcon::fromTheme("call-start"); 
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

void CFrmSession::menu_filter()
{
    //insert into events (class,type) values (11,22);
    //QSqlQuery query = QSqlQuery( m_evdb );
    //query.exec("insert into events (class,type) values (11,22);");
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
// transmitEvent
//

void CFrmSession::transmitEvent()
{
    QMessageBox::about(this, tr("Test"), tr("Carpe Diem") );
}

///////////////////////////////////////////////////////////////////////////////
// createFormGroupBox
//

void CFrmSession::createFormGroupBox()
{
    m_formGroupBox = new QGroupBox(tr("Form layout"));
    QFormLayout *layout = new QFormLayout;
    layout->addRow(new QLabel(tr("Line 1:")), new QLineEdit);
    layout->addRow(new QLabel(tr("Line 2, long text:")), new QComboBox);
    layout->addRow(new QLabel(tr("Line 3:")), new QSpinBox);
    m_formGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// addRow
//

void CFrmSession::addRow(const vscpEvent& ev, bool bReceive)
{

}

