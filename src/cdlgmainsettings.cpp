// cdlgmainsettings.cpp
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

#include "cdlgmainsettings.h"
#include "mainwindow.h"
#include "ui_cdlgmainsettings.h"

#include "cfrmsession.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMainSettings::CDlgMainSettings(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMainSettings)
{
  ui->setupUi(this);

  connect(ui->comboNumberBase, SIGNAL(currentIndexChanged(int)), this, SLOT(onBaseChange(int)));

  // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // * * * General * * *

  // Numeric base
  // m_baseIndex = pworks->m_base;
  ui->comboNumberBase->setCurrentIndex(static_cast<int>(pworks->m_base));
  // onBaseChange(static_cast<int>(m_baseIndex));

  // Preferred language
  ui->editPreferredLanguage->setText(pworks->m_preferredLanguage.c_str());

  // Darkthereme
  ui->chkDarkTheme->setChecked(pworks->m_bEnableDarkTheme);

  // Ask befor delete/clear
  ui->chkAskOnDelete->setChecked(pworks->m_bAskBeforeDelete);

  // Save format
  ui->chkAlwaysJSON->setChecked(pworks->m_bSaveAlwaysJSON);

  // * * * Session Window tab * * *

  // Session response timeout
  ui->spinSessionTimeout->setValue(pworks->m_session_timeout);

  // Max number of session events
  ui->editMaxSessionEvents->setText(QString::number(pworks->m_session_maxEvents));

  // Class display format
  ui->comboClassDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_ClassDisplayFormat));

  // Type display format
  ui->comboTypeDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_TypeDisplayFormat));

  // GUID display format
  ui->comboGuidDisplayFormat->setCurrentIndex(static_cast<int>(pworks->m_session_GuidDisplayFormat));

  // Automatic connect
  ui->chkAutomaticConnect->setChecked(pworks->m_session_bAutoConnect);

  // VSCP type token format
  ui->chkShowFullToken->setChecked(pworks->m_session_bShowFullTypeToken);

  // * * * config tab * * *

  // Config response timeout
  ui->spinConfigTimeout->setValue(pworks->m_config_timeout);

  // Base for config values
  ui->comboNumberBaseConfig->setCurrentIndex(static_cast<int>(pworks->m_config_base));

  // Disable colors from MDF
  ui->chkConfigDisableColors->setChecked(pworks->m_config_bDisableColors);

  // * * * Firmware * * *
  ui->chkDeviceCode->setChecked(pworks->m_firmware_devicecode_required);

  // * * *  logging tab * * *

  // File log level
  switch (pworks->m_fileLogLevel) {

    case spdlog::level::off:
      ui->comboFileLogLevel->setCurrentIndex(0);
      break;

    case spdlog::level::critical:
      ui->comboFileLogLevel->setCurrentIndex(1);
      break;

    case spdlog::level::err:
      ui->comboFileLogLevel->setCurrentIndex(2);
      break;

    case spdlog::level::warn:
      ui->comboFileLogLevel->setCurrentIndex(3);
      break;

    default:
    case spdlog::level::info:
      ui->comboFileLogLevel->setCurrentIndex(4);
      break;

    case spdlog::level::debug:
      ui->comboFileLogLevel->setCurrentIndex(5);
      break;

    case spdlog::level::trace:
      ui->comboFileLogLevel->setCurrentIndex(6);
      break;
  }

  ui->editFileLogPattern->setText(pworks->m_fileLogPattern.c_str());
  ui->editFileLogPath->setText(pworks->m_fileLogPath.c_str());
  ui->editFileLogMaxSize->setText(QString::number(pworks->m_maxFileLogSize));
  ui->editFileLogMaxFiles->setText(QString::number(pworks->m_maxFileLogFiles));

  // File log level
  switch (pworks->m_consoleLogLevel) {

    case spdlog::level::off:
      ui->comboConsoleLogLevel->setCurrentIndex(0);
      break;

    case spdlog::level::critical:
      ui->comboConsoleLogLevel->setCurrentIndex(1);
      break;

    case spdlog::level::err:
      ui->comboConsoleLogLevel->setCurrentIndex(2);
      break;

    case spdlog::level::warn:
      ui->comboConsoleLogLevel->setCurrentIndex(3);
      break;

    default:
    case spdlog::level::info:
      ui->comboConsoleLogLevel->setCurrentIndex(4);
      break;

    case spdlog::level::debug:
      ui->comboConsoleLogLevel->setCurrentIndex(5);
      break;

    case spdlog::level::trace:
      ui->comboConsoleLogLevel->setCurrentIndex(6);
      break;
  }

  ui->editConsoleLogPattern->setText(pworks->m_consoleLogPattern.c_str());

  // ------------------------------------------------------------------------

  // Local storage folder
  ui->pathLocalStorage->setText(pworks->m_shareFolder);

  // VSCP home folder
  ui->pathVscpHome->setText(pworks->m_vscpHomeFolder);

  // Path to config folder
  ui->pathConfigFile->setText(pworks->m_configFile);

  // Path to event db
  {
    QString path = pworks->m_shareFolder;
    path += "vscp_events.sqlite3";
    ui->pathVscpEventDb->setText(path);
  }

  // Event DB last download
  QString str = pworks->m_lastEventDbLoadDateTime.toString(Qt::ISODate);
  str += " @ server [";
  str += pworks->m_lastEventDbServerDateTime.toString(Qt::ISODate);
  str += "]";
  ui->lastDownload->setText(str);

  connect(ui->btnDownLoadNewEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onDownloadEventDb);
  connect(ui->btnReLoadEventDb, &QPushButton::clicked, this, &CDlgMainSettings::onReLoadEventDb);

  // Hook to row double clicked
  // connect(ui->listWidgetConnectionTypes, &QListWidget::itemDoubleClicked, this, &CDlgLevel1Filter::onDoubleClicked );
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMainSettings::~CDlgMainSettings()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void
CDlgMainSettings::done(int rv)
{
  if (QDialog::Accepted == rv) { // ok was pressed

    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // General
    pworks->m_base              = static_cast<numerical_base>(ui->comboNumberBase->currentIndex());
    pworks->m_bEnableDarkTheme  = ui->chkDarkTheme->isChecked();
    pworks->m_bAskBeforeDelete  = ui->chkAskOnDelete->isChecked();
    pworks->m_bSaveAlwaysJSON   = ui->chkAlwaysJSON->isChecked();
    pworks->m_preferredLanguage = ui->editPreferredLanguage->text().toStdString();

    // Session window
    pworks->m_session_timeout   = ui->spinSessionTimeout->value();
    pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
    pworks->m_session_ClassDisplayFormat =
      static_cast<CFrmSession::classDisplayFormat>(ui->comboClassDisplayFormat->currentIndex());
    pworks->m_session_TypeDisplayFormat =
      static_cast<CFrmSession::typeDisplayFormat>(ui->comboTypeDisplayFormat->currentIndex());
    pworks->m_session_GuidDisplayFormat =
      static_cast<CFrmSession::guidDisplayFormat>(ui->comboGuidDisplayFormat->currentIndex());
    pworks->m_session_bAutoConnect       = ui->chkAutomaticConnect->isChecked();
    pworks->m_session_bShowFullTypeToken = ui->chkShowFullToken->isChecked();

    // Config window
    pworks->m_config_timeout        = ui->spinConfigTimeout->value();
    pworks->m_config_base           = static_cast<numerical_base>(ui->comboNumberBaseConfig->currentIndex());
    pworks->m_config_bDisableColors = ui->chkConfigDisableColors->isChecked();

    // Firmware loading window
    pworks->m_firmware_devicecode_required = ui->chkDeviceCode->isChecked();

    // Logging

    // File log level
    int level = ui->comboFileLogLevel->currentIndex();
    switch (level) {

      case 0:
        pworks->m_fileLogLevel = spdlog::level::off;
        break;

      case 1:
        pworks->m_fileLogLevel = spdlog::level::critical;
        break;

      case 2:
        pworks->m_fileLogLevel = spdlog::level::err;
        break;

      case 3:
        pworks->m_fileLogLevel = spdlog::level::warn;
        break;

      default:
      case 4:
        pworks->m_fileLogLevel = spdlog::level::info;
        break;

      case 5:
        pworks->m_fileLogLevel = spdlog::level::debug;
        break;

      case 6:
        pworks->m_fileLogLevel = spdlog::level::trace;
        break;
    }

    pworks->m_fileLogPattern  = ui->editFileLogPattern->text().toStdString();
    pworks->m_fileLogPath     = ui->editFileLogPath->text().toStdString();
    pworks->m_maxFileLogSize  = vscp_readStringValue(ui->editFileLogMaxSize->text().toStdString());
    pworks->m_maxFileLogFiles = vscp_readStringValue(ui->editFileLogMaxFiles->text().toStdString());

    // Console log level
    level = ui->comboConsoleLogLevel->currentIndex();
    switch (level) {

      case 0:
        pworks->m_consoleLogLevel = spdlog::level::off;
        break;

      case 1:
        pworks->m_consoleLogLevel = spdlog::level::critical;
        break;

      case 2:
        pworks->m_consoleLogLevel = spdlog::level::err;
        break;

      case 3:
        pworks->m_consoleLogLevel = spdlog::level::warn;
        break;

      default:
      case 4:
        pworks->m_consoleLogLevel = spdlog::level::info;
        break;

      case 5:
        pworks->m_consoleLogLevel = spdlog::level::debug;
        break;

      case 6:
        pworks->m_consoleLogLevel = spdlog::level::trace;
        break;
    }

    // Set new values
    pworks->m_consoleLogPattern = ui->editConsoleLogPattern->text().toStdString();

    // Data
    pworks->writeSettings();
  }
  QDialog::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void
CDlgMainSettings::onBaseChange(int index)
{
}

///////////////////////////////////////////////////////////////////////////////
// onDownloadEventDb
//

void
CDlgMainSettings::onDownloadEventDb(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  // pworks->writeSettings();
  // QWidget* widget = this-> parentWidget();
  MainWindow* main = (MainWindow*)this->parentWidget();
  main->initForcedRemoteEventDbFetch();
}

///////////////////////////////////////////////////////////////////////////////
// onReLoadEventDb
//

void
CDlgMainSettings::onReLoadEventDb(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  if (!pworks->loadEventDb()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Unable to load events from VSCP event database."),
                             QMessageBox::Ok);
    return;
  }
  else {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Events reloaded from event database."),
                             QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMainSettings::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/settings";
  QDesktopServices::openUrl(QUrl(link));
}