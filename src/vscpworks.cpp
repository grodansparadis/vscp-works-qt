// vscpworks.h
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

#ifndef WIN32

#else
#include <windows.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include "filedownloader.h"
#include "vscpworks.h"

#include <mustache.hpp>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJSEngine>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextDocument>
#include <QUuid>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include <fstream>

// for convenience
using json = nlohmann::json;
using namespace kainjow::mustache;

///////////////////////////////////////////////////////////////////////////////
// vscpworks
//

vscpworks::vscpworks(int& argc, char** argv)
  : QApplication(argc, argv)
{
  m_base              = numerical_base::HEX; // Numerical base
  m_preferredLanguage = "en";                // Preferred language
  m_bAskBeforeDelete  = true;
  m_bEnableDarkTheme  = false;
  m_bSaveAlwaysJSON   = false;

  m_session_timeout   = 1000;
  m_session_maxEvents = -1;

  m_session_ClassDisplayFormat = CFrmSession::classDisplayFormat::symbolic;
  m_session_TypeDisplayFormat  = CFrmSession::typeDisplayFormat::symbolic;
  m_session_GuidDisplayFormat  = CFrmSession::guidDisplayFormat::symbolic;

  m_session_bAutoConnect       = true;
  m_session_bShowFullTypeToken = false;
  m_session_bAutoSaveTxRows    = true;

  // Config
  m_config_timeout = 1000;

  // Firmware
  m_firmware_devicecode_required = true;

  // Logging defaults
  m_fileLogLevel   = spdlog::level::info;
  m_fileLogPattern = "[%^%l%$] %v";

  // Go to users home directory
  QDir::setCurrent(QDir::homePath());

  // Configuration folder
  // --------------------
  // Linux: "/home/<USER>/.config/VSCP/vscpworks+"                      Config file is here (VSCP/vscp-works-qt)
  // Windows: c:\Users\<USER>\Appdata\roaming\vscpworks+
#ifdef WIN32
  QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  path += "VSCP/";
  path += QCoreApplication::applicationName();
  path += ".json";
  m_configFolder = path;
#else
  {
    // QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString path = ".config/VSCP/";
    path += QCoreApplication::applicationName();
    path += ".json";
    m_configFile = path;
  }
  fprintf(stderr, "Config file: %s\n", m_configFile.toStdString().c_str());
#endif

#ifdef WIN32
  {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    path += "/";
    path += QCoreApplication::applicationName();
    path += "/logs/vscpworks.log";
    m_fileLogPath = path.toStdString();
    fprintf(stderr, "log file: %s\n", m_fileLogPath.c_str());
  }
#else
  {
    // QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString path = "./.local/share/VSCP/logs/";
    path += QCoreApplication::applicationName();
    path += "/vscpworks.log";
    m_fileLogPath = path.toStdString();
    fprintf(stderr, "log file: %s\n", m_fileLogPath.c_str());
  }
#endif

  // Share folder
  // ------------
  // Linux: "/home/<USER>/.local/share/VSCP/vscpworks+"
  // Windows: C:/Users/<USER>/AppData/Local/vscpworks+"
  {
#if WIN32
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#else
    QString path = ".local/share/VSCP/vscpworks+";
#endif
    path += "/";

    // If folder does not exist, create it
    QDir dir(path);
    if (!dir.exists()) {
      dir.mkpath(path);
    }

    m_shareFolder = path;
    fprintf(stderr, "Share folder: %s\n", m_shareFolder.toStdString().c_str());

    // Make a folder for  receive sets
    dir.mkpath("./rxsets");
    // Make a folder for transmission sets
    dir.mkpath("./txsets");
    // Make a folder for autosaved/loaded data
    dir.mkpath("./cache");
    // Make a folder for log data
    dir.mkpath("./logs");
  }

  m_maxFileLogSize  = 5242880;
  m_maxFileLogFiles = 7;

  m_bEnableConsoleLog = true;
  m_consoleLogLevel   = spdlog::level::info;
  m_consoleLogPattern = "[vscpworks+] [%^%l%$] %v";

  // After the following it is possible to create and destroy event objects
  // dynamically at run-time - https://doc.qt.io/qt-6/qmetatype.html
  int idEvent   = qRegisterMetaType<vscpEvent>();
  int idEventEx = qRegisterMetaType<vscpEventEx>();
}

///////////////////////////////////////////////////////////////////////////////
// ~vscpworks
//

vscpworks::~vscpworks()
{
  // Save settings
  writeSettings();

  for (QMainWindow* pwnd : m_childWindows) {
    if (nullptr != pwnd) {
      pwnd->close();
      m_childWindows.remove(pwnd);
      delete pwnd;
      break;
    }
  }

  // Close the database
  sqlite3_close(m_db_vscp_works);

  // Clean up SQLite lib allocations
  sqlite3_shutdown();
}

///////////////////////////////////////////////////////////////////////////////
// decimalToStringInBase
//

QString
vscpworks::decimalToStringInBase(uint32_t value, int tobase)
{
  int base = 10;
  QString qstr;
  QString prefix;

  numerical_base selbase = (-1 == tobase) ? m_base : static_cast<numerical_base>(tobase);
  switch (selbase) {
    case numerical_base::HEX:
      prefix = "0x";
      base   = 16;
      break;
    case numerical_base::DECIMAL:
    default:
      prefix = "";
      base   = 10;
      break;
    case numerical_base::OCTAL:
      prefix = "0o";
      base   = 8;
      break;
    case numerical_base::BINARY:
      prefix = "0b";
      base   = 2;
      break;
  }

  return (prefix + QString::number(value, base));
};

///////////////////////////////////////////////////////////////////////////////
// decimalToStringInBase
//

QString
vscpworks::decimalToStringInBase(const QString& strvalue, int tobase)
{
  uint32_t value = vscp_readStringValue(strvalue.toStdString());
  return decimalToStringInBase(value, tobase);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionName
//

QString
vscpworks::getConnectionName(CVscpClient::connType type)
{
  QString str = tr("Unknown type");

  switch (type) {

    case CVscpClient::connType::NONE:
      str = tr("No connection");
      break;

    case CVscpClient::connType::TCPIP:
      str = tr("VSCP tcp/ip connection");
      break;

    case CVscpClient::connType::CANAL:
      str = tr("VSCP CANAL/Level I connection");
      break;

    case CVscpClient::connType::SOCKETCAN:
      str = tr("VSCP socketcan connection");
      break;

    case CVscpClient::connType::WS1:
      str = tr("VSCP websocket protocol 1 connection");
      break;

    case CVscpClient::connType::WS2:
      str = tr("VSCP websocket protocol 2 connection");
      break;

    case CVscpClient::connType::MQTT:
      str = tr("VSCP MQTT connection");
      break;

    case CVscpClient::connType::UDP:
      str = tr("VSCP UDP connection");
      break;

    case CVscpClient::connType::MULTICAST:
      str = tr("VSCP multicast connection");
      break;
  }

  return str;
}

///////////////////////////////////////////////////////////////////////////////
// loadSettings *
//

bool
vscpworks::loadSettings(void)
{
  json j;
  QString str;

  fprintf(stderr, "Loading application data\n");

  // If the database don't exist, bail out
  if (!QFile::exists(m_configFile)) {
    writeSettings();
    if (!QFile::exists(m_configFile)) {
      QString err = QString(tr("The VSCP works configuration file does not exist.  [%0]\n")).arg(m_configFile);
      fprintf(stderr, "%s", err.toStdString().c_str());
      return false;
    }
  }

  // Read in configuration file
  try {
    std::ifstream ifs(m_configFile.toStdString());
    j = json::parse(ifs);
  }
  catch (...) {
    QString err = QString(tr("The VSCP works configuration file is not valid.  [%0]")).arg(m_configFile);
    fprintf(stderr, "%s", err.toStdString().c_str());
    return false;
  }

  // System numerical base
  // ---------------------
  if (j.contains("numericBase") && j["numericBase"].is_number()) {
    m_base = static_cast<numerical_base>(j["numericBase"].get<int>());
  }

  // Preferred language
  if (j.contains("PreferredLanguage") && j["PreferredLanguage"].is_string()) {
    m_preferredLanguage = j["PreferredLanguage"].get<std::string>();
  }

  if (j.contains("bDarkTheme") && j["bDarkTheme"].is_boolean()) {
    m_bEnableDarkTheme = j["bDarkTheme"].get<bool>();
  }

  if (j.contains("bAskBeforeDelete") && j["bAskBeforeDelete"].is_boolean()) {
    m_bAskBeforeDelete = j["bAskBeforeDelete"].get<bool>();
  }

  if (j.contains("bSaveAlwaysJson") && j["bSaveAlwaysJson"].is_boolean()) {
    m_bSaveAlwaysJSON = j["bSaveAlwaysJson"].get<bool>();
  }

  // * * * Logging * * *
  m_bEnableFileLog = true;
  if (j.contains("fileLogLevel") && j["fileLogLevel"].is_number()) {
    int level = j["fileLogLevel"].get<int>();
    switch (level) {
      case 0:
        m_fileLogLevel = spdlog::level::trace;
        break;
      case 1:
        m_fileLogLevel = spdlog::level::debug;
        break;
      default:
      case 2:
        m_fileLogLevel = spdlog::level::info;
        break;
      case 3:
        m_fileLogLevel = spdlog::level::warn;
        break;
      case 4:
        m_fileLogLevel = spdlog::level::err;
        break;
      case 5:
        m_fileLogLevel = spdlog::level::critical;
        break;
      case 6:
        m_fileLogLevel   = spdlog::level::off;
        m_bEnableFileLog = false;
        break;
    }
  }

  if (j.contains("fileLogPattern") && j["fileLogPattern"].is_string()) {
    m_fileLogPattern = j["fileLogPattern"].get<std::string>();
  }

  if (j.contains("fileLogPath") && j["fileLogPath"].is_string()) {
    m_fileLogPath = j["fileLogPath"].get<std::string>();
  }

  if (j.contains("fileLogMaxSize") && j["fileLogMaxSize"].is_number()) {
    m_maxFileLogSize = j["fileLogMaxSize"].get<uint32_t>();
  }

  if (j.contains("fileLogMaxFiles") && j["fileLogMaxFiles"].is_number()) {
    m_maxFileLogFiles = j["fileLogMaxFiles"].get<uint16_t>();
  }

  // console log level
  m_bEnableConsoleLog = true;
  if (j.contains("consoleLogLevel") && j["consoleLogLevel"].is_number()) {
    int level = j["consoleLogLevel"].get<int>();
    switch (level) {
      case 0:
        m_consoleLogLevel = spdlog::level::trace;
        break;
      case 1:
        m_consoleLogLevel = spdlog::level::debug;
        break;
      default:
      case 2:
        m_consoleLogLevel = spdlog::level::info;
        break;
      case 3:
        m_consoleLogLevel = spdlog::level::warn;
        break;
      case 4:
        m_consoleLogLevel = spdlog::level::err;
        break;
      case 5:
        m_consoleLogLevel = spdlog::level::critical;
        break;
      case 6:
        m_consoleLogLevel   = spdlog::level::off;
        m_bEnableConsoleLog = false;
        break;
    }
  }

  if (j.contains("consoleLogPattern") && j["consoleLogPattern"].is_string()) {
    m_consoleLogPattern = j["consoleLogPattern"].get<std::string>();
  }

  // * * * Session * * *

  if (j.contains("maxSessionEvents") && j["maxSessionEvents"].is_number()) {
    m_session_maxEvents = j["maxSessionEvents"].get<int>();
  }

  if (j.contains("sessionTimeout") && j["sessionTimeout"].is_number()) {
    m_session_timeout = j["sessionTimeout"].get<uint32_t>();
  }

  if (j.contains("sessionClassDisplayFormat") && j["sessionClassDisplayFormat"].is_number()) {
    m_session_ClassDisplayFormat = static_cast<CFrmSession::classDisplayFormat>(j["sessionClassDisplayFormat"].get<int>());
  }

  if (j.contains("sessionTypeDisplayFormat") && j["sessionTypeDisplayFormat"].is_number()) {
    m_session_TypeDisplayFormat = static_cast<CFrmSession::typeDisplayFormat>(j["sessionTypeDisplayFormat"].get<int>());
  }

  if (j.contains("sessionGuidDisplayFormat") && j["sessionGuidDisplayFormat"].is_number()) {
    m_session_GuidDisplayFormat = static_cast<CFrmSession::guidDisplayFormat>(j["sessionGuidDisplayFormat"].get<int>());
  }

  if (j.contains("sessionAutoConnect") && j["sessionAutoConnect"].is_boolean()) {
    m_session_bAutoConnect = j["sessionAutoConnect"].get<bool>();
  }

  if (j.contains("sessionShowFullTypeToken") && j["sessionShowFullTypeToken"].is_boolean()) {
    m_session_bShowFullTypeToken = j["sessionShowFullTypeToken"].get<bool>();
  }

  if (j.contains("sessionAutoSaveTxRows") && j["sessionAutoSaveTxRows"].is_boolean()) {
    m_session_bAutoSaveTxRows = j["sessionAutoSaveTxRows"].get<bool>();
  }

  // * * * Config * * *
  if (j.contains("configNumericBase") && j["configNumericBase"].is_number()) {
    m_config_base = static_cast<numerical_base>(j["configNumericBase"].get<int>());
  }

  if (j.contains("configDisableColors") && j["configDisableColors"].is_boolean()) {
    m_config_bDisableColors = j["configDisableColors"].get<bool>();
  }

  if (j.contains("configTimeout") && j["configTimeout"].is_number()) {
    m_config_timeout = j["configTimeout"].get<uint32_t>();
  }

  // * * * Firmware * * *
  if (j.contains("firmwareDeviceCodeRequired") && j["firmwareDeviceCodeRequired"].is_boolean()) {
    m_firmware_devicecode_required = j["firmwareDeviceCodeRequired"].get<bool>();
  }

  // VSCP event database last load date/time
  // ---------------------------------------
  if (j.contains("last-eventdb-download") && j["last-eventdb-download"].is_number()) {
    m_lastEventDbLoadDateTime = QDateTime::fromMSecsSinceEpoch(j["last-eventdb-download"].get<uint64_t>());
  }

  // * * *  Read in defined connections  * * *
  // Note!!! Se notes in save

  if (j.contains("connections") && j["connections"].is_array()) {
    json conn_array = j["connections"];
    for (json::iterator it = conn_array.begin(); it != conn_array.end(); ++it) {
      addConnection(*it, false);
    }
  }

  // VSCP Home folder
  // ----------------

#ifdef WIN32
  if (j.contains("vscpHomeFolder") && j["vscpHomeFolder"].is_string()) {
    m_vscpHomeFolder = j["vscpHomeFolder"].get<std::string>();
  }
  else {
    m_vscpHomeFolder = settings.value("vscpHomeFolder", "c:/program files/vscp").toString();
  }
#else
  if (j.contains("vscpHomeFolder") && j["vscpHomeFolder"].is_string()) {
    m_vscpHomeFolder = j["vscpHomeFolder"].get<std::string>().c_str();
  }
  else {
    m_vscpHomeFolder = "/var/lib/vscp";
  }
#endif
  fprintf(stderr, "VSCP home folder: %s\n", m_vscpHomeFolder.toStdString().c_str());

  // Main window dimensions
  if (j.contains("mainwindow-dimensions") && j["mainwindow-dimensions"].is_array()) {
    json dim = j["mainwindow-dimensions"];
    if (dim.size() == 4) {
      m_mainWindowRect = QRect(dim[0].get<int>(),
                               dim[1].get<int>(),
                               dim[2].get<int>(),
                               dim[3].get<int>());
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeSettings
//

void
vscpworks::writeSettings()
{
  json j, jj;
  json conn_array = json::array();

  // General settings
  j["vscpHomeFolder"]        = m_vscpHomeFolder.toStdString();
  j["numericBase"]           = m_base;
  j["PreferredLanguage"]     = m_preferredLanguage;
  j["bDarkTheme"]            = m_bEnableDarkTheme;
  j["bAskBeforeDelete"]      = m_bAskBeforeDelete;
  j["bSaveAlwaysJson"]       = m_bSaveAlwaysJSON;
  j["last-eventdb-download"] = m_lastEventDbLoadDateTime.toMSecsSinceEpoch();

  // * * * Logging * * *
  int level = 4; // Default: 4 == "information";
  switch (m_fileLogLevel) {
    case spdlog::level::trace:
      level = 0;
      break;
    case spdlog::level::debug:
      level = 1;
      break;
    default:
    case spdlog::level::info:
      level = 2;
      break;
    case spdlog::level::warn:
      level = 3;
      break;
    case spdlog::level::err:
      level = 4;
      break;
    case spdlog::level::critical:
      level = 5;
      break;
    case spdlog::level::off:
      level = 6;
      break;
  };

  j["fileLogLevel"]    = level;
  j["fileLogPath"]     = m_fileLogPath;
  j["fileLogPattern"]  = m_fileLogPattern;
  j["fileLogMaxSize"]  = m_maxFileLogSize;
  j["fileLogMaxFiles"] = m_maxFileLogFiles;

  level = 4; // Default: 4 == "information";
  switch (m_consoleLogLevel) {
    case spdlog::level::trace:
      level = 0;
      break;
    case spdlog::level::debug:
      level = 1;
      break;
    default:
    case spdlog::level::info:
      level = 2;
      break;
    case spdlog::level::warn:
      level = 3;
      break;
    case spdlog::level::err:
      level = 4;
      break;
    case spdlog::level::critical:
      level = 5;
      break;
    case spdlog::level::off:
      level = 6;
      break;
  };

  j["consoleLogLevel"]   = level;
  j["consoleLogPattern"] = m_consoleLogPattern;

  // * * * Session * * *
  j["sessionTimeout"]            = m_session_timeout;
  j["maxSessionEvents"]          = m_session_maxEvents;
  j["sessionClassDisplayFormat"] = static_cast<int>(m_session_ClassDisplayFormat);
  j["sessionTypeDisplayFormat"]  = static_cast<int>(m_session_TypeDisplayFormat);
  j["sessionGuidDisplayFormat"]  = static_cast<int>(m_session_GuidDisplayFormat);
  j["sessionAutoConnect"]        = m_session_bAutoConnect;
  j["sessionShowFullTypeToken"]  = m_session_bShowFullTypeToken;
  j["sessionAutoSaveTxRows"]     = m_session_bAutoSaveTxRows;

  // * * * Config * * *
  j["configNumericBase"]   = static_cast<int>(m_config_base);
  j["configDisableColors"] = m_config_bDisableColors;
  j["configTimeout"]       = m_config_timeout;

  // * * * Firmware * * *
  j["firmwareDeviceCodeRequired"] = m_firmware_devicecode_required;

  QMap<std::string, json>::const_iterator it = m_mapConn.constBegin();
  while (it != m_mapConn.constEnd()) {
    // jj = json::parse((*it).toStdString());
    conn_array.push_back(it.value());
    it++;
  }

  j["connections"] = conn_array;

  // Main window dimensions
  {
    json dim = json::array();
    dim.push_back(m_mainWindowRect.x());
    dim.push_back(m_mainWindowRect.y());
    dim.push_back(m_mainWindowRect.width());
    dim.push_back(m_mainWindowRect.height());
    j["mainwindow-dimensions"] = dim;
  }

  spdlog::trace("{0}", j.dump(4).c_str());

  std::ofstream of(m_configFile.toStdString());
  of << std::setw(4) << j << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// addConnection
//

bool
vscpworks::addConnection(json& conn, bool bSave)
{
  spdlog::trace(conn.dump(4).c_str());

  // If no UUID is set, create one
  QString struuid;
  if (conn.contains("uuid") && conn["uuid"].is_string()) {
    // Make sure it's not empty
    struuid = conn["uuid"].get<std::string>().c_str();
    if (!struuid.trimmed().length()) {
      // Create a new UUID
      conn["uuid"] = QUuid::createUuid().toString().toStdString();
    }
  }
  else {
    // Create a new UUID
    conn["uuid"] = QUuid::createUuid().toString().toStdString();
  }

  struuid = conn["uuid"].get<std::string>().c_str();

  // Add configuration item to map
  m_mapConn[struuid.toStdString()] = conn;

  // Save settings if requested to do so
  if (bSave)
    writeSettings();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeConnection
//

bool
vscpworks::removeConnection(const QString& uuid, bool bSave)
{
  QMap<std::string, json>::iterator it;
  int n = m_mapConn.remove(uuid.toStdString());

  // Save settings if requested to do so
  if (bSave)
    writeSettings();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadEventDb
//

bool
vscpworks::loadEventDb(void)
{
  m_mutexVscpEventsMaps.lock();

  m_evdb         = QSqlDatabase::addDatabase("QSQLITE", "vscpevents");
  QString dbpath = m_shareFolder;
  dbpath += "vscp_events.sqlite3";

  // If the database don't exist, bail out
  if (!QFile::exists(dbpath)) {
    QString err = QString(tr("The VSCP event database does not exist. Is it available? [%s]")).arg(dbpath);
    fprintf(stderr, "%s", err.toStdString().c_str());
    m_mutexVscpEventsMaps.unlock();
    return false;
  }

  m_evdb.setDatabaseName(dbpath);

  if (!m_evdb.open()) {
    QString err = QString(tr("The VSCP event database could not be opened. Is it available? [%s]")).arg(dbpath);
    fprintf(stderr, "%s", err.toStdString().c_str());
    m_mutexVscpEventsMaps.unlock();
    return false;
  }
  else {
    QSqlQuery queryClass("SELECT * FROM vscp_class order by class", m_evdb);

    while (queryClass.next()) {
      uint16_t classid               = queryClass.value(0).toUInt();
      QString className              = queryClass.value(1).toString();
      QString classToken             = queryClass.value(2).toString();
      m_mapVscpClassToToken[classid] = classToken;

      QString sqlTypeQuery = QString("SELECT * FROM vscp_type WHERE link_to_class=%1").arg(classid);
      QSqlQuery queryType(sqlTypeQuery, m_evdb);

      while (queryType.next()) {
        uint typeIdx                                               = queryType.value(0).toUInt();
        uint16_t typeId                                            = queryType.value(1).toUInt();
        QString typeToken                                          = queryType.value(3).toString();
        uint32_t combined                                          = ((classid << 16) + typeId);
        m_mapVscpTypeToToken[(((uint32_t)classid << 16) + typeId)] = typeToken;
      }
    }
  }

  m_mutexVscpEventsMaps.unlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// openVscpWorksDatabase
//

bool
vscpworks::openVscpWorksDatabase(void)
{
  int rv;
  sqlite3_stmt* ppStmt;

  // Set up database
  QString eventdbname = m_shareFolder + "vscpworks.sqlite3";

  if (SQLITE_OK != sqlite3_open(eventdbname.toStdString().c_str(), &m_db_vscp_works)) {
    spdlog::error("Failed to open database: {0}\n", eventdbname.toStdString().c_str());
    return false;
  }

  // Create GUID table if it does not exist
  if (SQLITE_OK !=
      (rv = sqlite3_exec(m_db_vscp_works,
                         "CREATE TABLE IF NOT EXISTS guid ("
                         "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "guid	TEXT UNIQUE,"
                         "name	TEXT,"
                         "description   TEXT);",
                         NULL,
                         NULL,
                         NULL))) {
    spdlog::error("Failed to create GUID table. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    return false;
  }

  // Create GUID name index
  if (SQLITE_OK !=
      (rv = sqlite3_exec(m_db_vscp_works,
                         "CREATE INDEX IF NOT EXISTS \"idxGuidName\" ON \"guid\" (\"guid\" ASC)",
                         NULL,
                         NULL,
                         NULL))) {
    spdlog::error("Failed to create GUID table. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    return false;
  }

  // Create sensor index table if it does not exist
  if (SQLITE_OK !=
      (rv = sqlite3_exec(m_db_vscp_works,
                         "CREATE TABLE IF NOT EXISTS \"sensorindex\" ("
                         "\"idx\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "\"link_to_guid\"	INTEGER, "
                         "\"sensor\"	        INTEGER, "
                         "\"name\"	        TEXT, "
                         "\"description\"	TEXT );",
                         NULL,
                         NULL,
                         NULL))) {
    spdlog::error("Failed to create sensor index table. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    return false;
  }

  // Create sensor link + idx unique  index
  if (SQLITE_OK !=
      (rv = sqlite3_exec(m_db_vscp_works,
                         "CREATE UNIQUE INDEX IF NOT EXISTS \"idxSensors\" ON \"sensorindex\" (\"link_to_guid\" ASC, \"sensor\" ASC)",
                         NULL,
                         NULL,
                         NULL))) {
    spdlog::error("Failed to create GUID table. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    return false;
  }

  // Create log table if it does not exist
  if (SQLITE_OK !=
      (rv = sqlite3_exec(m_db_vscp_works,
                         "CREATE TABLE IF NOT EXISTS log ("
                         "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "level INTEGER,"
                         "datetime TEXT,"
                         "message TEXT);",
                         NULL,
                         NULL,
                         NULL))) {
    spdlog::error("Failed to create log table. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    return false;
  }

  // Load known GUID's to memory
  loadGuidTable();

  // Load known sensors to memory
  loadSensorTable();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadGuidTable
//

bool
vscpworks::loadGuidTable(void)
{
  int rv;
  sqlite3_stmt* ppStmt;

  m_mutexGuidMap.lock();

  // Query known GUID's
  if (SQLITE_OK !=
      (rv = sqlite3_prepare(m_db_vscp_works,
                            "SELECT * FROM guid order by name",
                            -1,
                            &ppStmt,
                            NULL))) {
    spdlog::error("Failed to query GUID's. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    m_mutexGuidMap.unlock();
    return false;
  }

  while (SQLITE_ROW == sqlite3_step(ppStmt)) {
    QString guid                  = QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 1));
    QString name                  = QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 2));
    m_mapGuidToSymbolicName[guid] = name;
  }
  sqlite3_finalize(ppStmt);

  m_mutexGuidMap.unlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadSensorTable
//

bool
vscpworks::loadSensorTable(void)
{
  int rv;
  sqlite3_stmt* ppStmt;

  m_mutexSensorIndexMap.lock();

  // Query known GUID's
  if (SQLITE_OK !=
      (rv = sqlite3_prepare(m_db_vscp_works,
                            "SELECT * FROM sensorindex order by sensor",
                            -1,
                            &ppStmt,
                            NULL))) {
    spdlog::error("Failed to query sensor indexes. rv={0} {1}", rv, sqlite3_errmsg(m_db_vscp_works));
    m_mutexSensorIndexMap.unlock();
    return false;
  }

  while (SQLITE_ROW == sqlite3_step(ppStmt)) {
    int link_to_guid                                             = sqlite3_column_int(ppStmt, 1);
    int sensor                                                   = sqlite3_column_int(ppStmt, 2);
    QString name                                                 = QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 3));
    m_mapSensorIndexToSymbolicName[(link_to_guid << 8) + sensor] = name;
  }
  sqlite3_finalize(ppStmt);

  m_mutexSensorIndexMap.unlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addGuid
//

bool
vscpworks::addGuid(QString guid, QString name)
{
  m_mutexGuidMap.lock();

  // If the GUID symbolic name already exist we
  // are OK and done
  if (0 != m_mapGuidToSymbolicName[guid].length()) {
    m_mutexGuidMap.unlock();
    return true;
  }

  // QString strInsert = "INSERT INTO guid (guid, name) VALUES (%1,%2);";
  // QSqlQuery queryClass(strInsert.arg(guid).arg(name), m_db_vscp_works);
  // if (queryClass.lastError().isValid()) {
  //   spdlog::error(std::string(tr("Failed to insert GUID into database %s")
  //                               .arg(queryClass.lastError().text())
  //                               .toStdString()));
  //   qDebug() << queryClass.lastError();
  //   return false;
  // }

  QString strInsert = tr("INSERT INTO guid (guid, name) VALUES (%1,%2);").arg(guid).arg(name);
  if (SQLITE_OK != sqlite3_exec(m_db_vscp_works, strInsert.toStdString().c_str(), NULL, NULL, NULL)) {
    spdlog::error(std::string(tr("Failed to insert GUID into database %s")
                                .arg(sqlite3_errmsg(m_db_vscp_works))
                                .toStdString()));
    return false;
  }

  // Add to loaded
  m_mapGuidToSymbolicName[guid] = name;

  m_mutexGuidMap.unlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// getIdxForGuidRecord
//

int
vscpworks::getIdxForGuidRecord(const QString& guid)
{
  int index = -1;
  sqlite3_stmt* ppStmt;

  m_mutexGuidMap.lock();

  QString strInsert = tr("SELECT * FROM guid WHERE guid='%1';").arg(guid);
  if (SQLITE_OK != sqlite3_prepare(m_db_vscp_works, strInsert.toStdString().c_str(), -1, &ppStmt, NULL)) {
    m_mutexGuidMap.unlock();
    return -1;
  }

  if (SQLITE_ROW == sqlite3_step(ppStmt)) {
    index = sqlite3_column_int(ppStmt, 0);
  }
  sqlite3_finalize(ppStmt);
  m_mutexGuidMap.unlock();
  return index;
}

///////////////////////////////////////////////////////////////////////////////
// getShortTypeToken
//

QString
vscpworks::getShortTypeToken(uint16_t vscpClass, uint16_t vscpType)
{
  // * * * Type * * *
  QString strTypeToken = m_mapVscpTypeToToken[((uint32_t)vscpClass << 16) + vscpType];
  if (vscpClass >= 1024) {
    strTypeToken = strTypeToken.right(strTypeToken.length() - 11); // Remove "VSCP2_TYPE_"
  }
  else {
    strTypeToken = strTypeToken.right(strTypeToken.length() - 10); // Remove "VSCP_TYPE_"
  }
  int posUnderscore = strTypeToken.indexOf("_");
  if (posUnderscore)
    posUnderscore++;
  return strTypeToken.right(strTypeToken.length() - posUnderscore);
}

///////////////////////////////////////////////////////////////////////////////
// getHelpUrlForClass
//

QString
vscpworks::getHelpUrlForClass(uint16_t vscpClass)
{
  // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol
  mustache templ{ "https://grodansparadis.github.io/vscp-doc-spec/#/./{{vscp-class-token}}" };

  kainjow::mustache::data _data;
  _data.set("vscp-class-token", m_mapVscpClassToToken[vscpClass].toLower().toStdString());
  return templ.render(_data).c_str();
}

///////////////////////////////////////////////////////////////////////////////
// getHelpUrlForType
//

QString
vscpworks::getHelpUrlForType(uint16_t vscpClass, uint16_t vscpType)
{
  // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6
  std::string strTemp = getHelpUrlForClass(vscpClass).toStdString();
  strTemp += "?id=type{{vscp-type-id}}";
  mustache templ{ strTemp };

  kainjow::mustache::data _data;
  _data.set("vscp-type-id", QString::number(vscpType).toStdString());
  return templ.render(_data).c_str();
}

///////////////////////////////////////////////////////////////////////////////
// getUnitInfo
//

CVscpUnit
vscpworks::getUnitInfo(uint16_t vscpClass, uint16_t vscpType, uint8_t unit)
{
  CVscpUnit u(unit);

  u.m_vscp_class = vscpClass;
  u.m_vscp_type  = vscpType;

  QString strQuery = "SELECT * FROM vscp_unit WHERE nunit='%1' AND link_to_class=%2 AND link_to_type=%3;";

  QSqlQuery query(m_evdb);
  query.exec(strQuery.arg(unit).arg(vscpClass).arg(vscpType));

  while (query.next()) {
    u.m_unit         = query.value(3).toInt();
    u.m_name         = query.value(4).toString().toStdString();
    u.m_description  = query.value(5).toString().toStdString();
    u.m_conversion0  = query.value(6).toString().toStdString();
    u.m_conversion   = query.value(7).toString().toStdString();
    u.m_symbol_ascii = query.value(8).toString().toStdString();
    u.m_symbol_utf8  = query.value(9).toString().toStdString();
  }

  return u;
}

///////////////////////////////////////////////////////////////////////////////
// addVscpEventToJsRenderFunction
//

bool
vscpworks::addVscpEventToJsRenderFunction(QJSEngine& engine, vscpEvent* pev)
{
  QJSValue result;
  std::string str;
  std::string strEvaluate;

  if (nullptr != pev)
    return false;

  strEvaluate = "var e = {};e.vscpData = [";
  vscp_writeDataToString(str, pev);
  strEvaluate += str.c_str();
  strEvaluate += "];e.sizeData=";
  strEvaluate += vscp_str_format("%d", pev->sizeData);
  strEvaluate += ";e.guid = [";
  vscp_writeGuidArrayToString(str, pev->GUID, true);
  strEvaluate += str;
  strEvaluate += "];e.vscpHead=";
  strEvaluate += vscp_str_format("%d", pev->head);
  strEvaluate += ";e.vscpCrc=";
  strEvaluate += vscp_str_format("%d", pev->crc);
  strEvaluate += ";e.vscpObid=";
  strEvaluate += vscp_str_format("%lu", pev->obid);
  strEvaluate += ";e.vscpTimeStamp=";
  strEvaluate += vscp_str_format("%lu", pev->timestamp);
  strEvaluate += ";e.vscpClass=";
  strEvaluate += vscp_str_format("%d", pev->vscp_class);
  strEvaluate += ";e.vscpType=";
  strEvaluate += vscp_str_format("%d", pev->vscp_type);
  strEvaluate += ";e.vscpYear=";
  strEvaluate += vscp_str_format("%d", pev->year);
  strEvaluate += ";e.vscpMonth=";
  strEvaluate += vscp_str_format("%d", pev->month);
  strEvaluate += ";e.vscpDay=";
  strEvaluate += vscp_str_format("%d", pev->day);
  strEvaluate += ";e.vscpHour=";
  strEvaluate += vscp_str_format("%d", pev->hour);
  strEvaluate += ";e.vscpMinute=";
  strEvaluate += vscp_str_format("%d", pev->minute);
  strEvaluate += ";e.vscpSecond=";
  strEvaluate += vscp_str_format("%d", pev->second);
  strEvaluate += ";";

  result = engine.evaluate(strEvaluate.c_str());
  return result.isError();
}

///////////////////////////////////////////////////////////////////////////////
// replaceVscpRenderVariables
//

std::string
vscpworks::replaceVscpRenderVariables(const std::string& str)
{
  std::string strwrk = str;

  mustache templVar{ strwrk };
  kainjow::mustache::data _data;
  _data.set("newline", "\r\n");
  _data.set("quote", "\"");
  _data.set("quote", "'");
  templVar.render(_data);

  return strwrk;
}

///////////////////////////////////////////////////////////////////////////////
// getVscpRenderData
//

QStringList
vscpworks::getVscpRenderData(uint16_t vscpClass, uint16_t vscpType, QString type)
{
  QStringList strList;
  QString strQuery = "SELECT * FROM vscp_render WHERE type='%1' AND link_to_class=%2 AND link_to_type=%3;";
  qDebug() << strQuery.arg(type).arg(vscpClass).arg(vscpType);

  QSqlQuery query(m_evdb);
  query.exec(strQuery.arg(type).arg(vscpClass).arg(vscpType));
  // Try if there is a general render definition if none
  // is defined for the event
  qDebug() << query.size();
  qDebug() << query.numRowsAffected();
  if (query.next()) {
    query.first();
  }
  else {
    // Definition for all events of class
    query.exec(strQuery.arg(type).arg(vscpClass).arg(-1));
    query.first();
  }
  do {

    // * * * VARIABLES * * *
    QString vscpVariables = query.value(4).toString();
    if (vscpVariables.startsWith("BASE64:", Qt::CaseInsensitive)) {
      vscpVariables = vscpVariables.right(vscpVariables.length() - 7);
      vscpVariables = QByteArray::fromBase64(vscpVariables.toLatin1(), QByteArray::Base64Encoding);
    }

    qDebug() << vscpVariables;
    qDebug() << "---------------------------------------------";
    vscpVariables.replace("\"", "&quote;").replace("&quote;", "'").replace("&amp;", "&").replace("&gt;", ">").replace("&lt;", "<");
    qDebug() << vscpVariables;

    // * * * TEMPLATES * * *
    QString vscpTemplate = query.value(5).toString();
    if (vscpTemplate.startsWith("BASE64:", Qt::CaseInsensitive)) {
      vscpTemplate = vscpTemplate.right(vscpTemplate.length() - 7);
      vscpTemplate = QByteArray::fromBase64(vscpVariables.toLatin1(), QByteArray::Base64Encoding);
    }
    vscpTemplate.replace("\"", "&quote;").replace("&quote;", "'").replace("&amp;", "&").replace("&gt;", ">").replace("&lt;", "<");
    qDebug() << vscpTemplate;
    strList << vscpVariables << vscpTemplate;
  } while (query.next());

  return strList;
}

///////////////////////////////////////////////////////////////////////////////
// getVscpRenderFunctions
//

bool
vscpworks::getVscpRenderFunctions(std::map<std::string, std::string>& map,
                                  std::string& strVariables)
{
  // QStringList strlst = QString(strVariables.c_str())::split("\n");
  std::deque<std::string> strlst;
  vscp_split(strlst, strVariables, "\n");

  std::string name;
  std::string func;
  size_t posFunc;
  size_t posColon;
  foreach (std::string str, strlst) {
    vscp_trim(str);
    if (std::string::npos != (posFunc = str.find("function()"))) {
      // We have a function  "id: function() {....}
      // it can have one line or be multiline 6
      if (std::string::npos == (posColon = str.find(":"))) {
        return false;
      }
      name = vscp_str_left(str, posColon);
      func = vscp_str_right(str, str.length() - posFunc);
    }
    else {
      func += str + "\n";
    }

    // When {} pairs are equal in func we are done
    int cnt = 0;
    foreach (QChar c, func) {
      if ('{' == c) {
        cnt++;
      }
      if ('}' == c) {
        cnt--;
      }
    }

    if (!cnt) {
      map[name] = func;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// renderVscpDataTemplate
//

std::string
vscpworks::renderVscpDataTemplate(std::map<std::string, std::string>& map,
                                  std::string& strTemplate)
{
  std::string strResult;

  return strResult;
}

///////////////////////////////////////////////////////////////////////////////
// newClientWindow
//

void
vscpworks::newChildWindow(QMainWindow* pwnd)
{
  m_childWindows.push_back(pwnd);
}

///////////////////////////////////////////////////////////////////////////////
// clearChildWindow
//

void
vscpworks::clearChildWindow(QMainWindow* pwnd)
{
  for (QMainWindow* pitem : m_childWindows) {
    if (pitem == pwnd) {
      pwnd->close();
      m_childWindows.remove(pwnd);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// downloadMDF
//

int
vscpworks::downloadMDF(CStandardRegisters& stdregs,
                       CMDF& mdf,
                       QString& path,
                       std::function<void(int, const char*)> statusCallback)
{
  if (nullptr != statusCallback) {
    statusCallback(0, "Starting MDF download");
  }
  spdlog::trace("Download MDF");

  // Get GUID
  cguid guid;
  stdregs.getGUID(guid);
  spdlog::trace("Standard register getGUID = {}", guid.toString());

  std::string url = stdregs.getMDF();
  spdlog::trace("URL for MDF = {}", url);

  // create a temporary file name for remote MDF
  std::string tempMdfFileName;
  for (int i = 0; i < url.length(); i++) {
    if ((url[i] == '/') || (url[i] == '\\')) {
      tempMdfFileName += "_";
    }
    else {
      tempMdfFileName += url[i];
    }
  }

  // mkstemp()
  std::string tempPath =
    QStandardPaths::writableLocation(QStandardPaths::TempLocation)
      .toStdString();
  tempPath += "/";
  tempPath += tempMdfFileName;

  path = tempPath.c_str();
  spdlog::debug("Temporary path: {}", tempPath);

  if (nullptr != statusCallback) {
    statusCallback(10, "Downloading MDF file...");
  }

  CURLcode curl_rv;
  curl_rv = mdf.downLoadMDF(url, tempPath);
  if (CURLE_OK != curl_rv) {
    if (nullptr != statusCallback) {
      statusCallback(10, "Failed to download MDF file for device.");
    }
    spdlog::error("Failed to download MDF {0} curl rv={1}", url, (int)curl_rv);
    QApplication::restoreOverrideCursor();
    return VSCP_ERROR_COMMUNICATION;
  }

  if (nullptr != statusCallback) {
    statusCallback(60, "MDF downloaded.");
  }
  spdlog::debug("MDF Downloaded to {}", tempPath);

  // * * * Parse  MDF * * *

  spdlog::debug("Parsing MDF");
  int rv = mdf.parseMDF(tempPath);
  if (VSCP_ERROR_SUCCESS != rv) {
    if (nullptr != statusCallback) {
      statusCallback(80, "Faild to parse MDF");
    }
    spdlog::error("Failed to parse MDF {0} rv={1}", tempPath, rv);
    return VSCP_ERROR_PARSING;
  }

  spdlog::debug("Parsing MDF OK");
  if (nullptr != statusCallback) {
    statusCallback(100, "MDF downloaded and parsed");
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// write_data
//

static size_t
write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  spdlog::trace("curl write_data");
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

///////////////////////////////////////////////////////////////////////////////
// downLoadFromURL
//

CURLcode
vscpworks::downLoadFromURL(const std::string& url, const std::string& tempFileName)
{
  CURL* curl;
  FILE* fp;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    fp = fopen(tempFileName.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);
    // always cleanup
    curl_easy_cleanup(curl);
    fclose(fp);
  }

  return res;
}