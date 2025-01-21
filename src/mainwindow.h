// mainwindow.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "vscp-client-base.h"

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

// types for main treelist items
enum class itemType {
    NONE=0, 
    CONNECTION=1000
};


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
                            const json& conn);
    ~treeWidgetItemConn();

    /// Getter/Setter for configuration object
    json *getJson() { return &m_conn; };
    void setJson(json& conn) { m_conn = conn; };

private:

    /*!
        JSON configuration for the
        communication client.
    */
    json m_conn;
    
};


///////////////////////////////////////////////////////////////////////////////
// MainWindow
//



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

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
    void newSession();
    void newNodeConfiguration();
    void newNodeScan();
    void newNodeBootload();
    void mdfEdit();

    /// Handle app. settings.
    void showMainsettings(void);

    /// Handle known GUID's
    void knownGuids(void);

    /// Handle session filters
    void sessionFilter(void);

    void editConnectionItem(void);
    void cloneConnectionItem(void);
    void removeConnectionItem(void);

    // New connections
    void newTcpipConnection(void);
    void newCanalConnection(void);
    void newSocketCanConnection(void);
    void newWs1Connection(void);
    void newWs2Connection(void);
    void newMqttConnection(void);
    void newUdpConnection(void);
    void newMulticastConnection(void);

    // Edit connections
    void editTcpipConnection(treeWidgetItemConn *itemConn);
    void editCanalConnection(treeWidgetItemConn *itemConn);
    void editSocketCanConnection(treeWidgetItemConn *itemConn);
    void editWs1Connection(treeWidgetItemConn *itemConn);
    void editWs2Connection(treeWidgetItemConn *itemConn);
    void editMqttConnection(treeWidgetItemConn *itemConn);
    void editUdpConnection(treeWidgetItemConn *itemConn);
    void editMulticastConnection(treeWidgetItemConn *itemConn);

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
                                        const json& conn);

    /*!
        Fill in loaded connections to the tree
        at correct positions
    */
    void addLoadedConnections(void);          

private:

  void createActions();
  void createStatusBar();
  void readSettings();
  bool maybeSave();
  bool saveFile(const QString &fileName);
  void setCurrentFile(const QString &fileName);
  QString strippedName(const QString &fullFileName);

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
