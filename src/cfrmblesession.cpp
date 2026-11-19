// cfrmblesession.cpp
//
// Bluetooth LE session window.
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

#ifdef WIN32
#include <pch.h> // First: windows.h, needed by the VSCP headers
#endif

#include "cfrmblesession.h"

#include "cdlgconnsettingsble.h"
#include "explorerstyle.h"
#ifndef VSCPWORKS_BLE_TEST
#include "vscpworks.h"
#endif

#include "vscp-ble-ad.h"
#include "vscp-ble-crypto.h"
#include "vscp-ble-e1.h"
#include "vscp-ble-l0.h"

#include <QAction>
#include <QBluetoothUuid>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QLowEnergyAdvertisingData>
#include <QLowEnergyAdvertisingParameters>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QStyle>
#include <QTabWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#include <spdlog/spdlog.h>

#include <algorithm>

namespace {

// G1 GATT UUIDs (vscp-ble / draft VSCP BLE specification)
QBluetoothUuid
g1Uuid(int n)
{
  return QBluetoothUuid(QString("9b7a%1-6e44-4b8a-9d64-565343502001").arg(n, 4, 16, QChar('0')));
}

const int MAX_EVENT_ROWS = 10000;

// Client Characteristic Configuration values (little-endian 0x0002 / 0x0001)
const QByteArray CCC_INDICATE = QByteArray::fromHex("0200");
const QByteArray CCC_NOTIFY   = QByteArray::fromHex("0100");

enum EventColumn
{
  EV_TIME = 0,
  EV_DEVICE,
  EV_PROFILE,
  EV_TRUST,
  EV_GUID,
  EV_CLASS,
  EV_TYPE,
  EV_DATA,
  EV_DETAILS,
  EV_COUNT
};

enum DeviceColumn
{
  DEV_ADDRESS = 0,
  DEV_NAME,
  DEV_RSSI,
  DEV_LAST,
  DEV_GUID,
  DEV_FRAMES,
  DEV_G1,
  DEV_COUNT
};

QString
g1StatusName(uint8_t status)
{
  static const char* names[] = { "OK",         "UNSUPPORTED_VERSION", "BAD_LENGTH", "BAD_FLAGS",
                                 "BAD_SEQUENCE", "BUSY",              "NOT_AUTHORIZED", "TOO_LARGE",
                                 "TIMEOUT",    "INVALID_EVENT",       "NOT_READY",  "INTERNAL_ERROR" };
  return (status < sizeof(names) / sizeof(names[0])) ? QString(names[status]) : QString("0x%1").arg(status, 2, 16);
}

enum BroadcastColumn
{
  BC_TIME = 0,
  BC_DEVICE,
  BC_NAME,
  BC_RSSI,
  BC_KIND,
  BC_ID,
  BC_DATA,
  BC_DECODED,
  BC_COUNT
};

const int MAX_BROADCAST_ROWS = 10000;

// Legacy advertising data (the only kind the Qt Bluetooth module can send)
const int LEGACY_ADV_MAX = 31;

// AD types (Bluetooth Assigned Numbers)
const uint8_t AD_FLAGS              = 0x01;
const uint8_t AD_SHORT_NAME         = 0x08;
const uint8_t AD_COMPLETE_NAME      = 0x09;
const uint8_t AD_SERVICE_DATA_16    = 0x16;
const uint8_t AD_SERVICE_DATA_32    = 0x20;
const uint8_t AD_SERVICE_DATA_128   = 0x21;
const uint8_t AD_MANUFACTURER       = 0xFF;
const uint8_t FLAGS_GENERAL_NO_BREDR = 0x06;

QString
hex16(quint16 v)
{
  return QString("0x%1").arg(QString("%1").arg(v, 4, 16, QChar('0')).toUpper());
}

QString
hexBytes(const QByteArray& data)
{
  return QString::fromLatin1(data.toHex(' ').toUpper());
}

// Names of some Company Identifiers seen often (Bluetooth SIG list)
QString
companyName(quint16 company)
{
  static const QHash<quint16, QString> names = {
    { 0x0006, "Microsoft" },  { 0x004C, "Apple" },   { 0x0059, "Nordic Semiconductor" },
    { 0x0075, "Samsung" },    { 0x0087, "Garmin" },  { 0x00E0, "Google" },
    { 0x0157, "Anhui Huami" }, { 0x02E5, "Espressif" }, { 0x038F, "Xiaomi" },
    { 0x0499, "Ruuvi Innovations" }, { 0xFFFF, "test / unassigned" },
  };
  return names.value(company);
}

QString
uuidText(const QBluetoothUuid& uuid)
{
  bool is16       = false;
  const quint16 v = uuid.toUInt16(&is16);
  if (is16) {
    const QString name = QBluetoothUuid::serviceClassToString(static_cast<QBluetoothUuid::ServiceClassUuid>(v));
    return (name.isEmpty() || name.startsWith("Unknown")) ? hex16(v) : QString("%1 %2").arg(hex16(v), name);
  }
  return uuid.toString(QUuid::WithoutBraces);
}

// Data that is all printable ASCII is shown as text
QString
printableText(const QByteArray& data)
{
  if (data.size() < 3) {
    return QString();
  }
  for (char c : data) {
    if ((c < 0x20) || (c > 0x7E)) {
      return QString();
    }
  }
  return QString("\"%1\"").arg(QString::fromLatin1(data));
}

/*!
  Decode well-known manufacturer data: VSCP frames (with the connection's
  Company Identifier) and Apple iBeacon
*/
QString
describeManufacturerData(quint16 company, const QByteArray& data, quint16 vscpCompany)
{
  const uint8_t* d = reinterpret_cast<const uint8_t*>(data.constData());
  if ((company == vscpCompany) && !data.isEmpty()) {
    switch (d[0]) {
      case VSCP_BLE_VERSION_E1:
        return QObject::tr("VSCP E1 frame");
      case VSCP_BLE_VERSION_A2:
        return QObject::tr("VSCP A2 frame");
      case VSCP_BLE_VERSION_F1:
        return QObject::tr("VSCP F1 fragment");
      default:
        if (0 == (d[0] & 0x0F)) {
          return QObject::tr("VSCP L0 frame (legacy)");
        }
        break;
    }
  }
  if ((0x004C == company) && (data.size() >= 23) && (0x02 == d[0]) && (0x15 == d[1])) {
    const QUuid uuid = QUuid::fromRfc4122(data.mid(2, 16));
    return QObject::tr("iBeacon %1 major %2 minor %3 tx power %4 dBm")
      .arg(uuid.toString(QUuid::WithoutBraces))
      .arg((d[18] << 8) | d[19])
      .arg((d[20] << 8) | d[21])
      .arg(static_cast<int8_t>(d[22]));
  }
  return printableText(data);
}

/*!
  Decode well-known service data: Eddystone UID, URL and TLM
*/
QString
describeServiceData(const QBluetoothUuid& uuid, const QByteArray& data)
{
  const uint8_t* d = reinterpret_cast<const uint8_t*>(data.constData());
  bool is16        = false;
  if ((0xFEAA == uuid.toUInt16(&is16)) && is16 && !data.isEmpty()) {
    if ((0x00 == d[0]) && (data.size() >= 18)) {
      return QObject::tr("Eddystone-UID namespace %1 instance %2 tx power %3 dBm")
        .arg(QString::fromLatin1(data.mid(2, 10).toHex()), QString::fromLatin1(data.mid(12, 6).toHex()))
        .arg(static_cast<int8_t>(d[1]));
    }
    if ((0x10 == d[0]) && (data.size() >= 3) && (d[2] < 4)) {
      static const char* schemes[] = { "http://www.", "https://www.", "http://", "https://" };
      static const char* expansions[] = { ".com/", ".org/", ".edu/", ".net/", ".info/", ".biz/", ".gov/",
                                          ".com",  ".org",  ".edu",  ".net",  ".info",  ".biz",  ".gov" };
      QString url = schemes[d[2]];
      for (int i = 3; i < data.size(); i++) {
        url += (d[i] < 14) ? QString(expansions[d[i]]) : QString(QChar(d[i]));
      }
      return QObject::tr("Eddystone-URL %1 tx power %2 dBm").arg(url).arg(static_cast<int8_t>(d[1]));
    }
    if ((0x20 == d[0]) && (data.size() >= 14) && (0x00 == d[1])) {
      const int mv          = (d[2] << 8) | d[3];
      const double temp     = static_cast<int16_t>((d[4] << 8) | d[5]) / 256.0;
      const quint32 advCnt  = (quint32(d[6]) << 24) | (d[7] << 16) | (d[8] << 8) | d[9];
      const quint32 secCnt  = (quint32(d[10]) << 24) | (d[11] << 16) | (d[12] << 8) | d[13];
      return QObject::tr("Eddystone-TLM battery %1 mV, %2 °C, %3 advertisements, up %4 s")
        .arg(mv)
        .arg(temp, 0, 'f', 1)
        .arg(advCnt)
        .arg(secCnt / 10);
    }
  }
  return printableText(data);
}

/*!
  Hex octets, optionally separated by spaces, colons, commas or dashes and
  prefixed with 0x
*/
bool
parseHexBytes(const QString& text, QByteArray& out)
{
  QString h = text;
  h.remove(QRegularExpression("0[xX]"));
  h.remove(QRegularExpression("[\\s:,-]"));
  if ((h.length() % 2) || !QRegularExpression("^[0-9A-Fa-f]*$").match(h).hasMatch()) {
    return false;
  }
  out = QByteArray::fromHex(h.toLatin1());
  return true;
}

/*!
  A service UUID in advertising (little-endian) byte order: 4 hex digits
  (16-bit), 8 hex digits (32-bit) or a 128-bit UUID
*/
bool
parseServiceUuid(const QString& text, QByteArray& le)
{
  QString s = text.trimmed();
  if (s.startsWith("0x", Qt::CaseInsensitive)) {
    s = s.mid(2);
  }
  if (QRegularExpression("^([0-9A-Fa-f]{4}|[0-9A-Fa-f]{8})$").match(s).hasMatch()) {
    le = QByteArray::fromHex(s.toLatin1());
    std::reverse(le.begin(), le.end());
    return true;
  }
  const QBluetoothUuid uuid(s);
  if (uuid.isNull()) {
    return false;
  }
  le = uuid.toRfc4122();
  std::reverse(le.begin(), le.end());
  return true;
}

void
appendAd(QByteArray& ad, uint8_t type, const QByteArray& value)
{
  ad.append(static_cast<char>(value.size() + 1));
  ad.append(static_cast<char>(type));
  ad.append(value);
}

/// The fields of the Advertise box
struct AdvertisingFields
{
  bool flags = true;
  QString name;
  bool hasManufacturer = false;
  quint16 company      = 0;
  QByteArray manufacturerData;
  QByteArray serviceUuid; ///< Little-endian, 2, 4 or 16 octets; empty: no service data
  QByteArray serviceData;
};

/*!
  Legacy advertising data from the fields. A name that does not fit is
  shortened (Shortened Local Name).
*/
bool
buildAdvertisingData(const AdvertisingFields& f, QByteArray& ad, QString& error)
{
  ad.clear();
  if (f.flags) {
    appendAd(ad, AD_FLAGS, QByteArray(1, static_cast<char>(FLAGS_GENERAL_NO_BREDR)));
  }
  if (f.hasManufacturer) {
    QByteArray v;
    v.append(static_cast<char>(f.company & 0xFF));
    v.append(static_cast<char>(f.company >> 8));
    v.append(f.manufacturerData);
    appendAd(ad, AD_MANUFACTURER, v);
  }
  if (!f.serviceUuid.isEmpty()) {
    const uint8_t type = (2 == f.serviceUuid.size())   ? AD_SERVICE_DATA_16
                         : (4 == f.serviceUuid.size()) ? AD_SERVICE_DATA_32
                                                       : AD_SERVICE_DATA_128;
    appendAd(ad, type, f.serviceUuid + f.serviceData);
  }
  if (!f.name.isEmpty()) {
    const int room = LEGACY_ADV_MAX - static_cast<int>(ad.size()) - 2;
    QString name   = f.name;
    if (name.toUtf8().size() <= room) {
      appendAd(ad, AD_COMPLETE_NAME, name.toUtf8());
    }
    else {
      while (!name.isEmpty() && (name.toUtf8().size() > room)) {
        name.chop(1);
      }
      if (!name.isEmpty()) {
        appendAd(ad, AD_SHORT_NAME, name.toUtf8());
      }
    }
  }
  if (ad.isEmpty()) {
    error = QObject::tr("Nothing to advertise");
    return false;
  }
  if (ad.size() > LEGACY_ADV_MAX) {
    error = QObject::tr("%1 octets: legacy advertising holds at most %2").arg(ad.size()).arg(LEGACY_ADV_MAX);
    return false;
  }
  return true;
}

/*!
  Raw advertising data must be a sequence of complete AD structures
  (length, type, value), optionally followed by zero padding
*/
bool
checkRawAdvertisingData(const QByteArray& ad, QString& error)
{
  if (ad.isEmpty()) {
    error = QObject::tr("Nothing to advertise");
    return false;
  }
  if (ad.size() > LEGACY_ADV_MAX) {
    error = QObject::tr("%1 octets: legacy advertising holds at most %2").arg(ad.size()).arg(LEGACY_ADV_MAX);
    return false;
  }
  int i = 0;
  while (i < ad.size()) {
    const int len = static_cast<uint8_t>(ad[i]);
    if (0 == len) {
      for (int j = i; j < ad.size(); j++) {
        if (0 != ad[j]) {
          error = QObject::tr("Data after a zero-length AD structure at octet %1").arg(i);
          return false;
        }
      }
      return true;
    }
    if (i + 1 + len > ad.size()) {
      error = QObject::tr("The AD structure at octet %1 is longer than the data").arg(i);
      return false;
    }
    i += 1 + len;
  }
  return true;
}

bool
hexToBytes(const std::string& str, uint8_t* out, size_t len)
{
  const QByteArray b = QByteArray::fromHex(QByteArray::fromStdString(str));
  if (static_cast<size_t>(b.size()) != len || (QString::fromStdString(str).length() != static_cast<int>(2 * len))) {
    return false;
  }
  memcpy(out, b.constData(), len);
  return true;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// CFrmBleSession
//

CFrmBleSession::CFrmBleSession(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_company(0xFFFF)
  , m_legacy(false)
  , m_agent(nullptr)
  , m_f1rx(new vscp_ble_f1_rx_t)
  , m_paused(false)
  , m_darkTheme(false)
  , m_ctrl(nullptr)
  , m_service(nullptr)
  , m_g1rx(new vscp_ble_g1_frag_rx_t)
  , m_g1Ready(false)
  , m_g1Busy(false)
  , m_g1BusySince(0)
  , m_g1NextTid(1)
  , m_g1MaxData(0)
  , m_g1MaxRecord(VSCP_BLE_G1_RECORD_HEADER)
  , m_localDevice(nullptr)
  , m_peripheral(nullptr)
  , m_advertising(false)
  , m_advErrors(0)
  , m_tableEvents(nullptr)
  , m_actTheme(nullptr)
{
  if (nullptr != pconn) {
    m_conn = *pconn;
  }
  const bool defaultDark = QCoreApplication::instance()->property("defaultDarkTheme").toBool();
#ifndef VSCPWORKS_BLE_TEST
  m_darkTheme = vscpworks_connectionDarkTheme(m_conn, defaultDark);
#else
  m_darkTheme = defaultDark;
#endif
  if (m_conn.contains("company") && m_conn["company"].is_number_integer()) {
    m_company = static_cast<uint16_t>(m_conn["company"].get<int>() & 0xFFFF);
  }
  m_legacy  = m_conn.value("legacy", false);
  m_adapter = QBluetoothAddress(QString::fromStdString(m_conn.value("adapter", std::string())));

  m_clock.start();
  vscp_ble_f1_rx_init(m_f1rx.get(), m_company);
  vscp_ble_g1_frag_rx_init(m_g1rx.get(), VSCP_BLE_G1_MAX_RECORD);

  loadKeys();
  loadReplayState();
  m_a2rx.company        = m_company;
  m_a2rx.lookup         = a2Lookup;
  m_a2rx.persist_replay = a2PersistReplay;
  m_a2rx.ctx            = this;

  setupUi();

  setWindowTitle(tr("VSCP Works - Bluetooth LE session - %1")
                   .arg(QString::fromStdString(m_conn.value("name", std::string()))));

  m_g1Timer.setInterval(1000);
  connect(&m_g1Timer, &QTimer::timeout, this, &CFrmBleSession::g1Tick);
  m_g1Timer.start();

  // Start receiving right away
  QTimer::singleShot(0, this, &CFrmBleSession::toggleScan);
}

CFrmBleSession::~CFrmBleSession()
{
  if (nullptr != m_agent) {
    m_agent->stop();
  }
  if (nullptr != m_ctrl) {
    m_ctrl->disconnectFromDevice();
  }
  if ((nullptr != m_peripheral) && m_advertising) {
    m_peripheral->stopAdvertising();
  }
  // Keys are secrets
  for (auto& k : m_keys) {
    vscp_ble_secure_zero(&k, sizeof(k));
  }
}

///////////////////////////////////////////////////////////////////////////////
// setupUi
//

void
CFrmBleSession::setupUi(void)
{
  resize(1250, 800);
  setMinimumSize(980, 620);

  QToolBar* toolbar = new QToolBar(tr("Bluetooth LE"), this);
  toolbar->setMovable(false);
  toolbar->setFloatable(false);
  toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_actScan = toolbar->addAction(tr("Scan"));
  m_actScan->setCheckable(true);
  m_actScan->setToolTip(tr("Receive Bluetooth LE advertising"));
  m_actPause = toolbar->addAction(tr("Pause"));
  m_actPause->setCheckable(true);
  m_actPause->setToolTip(tr("Stop adding to the lists (advertising is still received)"));
  QAction* actClear = toolbar->addAction(tr("Clear"));
  actClear->setToolTip(tr("Clear the event and advertising lists"));
  toolbar->addSeparator();
  m_actConnect = toolbar->addAction(tr("Connect G1"));
  m_actConnect->setToolTip(tr("Connect to the selected device (or the configured G1 node) over the G1 GATT service"));
  m_actDisconnect = toolbar->addAction(tr("Disconnect"));
  m_actPair       = toolbar->addAction(tr("Pair"));
  m_actPair->setToolTip(tr("Pair with the selected device. G1 needs authenticated (passkey) pairing."));
  toolbar->addSeparator();
  m_actTheme = toolbar->addAction(QString());
  m_actTheme->setToolTip(tr("Switch between dark and light theme"));
  QWidget* spacer = new QWidget(this);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  toolbar->addWidget(spacer);
  QAction* actHelp = toolbar->addAction(tr("Help"));
  actHelp->setToolTip(tr("Open the manual (F1)"));
  actHelp->setShortcut(QKeySequence(Qt::Key_F1));
  actHelp->setShortcutContext(Qt::WindowShortcut);
  explorerstyle::setPrimary(toolbar, m_actScan);

  connect(m_actScan, &QAction::triggered, this, &CFrmBleSession::toggleScan);
  connect(m_actPause, &QAction::toggled, this, [this](bool on) { m_paused = on; });
  connect(actClear, &QAction::triggered, this, &CFrmBleSession::clearEvents);
  connect(m_actConnect, &QAction::triggered, this, &CFrmBleSession::connectG1);
  connect(m_actDisconnect, &QAction::triggered, this, &CFrmBleSession::disconnectG1);
  connect(m_actPair, &QAction::triggered, this, &CFrmBleSession::pairDevice);
  connect(m_actTheme, &QAction::triggered, this, &CFrmBleSession::toggleTheme);
  connect(actHelp, &QAction::triggered, this, &CFrmBleSession::showHelp);

  // Devices
  m_tableDevices = new QTableWidget(0, DEV_COUNT, this);
  m_tableDevices->setHorizontalHeaderLabels(
    { tr("Address"), tr("Name"), tr("RSSI"), tr("Last frame"), tr("GUID"), tr("Frames"), tr("G1") });
  m_tableDevices->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableDevices->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableDevices->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableDevices->horizontalHeader()->setSectionResizeMode(DEV_GUID, QHeaderView::Stretch);
  m_tableDevices->verticalHeader()->hide();
  m_tableDevices->setShowGrid(false);
  connect(m_tableDevices, &QTableWidget::cellDoubleClicked, this, [this](int, int) { connectG1(); });

  // VSCP events
  m_tableEvents = new QTableWidget(0, EV_COUNT, this);
  m_tableEvents->setHorizontalHeaderLabels({ tr("Time"),
                                             tr("Device"),
                                             tr("Profile"),
                                             tr("Trust"),
                                             tr("GUID"),
                                             tr("Class"),
                                             tr("Type"),
                                             tr("Data"),
                                             tr("Details") });
  m_tableEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableEvents->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableEvents->horizontalHeader()->setSectionResizeMode(EV_DATA, QHeaderView::Stretch);
  m_tableEvents->verticalHeader()->hide();
  m_tableEvents->setShowGrid(false);

  // All advertising
  QWidget* broadcastPage = new QWidget(this);
  m_editBroadcastFilter  = new QLineEdit(broadcastPage);
  m_editBroadcastFilter->setPlaceholderText(tr("Filter: address, name, company, UUID, data or decoded text"));
  m_editBroadcastFilter->setClearButtonEnabled(true);
  m_chkChangesOnly = new QCheckBox(tr("Only changed data"), broadcastPage);
  m_chkChangesOnly->setChecked(true);
  m_chkChangesOnly->setToolTip(tr("Devices repeat their advertising: add a row only when a device's data changes"));
  m_tableBroadcasts = new QTableWidget(0, BC_COUNT, broadcastPage);
  m_tableBroadcasts->setHorizontalHeaderLabels(
    { tr("Time"), tr("Device"), tr("Name"), tr("RSSI"), tr("Kind"), tr("Company / UUID"), tr("Data"), tr("Decoded") });
  m_tableBroadcasts->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableBroadcasts->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableBroadcasts->horizontalHeader()->setSectionResizeMode(BC_DATA, QHeaderView::Stretch);
  m_tableBroadcasts->verticalHeader()->hide();
  m_tableBroadcasts->setShowGrid(false);
  m_tableBroadcasts->setFont(explorerstyle::monospaceFont());
  connect(m_editBroadcastFilter, &QLineEdit::textChanged, this, &CFrmBleSession::filterBroadcasts);
  QHBoxLayout* filterRow = new QHBoxLayout;
  filterRow->addWidget(m_editBroadcastFilter, 1);
  filterRow->addWidget(m_chkChangesOnly);
  QVBoxLayout* broadcastLayout = new QVBoxLayout(broadcastPage);
  broadcastLayout->setContentsMargins(8, 8, 8, 8);
  broadcastLayout->addLayout(filterRow);
  broadcastLayout->addWidget(m_tableBroadcasts, 1);

  m_tabs = new QTabWidget(this);
  m_tabs->addTab(m_tableEvents, tr("VSCP events"));
  m_tabs->addTab(broadcastPage, tr("All advertising"));

  // G1 send page
  QWidget* g1Box = new QWidget(this);
  m_g1Status     = new QLabel(tr("Not connected"), g1Box);
  m_g1Status->setWordWrap(true);
  m_editGuid  = new QLineEdit(g1Box);
  m_spinClass = new QSpinBox(g1Box);
  m_spinClass->setRange(0, 0xFFFF);
  m_spinClass->setValue(20);
  m_spinType = new QSpinBox(g1Box);
  m_spinType->setRange(0, 0xFFFF);
  m_spinType->setValue(9);
  m_editData = new QLineEdit("0,0,0", g1Box);
  m_editData->setToolTip(tr("Data octets separated by commas or spaces (decimal, 0x hex)"));
  m_btnSend = new QPushButton(tr("Send event"), g1Box);
  explorerstyle::setPrimary(m_btnSend);
  connect(m_btnSend, &QPushButton::clicked, this, &CFrmBleSession::sendEvent);

  // Sender GUID: FF:FF:FF:FF:FF:FF:FF:FE + adapter address + 00:00
  QBluetoothAddress local = m_adapter;
  if (local.isNull()) {
    local = QBluetoothLocalDevice().address();
  }
  uint8_t guid[16];
  memset(guid, 0xFF, 7);
  guid[7]             = 0xFE;
  const quint64 addr  = local.toUInt64();
  for (int i = 0; i < 6; i++) {
    guid[8 + i] = static_cast<uint8_t>(addr >> (8 * (5 - i)));
  }
  guid[14] = 0;
  guid[15] = 0;
  if (0 == addr) {
    guid[15] = 1; // No address (macOS): still a nonzero GUID
  }
  m_editGuid->setText(guidToString(guid));

  QFormLayout* g1Form = new QFormLayout;
  g1Form->addRow(tr("Sender GUID:"), m_editGuid);
  g1Form->addRow(tr("Class:"), m_spinClass);
  g1Form->addRow(tr("Type:"), m_spinType);
  g1Form->addRow(tr("Data:"), m_editData);
  QVBoxLayout* g1Layout = new QVBoxLayout(g1Box);
  g1Layout->setContentsMargins(8, 8, 8, 8);
  g1Layout->addWidget(m_g1Status);
  g1Layout->addLayout(g1Form);
  g1Layout->addWidget(m_btnSend);
  g1Layout->addStretch();

  QTabWidget* sideTabs = new QTabWidget(this);
  sideTabs->addTab(g1Box, tr("G1 connection"));
  sideTabs->addTab(createAdvertiseBox(), tr("Advertise"));

  QSplitter* top = new QSplitter(Qt::Horizontal, this);
  top->addWidget(m_tableDevices);
  top->addWidget(sideTabs);
  top->setStretchFactor(0, 3);
  top->setStretchFactor(1, 2);

  QSplitter* main = new QSplitter(Qt::Vertical, this);
  main->addWidget(top);
  main->addWidget(m_tabs);
  main->setStretchFactor(0, 1);
  main->setStretchFactor(1, 2);

  m_status = new QLabel(this);
  m_status->setProperty("explorerRole", "trace");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(14, 8, 14, 10);
  layout->setSpacing(10);
  layout->addWidget(toolbar);
  layout->addWidget(main, 1);
  layout->addWidget(m_status);

  applyTheme();
  updateG1Controls();
  updateAdvertisingPreview();
}

QWidget*
CFrmBleSession::createAdvertiseBox(void)
{
  QWidget* page = new QWidget(this);

  m_chkAdvFlags = new QCheckBox(tr("Flags (LE General Discoverable)"), page);
  m_chkAdvFlags->setChecked(true);
  m_editAdvName = new QLineEdit(page);
  m_editAdvName->setPlaceholderText(tr("Optional, shortened if it does not fit"));
  m_editAdvCompany = new QLineEdit(page);
  m_editAdvCompany->setPlaceholderText(tr("Empty: no manufacturer data"));
  m_editAdvCompany->setToolTip(tr("Bluetooth SIG Company Identifier, for example 0x004C. 0xFFFF is for tests only."));
  m_editAdvMfgData = new QLineEdit(page);
  m_editAdvMfgData->setPlaceholderText(tr("Hex octets, for example 01 02 03"));
  m_editAdvServiceUuid = new QLineEdit(page);
  m_editAdvServiceUuid->setPlaceholderText(tr("Empty: no service data. FEAA or a 128-bit UUID"));
  m_editAdvServiceData = new QLineEdit(page);
  m_editAdvServiceData->setPlaceholderText(tr("Hex octets"));
  m_chkAdvRaw = new QCheckBox(tr("Raw advertising data"), page);
  m_chkAdvRaw->setToolTip(tr("Enter the complete advertising data (AD structures: length, type, value) instead"));
  m_editAdvRaw = new QLineEdit(page);
  m_editAdvRaw->setPlaceholderText(tr("For example 02 01 06 05 FF FF FF 01 02"));
  m_spinAdvInterval = new QSpinBox(page);
  m_spinAdvInterval->setRange(100, 10240);
  m_spinAdvInterval->setValue(200);
  m_spinAdvInterval->setSuffix(tr(" ms"));
  m_spinAdvInterval->setToolTip(tr("Advertising interval. Non-connectable advertising sends at most every 100 ms."));
  m_lblAdvPreview = new QLabel(page);
  m_lblAdvPreview->setWordWrap(true);
  m_lblAdvPreview->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_lblAdvPreview->setFont(explorerstyle::monospaceFont());
  m_btnAdvertise = new QPushButton(tr("Start advertising"), page);
  explorerstyle::setPrimary(m_btnAdvertise);

  QFormLayout* form = new QFormLayout;
  form->addRow(QString(), m_chkAdvFlags);
  form->addRow(tr("Local name:"), m_editAdvName);
  form->addRow(tr("Company:"), m_editAdvCompany);
  form->addRow(tr("Manufacturer data:"), m_editAdvMfgData);
  form->addRow(tr("Service UUID:"), m_editAdvServiceUuid);
  form->addRow(tr("Service data:"), m_editAdvServiceData);
  form->addRow(QString(), m_chkAdvRaw);
  form->addRow(tr("Raw data:"), m_editAdvRaw);
  form->addRow(tr("Interval:"), m_spinAdvInterval);

  QVBoxLayout* layout = new QVBoxLayout(page);
  layout->setContentsMargins(8, 8, 8, 8);
  layout->addLayout(form);
  layout->addWidget(m_lblAdvPreview);
  layout->addWidget(m_btnAdvertise);
#if defined(Q_OS_WIN)
  QLabel* note = new QLabel(tr("The Qt Bluetooth module cannot advertise on Windows."), page);
  note->setWordWrap(true);
  layout->addWidget(note);
  m_btnAdvertise->setEnabled(false);
#elif defined(Q_OS_MACOS)
  QLabel* note = new QLabel(tr("macOS only advertises the local name: other data is not sent."), page);
  note->setWordWrap(true);
  layout->addWidget(note);
#endif
  layout->addStretch();

  for (QLineEdit* edit :
       { m_editAdvName, m_editAdvCompany, m_editAdvMfgData, m_editAdvServiceUuid, m_editAdvServiceData, m_editAdvRaw }) {
    connect(edit, &QLineEdit::textChanged, this, &CFrmBleSession::updateAdvertisingPreview);
  }
  connect(m_chkAdvFlags, &QCheckBox::toggled, this, &CFrmBleSession::updateAdvertisingPreview);
  connect(m_chkAdvRaw, &QCheckBox::toggled, this, &CFrmBleSession::updateAdvertisingPreview);
  connect(m_btnAdvertise, &QPushButton::clicked, this, &CFrmBleSession::toggleAdvertising);
  return page;
}

///////////////////////////////////////////////////////////////////////////////
// Theme
//

void
CFrmBleSession::applyTheme(void)
{
  static const QString tmpl = QStringLiteral(R"(
    CFrmBleSession, QDialog, QWidget { background-color: @bg@; color: @fg@; }
    QToolBar { background-color: @panel@; border: 1px solid @line@; border-radius: 14px; padding: 8px; spacing: 8px; }
    QToolButton { background: transparent; color: @fg@; border: 1px solid @border@; border-radius: 16px; padding: 6px 16px; }
    QToolButton:hover { border-color: @accent@; }
    QToolButton:checked { color: @accent@; border-color: @accent@; }
    QToolButton:disabled, QPushButton:disabled { color: @border@; border-color: @line@; }
    QToolButton[explorerRole="primary"] { background-color: @accent@; color: @onacc@; border: none; font-weight: 600; }
    QToolButton[explorerRole="primary"]:checked { background-color: @selbg@; color: @accent@; border: 1px solid @accent@; }
    QMenu { background-color: @panel@; color: @fg@; border: 1px solid @line@; }
    QMenu::item:selected { background-color: @line@; }
    QTabWidget::pane { background-color: @panel@; border: 1px solid @line@; border-radius: 12px; top: -1px; }
    QTabWidget > QWidget, QTabWidget QStackedWidget > QWidget { background-color: @panel@; }
    QTabBar::tab { background: transparent; color: @muted@; border: 1px solid transparent; border-radius: 12px;
      padding: 5px 14px; margin: 0 4px 4px 0; }
    QTabBar::tab:selected { color: @accent@; border-color: @accent@; }
    QTabBar::tab:hover { color: @fg@; }
    QLineEdit, QComboBox, QSpinBox { background-color: @bg@; color: @fg@; border: 1px solid @border@;
      border-radius: 8px; padding: 4px 8px; selection-background-color: @selbg@; selection-color: @accent@; }
    QLineEdit:focus, QSpinBox:focus { border-color: @accent@; }
    QLineEdit:disabled, QSpinBox:disabled { color: @muted@; border-color: @line@; }
    QCheckBox { background: transparent; color: @fg@; spacing: 6px; }
    QLabel { background: transparent; }
    QPushButton { background: transparent; color: @fg@; border: 1px solid @border@; border-radius: 16px; padding: 6px 16px; }
    QPushButton:hover { border-color: @accent@; }
    QPushButton[explorerRole="primary"] { background-color: @accent@; color: @onacc@; border: none; font-weight: 600; }
    QPushButton[explorerRole="primary"]:disabled { background-color: @line@; color: @muted@; }
    QTableWidget { background-color: @panel@; alternate-background-color: @panel@; color: @fg@;
      border: 1px solid @line@; border-radius: 12px; gridline-color: @line@; outline: 0; }
    QTableWidget::item { padding: 2px 4px; border-bottom: 1px solid @line@; }
    QTableWidget::item:selected { background-color: @selbg@; color: @accent@; }
    QHeaderView { background-color: @panel@; }
    QHeaderView::section { background-color: @panel@; color: @muted@; border: none; padding: 6px 4px;
      font-size: 11px; font-weight: 600; }
    QTableCornerButton::section { background-color: @panel@; border: none; }
    QSplitter::handle { background-color: @bg@; }
    QScrollBar:vertical { background: @panel@; width: 10px; margin: 2px; border: none; }
    QScrollBar:horizontal { background: @panel@; height: 10px; margin: 2px; border: none; }
    QScrollBar::handle { background: @border@; border-radius: 3px; min-height: 24px; min-width: 24px; }
    QScrollBar::handle:hover { background: @muted@; }
    QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
    QScrollBar::add-page, QScrollBar::sub-page { background: none; }
    QToolTip { background-color: @panel@; color: @fg@; border: 1px solid @border@; }
    QLabel[explorerRole="trace"] { color: @muted@; font-size: 11px; font-weight: 600; padding: 2px 4px; }
    QLabel[bleRole="ok"] { color: @muted@; }
    QLabel[bleRole="error"] { color: @err@; }
  )");
  const QStringList light = { "#f5f5f5", "#ffffff", "#e5e5e5", "#d4d4d4", "#171717", "#737373",
                              "#b45309", "#fef3c7", "#ffffff", "#b91c1c" };
  const QStringList dark  = { "#0b0b0b", "#141414", "#262626", "#3f3f3f", "#e5e5e5", "#a3a3a3",
                              "#fbbf24", "#3b2f0a", "#111111", "#fca5a5" };
  const QStringList names = { "@bg@",     "@panel@", "@line@",  "@border@", "@fg@",
                              "@muted@", "@accent@", "@selbg@", "@onacc@",  "@err@" };
  const QStringList& colors = m_darkTheme ? dark : light;
  QString sheet             = tmpl;
  for (int i = 0; i < names.size(); ++i) {
    sheet.replace(names.at(i), colors.at(i));
  }
  setStyleSheet(explorerstyle::styleSheet() + sheet);
  if (nullptr != m_actTheme) {
    m_actTheme->setText(m_darkTheme ? tr("Light theme") : tr("Dark theme"));
  }

  // Trust colors of the listed events
  if (nullptr != m_tableEvents) {
    for (int row = 0; row < m_tableEvents->rowCount(); row++) {
      if (QTableWidgetItem* item = m_tableEvents->item(row, EV_TRUST)) {
        item->setForeground(trustColor(static_cast<Trust>(item->data(Qt::UserRole).toInt())));
      }
    }
  }
}

QColor
CFrmBleSession::trustColor(Trust trust) const
{
  switch (trust) {
    case Trust::Authenticated:
      return QColor(m_darkTheme ? "#86efac" : "#15803d");
    case Trust::Link:
      return QColor(m_darkTheme ? "#67e8f9" : "#0e7490");
    case Trust::Public:
    default:
      return QColor(m_darkTheme ? "#fbbf24" : "#b45309");
  }
}

void
CFrmBleSession::toggleTheme(void)
{
  m_darkTheme = !m_darkTheme;
#ifndef VSCPWORKS_BLE_TEST
  vscpworks_setConnectionTheme(m_conn, m_darkTheme);
  emit connectionThemeChanged(vscpworks_connectionUuid(m_conn), m_darkTheme);
#endif
  applyTheme();
}

///////////////////////////////////////////////////////////////////////////////
// Keys and replay state
//

void
CFrmBleSession::loadKeys(void)
{
  m_keys.clear();
  if (!m_conn.contains("a2-keys") || !m_conn["a2-keys"].is_array()) {
    return;
  }
  for (const json& k : m_conn["a2-keys"]) {
    if (!k.is_object()) {
      continue;
    }
    vscp_ble_a2_key_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.company   = m_company;
    rec.key_id    = static_cast<uint16_t>(k.value("key-id", 0));
    rec.min_epoch = 0;
    rec.max_epoch = 0xFFFFFFFF;
    if ((0 == rec.key_id) || !parseGuid(QString::fromStdString(k.value("guid", std::string())), rec.guid) ||
        !hexToBytes(k.value("key", std::string()), rec.key, 16) ||
        !hexToBytes(k.value("nonce-prefix", std::string()), rec.nonce_prefix, 5)) {
      spdlog::warn("BLE session: invalid A2 key record skipped");
      continue;
    }
    m_keys.push_back(rec);
  }
}

QString
CFrmBleSession::replayStateFile(void) const
{
  QString uuid = QString::fromStdString(m_conn.value("uuid", std::string("none")));
  uuid.remove('{');
  uuid.remove('}');
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ble-replay-" + uuid + ".json";
}

void
CFrmBleSession::loadReplayState(void)
{
  QFile file(replayStateFile());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }
  const QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
  for (auto& k : m_keys) {
    const QString key = QString("%1/%2").arg(k.key_id).arg(guidToString(k.guid));
    if (obj.contains(key)) {
      const QJsonObject s = obj[key].toObject();
      k.have_replay       = 1;
      k.last_epoch        = static_cast<uint32_t>(s["epoch"].toDouble());
      k.last_sequence     = static_cast<uint32_t>(s["sequence"].toDouble());
    }
  }
}

bool
CFrmBleSession::saveReplayState(void)
{
  QJsonObject obj;
  for (const auto& k : m_keys) {
    if (k.have_replay) {
      QJsonObject s;
      s["epoch"]    = static_cast<double>(k.last_epoch);
      s["sequence"] = static_cast<double>(k.last_sequence);
      obj[QString("%1/%2").arg(k.key_id).arg(guidToString(k.guid))] = s;
    }
  }
  QDir().mkpath(QFileInfo(replayStateFile()).absolutePath());
  QSaveFile file(replayStateFile());
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  file.write(QJsonDocument(obj).toJson());
  return file.commit(); // Atomic: the old or the new state survives a crash
}

vscp_ble_a2_key_t*
CFrmBleSession::a2Lookup(void* ctx, uint16_t company, uint16_t keyId)
{
  CFrmBleSession* self = static_cast<CFrmBleSession*>(ctx);
  for (auto& k : self->m_keys) {
    if ((k.company == company) && (k.key_id == keyId)) {
      return &k;
    }
  }
  return nullptr;
}

int
CFrmBleSession::a2PersistReplay(void* ctx, const vscp_ble_a2_key_t* rec)
{
  Q_UNUSED(rec);
  // The record (in m_keys) already holds the new state; fail closed if it
  // cannot be stored
  return static_cast<CFrmBleSession*>(ctx)->saveReplayState() ? 0 : -1;
}

///////////////////////////////////////////////////////////////////////////////
// Scanning
//

void
CFrmBleSession::toggleScan(void)
{
  if ((nullptr != m_agent) && m_agent->isActive()) {
    m_agent->stop();
    m_actScan->setChecked(false);
    m_status->setText(tr("Scanning stopped"));
    return;
  }

  if (nullptr == m_agent) {
    m_agent = m_adapter.isNull() ? new QBluetoothDeviceDiscoveryAgent(this)
                                 : new QBluetoothDeviceDiscoveryAgent(m_adapter, this);
    m_agent->setLowEnergyDiscoveryTimeout(0); // Until stopped
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &CFrmBleSession::deviceSeen);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &CFrmBleSession::deviceUpdated);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &CFrmBleSession::scanError);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished, this, &CFrmBleSession::scanFinished);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &CFrmBleSession::scanFinished);
  }

  m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
  if (m_agent->isActive()) {
    m_actScan->setChecked(true);
    m_status->setText(tr("Scanning for VSCP frames (Company Identifier 0x%1%2), %3 A2 key(s)")
                        .arg(m_company, 4, 16, QChar('0'))
                        .arg((0xFFFF == m_company) ? tr(", test only") : QString())
                        .arg(m_keys.size()));
  }
  else {
    m_actScan->setChecked(false);
  }
}

void
CFrmBleSession::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
  Q_UNUSED(error);
  m_actScan->setChecked(false);
  QString text = m_agent->errorString();
  if (QBluetoothDeviceDiscoveryAgent::PoweredOffError == error) {
    text = tr("The Bluetooth adapter is powered off");
  }
  else if (QBluetoothDeviceDiscoveryAgent::MissingPermissionsError == error) {
    text = tr("No permission to use Bluetooth");
  }
  m_status->setText(tr("Scanning failed: %1").arg(text));
}

void
CFrmBleSession::scanFinished(void)
{
  m_actScan->setChecked(false);
}

void
CFrmBleSession::deviceSeen(const QBluetoothDeviceInfo& info)
{
  if (!(info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
    return;
  }
  m_devices[deviceKey(info)] = info;
  handleAdvertising(info);
}

void
CFrmBleSession::deviceUpdated(const QBluetoothDeviceInfo& info, QBluetoothDeviceInfo::Fields fields)
{
  Q_UNUSED(fields);
  m_devices[deviceKey(info)] = info;
  handleAdvertising(info);
}

void
CFrmBleSession::handleAdvertising(const QBluetoothDeviceInfo& info)
{
  handleBroadcast(info);

  if (info.serviceUuids().contains(g1Uuid(0))) {
    updateDeviceRow(info, QString(), nullptr);
  }

  const QList<QByteArray> values = info.manufacturerData().values(m_company);
  for (const QByteArray& value : values) {
    // The stack hands over the data after the Company Identifier: rebuild
    // the complete AD structure for vscp-ble
    if (value.isEmpty() || (value.size() > 252)) {
      continue;
    }
    QByteArray ad;
    ad.append(static_cast<char>(value.size() + 3));
    ad.append(static_cast<char>(0xFF));
    ad.append(static_cast<char>(m_company & 0xFF));
    ad.append(static_cast<char>(m_company >> 8));
    ad.append(value);

    // Unchanged data is reported again and again
    const QString key = deviceKey(info) + "/" + QString::number(static_cast<uint8_t>(value[0]));
    if (m_lastFrame.value(key) == ad) {
      continue;
    }
    m_lastFrame[key] = ad;
    handleFrame(info, ad);
  }
}

void
CFrmBleSession::handleFrame(const QBluetoothDeviceInfo& info, const QByteArray& adBytes)
{
  const uint8_t* ad   = reinterpret_cast<const uint8_t*>(adBytes.constData());
  const size_t len    = static_cast<size_t>(adBytes.size());
  const uint8_t ver   = ad[4];
  const QString dev   = deviceKey(info);
  vscp_ble_event_t ev;
  int rv;

  switch (ver) {
    case VSCP_BLE_VERSION_E1: {
      uint32_t seq = 0;
      rv           = vscp_ble_e1_decode(ad, len, m_company, &seq, &ev);
      if (VSCP_BLE_OK == rv) {
        updateDeviceRow(info, "E1", ev.guid);
        addEvent(dev, "E1", Trust::Public, ev, tr("seq %1").arg(seq));
      }
      else {
        logLine(tr("%1: E1 frame rejected (%2)").arg(dev, vscp_ble_strerror(rv)));
      }
    } break;

    case VSCP_BLE_VERSION_A2: {
      updateDeviceRow(info, "A2", nullptr);
      if (m_keys.empty()) {
        return; // Encrypted: nothing to show without a key
      }
      uint8_t plain[VSCP_BLE_A2_MAX_DATA];
      uint32_t epoch = 0, seq = 0;
      vscp_ble_a2_key_t* rec = nullptr;
      rv = vscp_ble_a2_receive(&m_a2rx, ad, len, &ev, plain, sizeof(plain), &epoch, &seq, &rec);
      if (VSCP_BLE_OK == rv) {
        updateDeviceRow(info, "A2", ev.guid);
        addEvent(dev, "A2", Trust::Authenticated, ev, tr("key %1 epoch %2 seq %3").arg(rec->key_id).arg(epoch).arg(seq));
        vscp_ble_secure_zero(plain, sizeof(plain));
      }
      else if (VSCP_BLE_ERR_KEY != rv) {
        // Unknown keys are normal (other senders); everything else is shown
        logLine(tr("%1: A2 frame rejected (%2)").arg(dev, vscp_ble_strerror(rv)));
      }
    } break;

    case VSCP_BLE_VERSION_F1: {
      static uint8_t buf[VSCP_BLE_F1_MAX_DATA];
      rv = vscp_ble_f1_rx_feed(m_f1rx.get(), ad, len, static_cast<uint32_t>(m_clock.elapsed()), &ev, buf, sizeof(buf));
      if (VSCP_BLE_OK == rv) {
        updateDeviceRow(info, "F1", ev.guid);
        addEvent(dev, "F1", Trust::Public, ev, tr("%1 octets reassembled").arg(ev.size));
      }
      else if ((VSCP_BLE_PENDING != rv) && (VSCP_BLE_DUPLICATE != rv)) {
        logLine(tr("%1: F1 fragment rejected (%2)").arg(dev, vscp_ble_strerror(rv)));
      }
    } break;

    default:
      if (m_legacy && (0 == (ver & 0x0F))) {
        vscp_ble_l0_event_t l0;
        if (VSCP_BLE_OK == vscp_ble_l0_decode(ad, len, m_company, &l0)) {
          uint8_t addr[6] = { 0 };
          const quint64 a = info.address().toUInt64();
          for (int i = 0; i < 6; i++) {
            addr[i] = static_cast<uint8_t>(a >> (8 * (5 - i)));
          }
          vscp_ble_l0_guid(ev.guid, addr, l0.node_id);
          ev.head       = l0.head;
          ev.vscp_class = l0.vscp_class;
          ev.vscp_type  = l0.vscp_type;
          ev.size       = l0.size;
          ev.data       = l0.data;
          updateDeviceRow(info, "L0", ev.guid);
          addEvent(dev, "L0", Trust::Public, ev, tr("node id %1 (legacy)").arg(l0.node_id));
        }
      }
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Event and device lists
//

void
CFrmBleSession::addEvent(const QString& device,
                         const QString& profile,
                         Trust trust,
                         const vscp_ble_event_t& ev,
                         const QString& details)
{
  if (m_paused) {
    return;
  }
  QString cls  = QString::number(ev.vscp_class);
  QString type = QString::number(ev.vscp_type);
#ifndef VSCPWORKS_BLE_TEST // The regression test runs without the VSCP Works application object
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  auto it           = pworks->m_mapVscpClassToToken.find(ev.vscp_class);
  if (it != pworks->m_mapVscpClassToToken.end()) {
    cls = QString("%1 (%2)").arg(it->second).arg(ev.vscp_class);
  }
  const QString tok = pworks->getShortTypeToken(ev.vscp_class, ev.vscp_type);
  if (!tok.isEmpty()) {
    type = QString("%1 (%2)").arg(tok).arg(ev.vscp_type);
  }
#endif

  QString trustText;
  switch (trust) {
    case Trust::Public:
      trustText = tr("public");
      break;
    case Trust::Authenticated:
      trustText = tr("authenticated");
      break;
    case Trust::Link:
      trustText = tr("secure link");
      break;
  }

  if (m_tableEvents->rowCount() >= MAX_EVENT_ROWS) {
    m_tableEvents->removeRow(0);
  }
  const int row = m_tableEvents->rowCount();
  m_tableEvents->insertRow(row);
  const QStringList cells = { QDateTime::currentDateTime().toString("HH:mm:ss.zzz"),
                              device,
                              profile,
                              trustText,
                              guidToString(ev.guid),
                              cls,
                              type,
                              hex(ev.data, ev.size),
                              details };
  for (int col = 0; col < EV_COUNT; col++) {
    QTableWidgetItem* item = new QTableWidgetItem(cells[col]);
    if (EV_TRUST == col) {
      item->setData(Qt::UserRole, static_cast<int>(trust));
      item->setForeground(trustColor(trust));
      item->setToolTip(Trust::Public == trust ? tr("Unauthenticated: anyone can send this. Never act on it for "
                                                   "anything privileged.")
                                              : tr("Verified sender"));
    }
    m_tableEvents->setItem(row, col, item);
  }
  m_tableEvents->scrollToBottom();
}

void
CFrmBleSession::logLine(const QString& text)
{
  spdlog::debug("BLE session: {}", text.toStdString());
  m_status->setText(text);
}

void
CFrmBleSession::updateDeviceRow(const QBluetoothDeviceInfo& info, const QString& frameKind, const uint8_t* guid)
{
  const QString key = deviceKey(info);
  int row           = -1;
  for (int r = 0; r < m_tableDevices->rowCount(); r++) {
    if (m_tableDevices->item(r, DEV_ADDRESS)->text() == key) {
      row = r;
      break;
    }
  }
  if (row < 0) {
    row = m_tableDevices->rowCount();
    m_tableDevices->insertRow(row);
    for (int col = 0; col < DEV_COUNT; col++) {
      m_tableDevices->setItem(row, col, new QTableWidgetItem());
    }
    m_tableDevices->item(row, DEV_ADDRESS)->setText(key);
    m_tableDevices->item(row, DEV_FRAMES)->setText("0");
  }
  m_tableDevices->item(row, DEV_NAME)->setText(info.name());
  m_tableDevices->item(row, DEV_RSSI)->setText(QString::number(info.rssi()));
  if (info.serviceUuids().contains(g1Uuid(0))) {
    m_tableDevices->item(row, DEV_G1)->setText(tr("yes"));
  }
  if (!frameKind.isEmpty()) {
    m_tableDevices->item(row, DEV_LAST)->setText(
      QString("%1 %2").arg(frameKind, QDateTime::currentDateTime().toString("HH:mm:ss")));
    m_tableDevices->item(row, DEV_FRAMES)->setText(
      QString::number(m_tableDevices->item(row, DEV_FRAMES)->text().toInt() + 1));
  }
  if (nullptr != guid) {
    m_tableDevices->item(row, DEV_GUID)->setText(guidToString(guid));
  }
}

void
CFrmBleSession::clearEvents(void)
{
  m_tableEvents->setRowCount(0);
  m_tableBroadcasts->setRowCount(0);
  m_lastBroadcast.clear();
}

///////////////////////////////////////////////////////////////////////////////
// All advertising
//

void
CFrmBleSession::handleBroadcast(const QBluetoothDeviceInfo& info)
{
  const QMultiHash<quint16, QByteArray> mfg = info.manufacturerData();
  for (auto it = mfg.cbegin(); it != mfg.cend(); ++it) {
    const QString name = companyName(it.key());
    addBroadcast(info,
                 tr("Manufacturer"),
                 name.isEmpty() ? hex16(it.key()) : QString("%1 %2").arg(hex16(it.key()), name),
                 it.value(),
                 describeManufacturerData(it.key(), it.value(), m_company));
  }

  const QMultiHash<QBluetoothUuid, QByteArray> svc = info.serviceData();
  for (auto it = svc.cbegin(); it != svc.cend(); ++it) {
    addBroadcast(info, tr("Service data"), uuidText(it.key()), it.value(), describeServiceData(it.key(), it.value()));
  }

  const QList<QBluetoothUuid> uuids = info.serviceUuids();
  if (!uuids.isEmpty()) {
    QStringList list;
    for (const QBluetoothUuid& u : uuids) {
      list << uuidText(u);
    }
    addBroadcast(info, tr("Services"), QString(), QByteArray(), list.join(", "));
  }

  if (mfg.isEmpty() && svc.isEmpty() && uuids.isEmpty() && !info.name().isEmpty()) {
    addBroadcast(info, tr("Name"), QString(), QByteArray(), QString());
  }
}

void
CFrmBleSession::addBroadcast(const QBluetoothDeviceInfo& info,
                             const QString& kind,
                             const QString& id,
                             const QByteArray& data,
                             const QString& decoded)
{
  if (m_paused) {
    return;
  }
  const QString device = deviceKey(info);

  // Devices repeat their advertising: by default only changes are listed.
  // Services and names have no data, so compare what is shown instead.
  const QString key   = device + "/" + kind + "/" + id;
  const QByteArray v  = data.isEmpty() ? (info.name() + "|" + decoded).toUtf8() : data;
  const bool changed  = !m_lastBroadcast.contains(key) || (m_lastBroadcast.value(key) != v);
  m_lastBroadcast[key] = v;
  if (!changed && m_chkChangesOnly->isChecked()) {
    return;
  }

  if (m_tableBroadcasts->rowCount() >= MAX_BROADCAST_ROWS) {
    m_tableBroadcasts->removeRow(0);
  }
  const int row = m_tableBroadcasts->rowCount();
  m_tableBroadcasts->insertRow(row);
  const QStringList cells = { QDateTime::currentDateTime().toString("HH:mm:ss.zzz"),
                              device,
                              info.name(),
                              QString::number(info.rssi()),
                              kind,
                              id,
                              hexBytes(data),
                              decoded };
  for (int col = 0; col < BC_COUNT; col++) {
    QTableWidgetItem* item = new QTableWidgetItem(cells[col]);
    if ((BC_DATA == col) || (BC_DECODED == col)) {
      item->setToolTip(cells[col]);
    }
    m_tableBroadcasts->setItem(row, col, item);
  }
  const bool visible = broadcastRowMatches(row);
  m_tableBroadcasts->setRowHidden(row, !visible);
  if (visible) {
    m_tableBroadcasts->scrollToBottom();
  }
}

bool
CFrmBleSession::broadcastRowMatches(int row) const
{
  const QString filter = m_editBroadcastFilter->text().trimmed();
  if (filter.isEmpty()) {
    return true;
  }
  for (int col = BC_DEVICE; col < BC_COUNT; col++) {
    const QTableWidgetItem* item = m_tableBroadcasts->item(row, col);
    if ((nullptr != item) && item->text().contains(filter, Qt::CaseInsensitive)) {
      return true;
    }
  }
  // Hex data may be typed without spaces
  QString compact = filter;
  compact.remove(' ');
  const QTableWidgetItem* data = m_tableBroadcasts->item(row, BC_DATA);
  return (nullptr != data) && QString(data->text()).remove(' ').contains(compact, Qt::CaseInsensitive);
}

void
CFrmBleSession::filterBroadcasts(void)
{
  for (int row = 0; row < m_tableBroadcasts->rowCount(); row++) {
    m_tableBroadcasts->setRowHidden(row, !broadcastRowMatches(row));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Sending general advertising
//

bool
CFrmBleSession::advertisingData(QByteArray& ad, QString& error) const
{
  if (m_chkAdvRaw->isChecked()) {
    if (!parseHexBytes(m_editAdvRaw->text(), ad)) {
      error = tr("The raw data must be hex octets");
      return false;
    }
    return checkRawAdvertisingData(ad, error);
  }

  AdvertisingFields f;
  f.flags = m_chkAdvFlags->isChecked();
  f.name  = m_editAdvName->text();
  const QString company = m_editAdvCompany->text().trimmed();
  if (!company.isEmpty()) {
    bool ok             = false;
    const uint value    = company.toUInt(&ok, 0);
    if (!ok || (value > 0xFFFF)) {
      error = tr("The Company Identifier must be a number 0-0xFFFF");
      return false;
    }
    f.hasManufacturer = true;
    f.company         = static_cast<quint16>(value);
    if (!parseHexBytes(m_editAdvMfgData->text(), f.manufacturerData)) {
      error = tr("The manufacturer data must be hex octets");
      return false;
    }
  }
  else if (!m_editAdvMfgData->text().trimmed().isEmpty()) {
    error = tr("Manufacturer data needs a Company Identifier");
    return false;
  }
  if (!m_editAdvServiceUuid->text().trimmed().isEmpty()) {
    if (!parseServiceUuid(m_editAdvServiceUuid->text(), f.serviceUuid)) {
      error = tr("The service UUID must be 4 or 8 hex digits or a 128-bit UUID");
      return false;
    }
    if (!parseHexBytes(m_editAdvServiceData->text(), f.serviceData)) {
      error = tr("The service data must be hex octets");
      return false;
    }
  }
  else if (!m_editAdvServiceData->text().trimmed().isEmpty()) {
    error = tr("Service data needs a service UUID");
    return false;
  }
  return buildAdvertisingData(f, ad, error);
}

void
CFrmBleSession::updateAdvertisingPreview(void)
{
  const bool raw = m_chkAdvRaw->isChecked();
  for (QWidget* w : std::initializer_list<QWidget*>{ m_chkAdvFlags,
                                                     m_editAdvName,
                                                     m_editAdvCompany,
                                                     m_editAdvMfgData,
                                                     m_editAdvServiceUuid,
                                                     m_editAdvServiceData }) {
    w->setEnabled(!raw && !m_advertising);
  }
  m_editAdvRaw->setEnabled(raw && !m_advertising);
  m_chkAdvRaw->setEnabled(!m_advertising);
  m_spinAdvInterval->setEnabled(!m_advertising);

  QByteArray ad;
  QString error;
  const bool ok = advertisingData(ad, error);
  m_lblAdvPreview->setText(ok ? tr("%1/%2 octets: %3").arg(ad.size()).arg(LEGACY_ADV_MAX).arg(hexBytes(ad)) : error);
  m_lblAdvPreview->setProperty("bleRole", ok ? "ok" : "error");
  m_lblAdvPreview->style()->unpolish(m_lblAdvPreview);
  m_lblAdvPreview->style()->polish(m_lblAdvPreview);
  if (!m_advertising) {
#if !defined(Q_OS_WIN)
    m_btnAdvertise->setEnabled(ok);
#endif
  }
}

void
CFrmBleSession::setAdvertising(bool on, const QString& text)
{
  m_advertising = on;
  m_btnAdvertise->setText(on ? tr("Stop advertising") : tr("Start advertising"));
  updateAdvertisingPreview();
  logLine(text);
}

void
CFrmBleSession::toggleAdvertising(void)
{
  if (m_advertising) {
    if (nullptr != m_peripheral) {
      m_peripheral->stopAdvertising();
    }
    setAdvertising(false, tr("Advertising stopped"));
    return;
  }

  QByteArray ad;
  QString error;
  if (!advertisingData(ad, error)) {
    setAdvertising(false, error);
    return;
  }

  QLowEnergyAdvertisingData data;
#if defined(Q_OS_LINUX)
  // BlueZ sends the data as given
  data.setRawData(ad);
#else
  // Other platforms build the data themselves
  data.setDiscoverability(m_chkAdvRaw->isChecked() || m_chkAdvFlags->isChecked()
                            ? QLowEnergyAdvertisingData::DiscoverabilityGeneral
                            : QLowEnergyAdvertisingData::DiscoverabilityNone);
  data.setLocalName(m_editAdvName->text());
  if (!m_editAdvCompany->text().trimmed().isEmpty()) {
    QByteArray mfg;
    parseHexBytes(m_editAdvMfgData->text(), mfg);
    data.setManufacturerData(static_cast<quint16>(m_editAdvCompany->text().trimmed().toUInt(nullptr, 0)), mfg);
  }
  if (m_chkAdvRaw->isChecked() || !m_editAdvServiceUuid->text().trimmed().isEmpty()) {
    setAdvertising(false, tr("Raw data and service data can only be advertised on Linux"));
    return;
  }
#endif

  if (nullptr == m_peripheral) {
    m_peripheral = m_adapter.isNull() ? QLowEnergyController::createPeripheral(this)
                                      : QLowEnergyController::createPeripheral(m_adapter, this);
    connect(m_peripheral, &QLowEnergyController::errorOccurred, this, [this](QLowEnergyController::Error e) {
      QString text = m_peripheral->errorString();
#if defined(Q_OS_LINUX)
      if (QLowEnergyController::AdvertisingError == e) {
        text += " " + tr("(advertising on Linux needs the CAP_NET_ADMIN capability, see the manual)");
      }
#else
      Q_UNUSED(e);
#endif
      m_advErrors++;
      setAdvertising(false, tr("Advertising failed: %1").arg(text));
    });
  }

  QLowEnergyAdvertisingParameters params;
  params.setMode(QLowEnergyAdvertisingParameters::AdvNonConnInd);
  const int interval = m_spinAdvInterval->value();
  params.setInterval(static_cast<quint16>(interval), static_cast<quint16>(interval));
  const int errors = m_advErrors;
  m_peripheral->startAdvertising(params, data);
  if (errors != m_advErrors) {
    return; // Already reported
  }
  setAdvertising(true, tr("Advertising %1 octets every %2 ms").arg(ad.size()).arg(interval));
}

void
CFrmBleSession::showHelp(void)
{
  QDesktopServices::openUrl(QUrl("https://grodansparadis.github.io/vscp-works-qt/#/ble_window"));
}

///////////////////////////////////////////////////////////////////////////////
// G1
//

void
CFrmBleSession::setG1State(const QString& text)
{
  m_g1Status->setText(text);
  logLine(tr("G1: %1").arg(text));
}

void
CFrmBleSession::updateG1Controls(void)
{
  const bool connected = (nullptr != m_ctrl);
  m_actConnect->setEnabled(!connected);
  m_actDisconnect->setEnabled(connected);
  m_btnSend->setEnabled(m_g1Ready && !m_g1Busy);
}

void
CFrmBleSession::connectG1(void)
{
  if (nullptr != m_ctrl) {
    return;
  }

  // The selected device, or the configured G1 node
  QBluetoothDeviceInfo info;
  const int row = m_tableDevices->currentRow();
  if (row >= 0) {
    info = m_devices.value(m_tableDevices->item(row, DEV_ADDRESS)->text());
  }
  if (!info.isValid()) {
    const QBluetoothAddress addr(QString::fromStdString(m_conn.value("device", std::string())));
    if (addr.isNull()) {
      setG1State(tr("Select a device in the list (or set a G1 node in the connection settings)"));
      return;
    }
    info = m_devices.value(addr.toString(), QBluetoothDeviceInfo(addr, QString(), 0));
    info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
  }

  m_g1Device = info;
  m_ctrl     = m_adapter.isNull() ? QLowEnergyController::createCentral(info, this)
                                  : QLowEnergyController::createCentral(info, m_adapter, this);
  connect(m_ctrl, &QLowEnergyController::connected, this, &CFrmBleSession::g1Connected);
  connect(m_ctrl, &QLowEnergyController::disconnected, this, &CFrmBleSession::g1Disconnected);
  connect(m_ctrl, &QLowEnergyController::discoveryFinished, this, &CFrmBleSession::g1DiscoveryFinished);
  connect(m_ctrl, &QLowEnergyController::errorOccurred, this, &CFrmBleSession::g1ControllerError);
  connect(m_ctrl, &QLowEnergyController::mtuChanged, this, [this](int mtu) {
    logLine(tr("G1: ATT MTU %1").arg(mtu));
  });

  m_g1Ready   = false;
  m_g1Busy    = false;
  m_g1NextTid = 1;
  vscp_ble_g1_frag_rx_init(m_g1rx.get(), VSCP_BLE_G1_MAX_RECORD);
  setG1State(tr("Connecting to %1...").arg(deviceKey(info)));
  updateG1Controls();
  m_ctrl->connectToDevice();
}

void
CFrmBleSession::disconnectG1(void)
{
  if (nullptr != m_ctrl) {
    m_ctrl->disconnectFromDevice();
  }
}

void
CFrmBleSession::g1Connected(void)
{
  setG1State(tr("Connected, discovering services..."));
  m_ctrl->discoverServices();
}

void
CFrmBleSession::g1Disconnected(void)
{
  // All G1 state is per connection
  if (nullptr != m_service) {
    m_service->deleteLater();
    m_service = nullptr;
  }
  if (nullptr != m_ctrl) {
    m_ctrl->deleteLater();
    m_ctrl = nullptr;
  }
  m_g1Ready = false;
  m_g1Busy  = false;
  vscp_ble_g1_frag_rx_reset(m_g1rx.get());
  setG1State(tr("Disconnected"));
  updateG1Controls();
}

void
CFrmBleSession::g1ControllerError(QLowEnergyController::Error error)
{
  Q_UNUSED(error);
  setG1State(tr("Error: %1").arg(m_ctrl ? m_ctrl->errorString() : QString()));
  if ((nullptr != m_ctrl) && (QLowEnergyController::UnconnectedState == m_ctrl->state())) {
    g1Disconnected();
  }
}

void
CFrmBleSession::g1DiscoveryFinished(void)
{
  if (!m_ctrl->services().contains(g1Uuid(0))) {
    setG1State(tr("The device has no VSCP G1 service"));
    m_ctrl->disconnectFromDevice();
    return;
  }
  m_service = m_ctrl->createServiceObject(g1Uuid(0), this);
  if (nullptr == m_service) {
    setG1State(tr("Could not use the G1 service"));
    m_ctrl->disconnectFromDevice();
    return;
  }
  connect(m_service, &QLowEnergyService::stateChanged, this, &CFrmBleSession::g1ServiceStateChanged);
  connect(m_service, &QLowEnergyService::characteristicChanged, this, &CFrmBleSession::g1CharacteristicChanged);
  connect(m_service, &QLowEnergyService::characteristicRead, this, &CFrmBleSession::g1CharacteristicRead);
  connect(m_service, &QLowEnergyService::descriptorWritten, this, &CFrmBleSession::g1DescriptorWritten);
  connect(m_service, &QLowEnergyService::errorOccurred, this, &CFrmBleSession::g1ServiceError);
  setG1State(tr("Discovering the G1 characteristics..."));
  m_service->discoverDetails(QLowEnergyService::SkipValueDiscovery);
}

void
CFrmBleSession::g1ServiceStateChanged(QLowEnergyService::ServiceState state)
{
  if (QLowEnergyService::RemoteServiceDiscovered != state) {
    return;
  }
  for (int n : { 1, 2, 3, 4, 5 }) {
    if (!m_service->characteristic(g1Uuid(n)).isValid()) {
      setG1State(tr("The G1 service lacks characteristic %1").arg(g1Uuid(n).toString()));
      m_ctrl->disconnectFromDevice();
      return;
    }
  }
  // Public information first, then subscribe to Control (needs a secure link)
  m_service->readCharacteristic(m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_CAPABILITIES)));
  m_service->readCharacteristic(m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_GUID)));
  const QLowEnergyDescriptor cccd =
    m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_CONTROL)).clientCharacteristicConfiguration();
  setG1State(tr("Enabling Control indications (the node may ask for pairing)..."));
  m_service->writeDescriptor(cccd, CCC_INDICATE);
}

void
CFrmBleSession::g1CharacteristicRead(const QLowEnergyCharacteristic& c, const QByteArray& value)
{
  const uint8_t* v = reinterpret_cast<const uint8_t*>(value.constData());
  if ((c.uuid() == g1Uuid(VSCP_BLE_G1_UUID_CAPABILITIES)) && (VSCP_BLE_G1_CAPABILITIES_LEN == value.size())) {
    logLine(tr("G1 capabilities: version %1, max event data %2, max record %3")
              .arg(v[0])
              .arg(vscp_ble_get16(v + 2))
              .arg(vscp_ble_get16(v + 4)));
  }
  else if ((c.uuid() == g1Uuid(VSCP_BLE_G1_UUID_GUID)) && (16 == value.size())) {
    updateDeviceRow(m_g1Device, QString(), v);
    logLine(tr("G1 node GUID %1").arg(guidToString(v)));
  }
}

void
CFrmBleSession::g1DescriptorWritten(const QLowEnergyDescriptor& d, const QByteArray& value)
{
  if (nullptr == m_service) {
    return;
  }
  const QLowEnergyDescriptor control =
    m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_CONTROL)).clientCharacteristicConfiguration();
  const QLowEnergyDescriptor eventTx =
    m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_EVENT_TX)).clientCharacteristicConfiguration();

  if ((d == control) && (value == CCC_INDICATE)) {
    // Negotiate the limits
    uint8_t payload[6];
    vscp_ble_g1_limits_t limits = { VSCP_BLE_G1_MAX_DATA, VSCP_BLE_G1_MAX_RECORD, 0 };
    vscp_ble_g1_limits_encode(payload, &limits);
    vscp_ble_g1_control_t c = { VSCP_BLE_G1_OP_HELLO, 0, 0, 6, payload };
    uint8_t buf[16];
    size_t len = 0;
    vscp_ble_g1_control_encode(buf, sizeof(buf), &c, &len);
    setG1State(tr("Sending HELLO..."));
    writeG1(g1Uuid(VSCP_BLE_G1_UUID_CONTROL), QByteArray(reinterpret_cast<const char*>(buf), static_cast<int>(len)));
  }
  else if ((d == eventTx) && (value == CCC_NOTIFY)) {
    m_g1Ready = true;
    setG1State(tr("Ready: %1, max event data %2 octets, ATT MTU %3")
                 .arg(deviceKey(m_g1Device))
                 .arg(m_g1MaxData)
                 .arg(m_ctrl->mtu()));
    updateG1Controls();
  }
}

void
CFrmBleSession::g1ServiceError(QLowEnergyService::ServiceError error)
{
  switch (error) {
    case QLowEnergyService::CharacteristicWriteError:
    case QLowEnergyService::DescriptorWriteError:
      setG1State(tr("The node refused a write. G1 needs an encrypted link with authenticated (passkey) pairing "
                    "and the node must authorize this computer. Pair first, then connect again."));
      break;
    case QLowEnergyService::CharacteristicReadError:
      setG1State(tr("A read was refused"));
      break;
    default:
      setG1State(tr("Service error %1").arg(static_cast<int>(error)));
      break;
  }
  if (m_g1Busy) {
    m_g1Busy = false;
    updateG1Controls();
  }
}

bool
CFrmBleSession::writeG1(const QBluetoothUuid& uuid, const QByteArray& value)
{
  if (nullptr == m_service) {
    return false;
  }
  const QLowEnergyCharacteristic c = m_service->characteristic(uuid);
  if (!c.isValid()) {
    return false;
  }
  // Write with response: G1 does not use write without response
  m_service->writeCharacteristic(c, value, QLowEnergyService::WriteWithResponse);
  return true;
}

void
CFrmBleSession::g1CharacteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value)
{
  if (c.uuid() == g1Uuid(VSCP_BLE_G1_UUID_CONTROL)) {
    handleControl(value);
  }
  else if (c.uuid() == g1Uuid(VSCP_BLE_G1_UUID_EVENT_TX)) {
    handleEventTx(value);
  }
}

void
CFrmBleSession::handleControl(const QByteArray& value)
{
  vscp_ble_g1_control_t c;
  if (VSCP_BLE_OK !=
      vscp_ble_g1_control_decode(reinterpret_cast<const uint8_t*>(value.constData()), static_cast<size_t>(value.size()), &c)) {
    logLine(tr("G1: malformed Control indication"));
    return;
  }

  switch (c.opcode) {
    case VSCP_BLE_G1_OP_HELLO_RESULT: {
      vscp_ble_g1_limits_t l;
      if ((VSCP_BLE_G1_OK != c.status) || (6 != c.plen) || (VSCP_BLE_OK != vscp_ble_g1_limits_decode(c.payload, &l))) {
        setG1State(tr("HELLO refused: %1").arg(g1StatusName(c.status)));
        return;
      }
      m_g1MaxData   = l.max_event_data;
      m_g1MaxRecord = l.max_record_size;
      vscp_ble_g1_frag_rx_init(m_g1rx.get(), m_g1MaxRecord);
      const QLowEnergyDescriptor cccd =
        m_service->characteristic(g1Uuid(VSCP_BLE_G1_UUID_EVENT_TX)).clientCharacteristicConfiguration();
      m_service->writeDescriptor(cccd, CCC_NOTIFY);
    } break;

    case VSCP_BLE_G1_OP_EVENT_RESULT:
      m_g1Busy = false;
      logLine(tr("G1: event %1 %2").arg(c.tid).arg(g1StatusName(c.status)));
      if (VSCP_BLE_G1_OK != c.status) {
        m_g1Status->setText(tr("The node rejected event %1: %2").arg(c.tid).arg(g1StatusName(c.status)));
      }
      updateG1Controls();
      break;

    case VSCP_BLE_G1_OP_TX_RESULT:
      if (VSCP_BLE_G1_OK != c.status) {
        logLine(tr("G1: the node could not send event %1 (%2)").arg(c.tid).arg(g1StatusName(c.status)));
      }
      break;

    default:
      break;
  }
}

void
CFrmBleSession::handleEventTx(const QByteArray& value)
{
  size_t reclen  = 0;
  uint8_t status = 0;
  uint32_t tid   = 0;
  const int rv   = vscp_ble_g1_frag_rx_feed(m_g1rx.get(),
                                          reinterpret_cast<const uint8_t*>(value.constData()),
                                          static_cast<size_t>(value.size()),
                                          static_cast<uint32_t>(m_clock.elapsed()),
                                          &tid,
                                          &reclen,
                                          &status);
  if (VSCP_BLE_PENDING == rv) {
    return;
  }
  if (VSCP_BLE_OK != rv) {
    logLine(tr("G1: bad Event TX fragment (%1)").arg(vscp_ble_strerror(rv)));
    return;
  }
  vscp_ble_event_t ev;
  if (VSCP_BLE_OK == vscp_ble_g1_record_decode(m_g1rx->buf, reclen, m_g1MaxData, &tid, &ev)) {
    addEvent(deviceKey(m_g1Device), "G1", Trust::Link, ev, tr("tid %1").arg(tid));
  }
  else {
    logLine(tr("G1: bad event record"));
  }
}

void
CFrmBleSession::sendEvent(void)
{
  if (!m_g1Ready || m_g1Busy) {
    return;
  }
  vscp_ble_event_t ev;
  if (!parseGuid(m_editGuid->text(), ev.guid)) {
    setG1State(tr("The sender GUID must be 16 octets and not all zero"));
    return;
  }
  QByteArray data;
  const QStringList parts = m_editData->text().split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
  for (const QString& p : parts) {
    bool ok       = false;
    const uint v  = p.toUInt(&ok, 0);
    if (!ok || (v > 255)) {
      setG1State(tr("Invalid data octet '%1'").arg(p));
      return;
    }
    data.append(static_cast<char>(v));
  }
  if (data.size() > m_g1MaxData) {
    setG1State(tr("At most %1 data octets on this connection").arg(m_g1MaxData));
    return;
  }
  ev.head       = 0;
  ev.vscp_class = static_cast<uint16_t>(m_spinClass->value());
  ev.vscp_type  = static_cast<uint16_t>(m_spinType->value());
  ev.size       = static_cast<uint16_t>(data.size());
  ev.data       = reinterpret_cast<const uint8_t*>(data.constData());

  const uint32_t tid = m_g1NextTid;
  m_g1NextTid        = (0xFFFFFFFF == m_g1NextTid) ? 1 : m_g1NextTid + 1;
  uint8_t record[VSCP_BLE_G1_MAX_RECORD];
  size_t reclen = 0;
  if (VSCP_BLE_OK != vscp_ble_g1_record_encode(record, sizeof(record), tid, &ev, &reclen)) {
    setG1State(tr("Could not encode the event"));
    return;
  }
  const int mtu = qMax(static_cast<int>(VSCP_BLE_G1_MIN_ATT_MTU), m_ctrl->mtu());
  vscp_ble_g1_frag_tx_t tx;
  vscp_ble_g1_frag_tx_begin(&tx, record, reclen, static_cast<uint16_t>(qMin(mtu, 517)));
  uint8_t frag[VSCP_BLE_G1_FRAG_HEADER + VSCP_BLE_G1_MAX_RECORD];
  size_t flen = 0;
  // Qt queues the writes and sends them in order, each with a response
  while (VSCP_BLE_OK == vscp_ble_g1_frag_tx_next(&tx, frag, sizeof(frag), &flen)) {
    writeG1(g1Uuid(VSCP_BLE_G1_UUID_EVENT_RX), QByteArray(reinterpret_cast<const char*>(frag), static_cast<int>(flen)));
  }
  m_g1Busy      = true; // One outstanding event: wait for EVENT_RESULT
  m_g1BusySince = m_clock.elapsed();
  updateG1Controls();
  logLine(tr("G1: sent event %1 (%2 octets of data)").arg(tid).arg(ev.size));
}

void
CFrmBleSession::g1Tick(void)
{
  const uint32_t now = static_cast<uint32_t>(m_clock.elapsed());
  if (vscp_ble_g1_frag_rx_expired(m_g1rx.get(), now)) {
    logLine(tr("G1: incomplete event from the node discarded (timeout)"));
  }
  if (m_g1Busy && (m_clock.elapsed() - m_g1BusySince > 10000)) {
    m_g1Busy = false;
    setG1State(tr("No EVENT_RESULT from the node within 10 s"));
    updateG1Controls();
  }
}

void
CFrmBleSession::pairDevice(void)
{
  const int row = m_tableDevices->currentRow();
  QBluetoothAddress addr;
  if (row >= 0) {
    addr = QBluetoothAddress(m_tableDevices->item(row, DEV_ADDRESS)->text());
  }
  if (addr.isNull()) {
    addr = QBluetoothAddress(QString::fromStdString(m_conn.value("device", std::string())));
  }
  if (addr.isNull()) {
    setG1State(tr("Select a device to pair with"));
    return;
  }
  if (nullptr == m_localDevice) {
    m_localDevice = m_adapter.isNull() ? new QBluetoothLocalDevice(this) : new QBluetoothLocalDevice(m_adapter, this);
    connect(m_localDevice,
            &QBluetoothLocalDevice::pairingFinished,
            this,
            [this](const QBluetoothAddress& a, QBluetoothLocalDevice::Pairing pairing) {
              setG1State(QBluetoothLocalDevice::Unpaired == pairing ? tr("%1 is not paired").arg(a.toString())
                                                                    : tr("Paired with %1").arg(a.toString()));
            });
    connect(m_localDevice, &QBluetoothLocalDevice::errorOccurred, this, [this](QBluetoothLocalDevice::Error) {
      setG1State(tr("Pairing failed. Pair with the system Bluetooth settings or bluetoothctl "
                    "(passkey entry), then connect again."));
    });
  }
  setG1State(tr("Pairing with %1 (enter the passkey the node shows)...").arg(addr.toString()));
  m_localDevice->requestPairing(addr, QBluetoothLocalDevice::AuthorizedPaired);
}

///////////////////////////////////////////////////////////////////////////////
// Helpers
//

QString
CFrmBleSession::deviceKey(const QBluetoothDeviceInfo& info)
{
  // macOS hides addresses and gives a UUID instead
  return info.address().isNull() ? info.deviceUuid().toString() : info.address().toString();
}

QString
CFrmBleSession::guidToString(const uint8_t* guid)
{
  QString s;
  for (int i = 0; i < 16; i++) {
    if (i) {
      s += ':';
    }
    s += QString("%1").arg(guid[i], 2, 16, QChar('0')).toUpper();
  }
  return s;
}

bool
CFrmBleSession::parseGuid(const QString& str, uint8_t guid[16])
{
  QString h = str.trimmed();
  h.remove(':');
  h.remove('-');
  const QByteArray b = QByteArray::fromHex(h.toLatin1());
  if ((32 != h.length()) || (16 != b.size())) {
    return false;
  }
  memcpy(guid, b.constData(), 16);
  return !vscp_ble_guid_is_zero(guid);
}

QString
CFrmBleSession::hex(const uint8_t* data, size_t len)
{
  QString s;
  for (size_t i = 0; i < len; i++) {
    if (i) {
      s += ' ';
    }
    s += QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper();
  }
  return s;
}
