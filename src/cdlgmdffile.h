// cdlgmdffile.h
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

#ifndef CDLGMDFFILE_H
#define CDLGMDFFILE_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

namespace Ui {
class CDlgMdfFile;
}

class CDlgMdfFile : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfFile(QWidget* parent = nullptr);
  ~CDlgMdfFile();

  static const int index_picture  = 0;
  static const int index_video    = 1;
  static const int index_firmware = 2;
  static const int index_driver   = 3;
  static const int index_setup    = 4;
  static const int index_manual   = 5;

  static const char pre_str_picture[];
  static const char pre_str_video[];
  static const char pre_str_firmware[];
  static const char pre_str_driver[];
  static const char pre_str_setup[];
  static const char pre_str_manual[];

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index Selected file item

  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_record_type type, int index = 0);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

public slots:

  // File item buttons
  void addFileItem(void);
  void editFileItem(void);
  void dupFileItem(void);
  void deleteFileItem(void);

  void accept(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfFile* ui;

  mdf_record_type m_type;
  CMDF* m_pmdf;
};

#endif // CDLGMDFFILE_H
