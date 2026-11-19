// ble_session_regressions.cpp
//
// Feeds advertising reports built from the vscp-ble interoperability
// vectors to the Bluetooth LE session window and checks the event list.
// No Bluetooth hardware is needed.

#include "../src/cfrmblesession.cpp"

#include <QApplication>
#include <QStandardPaths>
#include <QTableWidget>

#include <cstdio>

namespace {

int failures = 0;

void
check(bool ok, const char* what)
{
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) {
    failures++;
  }
}

QTableWidget*
eventTable(CFrmBleSession& w)
{
  for (QTableWidget* t : w.findChildren<QTableWidget*>()) {
    if (EV_COUNT == t->columnCount()) {
      return t;
    }
  }
  return nullptr;
}

// One advertising report from a device with Manufacturer Specific Data
void
report(CFrmBleSession& w, const QString& address, const QByteArray& frame)
{
  QBluetoothDeviceInfo info{ QBluetoothAddress(address), "vscp-node", 0 };
  info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
  // The stack reports the data after the Company Identifier
  const quint16 company = static_cast<uint8_t>(frame[2]) | (static_cast<uint8_t>(frame[3]) << 8);
  info.setManufacturerData(company, frame.mid(4));
  QMetaObject::invokeMethod(&w, "deviceSeen", Qt::DirectConnection, Q_ARG(QBluetoothDeviceInfo, info));
}

QString
cell(QTableWidget* t, int row, int col)
{
  return (row < t->rowCount() && t->item(row, col)) ? t->item(row, col)->text() : QString();
}

} // namespace

int
main(int argc, char** argv)
{
  QStandardPaths::setTestModeEnabled(true);
  QApplication app(argc, argv);

  json conn;
  conn["type"]    = VSCPWORKS_CONNTYPE_BLE;
  conn["name"]    = "BLE test";
  conn["uuid"]    = "ble-session-regression";
  conn["company"] = 0xFFFF;
  conn["legacy"]  = true;
  conn["a2-keys"] = json::array({ { { "key-id", 1 },
                                    { "guid", "00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F" },
                                    { "key", "000102030405060708090a0b0c0d0e0f" },
                                    { "nonce-prefix", "a0a1a2a3a4" } } });

  // Start from no replay state
  CFrmBleSession probe(nullptr, &conn);
  QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                "/ble-replay-ble-session-regression.json");

  CFrmBleSession w(nullptr, &conn);
  QTableWidget* events = eventTable(w);
  check(nullptr != events, "event table exists");
  if (nullptr == events) {
    return 1;
  }

  // E1 vector B
  const QByteArray e1 = QByteArray::fromHex("23ffffff010000000001000102030405060708090a0b0c0d0e0f0000000a000600021122");
  report(w, "C0:11:22:33:44:55", e1);
  check(1 == events->rowCount(), "E1 vector B gives one event");
  check("E1" == cell(events, 0, EV_PROFILE) && "11 22" == cell(events, 0, EV_DATA), "E1 profile and data");
  check(cell(events, 0, EV_GUID).startsWith("00:01:02:03"), "E1 GUID");
  check(cell(events, 0, EV_TRUST) == "public", "E1 is public");
  report(w, "C0:11:22:33:44:55", e1);
  check(1 == events->rowCount(), "an unchanged report is not shown again");

  // A2 known answer (key 1, epoch 1, sequence 2)
  const QByteArray a2 = QByteArray::fromHex("3affffff020100010000000100000002000102030405060708090a0b0c0d0e0f0000000a00060003"
                                            "3c4532039788b44a4ad321c6493748c0f6c4d1");
  report(w, "C0:11:22:33:44:55", a2);
  check(2 == events->rowCount(), "A2 vector gives one event");
  check("A2" == cell(events, 1, EV_PROFILE) && "11 22 33" == cell(events, 1, EV_DATA), "A2 decrypted data");
  check("authenticated" == cell(events, 1, EV_TRUST), "A2 is authenticated");

  // The same A2 frame relayed by another device is a replay
  report(w, "C0:11:22:33:44:66", a2);
  check(2 == events->rowCount(), "replayed A2 frame rejected");

  // A modified A2 frame
  QByteArray bad = a2;
  bad[bad.size() - 1] = static_cast<char>(bad[bad.size() - 1] ^ 1);
  report(w, "C0:11:22:33:44:77", bad);
  check(2 == events->rowCount(), "modified A2 frame rejected");

  // A new window keeps the replay state (persisted)
  {
    CFrmBleSession w2(nullptr, &conn);
    report(w2, "C0:11:22:33:44:55", a2);
    check(0 == eventTable(w2)->rowCount(), "replay state survives a restart");
  }

  // F1: 223-octet event in two fragments
  QByteArray data(223, 0);
  for (int i = 0; i < data.size(); i++) {
    data[i] = static_cast<char>(i);
  }
  vscp_ble_event_t ev = {};
  for (int i = 0; i < 16; i++) {
    ev.guid[i] = static_cast<uint8_t>(i);
  }
  ev.vscp_class = 10;
  ev.vscp_type  = 6;
  ev.size       = static_cast<uint16_t>(data.size());
  ev.data       = reinterpret_cast<const uint8_t*>(data.constData());
  for (unsigned i = 0; i < vscp_ble_f1_fragment_count(ev.size); i++) {
    uint8_t buf[256];
    size_t len = 0;
    vscp_ble_f1_encode(buf, sizeof(buf), 0xFFFF, 7, &ev, i, &len);
    report(w, "C0:11:22:33:44:88", QByteArray(reinterpret_cast<const char*>(buf), static_cast<int>(len)));
  }
  check(3 == events->rowCount(), "F1 event reassembled once");
  check("F1" == cell(events, 2, EV_PROFILE) && cell(events, 2, EV_DATA).startsWith("00 01 02") &&
          cell(events, 2, EV_DATA).endsWith("DE"),
        "F1 data");

  // L0 (legacy enabled in this connection)
  report(w, "C0:11:22:33:44:99", QByteArray::fromHex("14ffffff00123410000a000602aabb000000000000"));
  check(4 == events->rowCount(), "L0 frame decoded");
  check("L0" == cell(events, 3, EV_PROFILE) &&
          "FF:FF:FF:FF:FF:FF:FF:F8:C0:11:22:33:44:99:12:34" == cell(events, 3, EV_GUID),
        "L0 derived GUID");

  // Other Company Identifiers are ignored
  QByteArray other = e1;
  other[2]         = 0x34;
  other[3]         = 0x12;
  report(w, "C0:11:22:33:44:AA", other);
  check(4 == events->rowCount(), "other Company Identifier ignored");

  std::printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
