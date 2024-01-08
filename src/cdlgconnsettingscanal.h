// cdlgconnsettingscanal.h
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


#ifndef CDLGCONNSETTINGSCANAL_H
#define CDLGCONNSETTINGSCANAL_H

#include "canalconfigwizard.h"
#include "vscp_client_canal.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QJsonObject>

namespace Ui {
class CDlgConnSettingsCanal;
}


class CDlgConnSettingsCanal : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgConnSettingsCanal(QWidget *parent = nullptr);
    ~CDlgConnSettingsCanal();

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
    CVscpClient::connType getSelectedType(void);

    /*!
        Setters/getters for name/description
    */
    QString getName(void);
    void setName(const QString& str);

    /*!
        Setters/getters for path to driver
    */
    QString getPath(void);
    void setPath(const QString& str);

    /*!
        Setters/getters for config
    */
    QString getConfig(void);
    void setConfig(const QString& str);

    /*!
        Setters/getters for flags
    */
    uint32_t getFlags(void);
    std::string getFlagsStr(void);
    void setFlags(uint32_t flags);

    /*!
        Setters/getters for datarate
    */
    uint32_t getDataRate(void);
    void setDataRate(uint32_t datarate);

    /*!
        Setters/getters for JSON config object
    */
    QJsonObject getJson(void);
    void setJson(const QJsonObject *pobj);

private slots:
    void testDriver();
    void setDriverPath();
    void wizard();
    void filterwizard();

private:

    Ui::CDlgConnSettingsCanal *ui;

    /*! 
        This variable holds the connection type that 
        the used select
    */
    //CVscpClient::connType m_selected_type;

    // Dummy client
    vscpClientCanal m_vscpClient;

    // JSON configuration object
    QJsonObject m_jsonConfig;
};


#endif // CDLGCONNSETTINGSCANAL_H
