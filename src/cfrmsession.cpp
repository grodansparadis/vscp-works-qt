// cfrmsession.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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

#include <QtWidgets>
#include <QSqlTableModel>
#include <QTableView>
#include "cfrmsession.h"
//#include "ui_cfrmsession.h"

#include <stdlib.h>

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

QTableView *createView(QSqlTableModel *model, const QString &title = "")
{
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    return view;
}

CFrmSession::~CFrmSession()
{
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    //delete ui;
}



CFrmSession::CFrmSession(QWidget *parent) :
    QDialog(parent)
{
    //QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Initial default size of window
    int nWidth = 800;
    int nHeight = 400;
    if (parent != NULL) {
        setGeometry(parent->x() + parent->width()/2 - nWidth/2,
                    parent->y() + parent->height()/2 - nHeight/2,
                    nWidth, nHeight);
    }
    else {
        resize(nWidth, nHeight);
    }

    // We need a toolbar
    toolBar = new QToolBar();

    createMenu();
    createHorizontalGroupBox();
    createGridGroupBox();
    createFormGroupBox();
    createTxGridGroup();    

    bigEditor = new QTextEdit;
    bigEditor->setPlainText(tr("This widget takes up all the remaining space "
                               "in the top-level layout."));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Lay out things
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(toolBar);
    //mainLayout->addWidget(horizontalGroupBox);
    mainLayout->addWidget(gridGroupBox);
    mainLayout->addWidget(txGroupBox);
    
    //mainLayout->addWidget(formGroupBox);
    //mainLayout->addWidget(bigEditor);
    //mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);

    setWindowTitle(tr("Client Session"));
}

void CFrmSession::createMenu()
{
    menuBar = new QMenuBar;

    // File menu
    fileMenu = new QMenu(tr("&File"), this);
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    
    loadEventsAct = fileMenu->addAction(tr("Load VSCP events from file..."));
    saveEventsAct = fileMenu->addAction(tr("Save VSCP events to file..."));
    loadTxAct = fileMenu->addAction(tr("Load transmission set from file..."));
    saveTxAct = fileMenu->addAction(tr("Save transmission set to file"));
    
    exitAct = fileMenu->addAction(newIcon, tr("Close"), this, &QWidget::close);
    exitAct->setStatusTip(tr("Close client window"));
    
    menuBar->addMenu(fileMenu);
    toolBar->addAction(exitAct);
    
    // Edit menu
    editMenu = new QMenu(tr("&Edit"), this);
    cutAct = editMenu->addAction(tr("Cut"));
    copyAct = editMenu->addAction(tr("Copy"));
    pasteBeforeAct = editMenu->addAction(tr("Paste before"));
    pasteAfterAct = editMenu->addAction(tr("Paste after"));
    menuBar->addMenu(editMenu);

    // Host menu
    hostMenu = new QMenu(tr("&Host"), this);
    connectAct = hostMenu->addAction(tr("Connect to host..."));
    disconnectAct = hostMenu->addAction(tr("Disconnect from host..."));
    pauseAct = hostMenu->addAction(tr("Pause host"));
    addHostAct = hostMenu->addAction(tr("Add host..."));
    editHostAct = hostMenu->addAction(tr("Edit host settings..."));
    menuBar->addMenu(hostMenu);

    // View menu
    viewMenu = new QMenu(tr("&View"), this);
    viewMessageAct = viewMenu->addAction(tr("Message flow"));
    viewCountAct = viewMenu->addAction(tr("Message count"));
    viewMenu->addSeparator();
    viewClrRxListAct = viewMenu->addAction(tr("Clear receive list..."));
    viewClrTxListAct = viewMenu->addAction(tr("Clear transmission list..."));
    menuBar->addMenu(viewMenu);

    // VSCP menu
    vscpMenu = new QMenu(tr("&vscp"), this);
    readRegAct = vscpMenu->addAction(tr("Read register..."));
    writeRegAct = vscpMenu->addAction(tr("Write register..."));
    readAllRegAct = vscpMenu->addAction(tr("Read (all) registers..."));
    readGuidAct = vscpMenu->addAction(tr("Read GUID"));
    readMdfAct = vscpMenu->addAction(tr("Read MDF..."));
    loadMdfAct = vscpMenu->addAction(tr("Download MDF..."));
    menuBar->addMenu(vscpMenu);

    // Settings menu
    settingsMenu = new QMenu(tr("&Setting"), this);
    setFilterAct = settingsMenu->addAction(tr("Set/define filter..."));
    settingsAct = settingsMenu->addAction(tr("Settings..."));
    menuBar->addMenu(settingsMenu);

    // Tools menu
    toolsMenu = new QMenu(tr("&Tools"), this);
    menuBar->addMenu(toolsMenu);

    connect(exitAct, &QAction::triggered, this, &QDialog::accept);

// #ifndef QT_NO_CLIPBOARD
//     cutAct->setEnabled(false);

//     copyAct->setEnabled(false);
//     connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
//     connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
// #endif // !QT_NO_CLIPBOARD
}

void CFrmSession::createHorizontalGroupBox()
{
    horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
    QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i < NumButtons; ++i) {
        buttons[i] = new QPushButton(tr("Button %1").arg(i + 1));
        layout->addWidget(buttons[i]);
    }
    horizontalGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createRxGridGroup
//

void CFrmSession::createGridGroupBox()
{
    gridGroupBox = new QGroupBox(tr("Receive Events"));
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(1,1,1,1);

    // for (int i = 0; i < NumGridRows; ++i) {
    //     labels[i] = new QLabel(tr("Line %1:").arg(i + 1));
    //     lineEdits[i] = new QLineEdit;
    //     layout->addWidget(labels[i], i + 1, 0);
    //     layout->addWidget(lineEdits[i], i + 1, 1);
    // }
    rxTable = new QTableWidget;
    layout->addWidget(rxTable, 0, 0, 1, 4); //  fromRow, fromColumn, rowSpan, columnSpan

    smallEditor = new QTextEdit;
    smallEditor->setPlainText(tr("This widget takes up about two thirds of the "
                                 "grid layout."));
    layout->addWidget(smallEditor, 0, 4, 1, 1);

    layout->setColumnStretch(0, 30);
    layout->setColumnStretch(1, 10);
    gridGroupBox->setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// createTxGridGroup
//

void CFrmSession::createTxGridGroup()
{
    txGroupBox = new QGroupBox(tr("Transmit"));
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(1,1,1,1);

    txTable =new QTableWidget;
    layout->addWidget(txTable, 0, 0, 1, 4); //  fromRow, fromColumn, rowSpan, columnSpan

    // Transmit/Add/Clone/Delete/Edit/Save/Load
    txToolBar = new QToolBar;
    txToolBar->setOrientation(Qt::Vertical);

    // Transmit
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *transmitAct = new QAction(newIcon, tr("&Transmit"), this);
    transmitAct->setShortcuts(QKeySequence::New);
    transmitAct->setStatusTip(tr("Transmit selected event(s)"));
    connect(transmitAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(transmitAct);

    // Add tx row
    const QIcon addIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *addAct = new QAction(addIcon, tr("&Add"), this);
    addAct->setShortcuts(QKeySequence::New);
    addAct->setStatusTip(tr("Add transmit event"));
    connect(addAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(addAct);

    // Edit tx row
    const QIcon editIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *editAct = new QAction(editIcon, tr("&Edit"), this);
    editAct->setShortcuts(QKeySequence::New);
    editAct->setStatusTip(tr("Edit selected event"));
    connect(editAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(editAct);

    // Delete tx row
    const QIcon deleteIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *deleteAct = new QAction(deleteIcon, tr("&Transmit"), this);
    deleteAct->setShortcuts(QKeySequence::New);
    deleteAct->setStatusTip(tr("Delete selected event"));
    connect(deleteAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(deleteAct);

    // Clone tx row
    const QIcon cloneIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *cloneAct = new QAction(cloneIcon, tr("&Clone"), this);
    cloneAct->setShortcuts(QKeySequence::New);
    cloneAct->setStatusTip(tr("Clone selected event"));
    connect(cloneAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(cloneAct);

    txToolBar->addSeparator();

    // Save tx rows
    const QIcon saveIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::New);
    saveAct->setStatusTip(tr("Save selected transmit event(s)"));
    connect(saveAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(saveAct);

    // Load tx rows
    const QIcon loadIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *loadAct = new QAction(loadIcon, tr("&Load"), this);
    loadAct->setShortcuts(QKeySequence::New);
    loadAct->setStatusTip(tr("Load transmit event(s)"));
    connect(loadAct, &QAction::triggered, this, &CFrmSession::transmitEvent);
    txToolBar->addAction(loadAct);

    layout->addWidget(txToolBar, 0, 4, 1, 1);

    layout->setColumnStretch(0, 70);
    layout->setColumnStretch(1, 10);
    txGroupBox->setLayout(layout);
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
    formGroupBox = new QGroupBox(tr("Form layout"));
    QFormLayout *layout = new QFormLayout;
    layout->addRow(new QLabel(tr("Line 1:")), new QLineEdit);
    layout->addRow(new QLabel(tr("Line 2, long text:")), new QComboBox);
    layout->addRow(new QLabel(tr("Line 3:")), new QSpinBox);
    formGroupBox->setLayout(layout);
}
