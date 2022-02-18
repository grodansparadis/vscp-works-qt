// cdlgconnsettingstcpip.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGCONNSETTINGSTCPIP_H
#define CDLGCONNSETTINGSTCPIP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "vscp_client_tcp.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QJsonObject>

namespace Ui {
class CDlgConnSettingsTcpip;
}


class CDlgConnSettingsTcpip : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgConnSettingsTcpip(QWidget *parent = nullptr);
    ~CDlgConnSettingsTcpip();

    /*!
        Set inital focus to description
    */
    void setInitialFocus(void);

    /*!
        Called when the connection list is clicked
    */
    void onClicked(QListWidgetItem* item);

    /*!
        Called when the connection list is double clicked
    */
    void onDoubleClicked(QListWidgetItem* item);

    /*!
        Return the selected communication type
    */
    //CVscpClient::connType getSelectedType(void);

    /*!
        Setters/getters for name/description
    */
    QString getName(void);
    void setName(const QString& str);

    /*!
        Setters/getters for host
    */
    QString getHost(void);
    void setHost(const QString& str);

    /*!
        Setters/getters for user
    */
    QString getUser(void);
    void setUser(const QString& str);

    /*!
        Setters/getters for password
    */
    QString getPassword(void);
    void setPassword(const QString& str);

    /*!
        Setters/getters for interface (selected)
    */
    QString getInterface(void);
    void setInterface(const QString& str);

    /*!
        Setters/getters for connection timeout
    */
    uint32_t getConnectionTimeout(void);
    void setConnectionTimeout(uint32_t timeout);

    /*!
        Setters/getters for response timeout
    */
    uint32_t getResponseTimeout(void);
    void setResponseTimeout(uint32_t timeout);

    /*!
        Setters/getters for Full level II
    */
    bool getFullL2(void);
    void setFullL2(bool l2);

    /*!
        Setters/getters for Poll
    */
    bool getPoll(void);
    void setPoll(bool bPoll);

    /*!
        Setters/getters for TLS enable
    */
    bool isTLSEnabled(void);
    void enableTLS(bool bTLS);

    /*!
        Setters/getters for TLS enable
    */
    bool isVerifyPeerEnabled(void);
    void enableVerifyPeer(bool bverifypeer);

    /*!
        Setters/getters for TLS CA file
    */
    QString getCaFile(void);
    void setCaFile(const QString& str);

    /*!
        Setters/getters for TLS CA path
    */
    QString getCaPath(void);
    void setCaPath(const QString& str);

    /*!
        Setters/getters for TLS Cert file
    */
    QString getCertFile(void);
    void setCertFile(const QString& str);

    /*!
        Setters/getters for TLS key file
    */
    QString getKeyFile(void);
    void setKeyFile(const QString& str);

    /*!
        Setters/getters for TLS pw key file
    */
    QString getPwKeyFile(void);
    void setPwKeyFile(const QString& str);

    /*!
        Setters/getters for JSON config object
    */
    QJsonObject getJson(void);
    void setJson(const QJsonObject *pobj);

 private slots:

    /// Set filter button clicked
    void onSetFilter(void);

    /// Test connection button clicked
    void onTestConnection(void);

    /// Get interfaces button clicked
    void onGetInterfaces(void);

    /// TLS settings button clicked
    void onTLSSettings(void);

    /// Get help with settings
    void onGetHelp(void);

private:

    Ui::CDlgConnSettingsTcpip *ui;


    /// TLS flag (secure transport if enabled)
    bool m_bTLS;

    /*!
        the server certificate will be verified and the connection 
        aborted if the verification fails.
    */
    bool m_bVerifyPeer;

    /// CA file
    QString m_cafile;

    /// Path to CA file (can hold filename also)
    QString m_capath;

    /// Path to CERT file
    QString m_certfile;

    /// Key file
    QString m_keyfile;

    /// Password keyfile
    QString m_pwkeyfile;

    /*! 
        This variable holds the connection type that 
        the used select
    */
    // CVscpClient::connType m_selected_type;

    /// JSON configuration object
    QJsonObject m_jsonConfig;

    /// VSCP tcp/ip client
    vscpClientTcp m_client;

    /// VSCP tcp/ip main filter
    vscpEventFilter m_filter;
};


#endif // CDLGCONNSETTINGSTCPIP_H
