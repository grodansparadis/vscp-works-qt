// cdlgconnsettingsrest.h
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

#ifndef CDLGCONNSETTINGSREST_H
#define CDLGCONNSETTINGSREST_H

#include "connection_types.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgConnSettingsRest;
}


class CDlgConnSettingsRest : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgConnSettingsRest(QWidget *parent = nullptr);
    ~CDlgConnSettingsRest();

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
    connection_type getSelectedType(void);

    /*!
        Setters/getters for name/description
    */
    std::string getName(void);
    void setName(const std::string& str);

    /*!
        Setters/getters for path
    */
    std::string getPath(void);
    void setPath(const std::string& str);

private:

    Ui::CDlgConnSettingsRest *ui;

    void createMenu();
    void createHorizontalGroupBox();
    void createGridGroupBox();
    void createFormGroupBox();

    /*! 
        This variable holds the connection type that 
        the used select
    */
    connection_type m_selected_type;
};


#endif // CDLGCONNSETTINGSREST_H
