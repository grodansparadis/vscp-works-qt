// cdlgmdfregisterbit.h
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

#ifndef CDLGMDFBIT_H
#define CDLGMDFBIT_H

#include <mdf.h>
#include <vscpworks.h>

#include <QDialog>

namespace Ui {
class CDlgMdfBit;
}

class CDlgMdfBit : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfBit(QWidget* parent = nullptr);
  ~CDlgMdfBit();

  static const int index_name    = 0;
  static const int index_pos     = 1;
  static const int index_width   = 2;
  static const int index_default = 3;
  static const int index_min     = 4;
  static const int index_max     = 5;
  static const int index_access  = 6;

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
  void initDialogData(CMDF_Bit* pbit, int index = 0, mdf_record_type type = mdf_type_register);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // name
  QString getName(void);
  void setName(const QString& name);

  // pos
  uint8_t getPos(void);
  void setPos(uint8_t page);

  // width
  uint8_t getWidth(void);
  void setWidth(uint8_t offset);

  // default
  uint8_t getDefault(void);
  void setDefault(uint8_t value);

  // min
  uint8_t getMin(void);
  void setMin(uint8_t min);

  // max
  uint8_t getMax(void);
  void setMax(uint8_t max);

  // access
  mdf_access_mode getAccess(void);
  void setAccess(uint8_t access);

public slots:

  /*!
    Accept dialog data and write to register
  */
  void accept(void);

private:
  Ui::CDlgMdfBit* ui;

  /// Pointer to MDF
  CMDF* m_pmdf;

  /// Pointer to bit information
  CMDF_Bit* m_pbit;

  /// Selected field in dialog
  int m_index;

  /// MDF record owner type we are editing
  mdf_record_type m_type;
};

#endif // CDLGMDFREGISTERBIT_H