// cdlgtxedit.h
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

#ifndef CDLGTXEDIT_H
#define CDLGTXEDIT_H


#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgTxEdit;
}


class CDlgTxEdit : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgTxEdit(QWidget *parent = nullptr);
    ~CDlgTxEdit();

public:

    /*!
        Set initial focus to name field
    */
    void setInitialFocus(void);

    /*!
        Show information page for the currently  selected
        VSCP class
    */
    void showVscpClassInfo();

    /*!
        Show information page for the currently selected
        VSCP type
    */
    void showVscpTypeInfo();

    /*!
        Fill the VSCP class combobox with 
        VSCP classes.
        @param vscpClass VSCP class to select. Defaults
                to zero for the first item.
    */
    void fillVscpClass(uint16_t vscpclass = 0);

    /*!
        Fill the VSCP type combobox with 
        VSCP types.
        @param vscpClass Class o fill types for
        @param vscpType VSCP type to select. defaults to
                zero for the firsts item.
    */
    void fillVscpType(uint16_t vscpclass, uint16_t vscpType = 0);

    /*!
        Setters/getters for TX activate
    */
    bool getEnable(void);
    void setEnable(bool bActive);

    /*!
        Setters/getters for name
    */
    QString getName(void);
    void setName(const QString& str);

    /*!
        Setters/getters for count
    */
    uint8_t getCount(void);
    void setCount(uint8_t count);

    /*!
        Setters/getters for VSCP class/type
    */
    uint16_t getVscpClass(void);
    uint16_t getVscpType(void);
    void setVscpClassType(uint16_t vscpClass, uint16_t vscpType);

    /*!
        Setters/getters for priority
    */
    uint8_t getPriority(void);
    void setPriority(uint8_t priority);

    /*!
        Setters/getters for GUID
    */
    QString getGuid(void);
    void setGuid(const QString& guid);

    /*!
        Setters/getters for data
    */
    QString getData(void);
    void setData(const QString& data);

    /*!
        Setters/getters for period
    */
    uint32_t getPeriod(void);
    void setPeriod(uint32_t period);

private slots:
    /*!
        User changed VSCP class in combo
        @param index New selected index
    */
    void currentVscpClassIndexChanged(int index);

private:

    Ui::CDlgTxEdit *ui;

};


#endif // CDLGTXEDIT_H
