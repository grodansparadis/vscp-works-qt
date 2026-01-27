// cdlgactionparam.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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


// ----------------------------------------------------------------------------


class QTableWidgetItemBits : public QTableWidgetItem {

public:  

  QTableWidgetItemBits(const QString &text);
  ~QTableWidgetItemBits(void);

//private:

  CMDF_Bit *m_pBit;
  //uint8_t m_pos;
  //uint8_t m_width;
  uint8_t m_mask;
  int m_value; 
};


class CDlgActionParam : public QDialog
{
    Q_OBJECT

private:
    const int BIT_COLUMN_POS = 0;
    const int BIT_COLUMN_VALUE = 1;
    const int BIT_COLUMN_NAME = 2;

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

    /*!
      Setter for action parameter
      @param param Action parameter to set
    */
    void setActionParameter(uint8_t param) { m_actionParam = param; };

    /*!
      Getter for action parameter
      @return Action parameter
    */
    uint8_t getActionParameter(void);


public slots:
   
    //void accept(void);
    //void reject(void);

    /*!
      A cell has been clicked
    */
    void cellClicked(int row, int column);

    /*!
      Values has been edited
    */
    void valueChanged(int row, int column);

private slots:

/*
  Show help for this dialog
*/
void showHelp();

private:

    Ui::CDlgActionParam *ui;

    /// Level I action parameter value
    uint8_t m_actionParam;

};



#endif // CDlgActionParam_H