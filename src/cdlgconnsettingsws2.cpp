// cdlgconnsettingsws2.cpp
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
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>
#include <vscp-websocket.h>

#include "vscpworks.h"

#include "cdlgconnsettingsws2.h"
#include "ui_cdlgconnsettingsws2.h"

#include "cdlglevel2filter.h"
#include "cdlgtls.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QUrl>

#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsWs2::CDlgConnSettingsWs2(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsWs2)
  , m_bVerifyPeer(true)
{
  ui->setupUi(this);

  // Clear filter
  memset(&m_filter, 0, sizeof(vscp_event_filter_t));

  setConnectionTimeout(WS2_CONNECTION_TIMEOUT);
  setResponseTimeout(WS2_RESPONSE_TIMEOUT);

  setInitialFocus();

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CDlgConnSettingsWs2::accept);
  connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsWs2::onSetFilter);
  connect(ui->btnTLS, &QPushButton::clicked, this, &CDlgConnSettingsWs2::onTLSSettings);
  connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsWs2::onTestConnection);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);
  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsWs2::~CDlgConnSettingsWs2()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsWs2::setInitialFocus(void)
{
  ui->editDescription->setFocus();
}

// ----------------------------------------------------------------------------
// Getters / Setters
// ----------------------------------------------------------------------------

QString
CDlgConnSettingsWs2::getName(void)
{
  return (ui->editDescription->text());
}

void
CDlgConnSettingsWs2::setName(const QString& str)
{
  ui->editDescription->setText(str);
}

QString
CDlgConnSettingsWs2::getUrl(void)
{
  return (ui->editUrl->text().trimmed());
}

void
CDlgConnSettingsWs2::setUrl(const QString& str)
{
  ui->editUrl->setText(str);
}

QString
CDlgConnSettingsWs2::getUser(void)
{
  return (ui->editUsername->text());
}

void
CDlgConnSettingsWs2::setUser(const QString& str)
{
  ui->editUsername->setText(str);
}

QString
CDlgConnSettingsWs2::getPassword(void)
{
  return (ui->editPassword->text());
}

void
CDlgConnSettingsWs2::setPassword(const QString& str)
{
  ui->editPassword->setText(str);
}

QString
CDlgConnSettingsWs2::getKey(void)
{
  return (ui->editKey->text().trimmed());
}

void
CDlgConnSettingsWs2::setKey(const QString& str)
{
  // The default key is shown as an empty field
  if (0 == str.compare(VSCP_WS_DEFAULT_KEY, Qt::CaseInsensitive)) {
    ui->editKey->clear();
  }
  else {
    ui->editKey->setText(str);
  }
}

uint32_t
CDlgConnSettingsWs2::getConnectionTimeout(void)
{
  return (uint32_t) vscp_readStringValue(ui->editConnectTimeout->text().toStdString());
}

void
CDlgConnSettingsWs2::setConnectionTimeout(uint32_t timeout)
{
  ui->editConnectTimeout->setText(QString::number(timeout));
}

uint32_t
CDlgConnSettingsWs2::getResponseTimeout(void)
{
  return (uint32_t) vscp_readStringValue(ui->editResponseTimeout->text().toStdString());
}

void
CDlgConnSettingsWs2::setResponseTimeout(uint32_t timeout)
{
  ui->editResponseTimeout->setText(QString::number(timeout));
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsWs2::getJson(void)
{
  std::string str;

  m_jsonConfig["type"]               = static_cast<int>(CVscpClient::connType::WS2);
  m_jsonConfig["name"]               = getName().toStdString();
  m_jsonConfig["url"]                = getUrl().toStdString();
  m_jsonConfig["user"]               = getUser().toStdString();
  m_jsonConfig["password"]           = getPassword().toStdString();
  m_jsonConfig["key"]                = getKey().toStdString();
  m_jsonConfig["connection-timeout"] = getConnectionTimeout();
  m_jsonConfig["response-timeout"]   = getResponseTimeout();

  // TLS
  m_jsonConfig["bverifypeer"] = m_bVerifyPeer;
  m_jsonConfig["cafile"]      = m_cafile.toStdString();
  m_jsonConfig["capath"]      = m_capath.toStdString();
  m_jsonConfig["certfile"]    = m_certfile.toStdString();
  m_jsonConfig["keyfile"]     = m_keyfile.toStdString();
  m_jsonConfig["pwkeyfile"]   = m_pwkeyfile.toStdString();

  // Filter
  m_jsonConfig["priority-filter"] = m_filter.filter_priority;
  m_jsonConfig["priority-mask"]   = m_filter.mask_priority;
  m_jsonConfig["class-filter"]    = m_filter.filter_class;
  m_jsonConfig["class-mask"]      = m_filter.mask_class;
  m_jsonConfig["type-filter"]     = m_filter.filter_type;
  m_jsonConfig["type-mask"]       = m_filter.mask_type;
  vscp_writeGuidArrayToString(str, m_filter.filter_GUID);
  m_jsonConfig["guid-filter"] = str;
  vscp_writeGuidArrayToString(str, m_filter.mask_GUID);
  m_jsonConfig["guid-mask"] = str;

  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsWs2::setJson(const json* pobj)
{
  if (nullptr == pobj) {
    return;
  }

  m_jsonConfig = *pobj;

  auto getString = [this](const char* name, QString& value) {
    if (m_jsonConfig.contains(name) && m_jsonConfig[name].is_string()) {
      value = QString::fromStdString(m_jsonConfig[name].get<std::string>());
    }
  };

  QString str;

  if (m_jsonConfig.contains("name") && m_jsonConfig["name"].is_string()) {
    setName(m_jsonConfig["name"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("url") && m_jsonConfig["url"].is_string() &&
      !m_jsonConfig["url"].get<std::string>().empty()) {
    setUrl(m_jsonConfig["url"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("user") && m_jsonConfig["user"].is_string()) {
    setUser(m_jsonConfig["user"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("password") && m_jsonConfig["password"].is_string()) {
    setPassword(m_jsonConfig["password"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("key") && m_jsonConfig["key"].is_string()) {
    setKey(m_jsonConfig["key"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("connection-timeout") && m_jsonConfig["connection-timeout"].is_number()) {
    setConnectionTimeout(m_jsonConfig["connection-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("response-timeout") && m_jsonConfig["response-timeout"].is_number()) {
    setResponseTimeout(m_jsonConfig["response-timeout"].get<uint32_t>());
  }

  // TLS
  if (m_jsonConfig.contains("bverifypeer") && m_jsonConfig["bverifypeer"].is_boolean()) {
    m_bVerifyPeer = m_jsonConfig["bverifypeer"].get<bool>();
  }
  getString("cafile", m_cafile);
  getString("capath", m_capath);
  getString("certfile", m_certfile);
  getString("keyfile", m_keyfile);
  getString("pwkeyfile", m_pwkeyfile);

  // Filter
  memset(&m_filter, 0, sizeof(vscp_event_filter_t));
  if (m_jsonConfig.contains("priority-filter") && m_jsonConfig["priority-filter"].is_number()) {
    m_filter.filter_priority = m_jsonConfig["priority-filter"].get<uint8_t>();
  }

  if (m_jsonConfig.contains("priority-mask") && m_jsonConfig["priority-mask"].is_number()) {
    m_filter.mask_priority = m_jsonConfig["priority-mask"].get<uint8_t>();
  }

  if (m_jsonConfig.contains("class-filter") && m_jsonConfig["class-filter"].is_number()) {
    m_filter.filter_class = m_jsonConfig["class-filter"].get<uint16_t>();
  }

  if (m_jsonConfig.contains("class-mask") && m_jsonConfig["class-mask"].is_number()) {
    m_filter.mask_class = m_jsonConfig["class-mask"].get<uint16_t>();
  }

  if (m_jsonConfig.contains("type-filter") && m_jsonConfig["type-filter"].is_number()) {
    m_filter.filter_type = m_jsonConfig["type-filter"].get<uint16_t>();
  }

  if (m_jsonConfig.contains("type-mask") && m_jsonConfig["type-mask"].is_number()) {
    m_filter.mask_type = m_jsonConfig["type-mask"].get<uint16_t>();
  }

  if (m_jsonConfig.contains("guid-filter") && m_jsonConfig["guid-filter"].is_string()) {
    vscp_getGuidFromStringToArray(m_filter.filter_GUID, m_jsonConfig["guid-filter"].get<std::string>());
  }

  if (m_jsonConfig.contains("guid-mask") && m_jsonConfig["guid-mask"].is_string()) {
    vscp_getGuidFromStringToArray(m_filter.mask_GUID, m_jsonConfig["guid-mask"].get<std::string>());
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// validate
//

bool
CDlgConnSettingsWs2::validate(QString& strError)
{
  vscpWsUrl url;
  if (!CVscpWebSocket::parseUrl(getUrl().toStdString(), "/ws2", url)) {
    strError = tr("The host URL is not valid. Use ws://host:port/ws2, or wss://host:port/ws2 for TLS.");
    ui->editUrl->setFocus();
    return false;
  }

  QString key = getKey();
  if (!key.isEmpty()) {
    static const QRegularExpression hex("^[0-9a-fA-F]{32}$");
    if (!hex.match(key).hasMatch()) {
      strError = tr("The key must be 32 hexadecimal characters, or empty for the default key.");
      ui->editKey->setFocus();
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgConnSettingsWs2::accept(void)
{
  QString strError;
  if (!validate(strError)) {
    QMessageBox::warning(this, tr(APPNAME), strError);
    return;
  }
  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// onSetFilter
//

void
CDlgConnSettingsWs2::onSetFilter(void)
{
  CDlgLevel2Filter dlg;

  dlg.setFilter(&m_filter);

  if (QDialog::Accepted == dlg.exec()) {
    dlg.getFilter(&m_filter);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onTLSSettings
//

void
CDlgConnSettingsWs2::onTLSSettings(void)
{
  CDlgTLS dlg;

  QString strUrl = getUrl();
  bool bTLS      = strUrl.startsWith("wss://", Qt::CaseInsensitive) || strUrl.startsWith("https://", Qt::CaseInsensitive);

  dlg.enableTLS(bTLS);
  dlg.enableVerifyPeer(m_bVerifyPeer);
  dlg.setCaFile(m_cafile);
  dlg.setCaPath(m_capath);
  dlg.setCertFile(m_certfile);
  dlg.setKeyFile(m_keyfile);
  dlg.setPwKeyFile(m_pwkeyfile);

  if (QDialog::Accepted == dlg.exec()) {
    m_bVerifyPeer = dlg.isVerifyPeerEnabled();
    m_cafile      = dlg.getCaFile();
    m_capath      = dlg.getCaPath();
    m_certfile    = dlg.getCertFile();
    m_keyfile     = dlg.getKeyFile();
    m_pwkeyfile   = dlg.getPwKeyFile();

    // TLS is selected by the URL scheme, keep it in step
    if (dlg.isTLSEnabled() != bTLS) {
      int pos = strUrl.indexOf("://");
      QString rest = (pos >= 0) ? strUrl.mid(pos + 3) : strUrl;
      setUrl((dlg.isTLSEnabled() ? "wss://" : "ws://") + rest);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void
CDlgConnSettingsWs2::onTestConnection(void)
{
  QString strError;
  if (!validate(strError)) {
    QMessageBox::warning(this, tr(APPNAME), strError);
    return;
  }

  vscpClientWs2 client;
  if (!client.initFromJson(getJson().dump())) {
    QMessageBox::warning(this, tr(APPNAME), tr("Invalid connection settings."));
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  int rv = client.connect();
  if (VSCP_ERROR_SUCCESS != rv) {
    QApplication::restoreOverrideCursor();
    QString msg = tr("Failed to connect to the remote host.");
    if (VSCP_ERROR_USER == rv) {
      msg = tr("The server did not accept the user name, password or key.");
    }
    QString err = QString::fromStdString(client.getLastError());
    if (!err.isEmpty()) {
      msg += "\n\n" + err;
    }
    QMessageBox::warning(this, tr(APPNAME), msg);
    return;
  }

  QString strVersion;
  uint8_t major, minor, release, build;
  if (VSCP_ERROR_SUCCESS == client.getversion(&major, &minor, &release, &build)) {
    strVersion = tr("Server ws2 protocol version: %1.%2.%3.%4").arg(major).arg(minor).arg(release).arg(build);
  }

  client.disconnect();
  QApplication::restoreOverrideCursor();

  QString msg = tr("Connection test was successful");
  if (!strVersion.isEmpty()) {
    msg += "\n\n" + strVersion;
  }
  QMessageBox::information(this, tr(APPNAME), msg);
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgConnSettingsWs2::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connection_ws2";
  QDesktopServices::openUrl(QUrl(link));
}
