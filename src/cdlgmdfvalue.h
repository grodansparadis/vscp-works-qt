// cdlgmdfvalue.h
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

#ifndef CDLGMDFVALUE_H
#define CDLGMDFVALUE_H

#include <mdf.h>
#include <vscpworks.h>

#include <QDialog>

namespace Ui {
class CDlgMdfValue;
}

class CDlgMdfValue : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfValue(QWidget* parent = nullptr);
  ~CDlgMdfValue();

  static const int index_name    = 0;
  static const int index_value   = 1;


  static const char pre_str_registerbit[];

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pbit Pointer to MDF bit object
    @param index Selected file item

  */
  void initDialogData(CMDF_Value* pvalue, int index = 0);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // name
  QString getName(void);
  void setName(const QString& name);

  // value
  QString getValue(void);
  void setValue(const QString& value);

public slots:

  /*!
    Accept dialog data and write to register
  */
  void accept(void);

private:
  Ui::CDlgMdfValue* ui;

  /// Pointer to MDF
  CMDF* m_pmdf;

  /// Pointer to bit information
  CMDF_Value* m_pvalue;

  // Render data
  //std::map<uint8_t, CMDF_Bit*> m_bitmapValues;
};

#endif // CDlgMdfValue_H
