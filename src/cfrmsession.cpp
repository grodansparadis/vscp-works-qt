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

#include <QtWidgets>
#include <QSqlTableModel>
#include <QTableView>
#include "cfrmsession.h"

#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// initializeModel
//

void initializeModel(QSqlTableModel *model)
{
    model->setTable("events");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Dir"));    // Direction
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Class"));  // VSCP Class
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));   // VSCP Type
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("GUID"));   // GUID + node-id
}

///////////////////////////////////////////////////////////////////////////////
// CTor
//

QTableView *createView(QSqlTableModel *model, const QString &title = "VSCP Session")
{
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    return view;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmSession::~CFrmSession()
{
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    //delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// CFrmSession
//

CFrmSession::CFrmSession(QWidget *parent) :
    QDialog(parent)
{
    //QVBoxLayout *mainLayout = new QVBoxLayout(this);

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

    m_bigEditor = new QTextEdit;
    m_bigEditor->setPlainText(tr("<h1>Rubrik</h1>This <b>widget takes</b> up all the remaining space "
                               "in the top-level layout ddddd."));

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
    setWindowTitle(tr("Client Session"));
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

    const QIcon connectIcon = QIcon::fromTheme("call-start"); 
    const QIcon disconnectIcon = QIcon::fromTheme("call-stop"); 
    const QIcon filterIcon = QIcon::fromTheme("edit-find");
    
    m_loadEventsAct = m_fileMenu->addAction(tr("Load VSCP events from file..."));
    m_saveEventsAct = m_fileMenu->addAction(tr("Save VSCP events to file..."));
    m_loadTxAct = m_fileMenu->addAction(tr("Load transmission set from file..."));
    m_saveTxAct = m_fileMenu->addAction(tr("Save transmission set to file"));
    
    m_exitAct = m_fileMenu->addAction(windowCloseIcon, tr("Close session window"), this, &QWidget::close);
    m_exitAct->setStatusTip(tr("Close session window"));
    
    m_menuBar->addMenu(m_fileMenu);

    m_toolBar->addSeparator();
    
    // Connect
    m_connectAct = m_toolBar->addAction(connectIcon, tr("Connect"), this, &CFrmSession::menu_connect);
    m_connectAct->setStatusTip(tr("Connect/disconnect from remote host"));
    m_connectAct->setCheckable(true);

    // Filter
    m_setFilterAct = m_toolBar->addAction(filterIcon, tr("Enable filter"), this, &CFrmSession::menu_filter);
    m_setFilterAct->setStatusTip(tr("Enable/disable filter"));
    m_setFilterAct->setCheckable(true);

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
    QStringList headers(QString(tr("dir,class,type,guid")).split(','));
    m_rxTable = new QTableWidget;
    //QTableWidget::horizontalHeader().setStretchLastSection(true);
    //m_rxTable->setRowCount(4);
    //m_rxTable->setVerticalHeaderLabels(headers);
    m_rxTable->setColumnCount(4);
    m_rxTable->setColumnWidth(0, 20);     // Dir
    m_rxTable->setColumnWidth(1, 200);    // Class
    m_rxTable->setColumnWidth(2, 200);    // Type
    m_rxTable->horizontalHeader()->setStretchLastSection(true);
    m_rxTable->setHorizontalHeaderLabels(headers);
    layout->addWidget(m_rxTable, 0, 0, 1, 4); //  fromRow, fromColumn, rowSpan, columnSpan

    m_infoArea = new QTextEdit;
    m_infoArea->setAcceptRichText(true);
    QColor grey(Qt::red);
    m_infoArea->setTextBackgroundColor(grey);
    m_infoArea->insertHtml(tr("<h1>Header</h1>This <b>widget takes</b> up all the remaining space "
                               "in the top-level layout ddddd."));
    layout->addWidget(m_infoArea, 0, 4, 1, 1);

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

}


///////////////////////////////////////////////////////////////////////////////
// transmitEvent
//

void CFrmSession::transmitEvent()
{
    QMessageBox::about(this, tr("Test"), tr("Carpe Diem") );
}

void CFrmSession::createFormGroupBox()
{
    m_formGroupBox = new QGroupBox(tr("Form layout"));
    QFormLayout *layout = new QFormLayout;
    layout->addRow(new QLabel(tr("Line 1:")), new QLineEdit);
    layout->addRow(new QLabel(tr("Line 2, long text:")), new QComboBox);
    layout->addRow(new QLabel(tr("Line 3:")), new QSpinBox);
    m_formGroupBox->setLayout(layout);
}
