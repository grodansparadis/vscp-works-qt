// cdlgconnsettingsrawmqtt.h
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

#ifndef CDLGCONNSETTINGSRAWMQTT_H
#define CDLGCONNSETTINGSRAWMQTT_H

#include "vscphelper.h"
#include "vscp-client-mqtt.h"

#include <QDialog>
#include <QListWidgetItem>

// ----------------------------------------------------------------------------

namespace Ui {
class CDlgConnSettingsRawMqtt;
}

class CDlgConnSettingsRawMqtt : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgConnSettingsRawMqtt(QWidget* parent = nullptr);
  ~CDlgConnSettingsRawMqtt();

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
      Setters/getters for host/broker
  */
  QString getBroker(void);
  void setBroker(const QString& str);

  /*!
      Setters/getters for port
  */
  short getPort(void);
  void setPort(short port);

  /*!
      Setters/getters for user
  */
  QString getUser(void);
  void setUser(const QString& str);

  /*!
      Setters/getters for password
  */
  QString getPassword(void);
  void setPassword(const QString& str);
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
      Setters/getters for keepalive period
  */
  uint32_t getKeepAlive(void);
  void setKeepAlive(uint32_t timeout);

  /*!
      Setters/getters for extended security
  */
  bool isExtendedSecurityEnabled(void);
  void enableExtendedSecurity(bool bExtendedSecurity);

  /*!
      Setters/getters for TLS enable
  */
  bool isTLSEnabled(void);
  void enableTLS(bool bTLS);

  /*!
      Setters/getters for TLS enable
  */
  bool isVerifyPeerEnabled(void);
  void enableVerifyPeer(bool bverifypeer);

  /*!
      Setters/getters for TLS CA file
  */
  QString getCaFile(void);
  void setCaFile(const QString& str);

  /*!
      Setters/getters for TLS CA path
  */
  QString getCaPath(void);
  void setCaPath(const QString& str);

  /*!
      Setters/getters for TLS Cert file
  */
  QString getCertFile(void);
  void setCertFile(const QString& str);

  /*!
      Setters/getters for TLS key file
  */
  QString getKeyFile(void);
  void setKeyFile(const QString& str);

  /*!
      Setters/getters for TLS pw key file
  */
  QString getPwKeyFile(void);
  void setPwKeyFile(const QString& str);

  /*!
      Setters/getters for JSON config object
  */
  json getJson(void);
  void setJson(const json* pobj);

private slots:

  /// Add subscription
  void onAddSubscription(void);

  /// Edit subscription
  void onEditSubscription(void);

  /// Clone subscription
  void onCloneSubscription(void);

  /// Delete subscription
  void onDeleteSubscription(void);

  /// Add publish
  void onAddPublish(void);

  /// Edit publish
  void onEditPublish(void);

  /// Clone publish
  void onClonePublish(void);

  /// Delete publish
  void onDeletePublish(void);

  /// Test connection button clicked
  void onTestConnection(void);

  /// TLS settings button clicked
  void onTLSSettings(void);

  /// Get help with settings
  void onGetHelp(void);

  /// Subscription context menu
  void onSubscribeContextMenu(const QPoint& pos);

  /// Publish context menu
  void onPublishContextMenu(const QPoint& pos);

  /// Show help
  void showHelp(void);

private:
  Ui::CDlgConnSettingsRawMqtt* ui;

  /// TLS flag (secure transport if enabled)
  bool m_bTLS;

  /*!
      the server certificate will be verified and the connection
      aborted if the verification fails.
  */
  bool m_bVerifyPeer;

  /// CA file
  QString m_cafile;

  /// Path to CA file (can hold filename also)
  QString m_capath;

  /// Path to CERT file
  QString m_certfile;

  /// Key file
  QString m_keyfile;

  /// Password keyfile
  QString m_pwkeyfile;

  /*!
      This variable holds the connection type that
      the used select
  */
  CVscpClient::connType m_selected_type;

  // JSON configuration object
  json m_jsonConfig;

  /// VSCP MQTT client
  vscpClientMqtt m_client;
};

#endif // CDLGCONNSETTINGSRAWMQTT_H
