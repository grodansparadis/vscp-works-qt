// cdlgmdfeventdata.h
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

#ifndef CDLGMDFEVENTDATA_H
#define CDLGMDFEVENTDATA_H

#include <mdf.h>
#include <vscpworks.h>

#include <QDialog>

namespace Ui {
class CDlgMdfEventData;
}

class CDlgMdfEventData : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfEventData(QWidget* parent = nullptr);
  ~CDlgMdfEventData();

  static const int index_offset = 0;
  static const int index_name   = 1;

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdf Pointer to MDF object
    @param peventdata Pointer to event data object
    @param index The index for the field that will get focus
  */
  void initDialogData(CMDF* pmdf, CMDF_EventData* peventdata, int index = 0);

  /*!
    Set level 1 state
    This makes offset go up to only 7
  */
  void setLevel1(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  void setOffset(uint8_t offset);
  uint8_t getOffset(void);

  void setName(const QString& str);
  QString getName(void);

public slots:
  void accept(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfEventData* ui;

  /// Pointer to MDF
  CMDF* m_pmdf;

  /// Pointer to eventdata
  CMDF_EventData* m_peventdata;
};

#endif // CDLGMDFBOOTLOADER_H
