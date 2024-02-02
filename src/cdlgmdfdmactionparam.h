// cdlgmdfactionparam.h
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

#ifndef CDLGMDFDM_ACTION_PARAM_H
#define CDLGMDFDM_ACTION_PARAM_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>



namespace Ui {
class CDlgMdfDmActionParam;
}

class CDlgMdfDmActionParam : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfDmActionParam(QWidget* parent = nullptr);
  ~CDlgMdfDmActionParam();

  static const int index_name = 0;
  static const int index_offset = 1;
  static const int index_min = 2;
  static const int index_max = 3;

  static const char pre_str_actionparam[];

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pmdfobject Pointer to MDF object
    @param index Selected file item
      
  */
  void initDialogData(CMDF *pmdf, CMDF_ActionParameter* pactionparam, int index = 0);

  /// @brief  Rener action parameters
  void renderActionParams(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // name
  QString getName(void);
  void setName(const QString& name);

  // page
  uint32_t getOffset(void);
  void setOffset(uint32_t offset);

  // min
  uint8_t getMin(void);
  void setMin(uint8_t min);

  // max
  uint8_t getMax(void);
  void setMax(uint8_t max);

public slots:



  /*!
    Accept dialog data and write to register
  */
  void accept(void);


private:
  Ui::CDlgMdfDmActionParam* ui;

  /// Pointer to MDF
  CMDF *m_pmdf;

  CMDF_ActionParameter* m_pactionparam;

};

#endif // CDLGMDFDM_ACTION_PARAM_H
