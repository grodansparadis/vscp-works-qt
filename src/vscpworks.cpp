// vscpworks.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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
#include <QJsonArray>
#include <QJsonObject>
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
#ifdef WIN32
  m_fileLogPath = "vscpworks.log";
#else
  m_fileLogPath = "~/.local/share/VSCP/vscpworks+/logs/vscpworks.log";
#endif
  m_maxFileLogSize  = 5242880;
  m_maxFileLogFiles = 7;

  m_bEnableConsoleLog = true;
  m_consoleLogLevel   = spdlog::level::info;
  m_consoleLogPattern = "[vscpworks+] [%^%l%$] %v";

  // After the following it is possible to create and destroy event objects
  // dynamically at run-time
  int idEvent   = qRegisterMetaType<vscpEvent>();
  int idEventEx = qRegisterMetaType<vscpEventEx>();
}

///////////////////////////////////////////////////////////////////////////////
// ~vscpworks
//

vscpworks::~vscpworks()
{
  writeSettings();

  for (QMainWindow* pwnd : m_childWindows) {
    if (nullptr != pwnd) {
      pwnd->close();
      m_childWindows.remove(pwnd);
      delete pwnd;
      break;
    }
  }
  m_worksdb.close();
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

void
vscpworks::loadSettings(void)
{
  QString str;

  QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

  // Configuration folder
  // --------------------
  // Linux: "/home/akhe/.config"                      Config file is here (VSCP/vscp-works-qt)
  // Windows:
  {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    path += "/";
    path += QCoreApplication::applicationName();
    path += "/";
    m_configFolder = settings.value("configFolder", path).toString();
  }

  // Share folder
  // ------------
  // Linux: "/home/akhe/.local/share/vscp-works-qt"   user data is here
  // Windows:
  {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    path += "/";
    m_shareFolder = settings.value("shareFolder", path).toString();
    // If folder does not exist, create it
    QDir dir(path);
    if (!dir.exists()) {
      dir.mkpath(".");
    }
    // Make a folder for  receive sets
    dir.mkpath("./rxsets");
    // Make a folder for transmission sets
    dir.mkpath("./txsets");
    // Make a folder for autosaved/loaded data
    dir.mkpath("./cache");
  }

// VSCP Home folder
// ----------------
#ifdef WIN32
  m_vscpHomeFolder = settings.value("vscpHomeFolder", "c:/program files/vscp").toString();
#else
  m_vscpHomeFolder = settings.value("vscpHomeFolder", "/var/lib/vscp").toString();
#endif

  // System numerical base
  // ---------------------
  m_base = static_cast<numerical_base>(settings.value("numericBase", "0").toInt());

  // preferred language
  m_preferredLanguage = settings.value("PreferredLanguage", "en").toString().toStdString();
  ;

  m_bEnableDarkTheme = settings.value("bDarkTheme", true).toBool();
  m_bAskBeforeDelete = settings.value("bAskBeforeDelete", true).toBool();
  m_bSaveAlwaysJSON  = settings.value("bSaveAlwaysJson", true).toBool();

  // * * * Logging * * *
  m_bEnableFileLog = true;
  int level        = settings.value("fileLogLevel", 4).toInt(); // Default: 4 == "information";
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
  };
  m_fileLogPattern  = settings.value("fileLogPattern", "%c - [%^%l%$] %v").toString().toStdString();
  m_fileLogPath     = settings.value("fileLogPath", "~/.local/share/VSCP/vscpworks+/logs/vscpworks.log").toString().toStdString();
  m_maxFileLogSize  = settings.value("fileLogMaxSize", 5 * 1024 * 1024).toInt();
  m_maxFileLogFiles = settings.value("fileLogMaxFiles", 10).toInt();

  // console log level
  m_bEnableConsoleLog = true;
  level               = settings.value("consoleLogLevel", 4).toInt(); // Default: 4 == "information";
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
  };

  m_consoleLogPattern = settings.value("consoleLogPattern", "%c [%^%l%$] %v").toString().toStdString();

  // * * * Session * * *

  m_session_maxEvents          = settings.value("maxSessionEvents", -1).toInt();
  m_session_timeout            = settings.value("sessionTimeout", "1000").toUInt();
  m_session_ClassDisplayFormat = static_cast<CFrmSession::classDisplayFormat>(settings.value("sessionClassDisplayFormat",
                                                                                             static_cast<int>(CFrmSession::classDisplayFormat::symbolic))
                                                                                .toInt());
  m_session_TypeDisplayFormat  = static_cast<CFrmSession::typeDisplayFormat>(settings.value("sessionTypeDisplayFormat",
                                                                                           static_cast<int>(CFrmSession::typeDisplayFormat::symbolic))
                                                                              .toInt());
  m_session_GuidDisplayFormat  = static_cast<CFrmSession::guidDisplayFormat>(settings.value("sessionGuidDisplayFormat",
                                                                                           static_cast<int>(CFrmSession::guidDisplayFormat::guid))
                                                                              .toInt());
  m_session_bAutoConnect       = settings.value("sessionAutoConnect", true).toBool();
  m_session_bShowFullTypeToken = settings.value("sessionShowFullTypeToken", true).toBool();
  m_session_bAutoSaveTxRows    = settings.value("sessionAutoSaveTxRows", true).toBool();

  // * * * Config * * *
  m_config_base           = static_cast<numerical_base>(settings.value("configNumericBase", "1").toInt());
  m_config_bDisableColors = settings.value("configDisableColors", false).toBool();
  m_config_timeout        = settings.value("configTimeout", "1000").toUInt();

  // * * * Firmware * * *
  m_firmware_devicecode_required = settings.value("firmwareDeviceCodeRequired", true).toBool();

  // VSCP event database last load date/time
  // ---------------------------------------
  m_lastEventDbLoadDateTime = settings.value("last-eventdb-download", "1970-01-01T00:00:00Z").toDateTime();

  // * * *  Read in defined connections  * * *
  // Note!!! Se notes in save

  int size = settings.beginReadArray("hosts");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    QString strJson = settings.value("connection").toString();

    QJsonObject conn;
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    if (!doc.isNull()) {
      if (doc.isObject()) {
        conn = doc.object();
        addConnection(conn);
      }
      else {
        qDebug() << "Connection document is not an object";
      }
    }
    else {
      qDebug() << "Invalid JSON for connection object\n";
    }
  }
  settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
// writeSettings
//

void
vscpworks::writeSettings()
{
  QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

  // General settings
  settings.setValue("configFolder", m_configFolder);
  settings.setValue("shareFolder", m_shareFolder);
  settings.setValue("vscpHomeFolder", m_vscpHomeFolder);
  settings.setValue("numericBase", QString::number(static_cast<int>(m_base)));
  settings.setValue("PreferredLanguage", m_preferredLanguage.c_str());
  settings.setValue("bDarkTheme", m_bEnableDarkTheme);
  settings.setValue("bAskBeforeDelete", m_bAskBeforeDelete);
  settings.setValue("bSaveAlwaysJson", m_bSaveAlwaysJSON);
  settings.setValue("last-eventdb-download", m_lastEventDbLoadDateTime);

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

  settings.setValue("fileLogLevel", level);
  settings.setValue("fileLogPattern", QString::fromStdString(m_fileLogPattern));
  settings.setValue("fileLogPath", QString::fromStdString(m_fileLogPath));
  settings.setValue("fileLogMaxSize", m_maxFileLogSize);
  settings.setValue("fileLogMaxFiles", m_maxFileLogFiles);

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

  settings.setValue("consoleLogLevel", level);
  settings.setValue("consoleLogPattern", QString::fromStdString(m_consoleLogPattern));

  // * * * Session * * *

  settings.setValue("sessionTimeout", m_session_timeout);
  settings.setValue("maxSessionEvents", m_session_maxEvents);

  settings.setValue("sessionClassDisplayFormat", static_cast<int>(m_session_ClassDisplayFormat));
  settings.setValue("sessionTypeDisplayFormat", static_cast<int>(m_session_TypeDisplayFormat));
  settings.setValue("sessionGuidDisplayFormat", static_cast<int>(m_session_GuidDisplayFormat));

  settings.setValue("sessionAutoConnect", m_session_bAutoConnect);
  settings.setValue("sessionShowFullTypeToken", m_session_bShowFullTypeToken);
  settings.setValue("sessionAutoSaveTxRows", m_session_bAutoSaveTxRows);

  // * * * Config * * *
  settings.setValue("configNumericBase", static_cast<int>(m_config_base));
  settings.setValue("configDisableColors", m_config_bDisableColors);
  settings.setValue("configTimeout", m_config_timeout);

  // * * * Firmware * * *
  settings.setValue("firmwareDeviceCodeRequired", m_firmware_devicecode_required);

  writeConnections();
}

///////////////////////////////////////////////////////////////////////////////
// writeConnections
//

void
vscpworks::writeConnections(void)
{
  QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

  // Remove old entries
  int size = settings.beginReadArray("hosts");
  for (auto i = 0; i < size; i++) {
    settings.setArrayIndex(i);
    settings.remove("connection");
  }
  settings.endArray();

  // Connections
  settings.beginWriteArray("hosts");
  int i = 0;

  /*
      Note!
      We need to convert QJsonObject into string due to a bug
      that prevent saving/loading in some cases.
  */
  QMap<QString, QJsonObject>::const_iterator it = m_mapConn.constBegin();
  while (it != m_mapConn.constEnd()) {
    settings.setArrayIndex(i++);
    QJsonDocument doc(it.value());
    QString strJson(doc.toJson(QJsonDocument::Compact));
    // qDebug() << it.key() << ": " << it.value() << ":" << strJson;
    settings.setValue("connection", strJson);
    it++;
  }
  settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
// addConnection
//

bool
vscpworks::addConnection(QJsonObject& conn, bool bSave)
{
  if (!conn["uuid"].toString().trimmed().length()) {
    conn["uuid"] = QUuid::createUuid().toString();
  }

  // Add configuration item to map
  m_mapConn[conn["uuid"].toString().trimmed()] = conn;

  // Save settings if requested to do so
  if (bSave)
    writeConnections();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeConnection
//

bool
vscpworks::removeConnection(const QString& uuid, bool bSave)
{
  QMap<QString, QJsonObject>::iterator it;
  int n = m_mapConn.remove(uuid);

  // Save settings if requested to do so
  if (bSave)
    writeConnections();

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
// log
//

void
vscpworks::log(int level, const QString& message)
{
  // Log only messages
  // if (level <= m_logLevel) {

  //     QDateTime now = QDateTime::currentDateTime();

  //     QString strQuery = "INSERT INTO log (level, datetime, message) values (";
  //     strQuery +=  QString::number(level);
  //     strQuery += ",";
  //     strQuery += "'" + now.toString() + "'";
  //     strQuery += ",";
  //     strQuery += "'" +message + "'";
  //     strQuery += ");";

  //     QSqlQuery query = QSqlQuery( m_worksdb );
  //     if (!query.exec(strQuery)) {
  //         qDebug() << "Failed to insert log message";
  //     }
  //}
}

///////////////////////////////////////////////////////////////////////////////
// openVscpWorksDatabase
//

bool
vscpworks::openVscpWorksDatabase(void)
{
  // Set up database
  QString eventdbname = m_shareFolder + "vscpworks.sqlite3";

  QString dbName(eventdbname);
  m_worksdb = QSqlDatabase::addDatabase("QSQLITE", "vscpworks");
  m_worksdb.setDatabaseName(dbName);
  m_worksdb.open();

  // Create GUID table if it does not exist
  QSqlQuery query = QSqlQuery(m_worksdb);
  if (!query.exec("CREATE TABLE IF NOT EXISTS guid ("
                  "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                  "guid	TEXT UNIQUE,"
                  "name	TEXT,"
                  "description   TEXT);")) {
    qDebug() << query.lastError();
    return false;
  }

  // Create GUID name index
  if (!query.exec("CREATE INDEX IF NOT EXISTS \"idxGuidName\" ON \"guid\" (\"guid\" ASC)")) {
    qDebug() << query.lastError();
    return false;
  }

  if (!query.exec("CREATE TABLE IF NOT EXISTS \"sensorindex\" ("
                  "\"idx\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                  "\"link_to_guid\"	INTEGER, "
                  "\"sensor\"	        INTEGER, "
                  "\"name\"	        TEXT, "
                  "\"description\"	TEXT );")) {
    qDebug() << query.lastError();
    return false;
  }

  // Create sensor link + idx unique  index
  if (!query.exec("CREATE UNIQUE INDEX IF NOT EXISTS \"idxSensors\" ON \"sensorindex\" (\"link_to_guid\" ASC, \"sensor\" ASC)")) {
    qDebug() << query.lastError();
    return false;
  }

  // Create log table if it does not exist
  if (!query.exec("CREATE TABLE IF NOT EXISTS log ("
                  "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                  "level INTEGER,"
                  "datetime TEXT,"
                  "message TEXT);")) {
    qDebug() << query.lastError();
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
  m_mutexGuidMap.lock();

  QSqlQuery query("SELECT * FROM guid order by name", m_worksdb);
  if (QSqlError::NoError != query.lastError().type()) {
    m_mutexGuidMap.unlock();
    return false;
  }

  while (query.next()) {
    QString guid                  = query.value(1).toString();
    QString name                  = query.value(2).toString();
    m_mapGuidToSymbolicName[guid] = name;
  }

  m_mutexGuidMap.unlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadSensorTable
//

bool
vscpworks::loadSensorTable(void)
{
  m_mutexSensorIndexMap.lock();

  QSqlQuery query("SELECT * FROM sensorindex order by sensor", m_worksdb);
  if (QSqlError::NoError != query.lastError().type()) {
    m_mutexSensorIndexMap.unlock();
    return false;
  }

  while (query.next()) {
    int link_to_guid                                             = query.value(1).toInt();
    int sensor                                                   = query.value(2).toInt();
    QString name                                                 = query.value(3).toString();
    m_mapSensorIndexToSymbolicName[(link_to_guid << 8) + sensor] = name;
  }

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

  QString strInsert = "INSERT INTO guid (guid, name) VALUES (%1,%2);";
  QSqlQuery queryClass(strInsert.arg(guid).arg(name), m_worksdb);
  if (queryClass.lastError().isValid()) {
    spdlog::error(std::string(tr("Failed to insert GUID into database %s")
                                .arg(queryClass.lastError().text())
                                .toStdString()));
    qDebug() << queryClass.lastError();
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

  m_mutexGuidMap.lock();

  QString strInsert = "SELECT * FROM guid WHERE guid='%1';";

  QSqlQuery query(strInsert.arg(guid), m_worksdb);
  if (query.lastError().isValid()) {
    m_mutexGuidMap.unlock();
    return -1;
  }

  if (query.next()) {
    index = query.value(0).toInt();
  }

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
