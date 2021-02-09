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

#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QFile>

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// vscpworks
//

vscpworks::vscpworks(int &argc, char **argv) :
    QApplication(argc, argv)
{        
    m_base = numerical_base::HEX;   // Numerical base    
}

///////////////////////////////////////////////////////////////////////////////
// ~vscpworks
//

vscpworks::~vscpworks()
{
    writeSettings();
}

///////////////////////////////////////////////////////////////////////////////
// decimalToStringInBase
//

QString vscpworks::decimalToStringInBase(uint32_t value, int tobase) 
{
    int base = 10;
    QString qstr;
    QString prefix;

    //vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    numerical_base selbase = (-1 == tobase) ? m_base : static_cast<numerical_base>(base);
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
// readSettings *
//

void vscpworks::readSettings()
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

    // VSCP event database last load date/time
    // ---------------------------------------
    m_lastEventDbLoadDateTime = settings.value("last-eventdb-download", "1970-01-01T00:00:00Z").toDateTime();
    qDebug() << m_lastEventDbLoadDateTime;
    
    // Connections
    // -----------
    int size = settings.beginReadArray("hosts/connections");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        std::string conn = settings.value("conn", "").toString().toStdString();
    }
    settings.endArray(); 
}

///////////////////////////////////////////////////////////////////////////////
// writeSettings *
//

void vscpworks::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // General settings
    settings.setValue("configFolder", m_configFolder);
    settings.setValue("shareFolder", m_shareFolder);
    settings.setValue("vscpHomeFolder", m_vscpHomeFolder);
    settings.setValue("numericBase", QString::number(static_cast<int>(m_base)));
    settings.setValue("last-eventdb-download", m_lastEventDbLoadDateTime);

    // Connections
    settings.beginWriteArray("hosts/connections");
    int i = 0;    
    for (std::list<CVscpClient *>::iterator it = m_listConn.begin(); it != m_listConn.end(); ++it){    
        settings.setArrayIndex(i);
        settings.setValue("conn", QString::fromStdString((*it)->getName()));
        //settings.setValue("conn", it->toJson);
    }
    settings.endArray();
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
            mapVscpClassToToken[classid] = classToken;

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
                mapVscpTypeToToken[((classid << 16) + typeId)] = typeToken;
            }
        }

    }

    return true;
}