// cdlgmdfremotevar.h
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

#ifndef CDLGMDFREMOTEVAR_H
#define CDLGMDFREMOTEVAR_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>



namespace Ui {
class CDlgMdfRemoteVar;
}

class CDlgMdfRemoteVar : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfRemoteVar(QWidget* parent = nullptr);
  ~CDlgMdfRemoteVar();

  static const int index_name = 0;
  static const int index_type = 1;
  static const int index_span = 2;
  static const int index_page = 3;
  static const int index_offset = 4;
  static const int index_bit_offset = 5;
  static const int index_access = 6;
  static const int index_default = 7;
  static const int index_fgcolor = 8;
  static const int index_bgcolor = 9;

  static const char pre_str_remote_variable[];

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pmdfobject Pointer to MDF object
    @param index Selected file item
      
  */
  void initDialogData(CMDF *pmdf, CMDF_RemoteVariable* pvar, int index = 0);

  /*!
    Prevent page and offset from being edited
  */
  void setReadOnly(void);

  

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // name
  QString getName(void);
  void setName(const QString& name);

  // page
  uint16_t getPage(void);
  void setPage(uint16_t page);

  // offset
  uint32_t getOffset(void);
  void setOffset(uint32_t offset);

  // bit offset
  uint32_t getBitOffset(void);
  void setBitOffset(uint32_t offset);

  // type
  vscp_remote_variable_type getType(void);
  void setType(vscp_remote_variable_type type);

  // span
  uint16_t getSpan(void);
  void setSpan(uint16_t type);

  // access
  mdf_access_mode getAccess(void);
  void setAccess(mdf_access_mode access);

  // default
  QString getDefault(void);
  void setDefault(QString& val);

  // fg color
  uint32_t getForegroundColor(void);
  void setForegroundColor(uint32_t color);

  // bg color
  uint32_t getBackgroundColor(void);
  void setBackgroundColor(uint32_t color);

public slots:

    /*!
    Show dialog to select foregrund color
  */
  void showFgColorDlg(void);

  /*!
    Show dialog to select backgrund color
  */
  void showBgColorDlg(void);

  /*
    Set UNDEF for default value
  */
 void setUndef(void);

  /*!
    Accept dialog data and write to register
  */
  void accept(void);


private:
  Ui::CDlgMdfRemoteVar* ui;

  /// Pointer to MDF
  CMDF *m_pmdf;

  /// Pointer to register information
  CMDF_RemoteVariable* m_pvar;
};

#endif // CDLGMDFREGISTER_H
