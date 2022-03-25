// cdlgactionparam.h
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

#ifndef CDLGACTIONPARAM_H
#define CDLGACTIONPARAM_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

namespace Ui {
class CDlgActionParam;
}

class CParamBitItem : public QTableWidgetItem {

public:  

  CParamBitItem(void);
  ~CParamBitItem(void);

private:

  uint8_t m_pos;

  uint8_t m_width;

  int m_value;

  std::string m_strName;  
};


class CDlgActionParam : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgActionParam(QWidget *parent = nullptr);
    ~CDlgActionParam();

    /*!
      Set inital focus to description
    */
    void setInitialFocus(void);

    /*!
      Add one value to the value comobox
      @param value Value to add
      @param name Parameter name to add
    */
    void addValue(int value, std::string& name);

    /*!
      Set the value
      @param pos Value bit position
      @param width Value bit width
      @param value Value to set
    */
    void addBitValue(std::deque<CMDF_Bit *> *pbitlist);

    /*!
      Disable value part
    */
    void showValues(bool bShow);

    /*!
      Disable bit part
    */
    void showBits(bool bShow);


public slots:
   
    //void accept(void);
    //void reject(void);

private:

    Ui::CDlgActionParam *ui;

};



#endif // CDlgActionParam_H