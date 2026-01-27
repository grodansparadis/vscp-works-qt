// cdlgmdfaction.h
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

#ifndef CDLGMDFDM_ACTIONS_H
#define CDLGMDFDM_ACTIONS_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

namespace Ui {
class CDlgMdfDmAction;
}

class CDlgMdfDmAction : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfDmAction(QWidget* parent = nullptr);
  ~CDlgMdfDmAction();

  static const int index_code = 0;
  static const int index_name = 1;

  static const char pre_str_dm_action[];

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pmdfobject Pointer to MDF object
    @param index Selected file item

  */
  void initDialogData(CMDF* pmdf, CMDF_Action* paction, int index = 0);

  /*!
    Only enable parameter list
  */
  void setReadOnly(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // code
  uint8_t getCode(void);
  void setCode(uint8_t code);

  // name
  QString getName(void);
  void setName(const QString& name);

  // Render available actions
  void renderActionParams(void);

public slots:

  // Edit action item
  void editActionParam(void);

  // Add action item
  void addActionParam(void);

  // Duplicate action item
  void dupActionParam(void);

  // Delete action item
  void deleteActionParam(void);

  /*!
    Accept dialog data and write to register
  */
  void accept(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfDmAction* ui;

  /// Pointer to MDF
  CMDF* m_pmdf;

  CMDF_Action* m_paction;
};

#endif // CDLGMDFDM_ACTIONS_H
