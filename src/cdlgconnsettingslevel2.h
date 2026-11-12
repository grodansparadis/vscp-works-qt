// cdlgconnsettingslevel2.h
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

#ifndef CDLGCONNSETTINGSLEVEL2_H
#define CDLGCONNSETTINGSLEVEL2_H

#include "vscp-client-level2.h"

#include <QDialog>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Ui {
class CDlgConnSettingsLevel2;
}

class CDlgConnSettingsLevel2 : public QDialog {
  Q_OBJECT

public:
  explicit CDlgConnSettingsLevel2(QWidget* parent = nullptr);
  ~CDlgConnSettingsLevel2();

  /// Set initial focus to description
  void setInitialFocus(void);

  /// Setters/getters for name/description
  QString getName(void);
  void setName(const QString& str);

  /// Setters/getters for path to the driver
  QString getDriverPath(void);
  void setDriverPath(const QString& str);

  /// Setters/getters for the driver GUID
  QString getGuid(void);
  void setGuid(const QString& str);

  /// True if the configuration is embedded, false if a file is used
  bool isConfigEmbedded(void);
  void setConfigEmbedded(bool bEmbedded);

  /// Setters/getters for path to the configuration file
  QString getConfigPath(void);
  void setConfigPath(const QString& str);

  /// Setters/getters for the embedded configuration
  QString getEmbeddedConfig(void);
  void setEmbeddedConfig(const QString& str);

  /// Setters/getters for response timeout (ms)
  uint32_t getResponseTimeout(void);
  void setResponseTimeout(uint32_t timeout);

  /// Setters/getters for JSON config object
  json getJson(void);
  void setJson(const json* pobj);

public slots:

  /// Validate the settings before the dialog is closed with OK
  void accept(void) override;

private slots:

  /// Browse for the driver
  void onSetDriverPath(void);

  /// Browse for the configuration file
  void onSetConfigPath(void);

  /// Load the embedded configuration from a file
  void onLoadConfig(void);

  /// Save the embedded configuration to a file
  void onSaveConfig(void);

  /// File/embedded configuration selection changed
  void onConfigModeChanged(void);

  /// Set filter button clicked
  void onSetFilter(void);

  /// Test connection button clicked
  void onTestConnection(void);

  /// Help
  void showHelp(void);

private:
  /*!
    Check the settings
    @return Empty string if OK, otherwise a description of the problem
  */
  QString validate(void);

  Ui::CDlgConnSettingsLevel2* ui;

  // JSON configuration object
  json m_jsonConfig;

  /// Main filter
  vscpEventFilter m_filter;
};

#endif // CDLGCONNSETTINGSLEVEL2_H
