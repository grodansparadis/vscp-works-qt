// cdlgconnsettingsws2.cpp
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "cdlgconnsettingsws2.h"
#include "ui_cdlgconnsettingsws2.h"

#include "cdlglevel2filter.h"

#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsWs2::CDlgConnSettingsWs2(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsWs2)
{
  ui->setupUi(this);

  // Clear filter
  memset(&m_filter, 0, sizeof(vscpEventFilter));

  setInitialFocus();

  connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsWs2::onSetFilter);
  connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsWs2::onTestConnection);

  // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
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

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgConnSettingsWs2::getName(void)
{
  return (ui->editDescription->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgConnSettingsWs2::setName(const QString& str)
{
  ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString
CDlgConnSettingsWs2::getUrl(void)
{
  return (ui->editUrl->text());
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void
CDlgConnSettingsWs2::setUrl(const QString& str)
{
  ui->editUrl->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString
CDlgConnSettingsWs2::getUser(void)
{
  return (ui->editUsername->text());
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void
CDlgConnSettingsWs2::setUser(const QString& str)
{
  ui->editUsername->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString
CDlgConnSettingsWs2::getPassword(void)
{
  return (ui->editUsername->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void
CDlgConnSettingsWs2::setPassword(const QString& str)
{
  ui->editUsername->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
CDlgConnSettingsWs2::getConnectionTimeout(void)
{
  return m_client.getConnectionTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
CDlgConnSettingsWs2::setConnectionTimeout(uint32_t timeout)
{
  m_client.setConnectionTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
CDlgConnSettingsWs2::getResponseTimeout(void)
{
  return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
CDlgConnSettingsWs2::setResponseTimeout(uint32_t timeout)
{
  m_client.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsWs2::getJson(void)
{
  std::string str;

  m_jsonConfig["type"]     = static_cast<int>(CVscpClient::connType::WS2);
  m_jsonConfig["name"]     = getName().toStdString();
  m_jsonConfig["url"]      = getUrl().toStdString();
  m_jsonConfig["user"]     = getUser().toStdString();
  m_jsonConfig["password"] = getPassword().toStdString();

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

  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsWs2::setJson(const json* pobj)
{
  m_jsonConfig = *pobj;

  if (m_jsonConfig.contains("name") && m_jsonConfig["name"].is_string()) {
    setName(m_jsonConfig["name"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("url") && m_jsonConfig["url"].is_string()) {
    setUrl(m_jsonConfig["url"].get<std::string>().c_str());
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
                                  m_jsonConfig["guid-filter"].get<std::string>().c_str());
  }

  if (!m_jsonConfig.contains("guid-mask") && m_jsonConfig["guid-mask"].is_string()) {
    vscp_getGuidFromStringToArray(m_filter.mask_GUID,
                                  m_jsonConfig["guid-mask"].get<std::string>().c_str());
  }
}

// ----------------------------------------------------------------------------

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
// onTestConnection
//

void
CDlgConnSettingsWs2::onTestConnection(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // // Initialize host connection
  // if ( VSCP_ERROR_SUCCESS != m_client.init(getHost().toStdString().c_str(),
  //                                             getPort(),
  //                                             getUser().toStdString().c_str(),
  //                                             getPassword().toStdString().c_str() ) ) {
  //     QApplication::restoreOverrideCursor();
  //     QMessageBox::information(this, tr(APPNAME), tr("Failed to initialize tcp/ip client"));
  //     return;
  // }

  // // Connect to remote host
  // if ( VSCP_ERROR_SUCCESS != m_client.connect() ) {
  //     QApplication::restoreOverrideCursor();
  //     QMessageBox::information(this, tr(APPNAME), tr("Failed to connect to remote tcp/ip host"));
  //     m_client.disconnect();
  //     return;
  // }

  // // Get server version
  // uint8_t major_ver;
  // uint8_t minor_ver;
  // uint8_t release_ver;
  // uint8_t build_ver;
  // QString strVersion;
  // if ( VSCP_ERROR_SUCCESS == m_client.getversion( &major_ver,
  //                                                 &minor_ver,
  //                                                 &release_ver,
  //                                                 &build_ver ) ) {

  //     strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
  //                                     (int)major_ver,
  //                                     (int)minor_ver,
  //                                     (int)release_ver,
  //                                     (int)build_ver ).c_str();
  // }
  // else {
  //     strVersion = tr("Failed to get version from server");
  // }

  // // Disconnect from remote host
  // if ( VSCP_ERROR_SUCCESS != m_client.disconnect() ) {
  //     QApplication::restoreOverrideCursor();
  //     QMessageBox::information(this, tr(APPNAME), tr("Failed to disconnect from remote tcp/ip host"));
  //     return;
  // }

  QApplication::restoreOverrideCursor();

  QString msg = tr("Connection test was successful");
  msg += "\n";
  msg += "NOT IMPLEMENTED YET!";
  QMessageBox::information(this, tr(APPNAME), msg);
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgConnSettingsWs2::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=ws1";
  QDesktopServices::openUrl(QUrl(link));
}