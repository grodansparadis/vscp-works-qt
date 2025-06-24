// cfrmsession.cpp
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
#ifndef CFRMSESSION_H
#define CFRMSESSION_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <vscp.h>

#include <vscp-client-base.h>

#include "ctxevent.h"

#include <QDialog>
#include <QLCDNumber>
#include <QObject>
#include <QTableView>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QComboBox>
#include <QMutex>

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

// class CVscpClientCallback : public QObject
// {
//     Q_OBJECT

// public:
//     CVscpClientCallback() { m_value = 0; }

//     int value() const { return m_value; }

// public slots:
//     void eventReceived(vscpEvent *pev);

// signals:
//     void addRow(vscpEvent *pev, bool bReceive);

// private:
//     int m_value;
// };

// ----------------------------------------------------------------------------

/*!
    Class that represent a row in the TX list
*/
class CTxWidgetItem : public QTableWidgetItem {

public:
  CTxWidgetItem(const QString& text);
  virtual ~CTxWidgetItem();

  CTxEvent m_tx;
};

// ----------------------------------------------------------------------------

/*!
    The session window
*/

class CFrmSession : public QDialog {
  Q_OBJECT

public:
  CFrmSession(QWidget* parent, json* pconn);
  virtual ~CFrmSession();

  // Column positions in RX table
  const int rxrow_dir    = 0;
  const int rxrow_class  = 1;
  const int rxrow_type   = 2;
  const int rxrow_nodeid = 3;
  const int rxrow_guid   = 4;

  // Column positions in TX table
  const int txrow_enable = 0;
  const int txrow_name   = 1;
  const int txrow_period = 2;
  const int txrow_count  = 3;
  const int txrow_event  = 4;

  const int rxrow_role_flags = Qt::UserRole;

  const int txrow_role_flags = Qt::UserRole;

  const uint32_t RX_ROW_FLAG_RX      = 0x00000000; // Receive row
  const uint32_t RX_ROW_FLAG_TX      = 0x00000001; // Transmit row
  const uint32_t RX_ROW_MARKED       = 0x00000002; // Marked row
  const uint32_t RX_ROW_MARKED_CLASS = 0x00000004; // Transmit row
  const uint32_t RX_ROW_MARKED_TYPE  = 0x00000008; // Transmit row

  // VSCP Class display format
  // symbolic          - Just symbolic name
  // numerical_in_base - VSCP class code in selected base
  // numerical_hex_dex - VSCP class code in hex/dec
  // Symbolic_hex_dec  - Symbolic name + VSCP class code
  enum class classDisplayFormat { symbolic = 0,
                                  numerical_in_base,
                                  numerical_hex_dec,
                                  symbolic_hex_dec };

  // VSCP Type display format
  // symbolic          - Just symbolic name
  // numerical_in_base - VSCP type code in selected base
  // numerical_hex_dex - VSCP type code in hex/dec
  // Symbolic_hex_dec  - Symbolic name + VSCP type code
  enum class typeDisplayFormat { symbolic = 0,
                                 numerical_in_base,
                                 numerical_hex_dec,
                                 symbolic_hex_dec };

  // VSCP GUID display format
  // guid              - GUID
  // symbolic          - Symbolic code if possible
  // symbolic_guid     - Symbolic + GUID
  // guid_symbolic     - GUID + symbolic
  enum class guidDisplayFormat { guid = 0,
                                 symbolic,
                                 symbolic_guid,
                                 guid_symbolic };

  // void close(void);

  /*!
      This method is a middle man between the communication
      callback and the session class. It add a VSCP event to the rc table
      and to the receive event table.
      @param pev Pointer to received event
  */
  //void threadReceive(vscpEvent* pev);

  /*!
    This is the callback used by client thread to deliver events
    @param ev Reference to VSCP event
    @param pobj Pointer to object CFrmSession)
  */
  void receiveCallback(vscpEvent& ev, void *pobj);

  /*!
      Connect to remote host
  */
  void doConnectToRemoteHost(void);

  /*!
      Disconnect from remote host and update UI to
      indicate this
  */
  void doDisconnectFromRemoteHost(void);

  /*!
      Fill Event receive count.
      This data is field if no selection is active.
  */
  void fillReceiveEventCount(void);

  /*!
      Fill difference information between several selected
      receive event rows
  */
  void fillReceiveEventDiff(void);

  /*!
      Fill in info in RX status window
      @param pev Event to fill in info for.
  */
  void fillRxStatusInfo(int selectedRow);

  /*!
      Get class info string as of settings
      @param pev VSCP Event
      @param Formatted string for VSCVP class.
  */
  QString getClassInfo(const vscpEvent* pev);

  /*!
      Update VSCP class info for row
      @param item Pointer to QTableWidgetItem for row
      @param pev Pointer to event for which information should be filled in
  */
  void setClassInfoForRow(QTableWidgetItem* item, const vscpEvent* pev);

  /*!
      Get type info string as of settings
      @param pev VSCP Event
      @param Formatted string for VSCVP class.
  */
  QString getTypeInfo(const vscpEvent* pev);

  /*!
      Update VSCP type info for row
      @param item Pointer to QTableWidgetItem for row
      @param pev Pointer to event for which information should be filled in
  */
  void setTypeInfoForRow(QTableWidgetItem* item, const vscpEvent* pev);

  /*!
      Update node id info for row
      @param item Pointer to QTableWidgetItem for row
      @param pev Pointer to event for which information should be filled in
  */
  void setNodeIdInfoForRow(QTableWidgetItem* item, const vscpEvent* pev);

  /*!
      Update GUID info for row
      @param item Pointer to QTableWidgetItem for row
      @param pev Pointer to event for which information should be filled in
  */
  void setGuidInfoForRow(QTableWidgetItem* item, const vscpEvent* pev);

  /*!
      Add a new row to the TX list
      @param bEnable Boolean that set the item active or inactive
      @param name Name for the TX row
      @param count Number of events to send on activation
      @param period Period in milliseconds for active event
      @param event VSCP Event on stgring form that is sent on activation
      @return True on Success. False on failure
  */
  bool addTxRow(bool bEnable,
                const QString& name,
                uint16_t count,
                uint32_t period,
                const QString& event);

  /*!
    Return true if we are connected
    @return true if connected, false otherwise
  */
  bool isConnected(void);                

public slots:

  /*!
      Add a RX event to the receive list
      @param ev Event to add
      @param bReceive Set to true if this is a received event
  */
  void receiveRxRow(vscpEvent* pev);

  /*!
    Show help
  */
  void showHelp(void);

  /*!
      Add a TX event to the receive list
      @param ev Event to add
      @param bReceive Set to true if this is a received event
  */
  void receiveTxRow(vscpEvent* pev);

  /*!
      Update the current row info.
  */
  void
  updateCurrentRow(void);

  /*!
      Update row info for all rows.
  */
  void
  updateAllRows(void);

  /*!
      Connect to remote host and update UI to
      indicate the result of the operation.
      @param checked false if not connected. True if connected.
  */
  void connectToRemoteHost(bool checked);

  /*!
      Selections has changed.
      We display info about selected item if one item is selected
      and differential info if one then more item is selected.
      - Timing between events
      - Measurement difference if two or more measurements are selected with
        same class/type/unint.
      @param selected Items has been selected
      @param deselected Items that has been selected
  */
  void rxSelectionChange(const QItemSelection&, const QItemSelection&);

  /*!
      Show context menu for rx table
      @param pos Position where right click took place
  */
  void showRxContextMenu(const QPoint& pos);

  /*!
      Cell in rx table has been clicked
      @param row Row that has been clicked
      @param column Column that has been clicked.
  */
  void rxCellClicked(int row, int column);

  /*!
      Item changed in TX table
      @param item Pointer to Changed table item
  */
  void txItemChanged(QTableWidgetItem* item);

  /*!
      Item changed in TX table
      @param row Doubleclicked row
      @param column Doubleclicked column
  */
  void txRowDoubleClicked(int row, int column);

  /*!
      Show context menu for tx table
      @param pos Position where right click took place
  */
  void showTxContextMenu(const QPoint& pos);

  // RX Context

  /// Clear RX list
  void menu_clear_rxlist();

  /// Unselect all RX list
  void menu_unselect_all_rxlist();

  /// Open settings dialog
  void menu_open_main_settings(void);

  /// Clear all selections
  void clrAllRxSelections(void);

  /// Define a GUID for selected row
  void setGuid(void);

  /// Add a note for selected row(s)
  void addEventNote(void);

  /// Remove note on selected row(s)
  void removeEventNote(void);

  /// Add mark to selected row
  void setVscpRowMark(void);

  /// Remove mark for selected row
  void unsetVscpRowMark(void);

  /// Add mark to selected row(s) class
  void setVscpClassMark(void);

  /// Remove mark from selected row(s) class
  void unsetVscpClassMark(void);

  /// Add mark to selected row(s) type
  void setVscpTypeMark(void);

  /// Remove mark from selected row(s) type
  void unsetVscpTypeMark(void);

  /// Save Marked events to file
  void saveMarkRxToFile(void);

  /// Save RX table to file (selected parts or not)
  void saveRxToFile(void);

  /// Load RX data from file
  void loadRxFromFile(void);

  /// Copy RX event to clipboard
  void copyRxToClipboard(void);

  /// Copy RX event to TX
  void copyRxToTx(void);

  // TX Context

  /// Clear the TX list
  void menu_clear_txlist();

  /// Send selected TX event
  void sendTxEvent(void);

  /// Add new Tx event
  void addTxEvent(void);

  /// Edit Tx event
  void editTxEvent(void);

  /// Delete Tx event
  void deleteTxEvent(void);

  /// clone Tx event
  void cloneTxEvent(void);

  /// Clear TX selections
  void clrSelectionsTxEvent(void);

  /// Load Tx events
  void loadTxEvents(const QString& path = "");
  void loadTxEventsAct(void) { loadTxEvents(); };

  /// Load TX events on start
  void loadTxOnStart(void);

  /// Save Tx events
  void saveTxEvents(const QString& path = "", bool bSelected = true);
  void saveTxEventsAct(void) { saveTxEvents(); };
  void saveTxEventsAllAct(void) { saveTxEvents("",false); };

  /// Save TX events on save
  void saveTxOnExit(void);

  /// Copy RX event to clipboard
  void copyTxToClipboard(void);

  /// Connect to host
  void menu_connect(void);

  /// Disconnect to host
  //void menu_disconnect(void);

  /// Open settings dialog for connection
  void openConnectionSettings(void);

  /// Open dialog to let user select active subscribe topics
  void openMqttSubscribeTopics(void);

  /// Open dialog to let user select active publish topics
  void openMqttPublishTopics(void);

  /// Open dialog to let user select retain publish topics to clear
  void openClearMqttRetainPublishTopics(void);

signals:

  /// Data received from callback
  void dataReceived(vscpEvent* pev);

  

private:
  void createMenu();
  void createToolbar();
  void createHorizontalGroupBox();
  void createRxGroupBox();
  void createTxGridGroup();
  void createFormGroupBox();

  // Toolbar

  

  /// Configure receive filter
  void menu_filter_config();

  /// Enable&disable receive filter
  void menu_filter_enable();

  enum { NumGridRows = 8,
         NumButtons  = 4 };

  /// The VSCP client type
  CVscpClient::connType m_vscpConnType;

  /// Configuration data for the session
  json m_connObject;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;

  QMenuBar* m_menuBar;

  /// Toolbar
  QToolBar* m_toolBar;

  QGroupBox* m_horizontalGroupBox;
  QGroupBox* m_gridGroupBox;
  QGroupBox* m_txGroupBox;
  QGroupBox* m_formGroupBox;
  QTextBrowser* m_infoArea;

  QLabel* m_labels[NumGridRows];
  QLineEdit* m_lineEdits[NumGridRows];
  QPushButton* m_buttons[NumButtons];
  QDialogButtonBox* m_buttonBox;

  /// Toolbar combo for actiive filter
  QComboBox* m_filterComboBox;

  /// Toolbar combo for numerical base
  QComboBox* m_baseComboBox;

  /// Count of received events
  QLCDNumber* m_lcdNumber;

  /// Clear RX list button
  QAction* m_setClearRcvListActToolBar;

  /// Unselect all RX items
  QAction* m_setUnselectAllActToolBar;

  /// List for received events
  QTableWidget* m_rxTable;

  // Protect callback from multiple threads
  QMutex m_mutexReceiveCallBack;  

  /// Mutex that protect the rx -lists
  QMutex m_mutexRxList;

  /// Queue that holds received events
  std::deque<vscpEvent*> m_rxEvents;

  /// VSCP (class-id + token-id) -> received count
  std::map<uint32_t, uint32_t> m_mapRxEventToCount;

  /// VSCP (class-id + token-id) -> received count
  std::map<uint32_t, uint32_t> m_mapTxEventToCount;

  /// row -> comment
  std::map<int, QString> m_mapRxEventComment;

  /// row -> row-flags
  std::map<int, uint32_t> m_mapRxEventFlags;

  /// Mutex that protect the TX list
  QMutex m_mutexTxList;

  /// List for transmittable events
  QTableWidget* m_txTable;

  QMenu* m_fileMenu;
  QMenu* m_connMenu;
  QMenu* m_editMenu;
  QMenu* m_viewMenu;
  QMenu* m_vscpMenu;
  QMenu *m_mqttMenu;
  QMenu* m_settingsMenu;
  QMenu* m_toolsMenu;

  QToolBar* m_editToolBar;

  // File menu
  QAction* m_loadEventsAct;
  QAction* m_saveEventsAct;
  QAction* m_loadTxAct;
  QAction* m_saveTxAct;
  QAction* m_saveTxAllAct;
  QAction* m_exitAct;

  // Host meny
  QAction* m_connectAct;
  QAction* m_disconnectAct;
  //QAction* m_pauseConnAct; // Not used
  //QAction* m_addConnAct;   // Not used
  //QAction* m_editConnAct;
  QAction* m_mqttSubscriptions; // Only if connection is MQTT
  QAction* m_mqttPublishTopics; // Only if connection is MQTT

  // Edit menu
  QAction* m_copyRxAct;
  QAction* m_copyTxAct;
  QAction* m_copyRxToTxAct;
  QAction* m_clrRxSelectionsAct;
  QAction* m_clrRxListAct;
  QAction* m_clrTxListAct;
  QAction* m_toggleRxRowMarkAct;
  QAction* m_toggleRxClassMarkAct;
  QAction* m_toggleRxTypeMarkAct;
  QAction* m_addExCommentAct;
  QAction* m_deleteRxCommentAct;

  // VSCP menu
  QAction* m_readRegAct;
  QAction* m_writeRegAct;
  QAction* m_readAllRegAct;
  QAction* m_readGuidAct;
  QAction* m_readMdfAct;
  QAction* m_loadMdfAct;

  // Settings menu
  QAction* m_setFilterAct;
  QAction* m_settingsAct;

  QToolBar* m_txToolBar;

  QToolButton* m_connect;

  // Toolbar actions
  QAction* m_connectActBar;
  //QAction* m_connectActToolBar;
  QAction* m_setFilterActToolBar;
};

#endif // CFRMSESSION_H
