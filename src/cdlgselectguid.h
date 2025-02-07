// cdlgselectguid.h
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

#ifndef CDLGSELECTGUID_H
#define CDLGSELECTGUID_H

#include <vscp.h>
#include <vscphelper.h>

#include "sessionfilter.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSelectGuid;
}

class CDlgSelectGuid : public QDialog {
  Q_OBJECT

public:
  explicit CDlgSelectGuid(QWidget* parent = nullptr);
  ~CDlgSelectGuid();

  /*!
      Get guid value for pos
      @param pos Position 0-15 in GUID. 0 is LSB
      @return GUID value.
  */
  uint8_t getGuidValue(uint8_t pos);

  /*!
      Set GUID value for pos
      @param pos Position to set GUID value for
      @param value Value to set
  */
  void setGuidValue(uint8_t pos, uint8_t value);

  /*!
      Get operation for GUID pos
      @param pos Position 0-15 in GUID. 0 is LSB
      @return GUID operation.
  */
  CSessionFilter::constraint getGuidConstraint(uint8_t pos);

  /*!
      Set constraint for GUID at pos
      @param pos Pos to set constraint at
      @param op Constrant to set
  */
  void setGuidConstraint(uint8_t pos, CSessionFilter::constraint op);

public:
private slots:

  /// Fill GUID from edit field
  void fillGuid(void);

  /// Set GUID field from GUID values
  void getGuid(void);

  /// Look up GUID from known GUID's
  void askGuid(void);

  /// Fetch an dill in Known GUID
  void fetchKnownGuid(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgSelectGuid* ui;
};

#endif // CDLGSELECTGUID_H
