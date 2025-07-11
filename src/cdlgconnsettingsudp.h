// cdlgconnsettingsudp.h
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

#ifndef CDLGCONNSETTINGSUDP_H
#define CDLGCONNSETTINGSUDP_H

#include "vscp-client-udp.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QList>

namespace Ui {
class CDlgConnSettingsUdp;
}

class CDlgConnSettingsUdp : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgConnSettingsUdp(QWidget* parent = nullptr);
  ~CDlgConnSettingsUdp();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
      Setters/getters for name/description
  */
  QString getName(void);
  void setName(const QString& str);

  /*!
      Setters/getters for interface
  */
  QString getInterface(void);
  void setInterface(const QString& str);
  
  /*!
      Setters/getters for path
  */
  QString getIp(void);
  void setIp(const QString& str);

  /*!
      Setters/getters for encryption
  */
  int getEncryption(void);
  void setEncryption(int encryption);

  /*!
      Setters/getters for key
  */
  QString getKey(void);
  void setKey(const QString& str);

  /*!
      Setters/getters for connection timeout
  */
  uint32_t getConnectionTimeout(void);
  void setConnectionTimeout(uint32_t timeout);

  /*!
      Setters/getters for response timeout
  */
  uint32_t getResponseTimeout(void);
  void setResponseTimeout(uint32_t timeout);

  /*!
      Setters/getters for JSON config object
  */
  json getJson(void);
  void setJson(const json* pobj);

private slots:

  /// Set filter button clicked
  void onSetFilter(void);

  /// Test connection button clicked
  void onTestConnection(void);

  /// Select nwtwork interface button clicked
  void onSelectInterface(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgConnSettingsUdp* ui;

  // JSON configuration object
  json m_jsonConfig;

  /// VSCP tcp/ip client
  vscpClientUdp m_client;

  /// VSCP tcp/ip main filter
  vscpEventFilter m_filter;
};

#endif // CDLGCONNSETTINGSUDP_H
