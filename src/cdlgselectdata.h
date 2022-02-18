// cdlgselectdata.h
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

#ifndef CDLGSELECTDATA_H
#define CDLGSELECTDATA_H


#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSelectData;
}


class CDlgSelectData : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgSelectData(QWidget *parent = nullptr);
    ~CDlgSelectData();

public:

    /*!
        Add value to list
        @param pos Position for value to add
        @param value Value to add
        @param op Constraint value
    */
    void addValue(uint16_t pos, uint8_t value, uint8_t op);
    
    /*! 
        Set data 
        @param listData List with defined values (pos(8):op(8):value(8))

    */
    void setData(std::deque<uint32_t> listData);

    /*!
        Get data
        @return List with defined values (pos(8):op(8):value(8))
    */
    std::deque<uint32_t> getData(void);

private slots:

    /// Add data value to listbox
    void onAddValue(void);

    /// Delete data value from listbox
    void onDeleteValue(void);

private:

    Ui::CDlgSelectData *ui;

};


#endif // CDLGSELECTDATA_H
