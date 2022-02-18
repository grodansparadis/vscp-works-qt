// cfrmnodescan.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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
#ifndef CFRMNODESCAN_H
#define CFRMNODESCAN_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <vscp.h>
#include <vscp_client_base.h>

#include <QObject>
#include <QDialog>
#include <QTableView>
#include <QTableWidgetItem>
#include <QtSql>
#include <QLCDNumber>

QT_BEGIN_NAMESPACE
class QAction;
class QIcon;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QTextBrowser;
class QToolBar;
class QVBoxLayout;
class QAction;
class QTableWidgetItem;
class QTableWidget;
class QToolBox;
QT_END_NAMESPACE

#include <QMainWindow>
#include <QTableWidgetItem>

namespace Ui {
class CFrmNodeScan;
}


// ----------------------------------------------------------------------------


/*!
    The session window
*/

class CFrmNodeScan : public QMainWindow
{
  Q_OBJECT

 public:

    explicit CFrmNodeScan(QWidget *parent = nullptr, QJsonObject* pconnObj = nullptr);
    virtual ~CFrmNodeScan();

    /*!
        set Initial focus
    */
    void setInitialFocus(void);

    /*!
        This method is a middle man between the communication
        callback and the session class. It add a VSCP event to the rc table
        and to the receive event table.
        @param pev Pointer to received event
    */
    void threadReceive(vscpEvent* pev);

    /*!
        Connect to remote host
    */
    void doConnectToRemoteHost(void);

    /*!
        Disconnect from remote host and update UI to 
        indicate this
    */
    void doDisconnectFromRemoteHost(void);
    
 public slots:
    
    /// Dialog return
    void done(int r); 

    /*! 
        Add a RX event to the receive list
        @param ev Event to add
        @param bReceive Set to true if this is a received event
    */
    void 
    receiveRxRow(vscpEvent* pev);

    /*!
        Connect to remote host and update UI to 
        indicate the result of the operation.
        @param checked false if not connected. True if connected.
    */
    void 
    connectToRemoteHost(bool checked);

    /*!
        Show context menu for rx table
        @param pos Position where right click took place
    */
    void 
    showRegisterContextMenu(const QPoint& pos);

    /// Open settings dialog
    void 
    menu_open_main_settings(void);
 
 signals:

    /// Data received from callback
    void dataReceived(vscpEvent* pev);

 private:

    /// The VSCP client type
    CVscpClient::connType m_vscpConnType;

    /// Configuration data for the session
    QJsonObject m_connObject;

    /// A pointer to a VSCP Client 
    CVscpClient *m_vscpClient;

    /// List for received events
    QTableWidget *m_rxTable;

    /// Mutex that protect the rx -lists
    QMutex m_mutexRxList;

    /// Queue that holds received events
    std::deque<vscpEvent *> m_rxEvents;

    // The UI definition
    Ui::CFrmNodeScan *ui;

};

#endif // CFrmNodeScan_H
