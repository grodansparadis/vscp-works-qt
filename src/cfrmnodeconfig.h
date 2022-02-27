// cfrmnodeconfig.cpp
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
#ifndef CFRMNODECONFIG_H
#define CFRMNODECONFIG_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <vscp.h>
#include <mdf.h>
#include <register.h>
#include <vscp_client_base.h>
#include "ctxevent.h"

#include <QObject>
#include <QDialog>
#include <QTableView>
#include <QTableWidgetItem>
#include <QtSql>
#include <QLCDNumber>
#include <QTreeWidget>

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
class QSpinBox;
class QComboBox;
class QTreeWidgetItem;
QT_END_NAMESPACE

#include <QMainWindow>
#include <QTableWidgetItem>

namespace Ui {
class CFrmNodeConfig;
}


enum registerColumns {
  REG_COL_POS = 0,
  REG_COL_ACCESS,
  REG_COL_VALUE,
  REG_COL_NAME
};

enum remotevarColumns {
  REMOTEVAR_COL_NAME = 0,
  REMOTEVAR_COL_ACCESS,
  REMOTEVAR_COL_TYPE,
  REMOTEVAR_COL_VALUE
};

enum dmColumns {
  DM_LEVEL1_COL_ORIGIN = 0,
  DM_LEVEL1_COL_FLAGS,
  DM_LEVEL1_COL_CLASS_MASK,
  DM_LEVEL1_COL_CLASS_FILTER,
  DM_LEVEL1_COL_TYPE_MASK,
  DM_LEVEL1_COL_TYPE_FILTER,
  DM_LEVEL1_COL_ACTION,
  DM_LEVEL1_COL_PARAMETER
};

enum filesColumns {
  FILE_COL_NAME = 0
};

#define TREE_LIST_REGISTER_TYPE (QTreeWidgetItem::UserType + 1)

// ----------------------------------------------------------------------------


/*!
    Class that represent a row in the TX list
*/
class CRegisterWidgetItem : public QTreeWidgetItem
{

 public:
    CRegisterWidgetItem(const QString& text);
    virtual ~CRegisterWidgetItem();

    /*!
      Pointer to MDF register row for this tree item
    */
    //CMDF_Register *m_pmdfreg;

    /*!
      Register page
    */
    uint16_t m_regPage;

    /*!
      Register offset
    */
    uint32_t m_regOffset;
};


// ----------------------------------------------------------------------------


/*!
    The session window
*/

class CFrmNodeConfig : public QMainWindow
{
  Q_OBJECT

 public:

    explicit CFrmNodeConfig(QWidget *parent = nullptr, QJsonObject* pconnObj = nullptr);
    virtual ~CFrmNodeConfig();

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

    /*!
      Let user select a GUID to use 
    */
    void selectGuid(void);  // std::string strguid = "-"

    /*!
      Disable/enable MDF colors
    */
    void disableColors(bool bColors);

    /*!
      Base combo chaged value
      Update data 
      @param index Index of the new value
    */
    void onBaseChange(int index);

    /*!
      A new interface as been selected
      @param index Index for new interface
    */
    void onInterfaceChange(int index);

    /*!
      A new nodeid as been selected
      @param nodeid New node id
    */
    void onNodeIdChange(int nodeid);

    /*!
      Double click on register line. This juste select the row for all columns except 
      the value column. Double clicking on this column edit's the value.
    */
    void onRegisterTreeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);

    /*!
      Register value changed. If use edits the register value and it is changed
      then this method is called.
    */
    void onRegisterTreeWidgetCellChanged(QTreeWidgetItem *item, int column);

    /*!
      Fill standard register data for already 
      loaded registers
    */
    void renderStandardRegisters(void);

    /*!
      Fill register data from already loaded registers
    */
    void renderRegisters(void);

    /*!     
      Update data
    */
    void update(void);

    /*!
      Update changed registers
    */
    void updateChanged();

    /*!
      Update all registers and read MDF again
    */
    void updateFull(void);

    /*!
      Read selected registers 
    */
    void readSelectedRegisterValues(void);

    /*!
      Write selected registers 
    */
    void writeSelectedRegisterValues(void);

    /*!
      Set selected registers to default value
    */
    void defaultSelectedRegisterValues(void);

    /*!
      Set all registers to default values
    */
    void defaultRegisterAll(void);

    /*!
      Undo selected register values
    */
    void undoSelectedRegisterValues(void);

    /*!
      Redo selected register values
    */
    void redoSelectedRegisterValues(void);

    /*!
      Show menu to select register page
    */
    void gotoRegisterPageMenu(void);

    /*!
      Save selected register values
    */
    void saveSelectedRegisterValues(void);

    /*!
      Save selected register values
    */
    void saveAllRegisterValues(void);

    /*!
      Save selected register values
    */
    void loadRegisterValues(void);

    /*!
      Fill HTML area with MDF information from
      the selected device
    */
    void fillDeviceHtmlInfo(void);

 signals:

    /// Data received from callback
    void dataReceived(vscpEvent* pev);

 private:

    /// MDF definitions
    CMDF m_mdf;

    /// VSCP standard registers
    CStandardRegisters m_stdregs;

    /// VSCP device user registers
    CUserRegisters m_userregs;

    /// Number of updates. Cleared after a full update
    uint32_t m_nUpdates;

    // Points to top item for standar registers
    QTreeWidgetItem *m_StandardRegTopPage;

    // Holds widget items for register page headers
    // register -> header
    std::map<uint32_t, QTreeWidgetItem *> m_mapRegTopPages;

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
    Ui::CFrmNodeConfig *ui;

    /// Combo box for numerical base
    QComboBox *m_baseComboBox;

    /// Text box for configuration GUID
    QLineEdit *m_guidConfig;

    /// Button to open GUID selections dialog
    QPushButton *m_btnSetGUID;

    /// Spin box for configuration nodeid
    QSpinBox *m_nodeidConfig;

    // Contains interface information
    QComboBox *m_comboInterface;   
    
};

#endif // CFrmNodeConfig_H
