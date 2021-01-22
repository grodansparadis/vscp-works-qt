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

#include "connection_types.h"
#include <QMainWindow>

#include <list>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
//class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QModelIndex;
QT_END_NAMESPACE

class connection;

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

    void openConnectionSettingsDialog(connection_type type);

    /*!
        Called when the connection list is double clicked
    */
    void onDoubleClicked(QTreeWidgetItem* item);

    

protected:
    void showConnectionContextMenu(const QPoint&);
    

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
    std::list<connection *> m_listConnections;
};

#endif // MAINWINDOW_H
