// cdlgconnsettingsble.h
//
// Settings of a Bluetooth LE connection (VSCP over Bluetooth LE: E1, A2,
// F1 and L0 advertising, G1 GATT).
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

#ifndef CDLGCONNSETTINGSBLE_H
#define CDLGCONNSETTINGSBLE_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QTableWidget>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

/*!
  Connection type number of Bluetooth LE connections. Bluetooth LE is handled
  by VSCP Works itself (not by a CVscpClient), so it has a number outside the
  CVscpClient::connType range.
*/
constexpr int VSCPWORKS_CONNTYPE_BLE = 100;

/*!
  Connection object fields

    type          VSCPWORKS_CONNTYPE_BLE
    name, uuid    as for every connection
    company       Bluetooth SIG Company Identifier (0xFFFF = tests only)
    adapter       Local adapter address, "" = the default adapter
    device        Address of the G1 node to connect to by default ("" = none)
    legacy        Also decode L0 (version 0) legacy advertising
    a2-keys       Provisioned A2 keys (secret, kept in the keyring):
                  [ { "key-id": 1, "guid": "00:01:...", "key": "<32 hex>",
                      "nonce-prefix": "<10 hex>" } ]
*/
class CDlgConnSettingsBle : public QDialog
{
  Q_OBJECT

public:
  explicit CDlgConnSettingsBle(QWidget* parent = nullptr);
  ~CDlgConnSettingsBle();

  void setInitialFocus(void);

  /// Connection name
  QString getName(void);

  /// Connection object from the dialog
  json getJson(void);

  /// Fill the dialog from a connection object
  void setJson(const json* pobj);

private slots:
  void addKey(void);
  void removeKey(void);
  void generateKey(void);
  void validateAndAccept(void);

private:
  /// Check one key row; false and a message if it is invalid
  bool checkKeyRow(int row, QString& err);

  json m_conn;

  QLineEdit* m_editName;
  QLineEdit* m_editCompany;
  QComboBox* m_comboAdapter;
  QLineEdit* m_editDevice;
  QCheckBox* m_chkLegacy;
  QTableWidget* m_tableKeys;
};

#endif // CDLGCONNSETTINGSBLE_H
