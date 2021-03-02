// vscpworks.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#include <syslog.h>

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"
#include "filedownloader.h"

#include <mustache.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;
using namespace kainjow::mustache;

///////////////////////////////////////////////////////////////////////////////
// vscpworks
//

vscpworks::vscpworks(int &argc, char **argv) :
    QApplication(argc, argv)
{        
    m_base = numerical_base::HEX;   // Numerical base 
    m_bAskBeforeDelete = true;  
    m_logLevel = LOG_LEVEL_NONE;    // No logging

    m_session_maxEvents = -1;

    m_session_ClassDisplayFormat = CFrmSession::classDisplayFormat::symbolic;
    m_session_TypeDisplayFormat = CFrmSession::typeDisplayFormat::symbolic;
    m_session_GuidDisplayFormat = CFrmSession::guidDisplayFormat::symbolic;

    m_session_bAutoConnect = true;
    m_session_bShowFullTypeToken = false;

    // After the following it is possible to create and destroy event objects 
    // dynamically at run-time
    int idEvent = qRegisterMetaType<vscpEvent>();
    int idEventEx = qRegisterMetaType<vscpEventEx>();

    // QUuid uuid; 
    // uuid = QUuid::createUuid();
    // qDebug() << QUuid::createUuid().toString();
    // qDebug() << "\n";
    
}

///////////////////////////////////////////////////////////////////////////////
// ~vscpworks
//

vscpworks::~vscpworks()
{
    writeSettings();
    m_worksdb.close();
}

///////////////////////////////////////////////////////////////////////////////
// decimalToStringInBase
//

QString vscpworks::decimalToStringInBase(uint32_t value, int tobase) 
{
    int base = 10;
    QString qstr;
    QString prefix;

    numerical_base selbase = (-1 == tobase) ? m_base : static_cast<numerical_base>(tobase);
    switch (selbase) {
        case numerical_base::HEX:
            prefix = "0x";
            base = 16;
            break;
        case numerical_base::DECIMAL:
        default:
            prefix = "";
            base = 10;
            break;
        case numerical_base::OCTAL:
            prefix = "0o";
            base = 8;
            break;
        case numerical_base::BINARY:
            prefix = "0b";
            base = 2;
            break;
        }
    
        return (prefix + QString::number(value, base));
    };

///////////////////////////////////////////////////////////////////////////////
// decimalToStringInBase
//

QString vscpworks::decimalToStringInBase(const QString& strvalue, int tobase) 
{
    uint32_t value = vscp_readStringValue(strvalue.toStdString());
    return decimalToStringInBase(value, tobase);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionName
//

QString vscpworks::getConnectionName(CVscpClient::connType type)
{
    QString str = tr("Unknown type");

    switch (type) {

        case CVscpClient::connType::NONE: 
            str = tr("No connection");
            break;

        case CVscpClient::connType::LOCAL:
            str = tr("VSCP local connection");
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
            
        case CVscpClient::connType::REST:
            str = tr("VSCP REST connection"); 
            break;
            
        case CVscpClient::connType::RS232:
            str = tr("VSCP RS-232 connection"); 
            break;
            
        case CVscpClient::connType::RS485:
            str = tr("VSCP RS-485 connection"); 
            break;
            
        case CVscpClient::connType::RAWCAN:
            str = tr("Raw CAN connection"); 
            break;
            
        case CVscpClient::connType::RAWMQTT:
            str = tr("Raw MQTT connection");
            break;
    }

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// loadSettings *
//

void vscpworks::loadSettings(void)
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
        qDebug() << "Config folder = " << path;        
    }

    // Share folder
    // ------------
    // Linux: "/home/akhe/.local/share/vscp-works-qt"   user data is here
    // Windows: 
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        path += "/";
        // path += QCoreApplication::organizationName();
        // path += "/";        
        m_shareFolder = settings.value("shareFolder", path).toString();
        qDebug() << "" << path;
        // If folder does not exist, create it
        QDir dir(path);
        if (!dir.exists())  {
            dir.mkpath(".");
        }
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

    m_bAskBeforeDelete = settings.value("bAskBeforeDelete", true).toBool();

    m_logLevel = settings.value("logLevel", LOG_LEVEL_NONE).toInt();

    // * * * Session * * *

    m_session_maxEvents = settings.value("maxSessionEvents", -1).toInt();

    m_session_ClassDisplayFormat = static_cast<CFrmSession::classDisplayFormat>(settings.value("sessionClassDisplayFormat", 
            static_cast<int>(CFrmSession::classDisplayFormat::symbolic)).toInt());
    m_session_TypeDisplayFormat = static_cast<CFrmSession::typeDisplayFormat>(settings.value("sessionTypeDisplayFormat", 
            static_cast<int>(CFrmSession::typeDisplayFormat::symbolic)).toInt());
    m_session_GuidDisplayFormat = static_cast<CFrmSession::guidDisplayFormat>(settings.value("sessionGuidDisplayFormat", 
            static_cast<int>(CFrmSession::guidDisplayFormat::guid)).toInt());

    m_session_bAutoConnect =  settings.value("sessionAutoConnect", true).toBool();    
    m_session_bShowFullTypeToken =  settings.value("sessionShowFullTypeToken", true).toBool();        

    // VSCP event database last load date/time
    // ---------------------------------------
    m_lastEventDbLoadDateTime = settings.value("last-eventdb-download", "1970-01-01T00:00:00Z").toDateTime();
    qDebug() << m_lastEventDbLoadDateTime;
    
    // * * *  Read in defined connections  * * *
    
    int size = settings.beginReadArray("hosts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QJsonObject conn = settings.value("connection").toJsonObject();
        addConnection(conn);
    }
    settings.endArray(); 
}

///////////////////////////////////////////////////////////////////////////////
// writeSettings
//

void vscpworks::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // General settings
    settings.setValue("configFolder", m_configFolder);
    settings.setValue("shareFolder", m_shareFolder);
    settings.setValue("vscpHomeFolder", m_vscpHomeFolder);
    settings.setValue("numericBase", QString::number(static_cast<int>(m_base)));
    settings.setValue("bAskBeforeDelete", m_bAskBeforeDelete);
    settings.setValue("logLevel", m_logLevel);
    settings.setValue("last-eventdb-download", m_lastEventDbLoadDateTime);

    // * * * Session * * *

    settings.setValue("maxSessionEvents", m_session_maxEvents);

    settings.setValue("sessionClassDisplayFormat", static_cast<int>(m_session_ClassDisplayFormat));
    settings.setValue("sessionTypeDisplayFormat", static_cast<int>(m_session_TypeDisplayFormat));
    settings.setValue("sessionGuidDisplayFormat", static_cast<int>(m_session_GuidDisplayFormat));

    settings.setValue("sessionAutoConnect", m_session_bAutoConnect);
    settings.setValue("sessionShowFullTypeToken", m_session_bShowFullTypeToken);

    writeConnections();

    // TODO TEST remove
    QJsonObject j1
    {
        {"p1", 1},
        {"p1", 2}
    };

    QJsonObject j2
    {
        {"p1", 2},
        {"p1", 3}
    };

    QJsonArray aaa;
    aaa.push_back(1);
    aaa.push_back(2);
    aaa.push_back(3);

    QJsonObject jj
    {
        {"property1", 1},
        {"property2", 2},
        {"property3", "teststring"},
        {"property4", aaa },
    };
    // {"property4", [{"p1":1,"p2":2},{"p1":3,"p2":4}]}
    qDebug() << jj["property4"];

    settings.setValue("test", jj);

}


///////////////////////////////////////////////////////////////////////////////
// writeConnections
//

void vscpworks::writeConnections(void)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Remove old entries
    int size = settings.beginReadArray("hosts");
    for(auto i=0; i<size; i++) {
        settings.setArrayIndex(i);
        settings.remove("connection");
    }
    settings.endArray();
    
    // Connections
    settings.beginWriteArray("hosts");
    int i = 0;    
    //for (QMap<QString,QJsonObject>::iterator it = m_mapConn.begin(); it != m_mapConn.end(); ++it){    
    QMap<QString,QJsonObject>::const_iterator it = m_mapConn.constBegin();
    while (it != m_mapConn.constEnd()) {    
        settings.setArrayIndex(i);
        settings.setValue("connection", it.value());
        i++;
        it++;
    }
    settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
// addConnection
//

bool vscpworks::addConnection(QJsonObject& conn, bool bSave )
{
    QString uuid = QUuid::createUuid().toString();
    qDebug() << "uuid = " << uuid;
    conn["uuid"] = uuid;
    //conn.remove("uuid");

    // Add configuration item to map
    m_mapConn[uuid] = conn; 

    // QMap<QString,QJsonObject>::iterator it;
    // it = m_mapConn.find(uuid);
    // if ( m_mapConn.end() != it ) {
    //     qDebug() << it.key() << " " << it.value() << "<-----";
    // }

    // Save settings if requested to do so
    if (bSave) writeConnections();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeConnection
//

bool vscpworks::removeConnection(const QString& uuid, bool bSave )
{
    qDebug() << uuid;

    QMap<QString,QJsonObject>::iterator it;
    // for (it = m_mapConn.begin(); it != m_mapConn.end(); ++it){ 
    //     qDebug() << it.key() << " " << it.value();
    // } 

    //qDebug() << uuid;
    int n = m_mapConn.remove(uuid);

    // it = m_mapConn.find(uuid);
    // if ( m_mapConn.end() != it ) {
        
    //     qDebug() << it.value() << "second";
        
    //     // Remove configuration from map
    //     it = m_mapConn.erase(it);
    // }

    // Save settings if requested to do so
    if (bSave) writeConnections();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadEventDb
//

bool vscpworks::loadEventDb(void)
{
    QSqlDatabase evdb;
    evdb = QSqlDatabase::addDatabase("QSQLITE");
    QString dbpath = m_shareFolder;
    dbpath += "vscp_events.sqlite3";
    qDebug() << "db = " << dbpath;

    // If the database don't exist, bail out
    if (!QFile::exists(dbpath)) {
        QString err = QString(tr("The VSCP event database does not exist. Is it available? [%s]")).arg(dbpath);
        syslog(LOG_ERR, "%s", err.toStdString().c_str());
        return false;
    }

    evdb.setDatabaseName(dbpath);

    if (!evdb.open()) {
        //qDebug() << Database.lastError().text();
        QString err = QString(tr("The VSCP event database could not be opened. Is it available? [%s]")).arg(dbpath);
        syslog(LOG_ERR, "%s", err.toStdString().c_str());
        return false;
    }
    else {
        QSqlQuery queryClass("SELECT * FROM vscp_class order by class");

        while (queryClass.next()) {
            uint16_t classid = queryClass.value(0).toUInt();
            QString className = queryClass.value(1).toString();
            QString classToken = queryClass.value(2).toString();
            //qDebug() << classid << " - " << className << " - " << classToken;
            m_mapVscpClassToToken[classid] = classToken;

            QString sqlTypeQuery = QString("SELECT * FROM vscp_type WHERE link_to_class=%1").arg(classid);
            QSqlQuery queryType(sqlTypeQuery);
            //qDebug() << sqlTypeQuery;
            while (queryType.next()) {
                uint typeIdx = queryType.value(0).toUInt();
                uint16_t typeId = queryType.value(1).toUInt();
                QString typeToken = queryType.value(3).toString();
                uint32_t combined = ((classid << 16) + typeId);
                //qDebug() << typeIdx << " - " << typeId << " - " <<  typeToken  << " " << combined;
                //qDebug() << (combined & 0xffff) << (combined >> 16); 
                m_mapVscpTypeToToken[(((uint32_t)classid << 16) + typeId)] = typeToken;
                // qDebug() << mapVscpTypeToToken[(((uint32_t)classid << 16) + typeId)] << " " << (((uint32_t)classid << 16) + typeId) << " " << classid;
                // qDebug() << " OK";
            }
        }

    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// createVscpWorksDatabase
//

bool vscpworks::openVscpWorksDatabase(void)
{
    // Set up database
    QString eventdbname = m_shareFolder + "vscpworks.sqlite3";

    // The database exists we are done
    //if ( QFile::exists(eventdbname) ) return false;

    QString dbName(eventdbname);
    m_worksdb = QSqlDatabase::addDatabase( "QSQLITE" );
    m_worksdb.setDatabaseName( dbName );
    m_worksdb.open();

    // Create GUID database if it does not exist
    QSqlQuery query = QSqlQuery( m_worksdb );
    if (!query.exec("CREATE TABLE IF NOT EXISTS guid ("
                "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                "guid	TEXT,"
                "name	TEXT,"
                "description   TEXT);"
            ) ) {
        return false;
    }

    // Create GUID name index
    if (!query.exec("CREATE INDEX IF NOT EXISTS \"idxGuidName\" ON \"guid\" (\"guid\" ASC)")) {

    }

    // Create log database if it does not exist
    if (!query.exec("CREATE TABLE IF NOT EXISTS log ("
                "idx	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                "level INTEGER,"
                "datetime TEXT,"
                "message TEXT);"
            ) ) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// log
//

void vscpworks::log(int level, const QString& message)
{
    // Log only messages 
    if (level <= m_logLevel) {

        QDateTime now = QDateTime::currentDateTime();

        QString strQuery = "INSERT INTO log (level, datetime, message) values (";
        strQuery +=  QString::number(level);
        strQuery += ",";
        strQuery += "'" + now.toString() + "'";
        strQuery += ",";
        strQuery += "'" +message + "'";
        strQuery += ");";

        QSqlQuery query = QSqlQuery( m_worksdb );
        if (!query.exec(strQuery)) {
            qDebug() << "Failed to insert log message";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// getShortTypeToken
//

QString vscpworks::getShortTypeToken(uint16_t vscpClass, uint16_t vscpType)
{
    // * * * Type * * *
    QString strTypeToken = m_mapVscpTypeToToken[((uint32_t)vscpClass << 16) + vscpType];    
    if (vscpClass >= 1024) {
        strTypeToken = strTypeToken.right(strTypeToken.length()-11);    // Remove "VSCP2_TYPE_"
    }
    else {
        strTypeToken = strTypeToken.right(strTypeToken.length()-10);    // Remove "VSCP_TYPE_"
    }
    int posUnderscore = strTypeToken.indexOf("_");
    if (posUnderscore) posUnderscore++;
    return strTypeToken.right(strTypeToken.length()-posUnderscore);
}

///////////////////////////////////////////////////////////////////////////////
// getHelpUrlForClass
//

QString vscpworks::getHelpUrlForClass(uint16_t vscpClass)
{
    // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol
    mustache templ{"https://grodansparadis.github.io/vscp-doc-spec/#/./{{vscp-class-token}}"};
    
    kainjow::mustache::data _data;
    _data.set("vscp-class-token", m_mapVscpClassToToken[vscpClass].toLower().toStdString());
    return templ.render(_data).c_str();
}

///////////////////////////////////////////////////////////////////////////////
// getHelpUrlForType
//

QString vscpworks::getHelpUrlForType(uint16_t vscpClass, uint16_t vscpType)
{
    // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6
    std::string strTemp = getHelpUrlForClass(vscpClass).toStdString();
    strTemp += "?id=type{{vscp-type-id}}";
    mustache templ{strTemp};

    kainjow::mustache::data _data;
    _data.set("vscp-type-id", QString::number(vscpType).toStdString());
    return templ.render(_data).c_str();
}