// mainwindow.cpp
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


#include <QtWidgets>
#include <QMessageBox>
#include <QJSEngine>

//#include <QtSerialPort/QSerialPort>
//#include <QCanBus>


#include "version.h"
#include "cfrmsession.h"
#include "connection.h"
#include "cdlgnewconnection.h"
#include "cdlgconnsettingslocal.h"
#include "cdlgconnsettingscanal.h"
#include "cdlgconnsettingstcpip.h"
#include "cdlgconnsettingssocketcan.h"
#include "cdlgconnsettingsmqtt.h"
#include "cdlgconnsettingsws1.h"
#include "cdlgconnsettingsws2.h"
#include "cdlgconnsettingsrest.h"
#include "cdlgconnsettingsudp.h"
#include "cdlgconnsettingsmulticast.h"
#include "cdlgconnsettingsrawcan.h"
#include "cdlgconnsettingsrawmqtt.h"
#include "mainwindow.h"

///////////////////////////////////////////////////////////////////////////////
// MainWindow
//

MainWindow::MainWindow()
    : m_connTreeTable(new QTreeWidget)
{ 
    QCoreApplication::setOrganizationName("VSCP");
    QCoreApplication::setOrganizationDomain("vscp.org");
    QCoreApplication::setApplicationName("vscpworks+");

    // Enable custom context menu
    m_connTreeTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Open communication item on double click
    connect(m_connTreeTable, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onDoubleClicked );

    // Open pop up menu on right click
    connect(m_connTreeTable, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    

    // if (QCanBus::instance()->plugins().contains(QStringLiteral("socketcan"))) {
    //     // plugin available
    // }

    /*
    QJSEngine myEngine;
    QJSValue three = myEngine.evaluate("1 + 2 + Math.PI");

    QString str;
    QTextStream(&str) << tr("Value = ") << three.toInt();
    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.exec();
    */

    setCentralWidget(m_connTreeTable);  // table widget
    createActions();
    createStatusBar();

    readSettings();

    //connect(m_textEdit->document(), &QTextDocument::contentsChanged,
    //        this, &MainWindow::connectionsWasModified);

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);

    QStringList headers(QString(tr("Connection")).split(','));
    //m_connTreeTable->setColumnCount(2);
    //m_connTreeTable->setColumnWidth(0, 20);     // Type
    //m_connTreeTable->setColumnWidth(1, 200);    // Description
    //m_connTreeTable->horizontalHeader()->setStretchLastSection(true);
    //m_connTreeTable->setHorizontalHeaderLabels(headers);
    m_connTreeTable->setHeaderLabel(tr("--- Saved Connections ---"));

    // Add root items
    
    const QIcon iconConnections = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    const QIcon iconLocal = QIcon::fromTheme("network-offline", QIcon(":info.png"));
    
    //const QIcon iconTest = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    const QIcon iconTest(":process_accept.png");
    
    // Local
    QStringList strlist_local(QString(tr("Local Connections")).split(','));
    QTreeWidgetItem *topitem_local = new QTreeWidgetItem(strlist_local, LOCAL);
    topitem_local->setIcon(0,iconTest);
    topitem_local->setToolTip(0,tr("Holds local connections. Typically logfile and debug content containing VSCP events."));
    m_connTreeTable->addTopLevelItem(topitem_local);    

    // canal
    QStringList strlist_canal(QString(tr("CANAL Connections")).split(','));
    QTreeWidgetItem *topitem_canal = new QTreeWidgetItem(strlist_canal, CANAL);
    topitem_canal->setIcon(0,iconTest);
    topitem_canal->setToolTip(0,"Holds VSCP CANAL connections.");
    m_connTreeTable->addTopLevelItem(topitem_canal);

    // Socketcan
    QStringList strlist_socketcan(QString(tr("Socketcan Connections")).split(','));
    QTreeWidgetItem *topitem_socketcan = new QTreeWidgetItem(strlist_socketcan, SOCKETCAN);
    topitem_socketcan->setIcon(0,iconTest);
    topitem_socketcan->setToolTip(0,"Holds VSCP socketcan connections.");
    m_connTreeTable->addTopLevelItem(topitem_socketcan);

    // tcp/ip
    QStringList strlist_tcpip(QString(tr("TCP/IP Connections")).split(','));
    QTreeWidgetItem *topitem_tcpip = new QTreeWidgetItem(strlist_tcpip, TCPIP);
    topitem_tcpip->setIcon(0,iconTest);
    topitem_tcpip->setToolTip(0,"Holds VSCP tcp/ip connections.");
    m_connTreeTable->addTopLevelItem(topitem_tcpip);

    // MQTT
    QStringList strlist_mqtt(QString(tr("MQTT Connections")).split(','));
    QTreeWidgetItem *topitem_mqtt = new QTreeWidgetItem(strlist_mqtt, MQTT);
    topitem_mqtt->setIcon(0,iconTest);
    topitem_mqtt->setToolTip(0,"Holds VSCP MQTT connections.");
    m_connTreeTable->addTopLevelItem(topitem_mqtt);

    // WS2
    QStringList strlist_ws2(QString(tr("WS2 Connections")).split(','));
    QTreeWidgetItem *topitem_ws2 = new QTreeWidgetItem(strlist_ws2, WS2);
    topitem_ws2->setIcon(0,iconTest);
    topitem_ws2->setToolTip(0,"Holds VSCP websocket ws2 connections.");
    m_connTreeTable->addTopLevelItem(topitem_ws2);

    // WS1
    QStringList strlist_ws1(QString(tr("WS1 Connections")).split(','));
    QTreeWidgetItem *topitem_ws1 = new QTreeWidgetItem(strlist_ws1, WS1);
    topitem_ws1->setIcon(0,iconTest);
    topitem_ws1->setToolTip(0,"Holds VSCP websocket ws1 connections.");
    m_connTreeTable->addTopLevelItem(topitem_ws1);

    // UDP
    QStringList strlist_udp(QString(tr("UDP Connections")).split(','));
    QTreeWidgetItem *topitem_udp = new QTreeWidgetItem(strlist_udp, UDP);
    topitem_udp->setIcon(0,iconTest);
    topitem_udp->setToolTip(0,"Holds VSCP UDP connections.");
    m_connTreeTable->addTopLevelItem(topitem_udp);

    // Multicast
    QStringList strlist_multicast(QString(tr("Multicast Connections")).split(','));
    QTreeWidgetItem *topitem_multicast = new QTreeWidgetItem(strlist_multicast, MULTICAST);
    topitem_multicast->setIcon(0,iconTest);
    topitem_multicast->setToolTip(0,"Holds VSCP multicast connections.");
    m_connTreeTable->addTopLevelItem(topitem_multicast);

    // REST
    QStringList strlist_rest(QString(tr("REST Connections")).split(','));
    QTreeWidgetItem *topitem_rest = new QTreeWidgetItem(strlist_rest, REST);
    topitem_rest->setIcon(0,iconTest);
    topitem_rest->setToolTip(0,"Holds VSCP REST connections.");
    m_connTreeTable->addTopLevelItem(topitem_rest);

    // RAWCAN
    QStringList strlist_rawcan(QString(tr("RAWCAN Connections")).split(','));
    QTreeWidgetItem *topitem_rawcan = new QTreeWidgetItem(strlist_rawcan, RAWCAN);
    topitem_rawcan->setIcon(0,iconTest);
    topitem_rawcan->setToolTip(0,"Holds generic CAN connections.");
    m_connTreeTable->addTopLevelItem(topitem_rawcan);

    // RAWMQTT
    QStringList strlist_rawmqtt(QString(tr("RAWMQTT Connections")).split(','));
    QTreeWidgetItem *topitem_rawmqtt = new QTreeWidgetItem(strlist_rawmqtt, RAWMQTT);
    topitem_rawmqtt->setIcon(0,iconTest);
    topitem_rawmqtt->setToolTip(0,"Holds generic MQTT connections.");
    m_connTreeTable->addTopLevelItem(topitem_rawmqtt);

    // TEST
    QStringList strlist_test(QString(tr("Fluorine")).split(',')); 
    QTreeWidgetItem *topitem_test = new QTreeWidgetItem(topitem_tcpip,strlist_test);
    const QIcon iconFluorine = QIcon::fromTheme("network-transmit-receive", QIcon(":add.png"));
    topitem_test->setIcon(0,iconFluorine);
    topitem_test->setToolTip(0,"This is just a test connection from a snowy country named Sweden.");
    m_connTreeTable->addTopLevelItem(topitem_test);
}

///////////////////////////////////////////////////////////////////////////////
// closeEvent
//

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } 
    else {
        event->ignore();
    }
}


///////////////////////////////////////////////////////////////////////////////
// newConnection
//

void MainWindow::newConnection()
{
    if (maybeSave()) {
        //m_textEdit->clear();
        setCurrentFile(QString());
    }

    CDlgNewConnection dlg(this);

    if (QDialog::Accepted == dlg.exec()) {
         
        switch (dlg.getSelectedType()) { 
            
            case LOCAL:
                newLocalConnection();
                break;

            case TCPIP:
                newTcpipConnection();
                break;

            case CANAL: 
                newCanalConnection();
                break;

            case SOCKETCAN: 
                newSocketCanConnection();
                break;

            case WS1: 
                newWs1Connection();
                break;

            case WS2: 
                newWs2Connection();
                break;

            case MQTT: 
                newMqttConnection();
                break;

            case UDP: 
                newUdpConnection();
                break;

            case MULTICAST: 
                newMulticastConnection();
                break;

            case REST: 
                newRestConnection();
                break;

            case RAWCAN: 
                newRawCanConnection();
                break;

            case RAWMQTT: 
                newRawMqttConnection();
                break;
                                                    
            default:
                break;    
        }

    }
}

///////////////////////////////////////////////////////////////////////////////
// editConnectionItem
//

void MainWindow::editConnectionItem()
{
    QMessageBox::about(this, tr("Info"), tr("editConnectionItem") );
}

///////////////////////////////////////////////////////////////////////////////
// cloneConnectionItem
//

void MainWindow::cloneConnectionItem()
{
    QMessageBox::about(this, tr("Info"), tr("cloneConnectionItem") );
}

///////////////////////////////////////////////////////////////////////////////
// removeConnectionItem
//

void MainWindow::removeConnectionItem()
{
    QMessageBox::about(this, tr("Info"), tr("removeConnectionItem") );
}

///////////////////////////////////////////////////////////////////////////////
// onDoubleClicked
//

void MainWindow::onDoubleClicked(QTreeWidgetItem* item)
{
    QMessageBox msgBox;
    msgBox.setText("Double click");
    msgBox.exec();
    newTcpipConnection();
}

///////////////////////////////////////////////////////////////////////////////
// showContextMenu
//

void MainWindow::showContextMenu(const QPoint& pos)
{
    // Context Menu Creation
    QModelIndex selected = m_connTreeTable->indexAt(pos);
    QModelIndex parent = selected.parent();
    QTreeWidgetItem *item = m_connTreeTable->itemAt(pos);

    if (nullptr != item) {
        statusBar()->showMessage(item->text(0));
    }

    int row = selected.row();

    if (QModelIndex() == parent) return;

    QMenu *menu=new QMenu(this);
    menu->addAction(QString("Add new connection"), this, SLOT(newConnection()));
    menu->addAction(QString("Edit this connection"), this, SLOT(editConnectionItem()));
    menu->addAction(QString("Remove this connection"),this, SLOT(removeConnectionItem()));
    menu->addAction(QString("Clone this connection"),this, SLOT(cloneConnectionItem()));

    menu->popup(m_connTreeTable->viewport()->mapToGlobal(pos));

}

///////////////////////////////////////////////////////////////////////////////
// open
//

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadConfiguration(fileName);
    }
}

///////////////////////////////////////////////////////////////////////////////
// save
//

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

///////////////////////////////////////////////////////////////////////////////
// saveAs
//

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

///////////////////////////////////////////////////////////////////////////////
// about
//

void MainWindow::about()
{
   QString str;
   QTextStream(&str) << tr("<b>VSCP Works+</b> is the second generation toolbox for general VSCP configuration, " 
            "diagnostics and other VSCP related tasks.<br/>"
            "<p><b>Version</b>: ") <<
            tr(VSCPWORKS_DISPLAY_VERSION) <<
            "</p><p>" <<
            tr(VSCPWORKS_COPYRIGHT_HTML) <<
            "</p>";
   QMessageBox::about(this, tr("About VSCP Works+"), str );
}



///////////////////////////////////////////////////////////////////////////////
// connectionsWasModified
//

void MainWindow::connectionsWasModified()
{
    //setWindowModified(m_textEdit->document()->isModified());
}

///////////////////////////////////////////////////////////////////////////////
// createActions
//

void MainWindow::createActions()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New connection..."), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New connection"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newConnection);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);


    // const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    // QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    // openAct->setShortcuts(QKeySequence::Open);
    // openAct->setStatusTip(tr("Open an existing file"));
    // connect(openAct, &QAction::triggered, this, &MainWindow::open);
    // fileMenu->addAction(openAct);
    // fileToolBar->addAction(openAct);

    // const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    // QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    // saveAct->setShortcuts(QKeySequence::Save);
    // saveAct->setStatusTip(tr("Save the document to disk"));
    // connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    // fileMenu->addAction(saveAct);
    // fileToolBar->addAction(saveAct);

    // const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    // QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    // saveAsAct->setShortcuts(QKeySequence::SaveAs);
    // saveAsAct->setStatusTip(tr("Save the document under a new name"));

    fileMenu->addSeparator();

    const QIcon newSessionIcon = QIcon::fromTheme("emblem-symbolic-link");
    QAction *newSessionAct = new QAction(newSessionIcon, tr("&Session window..."), this);
    newSessionAct->setShortcuts(QKeySequence::SaveAs);
    newSessionAct->setStatusTip(tr("Open a new VSCP Session window"));
    connect(newSessionAct, &QAction::triggered, this, &MainWindow::newSession);
    fileMenu->addAction(newSessionAct);
    fileToolBar->addAction(newSessionAct);

    const QIcon newDevConfigIcon = QIcon::fromTheme("document-properties");
    QAction *newDevConfigAct = new QAction(newDevConfigIcon, tr("&Configuration..."), this);
    newDevConfigAct->setShortcuts(QKeySequence::SaveAs);
    newDevConfigAct->setStatusTip(tr("Open a new VSCP configuration window"));
    connect(newDevConfigAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(newDevConfigAct);
    fileToolBar->addAction(newDevConfigAct);    

    const QIcon scanForDeviceIcon = QIcon::fromTheme("edit-find"); 
    QAction *scanForDeviceAct = new QAction(scanForDeviceIcon, tr("Scan for &Device..."), this); 
    scanForDeviceAct->setShortcuts(QKeySequence::SaveAs);
    scanForDeviceAct->setStatusTip(tr("Open a new device scan window."));
    connect(scanForDeviceAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(scanForDeviceAct);
    fileToolBar->addAction(scanForDeviceAct);

    const QIcon bootloaderIcon = QIcon::fromTheme("emblem-downloads");
    QAction *bootloaderAct = new QAction(bootloaderIcon, tr("&Bootloader wizard..."), this);
    bootloaderAct->setShortcuts(QKeySequence::SaveAs);
    bootloaderAct->setStatusTip(tr("Open a new VSCP bootloader wizard."));
    connect(bootloaderAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(bootloaderAct);
    fileToolBar->addAction(bootloaderAct);

    const QIcon newMdfEditorIcon = QIcon::fromTheme("emblem-documents"); // applications-office
    QAction *newMdfEditorAct = new QAction(newMdfEditorIcon, tr("&MDF editor..."), this); 
    newMdfEditorAct->setShortcuts(QKeySequence::SaveAs);
    newMdfEditorAct->setStatusTip(tr("Open a new NDF editor"));
    connect(newMdfEditorAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(newMdfEditorAct);
    fileToolBar->addAction(newMdfEditorAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);

    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    //connect(cutAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    //connect(copyAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    //connect(pasteAct, &QAction::triggered, m_connTreeTable, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();

#endif // !QT_NO_CLIPBOARD

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);

    copyAct->setEnabled(false);
    //connect(m_textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    //connect(m_textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////////////////
// createStatusBar *
//

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

///////////////////////////////////////////////////////////////////////////////
// readSettings *
//

void MainWindow::readSettings()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    // QList <QScreen*> screens = QGuiApplication::screens();  // Multiscreen
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } 
    else {
        restoreGeometry(geometry);
    }

    int size = settings.beginReadArray("hosts/connections");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        std::string conn = settings.value("conn", "").toString().toStdString();
    }
    settings.endArray();

    //int margin = settings.value("editor/wrapMargin").toInt();
    //int margin = settings.value("editor/wrapMargin", 80).toInt();

    //std::string source { "Hello, World!" };
    //QVariant destination;        
    //destination = QString::fromStdString(source);

 
    std::string cfgfolder = settings.value("cfgfolder", "").toString().toStdString();

}

///////////////////////////////////////////////////////////////////////////////
// writeSettings *
//

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    //settings.setValue("editor/wrapMargin", 68);
    settings.setValue("editor/wrapMargin", "{ 'test': ['aaa','bbb']}");

    settings.beginWriteArray("hosts/connections");
    //for (int i = 0; i < m_connections.size(); ++i) {
    int i = 0;    
    for (std::list<connection *>::iterator it = m_listConnections.begin(); it != m_listConnections.end(); ++it){    
        settings.setArrayIndex(i);
        settings.setValue("conn", QString::fromStdString((*it)->getName()));
        //settings.setValue("conn", it->toJson);
    }
    settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
// maybeSave
//

bool MainWindow::maybeSave()
{
    // if (!m_textEdit->document()->isModified()) {
    //     return true;
    // }

    // const QMessageBox::StandardButton ret
    //     = QMessageBox::warning(this, tr("Application"),
    //                            tr("The document has been modified.\n"
    //                               "Do you want to save your changes?"),
    //                            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    // switch (ret) {
    //     case QMessageBox::Save:
    //         return save();
    //     case QMessageBox::Cancel:
    //         return false;
    //     default:
    //         break;
    // }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadConfiguration
//

void MainWindow::loadConfiguration(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    //m_textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

///////////////////////////////////////////////////////////////////////////////
// saveFile
//

bool MainWindow::saveFile(const QString &fileName)
{
    /*
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setCurrentFile
//

void MainWindow::setCurrentFile(const QString &fileName)
{
    // curFile = fileName;
    // m_textEdit->document()->setModified(false);
    // setWindowModified(false);

    // QString shownName = curFile;
    // if (curFile.isEmpty())
    //     shownName = "untitled.txt";
    // setWindowFilePath(shownName);
}

///////////////////////////////////////////////////////////////////////////////
// strippedName
//

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

///////////////////////////////////////////////////////////////////////////////
// commitData
//

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        // if (m_textEdit->document()->isModified()) {
        //     save();
        // }
    }
}

///////////////////////////////////////////////////////////////////////////////
// newSession
//

void MainWindow::newSession()
{
    CFrmSession *w = new CFrmSession(this);
    w->show();
}

///////////////////////////////////////////////////////////////////////////////
// newLocalConnection
//

void MainWindow::newLocalConnection()
{
    CDlgConnSettingsLocal dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newTcpipConnection
//

void MainWindow::newTcpipConnection()
{
    CDlgConnSettingsTcpip dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    } 
}

///////////////////////////////////////////////////////////////////////////////
// newCanalConnection
//

void MainWindow::newCanalConnection()
{
    CDlgConnSettingsCanal dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    } 
}

///////////////////////////////////////////////////////////////////////////////
// newSocketCanConnection
//

void MainWindow::newSocketCanConnection()
{
    CDlgConnSettingsSocketCan dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newMqttConnection
//

void MainWindow::newMqttConnection()
{
    CDlgConnSettingsMqtt dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newWs1Connection
//

void MainWindow::newWs1Connection()
{
    CDlgConnSettingsWs1 dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newWs2Connection
//

void MainWindow::newWs2Connection()
{
    CDlgConnSettingsWs2 dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}



///////////////////////////////////////////////////////////////////////////////
// newUdpConnection
//

void MainWindow::newUdpConnection()
{
    CDlgConnSettingsUdp dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newMulticastConnection
//

void MainWindow::newMulticastConnection()
{
    CDlgConnSettingsMulticast dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newRestConnection
//

void MainWindow::newRestConnection()
{
    CDlgConnSettingsRest dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newRawCanConnection
//

void MainWindow::newRawCanConnection()
{
    CDlgConnSettingsRawCan dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

///////////////////////////////////////////////////////////////////////////////
// newRawMqttConnection
//

void MainWindow::newRawMqttConnection()
{
    CDlgConnSettingsRawMqtt dlg(this);

    if (QDialog::Accepted == dlg.exec()) {

    }
}

#endif


