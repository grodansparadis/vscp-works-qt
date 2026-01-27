// cdlgeditsensorindex.h
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

#ifndef CDLGLEDITSENSORINDEX_H
#define CDLGLEDITSENSORINDEX_H

#include <vscpworks.h>

#include <QDialog>

namespace Ui {
class CDlgEditSensorIndex;
}

class CDlgEditSensorIndex : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgEditSensorIndex(QWidget* parent = nullptr);
  ~CDlgEditSensorIndex();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
      Set edit mode.
      GUID will be READ ONLY
  */
  void setEditMode(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      GUID getter/setters
  */
  void setSensor(uint8_t sensorindex);
  uint8_t getSensor(void);

  /*!
      Name getter/setters
  */
  void setName(const QString& name);
  QString getName(void);

  /*!
      Priority filter getter/setters
  */
  void setDescription(const QString& description);
  QString getDescription(void);

public slots:
  /// Help
  void showHelp(void);

private:
  Ui::CDlgEditSensorIndex* ui;
};

#endif // CDLGLEDITSENSORINDEX_H
