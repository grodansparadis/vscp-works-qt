// mainwindow.h
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "connection_types.h"
#include "vscp_client_base.h"

#include <QMainWindow>
#include <QTreeWidgetItem>

#include <list>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
class QTreeWidget;
class QTreeWidgetItem;
class QModelIndex;
QT_END_NAMESPACE

class QTreeWidgetItemConn : public QTreeWidgetItem
{

public:
    QTreeWidgetItemConn(QTreeWidgetItem *topItem, CVscpClient *client);
    ~QTreeWidgetItemConn();

private:
    CVscpClient *m_client;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void loadConfiguration(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newConnection();
    void open();
    bool save();
    bool saveAs();
    void about();
    void connectionsWasModified();
#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif
    void newSession();

    void editConnectionItem(uint32_t connectionIndex);
    void cloneConnectionItem(uint32_t connectionIndex);
    void removeConnectionItem(uint32_t connectionIndex);

    // New connections
    void newLocalConnection();
    void newTcpipConnection();
    void newCanalConnection();
    void newSocketCanConnection();
    void newWs1Connection();
    void newWs2Connection();
    void newMqttConnection();
    void newUdpConnection();
    void newMulticastConnection();
    void newRestConnection();
    void newRawCanConnection();
    void newRawMqttConnection();

    void openConnectionSettingsDialog(CVscpClient::connType type);

    /*!
        Called when the connection list is double clicked
    */
    void onDoubleClicked(QTreeWidgetItem* item);

    

protected:

    /*
        Display a context meny when user right click on
        an item in the connection tree
        @param point Coordinates for user right clicked.
    */
    void showConnectionContextMenu(const QPoint&);
    
    /*!
        Add one child item to the connection tree
        @param topitem Pointer to top item this subitem belongs to
        @param name Name for sub item.
    */
    void addChildItemToConnectionTree(QTreeWidgetItem *topitem, std::string name);

    void addChildItemToConnectionTree(QTreeWidgetItem *topitem, CVscpClient *client);

private:

    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    //QPlainTextEdit *m_textEdit;
    //QTableWidget *m_connTable;
    QTreeWidget *m_connTreeTable;

    QString curFile;

    /// List with defined connections
    std::list<CVscpClient *> m_listConn;

    // Top items in tree control
    QTreeWidgetItem *m_topitem_local;
    QTreeWidgetItem *m_topitem_canal;
    QTreeWidgetItem *m_topitem_socketcan;
    QTreeWidgetItem *m_topitem_tcpip;
    QTreeWidgetItem *m_topitem_mqtt;
    QTreeWidgetItem *m_topitem_ws1;
    QTreeWidgetItem *m_topitem_ws2;
    QTreeWidgetItem *m_topitem_udp;
    QTreeWidgetItem *m_topitem_multicast;
    QTreeWidgetItem *m_topitem_rest;
    QTreeWidgetItem *m_topitem_rawcan;
    QTreeWidgetItem *m_topitem_rawmqtt;
};

#endif // MAINWINDOW_H
