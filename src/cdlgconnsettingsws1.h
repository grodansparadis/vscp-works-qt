// cdlgconnsettingsws1.h
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

#ifndef CDLGCONNSETTINGSWS1_H
#define CDLGCONNSETTINGSWS1_H

#include "vscp_client_ws1.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QJsonObject>

namespace Ui {
class CDlgConnSettingsWs1;
}


class CDlgConnSettingsWs1 : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgConnSettingsWs1(QWidget *parent = nullptr);
    ~CDlgConnSettingsWs1();

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
        Setters/getters for JSON config object
    */
    QJsonObject getJson(void);
    void setJson(const QJsonObject* pobj);

private:

    Ui::CDlgConnSettingsWs1 *ui;

    /*! 
        This variable holds the connection type that 
        the used select
    */
    CVscpClient::connType m_selected_type;

    // JSON configuration object
    QJsonObject m_jsonConfig;
};


#endif // CDLGCONNSETTINGSWS1_H
