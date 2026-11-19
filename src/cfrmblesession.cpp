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

#include "cfrmblesession.h"

#include "cdlgconnsettingsble.h"
#ifndef VSCPWORKS_BLE_TEST
#include "vscpworks.h"
#endif

#include "vscp-ble-ad.h"
#include "vscp-ble-crypto.h"
#include "vscp-ble-e1.h"
#include "vscp-ble-l0.h"

#include <QAction>
#include <QBluetoothUuid>
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
#include <QPushButton>
#include <QSaveFile>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QTableWidget>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#include <spdlog/spdlog.h>

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
{
  if (nullptr != pconn) {
    m_conn = *pconn;
  }
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
  resize(1200, 760);

  QToolBar* toolbar = new QToolBar(this);
  m_actScan         = toolbar->addAction(QIcon(":/search.png"), tr("Scan"));
  m_actScan->setCheckable(true);
  m_actScan->setToolTip(tr("Receive VSCP events from Bluetooth LE advertising"));
  m_actPause = toolbar->addAction(tr("Pause"));
  m_actPause->setCheckable(true);
  m_actPause->setToolTip(tr("Stop adding events to the list (they are still received)"));
  QAction* actClear = toolbar->addAction(tr("Clear"));
  toolbar->addSeparator();
  m_actConnect = toolbar->addAction(tr("Connect G1"));
  m_actConnect->setToolTip(tr("Connect to the selected device (or the configured G1 node) over the G1 GATT service"));
  m_actDisconnect = toolbar->addAction(tr("Disconnect"));
  m_actPair       = toolbar->addAction(tr("Pair"));
  m_actPair->setToolTip(tr("Pair with the selected device. G1 needs authenticated (passkey) pairing."));
  toolbar->addSeparator();
  QAction* actHelp = toolbar->addAction(tr("Help"));

  connect(m_actScan, &QAction::triggered, this, &CFrmBleSession::toggleScan);
  connect(m_actPause, &QAction::toggled, this, [this](bool on) { m_paused = on; });
  connect(actClear, &QAction::triggered, this, &CFrmBleSession::clearEvents);
  connect(m_actConnect, &QAction::triggered, this, &CFrmBleSession::connectG1);
  connect(m_actDisconnect, &QAction::triggered, this, &CFrmBleSession::disconnectG1);
  connect(m_actPair, &QAction::triggered, this, &CFrmBleSession::pairDevice);
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
  connect(m_tableDevices, &QTableWidget::cellDoubleClicked, this, [this](int, int) { connectG1(); });

  // Events
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

  // G1 send box
  QGroupBox* g1Box = new QGroupBox(tr("G1 connection"), this);
  m_g1Status       = new QLabel(tr("Not connected"), g1Box);
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
  g1Layout->addWidget(m_g1Status);
  g1Layout->addLayout(g1Form);
  g1Layout->addWidget(m_btnSend);
  g1Layout->addStretch();

  QSplitter* top = new QSplitter(Qt::Horizontal, this);
  top->addWidget(m_tableDevices);
  top->addWidget(g1Box);
  top->setStretchFactor(0, 3);
  top->setStretchFactor(1, 1);

  QSplitter* main = new QSplitter(Qt::Vertical, this);
  main->addWidget(top);
  main->addWidget(m_tableEvents);
  main->setStretchFactor(0, 1);
  main->setStretchFactor(1, 2);

  m_status = new QLabel(this);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->addWidget(toolbar);
  layout->addWidget(main, 1);
  layout->addWidget(m_status);

  updateG1Controls();
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
  QColor color;
  switch (trust) {
    case Trust::Public:
      trustText = tr("public");
      color     = QColor(200, 120, 0);
      break;
    case Trust::Authenticated:
      trustText = tr("authenticated");
      color     = QColor(0, 140, 0);
      break;
    case Trust::Link:
      trustText = tr("secure link");
      color     = QColor(0, 110, 180);
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
      item->setForeground(color);
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
