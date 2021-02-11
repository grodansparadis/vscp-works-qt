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

#include "vscp_client_base.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QJsonObject>

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

class FileDownloader;

///////////////////////////////////////////////////////////////////////////////
// treeWidgetItemConn
//

class treeWidgetItemConn : public QTreeWidgetItem
{

public:
    /*!
        Create connection tree list item

        @param topItem Pointer to tree list top item
        @param client Pointer to communication client
    */
    treeWidgetItemConn(QTreeWidgetItem *topItem, 
                            QJsonObject *conf);
    ~treeWidgetItemConn();

    /// Getter/Setter for configuration object
    QJsonObject *getConfObject() { return m_pconf; };
    void setConfObject(QJsonObject *conf) { m_pconf = conf; };

private:

    /*!
        JSON configuration for the
        communication client.
    */
    QJsonObject *m_pconf;
    
};



///////////////////////////////////////////////////////////////////////////////
// MainWindow
//



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void loadConfiguration(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void initRemoteEventDbFetch();
    void initForcedRemoteEventDbFetch();

private slots:
    void checkRemoteEventDbVersion();
    void downloadedEventDb();
    void newConnection();
    void open();
    bool save();
    bool saveAs();
    void about();
    void connectionsWasModified();
#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif
    void newSession(void);

    void showMainsettings(void);

    void editConnectionItem(void);
    void cloneConnectionItem(void);
    void removeConnectionItem(void);

    // New connections
    void newLocalConnection(void);
    void newTcpipConnection(void);
    void newCanalConnection(void);
    void newSocketCanConnection(void);
    void newWs1Connection(void);
    void newWs2Connection(void);
    void newMqttConnection(void);
    void newUdpConnection(void);
    void newMulticastConnection(void);
    void newRestConnection(void);
    void newRawCanConnection(void);
    void newRawMqttConnection(void);

    void openConnectionSettingsDialog(CVscpClient::connType type);

    /*!
        Called when the connection list is double clicked
    */
    void onDoubleClicked(QTreeWidgetItem* item);
    

protected:        

    /*
        Display a context menu when user right click on
        an item in the connection tree
        @param point Coordinates for user right clicked.
    */
    void showConnectionContextMenu(const QPoint&);
    
    /*!
        Create connection tree list item

        @param topItem Pointer to tree list top item
        @param client Pointer to communication client
    */
    void addChildItemToConnectionTree(QTreeWidgetItem *topitem,  
                                        QJsonObject *pconn);

    /*!
        Fill in loaded connections to the tree
        at correct positions
    */
    void addLoadedConnections(void);                                        

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
