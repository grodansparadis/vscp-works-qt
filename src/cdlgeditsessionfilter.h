// cdlgeditsessionfilter.h
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

#ifndef CDLGEDITSESSIONFILTER_H
#define CDLGEDITSESSIONFILTER_H

#include "sessionfilter.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgEditSessionFilter;
}

class CDlgEditSessionFilter : public QDialog {
  Q_OBJECT

public:
  explicit CDlgEditSessionFilter(QWidget* parent = nullptr);
  ~CDlgEditSessionFilter();

  // Use to store constraint type
  const uint16_t role_constraint_type = Qt::UserRole;

public:
  /// Set focus on
  void setInitialFocus(void);

  /*!
      Check if a constraint is already defined
      @param chk Constraint to check for
      @return True if allready defined
  */
  bool isConstraintDefined(uint8_t chk);

  /*!
      Get pointer to session filter
      @return Pointer to session filter
  */
  CSessionFilter* getSessionFilter(void) { return &m_sessionFilter; };

  // add/edit constraint for Receive
  void addConstraintReceive(void);
  void editConstraintReceive(void);

  // add/edit constraint for Transmit
  void addConstraintTransmit(void);
  void editConstraintTransmit(void);

  // add/edit constraint for Level I
  void addConstraintLevel1(void);
  void editConstraintLevel1(void);

  // add/edit constraint for Level II
  void addConstraintLevel2(void);
  void editConstraintLevel2(void);

  // add/edit constraint for Class/Type
  void addConstraintClass(void);
  void editConstraintClass(void);

  // add/edit constraint for GUID
  void addConstraintGuid(void);
  void editConstraintGuid(void);

  // add/edit constraint for OBID
  void addConstraintObid(void);
  void editConstraintObid(void);

  // add/edit constraint for Date
  void addConstraintDate(void);
  void editConstraintDate(void);

  // add/edit constraint for TimeStamp
  void addConstraintTimeStamp(void);
  void editConstraintTimeStamp(void);

  // add/edit constraint for Data
  void addConstraintData(void);
  void editConstraintData(void);

  // add/edit constraint for Data Size
  void addConstraintDataSize(void);
  void editConstraintDataSize(void);

  // add/edit constraint for Priority
  void addConstraintPriority(void);
  void editConstraintPriority(void);

  // add/edit constraint for Must be measurement
  void addConstraintMeasurement(void);
  void editConstraintMeasurement(void);

  // add/edit constraint for sensor index
  void addConstraintSensorIndex(void);
  void editConstraintSensorIndex(void);

  // add/edit constraint for measurement value
  void addConstraintValue(void);
  void editConstraintValue(void);

  // add/edit constraint for unit
  void addConstraintUnit(void);
  void editConstraintUnit(void);

  // add/edit constraint for data coding
  void addConstraintDataCoding(void);
  void editConstraintDataCoding(void);

  // add/edit constraint for script
  void addConstraintScript(void);
  void editConstraintScript(void);

private slots:

  /// Add filter constraint
  void addConstraint(void);

  /// Edit filter constraint
  void editConstraint(void);

  /// Delete filter constraint
  void deleteConstraint(void);

  /// Context menu
  void showContextMenu(const QPoint& pos);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgEditSessionFilter* ui;

  // Settings for the filter
  CSessionFilter m_sessionFilter;
};

#endif // CDLGEDITSESSIONFILTER_H
