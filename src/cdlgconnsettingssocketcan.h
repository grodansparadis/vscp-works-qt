// cdlgconnsettingssocketcan.h
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

#ifndef CDLGCONNSETTINGSSOCKETCAN_H
#define CDLGCONNSETTINGSSOCKETCAN_H

#ifndef WIN32

#include "vscp-client-socketcan.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QJsonObject>
#include <QList>

// ----------------------------------------------------------------------------

class CFilterListItem : public QListWidgetItem {
 
 public:
    CFilterListItem(const QString &name, uint32_t filter, uint32_t mask, bool bInvert);
    ~CFilterListItem();

 public:
    QString m_name;     // Name/description for filters
    uint32_t m_filter;  // Filter
    uint32_t m_mask;    // Mask
    bool m_bInvert;     // Invert flags
};

// ----------------------------------------------------------------------------

namespace Ui {
class CDlgConnSettingsSocketCan;
}

class CDlgConnSettingsSocketCan : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgConnSettingsSocketCan(QWidget *parent = nullptr);
    ~CDlgConnSettingsSocketCan();


    typedef struct filterlist {
        QString m_name;
        uint32_t m_filter;
        uint32_t m_mask;
        bool m_bInvert;
    } filterlist;


    /*!
        Set inital focus to description
    */
    void setInitialFocus(void);


    /*!
        Called when the test connection is clicked
    */
    void onTestConnection(void);

    /*!
        Setters/getters for JSON config object
    */
    QJsonObject getJson(void);
    void setJson(const QJsonObject *pobj);


    /*!
        Setters/getters for name/description
    */
    QString getName(void);
    void setName(const QString& str);

    /*!
        Setters/getters for device
    */
    QString getDevice(void);
    void setDevice(const QString& str);

    /*!
        Setters/getters for flags
    */
    uint32_t getFlags(void);
    void setFlags(uint32_t flags);

    /*!
        Setters/getters for timeout
    */
    uint32_t getResponseTimeout(void);
    void setResponseTimout(uint32_t timeout);

 public slots:

    void onClicked(QListWidgetItem* item);

    void onDoubleClicked(QListWidgetItem* item);

    /*!
        Pop up menu for filter list
    */
    void showContextMenu(const QPoint& pos);

    /*!
        Called when the add filter button is clicked
    */
    void onAddFilter(void);

    /*!
        Called when the delete filter button is clicked
    */
    void onDeleteFilter(void);

    /*!
        Called when the delete filter button is clicked
    */
    void onEditFilter(void);

    /*!
        Called when the clone filter button is clicked
    */
    void onCloneFilter(void);

    /*!
        Called when the set filter button is clicked
    */
    void onSetFlags(void);

 private:

    Ui::CDlgConnSettingsSocketCan *ui;

    /// JSON configuration object
    QJsonObject m_jsonConfig;

    /// SocketCan client
    vscpClientSocketCan m_clientSocketcan;
};

#endif  // WIN32

#endif // CDLGCONNSETTINGSSOCKETCAN_H
