// cdlgconnsettingsble.cpp
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

#include "cdlgconnsettingsble.h"

#include <QBluetoothLocalDevice>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace {

enum KeyColumn
{
  COL_KEYID = 0,
  COL_GUID,
  COL_KEY,
  COL_PREFIX,
  COL_COUNT
};

bool
isHex(const QString& s, int octets)
{
  static const QRegularExpression re("^[0-9A-Fa-f]*$");
  return (s.length() == 2 * octets) && re.match(s).hasMatch();
}

// "00:01:..." or "0001..." -> 32 hex digits, "" if invalid or all zero
QString
normalizeGuid(const QString& s)
{
  QString h = s.trimmed();
  h.remove(':');
  h.remove('-');
  if (!isHex(h, 16) || (QString(32, '0') == h)) {
    return QString();
  }
  QString out;
  for (int i = 0; i < 32; i += 2) {
    if (i) {
      out += ':';
    }
    out += h.mid(i, 2).toUpper();
  }
  return out;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// CDlgConnSettingsBle
//

CDlgConnSettingsBle::CDlgConnSettingsBle(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Bluetooth LE connection"));
  resize(760, 520);

  m_editName    = new QLineEdit(this);
  m_editCompany = new QLineEdit("0xFFFF", this);
  m_editCompany->setToolTip(tr("Bluetooth SIG Company Identifier of the VSCP frames. "
                               "0xFFFF is for tests only."));
  m_comboAdapter = new QComboBox(this);
  m_comboAdapter->addItem(tr("Default adapter"), QString());
  for (const QBluetoothHostInfo& host : QBluetoothLocalDevice::allDevices()) {
    m_comboAdapter->addItem(QString("%1 (%2)").arg(host.name(), host.address().toString()),
                            host.address().toString());
  }
  m_editDevice = new QLineEdit(this);
  m_editDevice->setPlaceholderText(tr("Optional: address of a G1 node, e.g. AA:BB:CC:DD:EE:FF"));
  m_chkLegacy = new QCheckBox(tr("Also decode legacy L0 (version 0) advertising"), this);

  QFormLayout* form = new QFormLayout;
  form->addRow(tr("Name:"), m_editName);
  form->addRow(tr("Company Identifier:"), m_editCompany);
  form->addRow(tr("Adapter:"), m_comboAdapter);
  form->addRow(tr("G1 node:"), m_editDevice);
  form->addRow(QString(), m_chkLegacy);

  // A2 keys
  QGroupBox* keysBox = new QGroupBox(tr("A2 keys (authenticated, encrypted advertising)"), this);
  m_tableKeys        = new QTableWidget(0, COL_COUNT, keysBox);
  m_tableKeys->setHorizontalHeaderLabels({ tr("Key id"), tr("Sender GUID"), tr("AES key (32 hex)"),
                                           tr("Nonce prefix (10 hex)") });
  m_tableKeys->horizontalHeader()->setSectionResizeMode(COL_GUID, QHeaderView::Stretch);
  m_tableKeys->horizontalHeader()->setSectionResizeMode(COL_KEY, QHeaderView::Stretch);
  m_tableKeys->setSelectionBehavior(QAbstractItemView::SelectRows);

  QPushButton* btnAdd      = new QPushButton(tr("Add"), keysBox);
  QPushButton* btnRemove   = new QPushButton(tr("Remove"), keysBox);
  QPushButton* btnGenerate = new QPushButton(tr("New random key"), keysBox);
  btnGenerate->setToolTip(tr("Fill the selected row with a new random AES key and nonce prefix. "
                             "The same values must be provisioned in the node."));
  connect(btnAdd, &QPushButton::clicked, this, &CDlgConnSettingsBle::addKey);
  connect(btnRemove, &QPushButton::clicked, this, &CDlgConnSettingsBle::removeKey);
  connect(btnGenerate, &QPushButton::clicked, this, &CDlgConnSettingsBle::generateKey);

  QHBoxLayout* keyButtons = new QHBoxLayout;
  keyButtons->addWidget(btnAdd);
  keyButtons->addWidget(btnRemove);
  keyButtons->addWidget(btnGenerate);
  keyButtons->addStretch();

  QLabel* keyInfo = new QLabel(tr("Keys are stored in the system keyring, not in the configuration file. "
                                  "They are never part of exported connection data."),
                               keysBox);
  keyInfo->setWordWrap(true);

  QVBoxLayout* keysLayout = new QVBoxLayout(keysBox);
  keysLayout->addWidget(m_tableKeys);
  keysLayout->addLayout(keyButtons);
  keysLayout->addWidget(keyInfo);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &CDlgConnSettingsBle::validateAndAccept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addWidget(keysBox, 1);
  layout->addWidget(buttons);
}

CDlgConnSettingsBle::~CDlgConnSettingsBle() {}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsBle::setInitialFocus(void)
{
  m_editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgConnSettingsBle::getName(void)
{
  return m_editName->text().trimmed();
}

///////////////////////////////////////////////////////////////////////////////
// addKey / removeKey / generateKey
//

void
CDlgConnSettingsBle::addKey(void)
{
  const int row = m_tableKeys->rowCount();
  m_tableKeys->insertRow(row);
  m_tableKeys->setItem(row, COL_KEYID, new QTableWidgetItem(QString::number(row + 1)));
  m_tableKeys->setItem(row, COL_GUID, new QTableWidgetItem());
  m_tableKeys->setItem(row, COL_KEY, new QTableWidgetItem());
  m_tableKeys->setItem(row, COL_PREFIX, new QTableWidgetItem());
  m_tableKeys->setCurrentCell(row, COL_GUID);
  m_tableKeys->editItem(m_tableKeys->item(row, COL_GUID));
}

void
CDlgConnSettingsBle::removeKey(void)
{
  const int row = m_tableKeys->currentRow();
  if (row >= 0) {
    m_tableKeys->removeRow(row);
  }
}

void
CDlgConnSettingsBle::generateKey(void)
{
  int row = m_tableKeys->currentRow();
  if (row < 0) {
    addKey();
    row = m_tableKeys->rowCount() - 1;
  }
  // QRandomGenerator::system() is a cryptographically secure source
  auto randomHex = [](int octets) {
    QString s;
    for (int i = 0; i < octets; i++) {
      s += QString("%1").arg(QRandomGenerator::system()->bounded(256), 2, 16, QChar('0'));
    }
    return s;
  };
  m_tableKeys->item(row, COL_KEY)->setText(randomHex(16));
  m_tableKeys->item(row, COL_PREFIX)->setText(randomHex(5));
}

///////////////////////////////////////////////////////////////////////////////
// checkKeyRow
//

bool
CDlgConnSettingsBle::checkKeyRow(int row, QString& err)
{
  auto text = [this, row](int col) {
    QTableWidgetItem* item = m_tableKeys->item(row, col);
    return (nullptr == item) ? QString() : item->text().trimmed();
  };
  bool ok         = false;
  const int keyId = text(COL_KEYID).toInt(&ok, 0);
  if (!ok || (keyId < 1) || (keyId > 0xFFFF)) {
    err = tr("Row %1: the key id must be 1-65535.").arg(row + 1);
    return false;
  }
  if (normalizeGuid(text(COL_GUID)).isEmpty()) {
    err = tr("Row %1: the sender GUID must be 16 octets and not all zero.").arg(row + 1);
    return false;
  }
  if (!isHex(text(COL_KEY), 16)) {
    err = tr("Row %1: the AES key must be 32 hexadecimal digits.").arg(row + 1);
    return false;
  }
  if (!isHex(text(COL_PREFIX), 5)) {
    err = tr("Row %1: the nonce prefix must be 10 hexadecimal digits.").arg(row + 1);
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validateAndAccept
//

void
CDlgConnSettingsBle::validateAndAccept(void)
{
  bool ok = false;
  const uint company = m_editCompany->text().trimmed().toUInt(&ok, 0);
  if (!ok || (company > 0xFFFF)) {
    QMessageBox::warning(this, windowTitle(), tr("The Company Identifier must be 0-0xFFFF."));
    return;
  }
  const QString device = m_editDevice->text().trimmed();
  if (!device.isEmpty() && QBluetoothAddress(device).isNull()) {
    QMessageBox::warning(this, windowTitle(), tr("The G1 node address is not a Bluetooth address."));
    return;
  }
  for (int row = 0; row < m_tableKeys->rowCount(); row++) {
    QString err;
    if (!checkKeyRow(row, err)) {
      QMessageBox::warning(this, windowTitle(), err);
      return;
    }
  }
  accept();
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsBle::getJson(void)
{
  m_conn["type"]    = VSCPWORKS_CONNTYPE_BLE;
  m_conn["name"]    = getName().toStdString();
  m_conn["company"] = m_editCompany->text().trimmed().toUInt(nullptr, 0) & 0xFFFF;
  m_conn["adapter"] = m_comboAdapter->currentData().toString().toStdString();
  m_conn["device"]  = m_editDevice->text().trimmed().toUpper().toStdString();
  m_conn["legacy"]  = m_chkLegacy->isChecked();

  json keys = json::array();
  for (int row = 0; row < m_tableKeys->rowCount(); row++) {
    QString err;
    if (!checkKeyRow(row, err)) {
      continue;
    }
    json k;
    k["key-id"]       = m_tableKeys->item(row, COL_KEYID)->text().trimmed().toInt(nullptr, 0);
    k["guid"]         = normalizeGuid(m_tableKeys->item(row, COL_GUID)->text()).toStdString();
    k["key"]          = m_tableKeys->item(row, COL_KEY)->text().trimmed().toLower().toStdString();
    k["nonce-prefix"] = m_tableKeys->item(row, COL_PREFIX)->text().trimmed().toLower().toStdString();
    keys.push_back(k);
  }
  m_conn["a2-keys"] = keys;
  return m_conn;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsBle::setJson(const json* pobj)
{
  if (nullptr == pobj) {
    return;
  }
  m_conn = *pobj;

  m_editName->setText(QString::fromStdString(m_conn.value("name", std::string())));
  const int company = m_conn.contains("company") && m_conn["company"].is_number_integer()
                        ? m_conn["company"].get<int>()
                        : 0xFFFF;
  m_editCompany->setText(QString("0x%1").arg(company, 4, 16, QChar('0')).toUpper().replace("0X", "0x"));

  const QString adapter = QString::fromStdString(m_conn.value("adapter", std::string()));
  int idx               = m_comboAdapter->findData(adapter);
  if ((idx < 0) && !adapter.isEmpty()) {
    // Adapter not present now: keep the setting
    m_comboAdapter->addItem(tr("%1 (not present)").arg(adapter), adapter);
    idx = m_comboAdapter->count() - 1;
  }
  m_comboAdapter->setCurrentIndex(idx < 0 ? 0 : idx);

  m_editDevice->setText(QString::fromStdString(m_conn.value("device", std::string())));
  m_chkLegacy->setChecked(m_conn.value("legacy", false));

  m_tableKeys->setRowCount(0);
  if (m_conn.contains("a2-keys") && m_conn["a2-keys"].is_array()) {
    for (const json& k : m_conn["a2-keys"]) {
      if (!k.is_object()) {
        continue;
      }
      const int row = m_tableKeys->rowCount();
      m_tableKeys->insertRow(row);
      m_tableKeys->setItem(row, COL_KEYID, new QTableWidgetItem(QString::number(k.value("key-id", 0))));
      m_tableKeys->setItem(row, COL_GUID, new QTableWidgetItem(QString::fromStdString(k.value("guid", std::string()))));
      m_tableKeys->setItem(row, COL_KEY, new QTableWidgetItem(QString::fromStdString(k.value("key", std::string()))));
      m_tableKeys->setItem(row,
                           COL_PREFIX,
                           new QTableWidgetItem(QString::fromStdString(k.value("nonce-prefix", std::string()))));
    }
  }
}
