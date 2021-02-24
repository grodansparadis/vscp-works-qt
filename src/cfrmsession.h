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
#ifndef CFRMSESSION_H
#define CFRMSESSION_H

#include <vscp.h>

#include <vscp_client_base.h>

#include <QObject>
#include <QDialog>
#include <QTableView>
#include <QtSql>

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
class QTableWidget;
class QToolBox;
QT_END_NAMESPACE



class CVscpClientCallback : public QObject
{
    Q_OBJECT

public:
    CVscpClientCallback() { m_value = 0; }

    int value() const { return m_value; }

public slots:
    void eventReceived(vscpEvent *pev);

signals:
    void addRow(vscpEvent *pev, bool bReceive);

private:
    int m_value;
};


class CFrmSession : public QDialog
{
    Q_OBJECT

 public:
    CFrmSession(QWidget *parent);
    ~CFrmSession();

    // VSCP Class display format
    // symbolic          - Just symbolic name
    // numerical_in_base - VSCP class code in selected base
    // numerical_hex_dex - VSCP class code in hex/dec
    // Symbolic_hex_dec  - Symbolic name + VSCP class code 
    enum class classDisplayFormat {symbolic, numerical_in_base, numerical_hex_dec, symbolic_hex_dec};

    // VSCP Type display format
    // symbolic          - Just symbolic name
    // numerical_in_base - VSCP type code in selected base
    // numerical_hex_dex - VSCP type code in hex/dec
    // Symbolic_hex_dec  - Symbolic name + VSCP type code 
    enum class tyypeDisplayFormat {symbolic, numerical_in_base, numerical_hex_dec, symbolic_hex_dec};

    // VSCP GUID display format
    // guid              - GUID
    // symbolic          - Symbolic code if possible
    // symbolic_guid     - Symbolic + GUID
    // guid_symbolic     - GUID + symbolic
    enum class guidDisplayFormat {guid, symbolic, symbolic_guid, guid_symbolic};

    

 public slots:

    /*! 
        Add an event to the receive list
        @param ev Event to add
        @param bReceive Set to true if this is a received event
    */
    void addRow(const vscpEvent& ev, bool bReceive);

 private:
    void createMenu();
    void createHorizontalGroupBox();
    void createRxGroupBox();
    void createTxGridGroup();
    void createFormGroupBox();

    void menu_connect();
    void menu_filter();

    void transmitEvent();

    enum { NumGridRows = 8, NumButtons = 4 };

    /// The VSCP client type
    CVscpClient::connType m_vscpConnType;

    /// A pointer to a VSCP Client 
    CVscpClient *m_vscpClient;

    /// Event database
    //QSqlDatabase m_evdb;
    //QSqlTableModel *m_rxmodel;

    QMenuBar *m_menuBar;
    
    /// Toolbar
    QToolBar *m_toolBar;
    
    QGroupBox *m_horizontalGroupBox;
    QGroupBox *m_gridGroupBox;
    QGroupBox *m_txGroupBox;
    QGroupBox *m_formGroupBox;
    QTextBrowser *m_infoArea;
    QTextBrowser *m_bigEditor;
    QLabel *m_labels[NumGridRows];
    QLineEdit *m_lineEdits[NumGridRows];
    QPushButton *m_buttons[NumButtons];
    QDialogButtonBox *m_buttonBox;

    /// Toolbar combo for actiive filter
    QComboBox *m_filterComboBox;

    /// Toolbar combo for numerical base
    QComboBox *m_baseComboBox;      

    QTableWidget *m_rxTable;
    //QTableView   *m_rxTable;
    QTableWidget *m_txTable;

    QMenu *m_fileMenu;
    QMenu *m_hostMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_vscpMenu;
    QMenu *m_settingsMenu;
    QMenu *m_toolsMenu;
  
    QToolBar *m_editToolBar;

    // File menu
    QAction *m_loadEventsAct;
    QAction *m_saveEventsAct;
    QAction *m_loadTxAct;
    QAction *m_saveTxAct;
    QAction *m_exitAct;

    // Host meny
    QAction *m_connectAct;
    QAction *m_disconnectAct;
    QAction *m_pauseAct;
    QAction *m_addHostAct;
    QAction *m_editHostAct;

    // Edit menu
    QAction *m_cutAct;
    QAction *m_copyAct;
    QAction *m_pasteBeforeAct;
    QAction *m_pasteAfterAct;

    // View menu
    QAction *m_viewMessageAct;
    QAction *m_viewCountAct;
    QAction *m_viewClrRxListAct;
    QAction *m_viewClrTxListAct;

    // VSCP menu
    QAction *m_readRegAct;
    QAction *m_writeRegAct;
    QAction *m_readAllRegAct;
    QAction *m_readGuidAct;
    QAction *m_readMdfAct;
    QAction *m_loadMdfAct;

    // Settings menu
    QAction *m_setFilterAct;
    QAction *m_settingsAct;

    QToolBar *m_txToolBar;
};

#endif // CFRMSESSION_H
