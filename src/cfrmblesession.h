// cfrmblesession.h
//
// Bluetooth LE session window: receives VSCP events from Bluetooth LE
// advertising (E1, A2, F1, L0) and talks to a node over the G1 GATT
// service. Uses the vscp-ble library for all frame handling.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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

#ifndef CFRMBLESESSION_H
#define CFRMBLESESSION_H

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QDialog>
#include <QElapsedTimer>
#include <QHash>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QPointer>
#include <QTimer>

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "vscp-ble-a2.h"
#include "vscp-ble-f1.h"
#include "vscp-ble-g1.h"

class QAction;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

class CFrmBleSession : public QDialog
{
  Q_OBJECT

public:
  explicit CFrmBleSession(QWidget* parent, json* pconn);
  ~CFrmBleSession();

private slots:
  // Scanning
  void toggleScan(void);
  void deviceSeen(const QBluetoothDeviceInfo& info);
  void deviceUpdated(const QBluetoothDeviceInfo& info, QBluetoothDeviceInfo::Fields fields);
  void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
  void scanFinished(void);

  // G1
  void connectG1(void);
  void disconnectG1(void);
  void pairDevice(void);
  void g1Connected(void);
  void g1Disconnected(void);
  void g1DiscoveryFinished(void);
  void g1ControllerError(QLowEnergyController::Error error);
  void g1ServiceStateChanged(QLowEnergyService::ServiceState state);
  void g1CharacteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value);
  void g1CharacteristicRead(const QLowEnergyCharacteristic& c, const QByteArray& value);
  void g1DescriptorWritten(const QLowEnergyDescriptor& d, const QByteArray& value);
  void g1ServiceError(QLowEnergyService::ServiceError error);
  void sendEvent(void);
  void g1Tick(void);

  void clearEvents(void);
  void showHelp(void);

private:
  enum class Trust
  {
    Public,        ///< E1, F1, L0: anyone can send this
    Authenticated, ///< A2 tag verified with a provisioned key
    Link           ///< G1 over an encrypted, authenticated link
  };

  void setupUi(void);
  void loadKeys(void);
  void loadReplayState(void);
  bool saveReplayState(void);
  QString replayStateFile(void) const;

  /// Decode the VSCP frames in one advertising report
  void handleAdvertising(const QBluetoothDeviceInfo& info);
  void handleFrame(const QBluetoothDeviceInfo& info, const QByteArray& ad);

  /// Add a received event to the event list
  void addEvent(const QString& device,
                const QString& profile,
                Trust trust,
                const vscp_ble_event_t& ev,
                const QString& details);
  void logLine(const QString& text);
  void updateDeviceRow(const QBluetoothDeviceInfo& info, const QString& frameKind, const uint8_t* guid);
  void setG1State(const QString& text);
  void updateG1Controls(void);
  bool writeG1(const QBluetoothUuid& uuid, const QByteArray& value);
  void handleControl(const QByteArray& value);
  void handleEventTx(const QByteArray& value);

  static QString deviceKey(const QBluetoothDeviceInfo& info);
  static QString guidToString(const uint8_t* guid);
  static bool parseGuid(const QString& str, uint8_t guid[16]);
  static QString hex(const uint8_t* data, size_t len);

  // A2 key store callbacks (vscp-ble)
  static vscp_ble_a2_key_t* a2Lookup(void* ctx, uint16_t company, uint16_t keyId);
  static int a2PersistReplay(void* ctx, const vscp_ble_a2_key_t* rec);

  json m_conn;
  uint16_t m_company;
  bool m_legacy;
  QBluetoothAddress m_adapter;

  // Receiving
  QBluetoothDeviceDiscoveryAgent* m_agent;
  QHash<QString, QBluetoothDeviceInfo> m_devices;
  QHash<QString, QByteArray> m_lastFrame; ///< device + version -> last frame (BlueZ repeats unchanged data)
  std::vector<vscp_ble_a2_key_t> m_keys;
  vscp_ble_a2_receiver_t m_a2rx;
  std::unique_ptr<vscp_ble_f1_rx_t> m_f1rx;
  QElapsedTimer m_clock;
  bool m_paused;

  // G1
  QLowEnergyController* m_ctrl;
  QLowEnergyService* m_service;
  QBluetoothDeviceInfo m_g1Device;
  std::unique_ptr<vscp_ble_g1_frag_rx_t> m_g1rx;
  bool m_g1Ready;      ///< HELLO done
  bool m_g1Busy;       ///< Waiting for EVENT_RESULT
  qint64 m_g1BusySince; ///< When the event was written (ms)
  uint32_t m_g1NextTid;
  uint16_t m_g1MaxData;
  uint16_t m_g1MaxRecord;
  QTimer m_g1Timer;
  QBluetoothLocalDevice* m_localDevice;

  // Widgets
  QTableWidget* m_tableDevices;
  QTableWidget* m_tableEvents;
  QLabel* m_status;
  QLabel* m_g1Status;
  QLineEdit* m_editGuid;
  QSpinBox* m_spinClass;
  QSpinBox* m_spinType;
  QLineEdit* m_editData;
  QPushButton* m_btnSend;
  QAction* m_actScan;
  QAction* m_actPause;
  QAction* m_actConnect;
  QAction* m_actDisconnect;
  QAction* m_actPair;
};

#endif // CFRMBLESESSION_H
