// cdlgconnsettingslevel2.cpp
//
// Connection settings for a VSCP Level II driver (vscpl2drv-*)
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

#include "vscpworks.h"

#include "cdlgconnsettingslevel2.h"
#include "ui_cdlgconnsettingslevel2.h"

#include "cdlglevel2filter.h"

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QUrl>

#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsLevel2::CDlgConnSettingsLevel2(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsLevel2)
{
  ui->setupUi(this);

  vscp_clearVSCPFilter(&m_filter);

  ui->editConfig->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  ui->radioConfigFile->setChecked(true);
  setResponseTimeout(vscpClientLevel2::DEFAULT_RESPONSE_TIMEOUT);
  onConfigModeChanged();

  connect(ui->btnSetDriverPath, &QToolButton::clicked, this, &CDlgConnSettingsLevel2::onSetDriverPath);
  connect(ui->btnSetConfigPath, &QToolButton::clicked, this, &CDlgConnSettingsLevel2::onSetConfigPath);
  connect(ui->btnLoadConfig, &QPushButton::clicked, this, &CDlgConnSettingsLevel2::onLoadConfig);
  connect(ui->btnSaveConfig, &QPushButton::clicked, this, &CDlgConnSettingsLevel2::onSaveConfig);
  connect(ui->radioConfigFile, &QRadioButton::toggled, this, &CDlgConnSettingsLevel2::onConfigModeChanged);
  connect(ui->btnSetFilter, &QPushButton::clicked, this, &CDlgConnSettingsLevel2::onSetFilter);
  connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsLevel2::onTestConnection);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);
  connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &CDlgConnSettingsLevel2::showHelp);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsLevel2::~CDlgConnSettingsLevel2()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsLevel2::setInitialFocus(void)
{
  ui->editName->setFocus();
}

// ----------------------------------------------------------------------------
// Getters / Setters
// ----------------------------------------------------------------------------

QString
CDlgConnSettingsLevel2::getName(void)
{
  return ui->editName->text();
}

void
CDlgConnSettingsLevel2::setName(const QString& str)
{
  ui->editName->setText(str);
}

QString
CDlgConnSettingsLevel2::getDriverPath(void)
{
  return ui->editDriverPath->text().trimmed();
}

void
CDlgConnSettingsLevel2::setDriverPath(const QString& str)
{
  ui->editDriverPath->setText(str);
}

QString
CDlgConnSettingsLevel2::getGuid(void)
{
  return ui->editGuid->text().trimmed();
}

void
CDlgConnSettingsLevel2::setGuid(const QString& str)
{
  ui->editGuid->setText(str);
}

bool
CDlgConnSettingsLevel2::isConfigEmbedded(void)
{
  return ui->radioConfigEmbedded->isChecked();
}

void
CDlgConnSettingsLevel2::setConfigEmbedded(bool bEmbedded)
{
  if (bEmbedded) {
    ui->radioConfigEmbedded->setChecked(true);
  }
  else {
    ui->radioConfigFile->setChecked(true);
  }
  onConfigModeChanged();
}

QString
CDlgConnSettingsLevel2::getConfigPath(void)
{
  return ui->editConfigPath->text().trimmed();
}

void
CDlgConnSettingsLevel2::setConfigPath(const QString& str)
{
  ui->editConfigPath->setText(str);
}

QString
CDlgConnSettingsLevel2::getEmbeddedConfig(void)
{
  return ui->editConfig->toPlainText();
}

void
CDlgConnSettingsLevel2::setEmbeddedConfig(const QString& str)
{
  ui->editConfig->setPlainText(str);
}

uint32_t
CDlgConnSettingsLevel2::getResponseTimeout(void)
{
  return static_cast<uint32_t>(ui->spinResponseTimeout->value());
}

void
CDlgConnSettingsLevel2::setResponseTimeout(uint32_t timeout)
{
  ui->spinResponseTimeout->setValue(static_cast<int>(timeout));
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsLevel2::getJson(void)
{
  std::string str;

  m_jsonConfig["type"]             = static_cast<int>(CVscpClient::connType::LEVEL2);
  m_jsonConfig["name"]             = getName().toStdString();
  m_jsonConfig["path-driver"]      = getDriverPath().toStdString();
  m_jsonConfig["guid"]             = getGuid().toStdString();
  m_jsonConfig["response-timeout"] = getResponseTimeout();

  // The configuration file path is kept also when the configuration is
  // embedded, so switching back restores it. "config" takes precedence in
  // the client, so it is removed when a file is used.
  m_jsonConfig["path-config"] = getConfigPath().toStdString();
  if (isConfigEmbedded()) {
    const std::string text = getEmbeddedConfig().toStdString();
    json embedded          = json::parse(text, nullptr, false);
    if (!embedded.is_discarded() && (embedded.is_object() || embedded.is_array())) {
      // Stored as JSON so it stays readable in the settings
      m_jsonConfig["config"] = embedded;
    }
    else {
      m_jsonConfig["config"] = text;
    }
  }
  else {
    m_jsonConfig.erase("config");
  }

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
CDlgConnSettingsLevel2::setJson(const json* pobj)
{
  if (nullptr == pobj) {
    return;
  }
  m_jsonConfig = *pobj;

  if (m_jsonConfig.contains("name") && m_jsonConfig["name"].is_string()) {
    setName(m_jsonConfig["name"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("path-driver") && m_jsonConfig["path-driver"].is_string()) {
    setDriverPath(m_jsonConfig["path-driver"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("guid") && m_jsonConfig["guid"].is_string()) {
    setGuid(m_jsonConfig["guid"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("response-timeout") && m_jsonConfig["response-timeout"].is_number_unsigned()) {
    setResponseTimeout(m_jsonConfig["response-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("path-config") && m_jsonConfig["path-config"].is_string()) {
    setConfigPath(m_jsonConfig["path-config"].get<std::string>().c_str());
  }

  bool bEmbedded = false;
  if (m_jsonConfig.contains("config")) {
    const json& jc = m_jsonConfig["config"];
    if (jc.is_object() || jc.is_array()) {
      setEmbeddedConfig(jc.dump(2).c_str());
      bEmbedded = !(jc.is_object() && jc.empty());
    }
    else if (jc.is_string()) {
      setEmbeddedConfig(jc.get<std::string>().c_str());
      bEmbedded = !jc.get<std::string>().empty();
    }
  }
  setConfigEmbedded(bEmbedded);

  // Filter
  vscp_clearVSCPFilter(&m_filter);
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
    vscp_parseGuid(m_filter.filter_GUID, m_jsonConfig["guid-filter"].get<std::string>());
  }
  if (m_jsonConfig.contains("guid-mask") && m_jsonConfig["guid-mask"].is_string()) {
    vscp_parseGuid(m_filter.mask_GUID, m_jsonConfig["guid-mask"].get<std::string>());
  }
}

///////////////////////////////////////////////////////////////////////////////
// validate
//

QString
CDlgConnSettingsLevel2::validate(void)
{
  if (getDriverPath().isEmpty()) {
    return tr("Set the path to the Level II driver.");
  }

  if (!getGuid().isEmpty()) {
    uint8_t guid[16];
    if (!vscp_parseGuid(guid, getGuid().toStdString())) {
      return tr("The GUID is not valid. Use the form FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:00 "
                "or leave it empty.");
    }
  }

  if (isConfigEmbedded()) {
    const QString text = getEmbeddedConfig().trimmed();
    if (text.isEmpty()) {
      return tr("The embedded driver configuration is empty.");
    }
    if (text.startsWith('{') || text.startsWith('[')) {
      json j = json::parse(text.toStdString(), nullptr, false);
      if (j.is_discarded()) {
        return tr("The embedded driver configuration looks like JSON but is not valid JSON.");
      }
    }
  }
  else if (getConfigPath().isEmpty()) {
    return tr("Set the path to the driver configuration file, or use an embedded configuration.");
  }

  return QString();
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgConnSettingsLevel2::accept(void)
{
  QString err = validate();
  if (!err.isEmpty()) {
    QMessageBox::warning(this, tr(APPNAME), err);
    return;
  }

  if (!isConfigEmbedded() && !QFileInfo::exists(getConfigPath())) {
    if (QMessageBox::Yes != QMessageBox::question(this,
                                                  tr(APPNAME),
                                                  tr("The configuration file does not exist. Save anyway?"))) {
      return;
    }
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// onConfigModeChanged
//

void
CDlgConnSettingsLevel2::onConfigModeChanged(void)
{
  const bool bEmbedded = isConfigEmbedded();
  ui->editConfigPath->setEnabled(!bEmbedded);
  ui->btnSetConfigPath->setEnabled(!bEmbedded);
  ui->editConfig->setEnabled(bEmbedded);
  ui->btnLoadConfig->setEnabled(bEmbedded);
  ui->btnSaveConfig->setEnabled(bEmbedded);
}

///////////////////////////////////////////////////////////////////////////////
// onSetDriverPath
//

void
CDlgConnSettingsLevel2::onSetDriverPath(void)
{
#ifdef WIN32
  const QString filter = tr("Drivers (*.dll);;All (*)");
#elif defined(__APPLE__)
  const QString filter = tr("Drivers (*.dylib *.so);;All (*)");
#else
  const QString filter = tr("Drivers (*.so);;All (*)");
#endif
  QString start = getDriverPath().isEmpty() ? QString("/var/lib/vscp/drivers/level2") : QFileInfo(getDriverPath()).path();
  QString filename =
    QFileDialog::getOpenFileName(this, tr("Set path to Level II driver"), start, filter);
  if (!filename.isEmpty()) {
    setDriverPath(filename);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onSetConfigPath
//

void
CDlgConnSettingsLevel2::onSetConfigPath(void)
{
  QString start = getConfigPath().isEmpty() ? QString("/etc/vscp") : QFileInfo(getConfigPath()).path();
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Set path to driver configuration file"),
                                                  start,
                                                  tr("Configuration (*.json *.xml *.conf);;All (*)"));
  if (!filename.isEmpty()) {
    setConfigPath(filename);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onLoadConfig
//

void
CDlgConnSettingsLevel2::onLoadConfig(void)
{
  QString start = getConfigPath().isEmpty() ? QString("/etc/vscp") : QFileInfo(getConfigPath()).path();
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Load driver configuration"),
                                                  start,
                                                  tr("Configuration (*.json *.xml *.conf);;All (*)"));
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  if (file.size() > (1024 * 1024)) {
    QMessageBox::warning(this, tr(APPNAME), tr("The file is too large to embed (max 1 MB)."));
    return;
  }
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr(APPNAME), tr("Unable to read %1").arg(filename));
    return;
  }
  setEmbeddedConfig(QString::fromUtf8(file.readAll()));
}

///////////////////////////////////////////////////////////////////////////////
// onSaveConfig
//

void
CDlgConnSettingsLevel2::onSaveConfig(void)
{
  QString filename = QFileDialog::getSaveFileName(this,
                                                  tr("Save driver configuration"),
                                                  QString(),
                                                  tr("Configuration (*.json *.xml *.conf);;All (*)"));
  if (filename.isEmpty()) {
    return;
  }

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
      (-1 == file.write(getEmbeddedConfig().toUtf8()))) {
    QMessageBox::warning(this, tr(APPNAME), tr("Unable to write %1").arg(filename));
  }
}

///////////////////////////////////////////////////////////////////////////////
// onSetFilter
//

void
CDlgConnSettingsLevel2::onSetFilter(void)
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
CDlgConnSettingsLevel2::onTestConnection(void)
{
  QString err = validate();
  if (!err.isEmpty()) {
    QMessageBox::warning(this, tr(APPNAME), err);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // A local client so nothing is left loaded after the test
  vscpClientLevel2 client;
  json jConfig = getJson();
  if (!client.initFromJson(jConfig.dump())) {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr(APPNAME), tr("Invalid Level II driver settings. See the log for details."));
    return;
  }

  int rv = client.connect();
  if (VSCP_ERROR_SUCCESS != rv) {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,
                         tr(APPNAME),
                         tr("The driver could not be loaded or opened (rv=%1).\n\n"
                            "Check the driver path, that it is a Level II driver and that "
                            "the configuration is valid for it. See the log for details.")
                           .arg(rv));
    return;
  }

  uint8_t major = 0, minor = 0, release = 0, build = 0;
  client.getversion(&major, &minor, &release, &build);
  QString vendor = QString::fromStdString(client.getVendorString());

  client.disconnect();
  QApplication::restoreOverrideCursor();

  QString msg = tr("The driver loaded and opened successfully.\n\nVersion: %1.%2.%3.%4")
                  .arg(major)
                  .arg(minor)
                  .arg(release)
                  .arg(build);
  if (!vendor.isEmpty()) {
    msg += "\n" + vendor;
  }
  QMessageBox::information(this, tr(APPNAME), msg);
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgConnSettingsLevel2::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connection_level2";
  QDesktopServices::openUrl(QUrl(link));
}
