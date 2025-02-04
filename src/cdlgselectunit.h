// cdlgselectunit.h
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

#ifndef CDLGSELECTUNIT_H
#define CDLGSELECTUNIT_H

#include "sessionfilter.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSelectMeasurementUnit;
}

class CDlgSelectMeasurementUnit : public QDialog {
  Q_OBJECT

public:
  explicit CDlgSelectMeasurementUnit(QWidget* parent = nullptr);
  ~CDlgSelectMeasurementUnit();

public:
  /// Get set Unit value
  uint8_t getMeasurementUnitValue();

  /// Set Unit value
  void setMeasurementUnitValue(uint8_t value);

  /// Get unit constraint
  CSessionFilter::constraint getMeasurementUnitConstraint();

  /// Set Priority constraint
  void setMeasurementUnitConstraint(CSessionFilter::constraint op);

private slots:

  /// Help
  void showHelp(void);

private:
  Ui::CDlgSelectMeasurementUnit* ui;
};

#endif // CDLGSELECTUNIT_H
