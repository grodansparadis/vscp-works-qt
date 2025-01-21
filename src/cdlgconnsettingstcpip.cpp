// cdlgconnsettingstcpip.cpp
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include "vscpworks.h"

#include "cdlgconnsettingstcpip.h"
#include "ui_cdlgconnsettingstcpip.h"

#include "cdlglevel2filter.h"
#include "cdlgtls.h"

#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsTcpip::CDlgConnSettingsTcpip(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsTcpip)
{
  ui->setupUi(this);

  // Set defaults
  m_bTLS = false;
  setConnectionTimeout(TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS);
  setResponseTimeout(TCPIP_DEFAULT_RESPONSE_TIMEOUT);
  setHost("tcp://localhost:9598");
  setUser("admin");
  setPassword("secret");

  // Clear filter
  memset(&m_filter, 0, sizeof(vscpEventFilter));

  connect(ui->btnTLS, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onTLSSettings);
  connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onSetFilter);
  connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onTestConnection);
  connect(ui->btnGetInterfaces, &QPushButton::clicked, this, &CDlgConnSettingsTcpip::onGetInterfaces);

  // Help button
  connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &CDlgConnSettingsTcpip::onGetHelp);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsTcpip::~CDlgConnSettingsTcpip()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsTcpip::setInitialFocus(void)
{
  ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onGetHelp
//

void
CDlgConnSettingsTcpip::onGetHelp()
{
  QUrl helpUrl("https://docs.vscp.org/");
  QDesktopServices::openUrl(helpUrl);
}

// Getters / Setters

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgConnSettingsTcpip::getName(void)
{
  return (ui->editName->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgConnSettingsTcpip::setName(const QString& str)
{
  ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getHost
//

QString
CDlgConnSettingsTcpip::getHost(void)
{
  return (ui->editHost->text());
}

///////////////////////////////////////////////////////////////////////////////
// setHost
//

void
CDlgConnSettingsTcpip::setHost(const QString& str)
{
  ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString
CDlgConnSettingsTcpip::getUser(void)
{
  return (ui->editUser->text());
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void
CDlgConnSettingsTcpip::setUser(const QString& str)
{
  ui->editUser->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString
CDlgConnSettingsTcpip::getPassword(void)
{
  return (ui->editPassword->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void
CDlgConnSettingsTcpip::setPassword(const QString& str)
{
  ui->editPassword->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getInterface
//

QString
CDlgConnSettingsTcpip::getInterface(void)
{
  return ui->comboInterface->currentText();
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//

void
CDlgConnSettingsTcpip::setInterface(const QString& str)
{
  ui->comboInterface->setCurrentText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
CDlgConnSettingsTcpip::getConnectionTimeout(void)
{
  return m_client.getConnectionTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
CDlgConnSettingsTcpip::setConnectionTimeout(uint32_t timeout)
{
  m_client.setConnectionTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
CDlgConnSettingsTcpip::getResponseTimeout(void)
{
  return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
CDlgConnSettingsTcpip::setResponseTimeout(uint32_t timeout)
{
  m_client.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getFullL2
//

bool
CDlgConnSettingsTcpip::getFullL2(void)
{
  return ui->chkFullLevel2->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// setFullL2
//

void
CDlgConnSettingsTcpip::setFullL2(bool l2)
{
  ui->chkFullLevel2->setChecked(l2);
}

///////////////////////////////////////////////////////////////////////////////
// getPoll
//

bool
CDlgConnSettingsTcpip::getPoll(void)
{
  return ui->chkPoll->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// setPoll
//

void
CDlgConnSettingsTcpip::setPoll(bool bpoll)
{
  ui->chkPoll->setChecked(bpoll);
}

///////////////////////////////////////////////////////////////////////////////
// isTLSEnabled
//

bool
CDlgConnSettingsTcpip::isTLSEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableTLS
//

void
CDlgConnSettingsTcpip::enableTLS(bool btls)
{
  m_bTLS = btls;
}

///////////////////////////////////////////////////////////////////////////////
// isVerifyPeerEnabled
//

bool
CDlgConnSettingsTcpip::isVerifyPeerEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableVerifyPeer
//

void
CDlgConnSettingsTcpip::enableVerifyPeer(bool bverifypeer)
{
  m_bVerifyPeer = bverifypeer;
}

///////////////////////////////////////////////////////////////////////////////
// getCaFile
//

QString
CDlgConnSettingsTcpip::getCaFile(void)
{
  return m_cafile;
}

///////////////////////////////////////////////////////////////////////////////
// setCaFile
//

void
CDlgConnSettingsTcpip::setCaFile(const QString& str)
{
  m_cafile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCaPath
//

QString
CDlgConnSettingsTcpip::getCaPath(void)
{
  return m_capath;
}

///////////////////////////////////////////////////////////////////////////////
// setCaPath
//

void
CDlgConnSettingsTcpip::setCaPath(const QString& str)
{
  m_capath = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCertFile
//

QString
CDlgConnSettingsTcpip::getCertFile(void)
{
  return m_certfile;
}

///////////////////////////////////////////////////////////////////////////////
// setCertFile
//

void
CDlgConnSettingsTcpip::setCertFile(const QString& str)
{
  m_certfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getKeyFile
//

QString
CDlgConnSettingsTcpip::getKeyFile(void)
{
  return m_keyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setKeyFile
//

void
CDlgConnSettingsTcpip::setKeyFile(const QString& str)
{
  m_keyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getPwKeyFile
//

QString
CDlgConnSettingsTcpip::getPwKeyFile(void)
{
  return m_pwkeyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setPwKeyFile
//

void
CDlgConnSettingsTcpip::setPwKeyFile(const QString& str)
{
  m_pwkeyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getJsonObj
//

json
CDlgConnSettingsTcpip::getJson(void)
{
  std::string str;

  m_jsonConfig["type"]               = static_cast<int>(CVscpClient::connType::TCPIP);
  m_jsonConfig["name"]               = getName().toStdString();
  m_jsonConfig["host"]               = getHost().toStdString();
  m_jsonConfig["user"]               = getUser().toStdString();
  m_jsonConfig["password"]           = getPassword().toStdString();
  m_jsonConfig["connection-timeout"] = (int)getConnectionTimeout();
  m_jsonConfig["response-timeout"]   = (int)getResponseTimeout();
  m_jsonConfig["bpoll"]              = getPoll();
  m_jsonConfig["bfull-l2"]           = getFullL2();
  m_jsonConfig["selected-interface"] = getInterface().toStdString();

  m_jsonConfig["btls"]        = isTLSEnabled();
  m_jsonConfig["bverifypeer"] = isVerifyPeerEnabled();
  m_jsonConfig["cafile"]      = getCaFile().toStdString();
  m_jsonConfig["capath"]      = getCaPath().toStdString();
  m_jsonConfig["certfile"]    = getCertFile().toStdString();
  m_jsonConfig["keyfile"]     = getKeyFile().toStdString();
  m_jsonConfig["pwkeyfile"]   = getPwKeyFile().toStdString();

  // Filter
  m_jsonConfig["priority-filter"] = m_filter.filter_priority;
  m_jsonConfig["priority-mask"]   = m_filter.mask_priority;
  m_jsonConfig["class-filter"]    = m_filter.filter_class;
  m_jsonConfig["class-mask"]      = m_filter.mask_class;
  m_jsonConfig["type-filter"]     = m_filter.filter_type;
  m_jsonConfig["type-mask"]       = m_filter.mask_type;
  vscp_writeGuidArrayToString(str, m_filter.filter_GUID);
  m_jsonConfig["guid-filter"] = str.c_str();
  vscp_writeGuidArrayToString(str, m_filter.mask_GUID);
  m_jsonConfig["guid-mask"] = str.c_str();

  // Save all fetched interfaces
  json interfaceArray = json::array();
  for (int i = 0; i < ui->comboInterface->count(); i++) {
    json j;
    j["if-item"] = ui->comboInterface->itemText(i).toStdString();
    interfaceArray.push_back(j);
  }

  m_jsonConfig["interfaces"] = interfaceArray;

  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// SetinitFromJsonObj
//

void
CDlgConnSettingsTcpip::setJson(const json* pobj)
{
  m_jsonConfig = *pobj;

  if (m_jsonConfig.contains("name") && m_jsonConfig["name"].is_string()) {
    setName(m_jsonConfig["name"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("host") && m_jsonConfig["host"].is_string()) {
    setHost(m_jsonConfig["host"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("user") && m_jsonConfig["user"].is_string()) {
    setUser(m_jsonConfig["user"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("password") && m_jsonConfig["password"].is_string()) {
    setPassword(m_jsonConfig["password"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("connection-timeout") && m_jsonConfig["connection-timeout"].is_number()) {
    setConnectionTimeout(m_jsonConfig["connection-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("response-timeout") && m_jsonConfig["response-timeout"].is_number()) {
    setResponseTimeout(m_jsonConfig["response-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("bpoll") && m_jsonConfig["bpoll"].is_boolean()) {
    setPoll((short)m_jsonConfig["bpoll"].get<bool>());
  }

  if (m_jsonConfig.contains("bfull-l2") && m_jsonConfig["bfull-l2"].is_boolean()) {
    setFullL2((short)m_jsonConfig["bfull-l2"].get<bool>());
  }

  if (m_jsonConfig.contains("btls") && m_jsonConfig["btls"].is_boolean()) {
    enableTLS((short)m_jsonConfig["btls"].get<bool>());
  }

  if (m_jsonConfig.contains("bverifypeer") && m_jsonConfig["bverifypeer"].is_boolean()) {
    enableVerifyPeer((short)m_jsonConfig["bverifypeer"].get<bool>());
  }

  if (m_jsonConfig.contains("cafile") && m_jsonConfig["cafile"].is_string()) {
    setCaFile(m_jsonConfig["cafile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("capath") && m_jsonConfig["capath"].is_string()) {
    setCaPath(m_jsonConfig["capath"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("certfile") && m_jsonConfig["certfile"].is_string()) {
    setCertFile(m_jsonConfig["certfile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("keyfile") && m_jsonConfig["keyfile"].is_string()) {
    setKeyFile(m_jsonConfig["keyfile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("pwkeyfile") && m_jsonConfig["pwkeyfile"].is_string()) {
    setPwKeyFile(m_jsonConfig["pwkeyfile"].get<std::string>().c_str());
  }

  // Get main filter
  memset(&m_filter, 0, sizeof(vscpEventFilter));
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
    vscp_getGuidFromStringToArray(m_filter.filter_GUID,
                                  m_jsonConfig["guid-filter"].get<std::string>());
  }

  if (m_jsonConfig.contains("guid-mask") && m_jsonConfig["guid-mask"].is_string()) {
    vscp_getGuidFromStringToArray(m_filter.mask_GUID,
                                  m_jsonConfig["guid-mask"].get<std::string>());
  }

  // Interfaces
  if (m_jsonConfig.contains("interfaces") && m_jsonConfig["interfaces"].is_array()) {

    json interfaceArray = m_jsonConfig["interfaces"];

    for (auto v : interfaceArray) {
      json item = v;
      if (item.contains("if-item") && item["if-item"].is_string()) {
        ui->comboInterface->addItem(item["if-item"].get<std::string>().c_str());
      }
    }
  }

  QString selectedInterface;
  if (m_jsonConfig.contains("selected-interface") && m_jsonConfig["selected-interface"].is_string()) {
    selectedInterface = m_jsonConfig["selected-interface"].get<std::string>().c_str();
  }

  // Select interface
  int idx;
  qDebug() << getInterface();
  if (-1 != (idx = ui->comboInterface->findText(selectedInterface))) {
    ui->comboInterface->setCurrentIndex(idx);
  }
  else {
    setInterface(selectedInterface);
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// onSetFilter
//

void
CDlgConnSettingsTcpip::onSetFilter(void)
{
  CDlgLevel2Filter dlg;

  dlg.setFilter(&m_filter);

  if (QDialog::Accepted == dlg.exec()) {
    dlg.getFilter(&m_filter);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void
CDlgConnSettingsTcpip::onTestConnection(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // Initialize host connection
  if (VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
                                          getUser().toStdString().c_str(),
                                          getPassword().toStdString().c_str())) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to initialize tcp/ip client"));
    return;
  }

  // Connect to remote host
  if (VSCP_ERROR_SUCCESS != m_client.connect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to connect to remote tcp/ip host"));
    m_client.disconnect();
    return;
  }

  // Get server version
  uint8_t major_ver;
  uint8_t minor_ver;
  uint8_t release_ver;
  uint8_t build_ver;
  QString strVersion;
  if (VSCP_ERROR_SUCCESS == m_client.getversion(&major_ver,
                                                &minor_ver,
                                                &release_ver,
                                                &build_ver)) {

    strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
                                 (int)major_ver,
                                 (int)minor_ver,
                                 (int)release_ver,
                                 (int)build_ver)
                   .c_str();
  }
  else {
    strVersion = tr("Failed to get version from server");
  }

  // Disconnect from remote host
  if (VSCP_ERROR_SUCCESS != m_client.disconnect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to disconnect from remote tcp/ip host"));
    return;
  }

  QApplication::restoreOverrideCursor();

  QString msg = tr("Connection test was successful");
  msg += "\n";
  msg += strVersion;
  QMessageBox::information(this, tr(APPNAME), msg);
}

///////////////////////////////////////////////////////////////////////////////
// onGetInterfaces
//

void
CDlgConnSettingsTcpip::onGetInterfaces(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // Initialize host connection
  if (VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
                                          getUser().toStdString().c_str(),
                                          getPassword().toStdString().c_str())) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to initialize tcp/ip object"));
    return;
  }

  m_client.setResponseTimeout(2000);

  // Connect to remote host
  if (VSCP_ERROR_SUCCESS != m_client.connect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to connect to remote tcp/ip host"));
    m_client.disconnect();
    return;
  }

  ui->comboInterface->clear();

  // Get interfaces
  std::deque<std::string> iflist;
  if (VSCP_ERROR_SUCCESS == m_client.getinterfaces(iflist)) {
    for (int i = 0; i < iflist.size(); i++) {

      std::deque<std::string> tokens;
      // "65534,1,FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:FF:FE:00:00,Internal Server Client.|Started at 2021-02-16T04:05:15Z | Started at 2021-02-16T04:05:15Z"
      vscp_split(tokens, iflist[i], ",");

      QString interface_id;
      QString interface_type;
      QString interface_guid;

      // Get id
      if (!tokens.empty()) {
        interface_id = tokens.front().c_str();
        tokens.pop_front();
      }

      // Get type
      QString interface_typestr;
      if (!tokens.empty()) {
        interface_type = tokens.front().c_str();
        tokens.pop_front();
        // Set interface type description
        switch (interface_type.toInt()) {

          case 0:
          default:
            interface_typestr = "Unknow";
            break;

          case 1:
            interface_typestr = "Internal to daemon/server";
            break;

          case 2:
            interface_typestr = "Level I driver";
            break;

          case 3:
            interface_typestr = "Level II driver";
            break;

          case 4:
            interface_typestr = "TCP/IP client";
            break;

          case 5:
            interface_typestr = "UDP client";
            break;

          case 6:
            interface_typestr = "Web server";
            break;

          case 7:
            interface_typestr = "Websocket client";
            break;

          case 8:
            interface_typestr = "REST client";
            break;
        }
      }

      // Get GUID
      if (!tokens.empty()) {
        interface_guid = tokens.front().c_str();
        tokens.pop_front();
      }

      // ... skip the rest

      QString item = vscp_str_format("%s type=%s (%s) obid=%s",
                                     interface_guid.toStdString().c_str(),
                                     interface_type.toStdString().c_str(),
                                     interface_typestr.toStdString().c_str(),
                                     interface_id.toStdString().c_str())
                       .c_str();
      ui->comboInterface->addItem(item, i);

      qDebug() << iflist.size();
      qDebug() << iflist[i].c_str();
    }
  }
  else {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to get interfaces from server"));
    m_client.disconnect();
    return;
  }

  // Disconnect from remote host
  if (VSCP_ERROR_SUCCESS != m_client.disconnect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, tr(APPNAME), tr("Failed to disconnect from remote tcp/ip host"));
    return;
  }

  QApplication::restoreOverrideCursor();
  QMessageBox::information(this, tr(APPNAME), tr("Interfaces fetched from remote server"));
}

///////////////////////////////////////////////////////////////////////////////
// onTLSSettings
//

void
CDlgConnSettingsTcpip::onTLSSettings(void)
{
  CDlgTLS dlg;

  dlg.enableTLS(m_bTLS);
  dlg.enableVerifyPeer(m_bVerifyPeer);
  dlg.setCaFile(m_cafile);
  dlg.setCaPath(m_capath);
  dlg.setCertFile(m_certfile);
  dlg.setKeyFile(m_keyfile);
  dlg.setPwKeyFile(m_pwkeyfile);

  if (QDialog::Accepted == dlg.exec()) {
    m_bTLS        = dlg.isTLSEnabled();
    m_bVerifyPeer = dlg.isVerifyPeerEnabled();
    m_cafile      = dlg.getCaFile();
    m_capath      = dlg.getCaPath();
    m_certfile    = dlg.getCertFile();
    m_keyfile     = dlg.getKeyFile();
    m_pwkeyfile   = dlg.getPwKeyFile();
  }
}
