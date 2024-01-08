// cdlgmdfregisterlist.h
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

#ifndef CDLGMDFREGISTERLIST_H
#define CDLGMDFREGISTERLIST_H

#include <vscpworks.h>
#include <mdf.h>

#include "cdlgmdfregister.h"

#include <QDialog>




namespace Ui {
class CDlgMdfRegisterList;
}

class CDlgMdfRegisterList : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfRegisterList(QWidget* parent = nullptr);
  ~CDlgMdfRegisterList();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
      Set edit mode.
      GUID will be READ ONLY
  */
  void setEditMode();

  /*!
    Init dialog data
    @param pmdf Pointer to MDF object
  */
  void initDialogData(CMDF *pmdf, uint16_t page=0);

  /*!
    Fill page combo box with page information
  */
  void renderComboPage(void);

  /*!
    Fill in contact items
  */
  void renderRegisterItems(void);


  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  // void setValue(const QString& name);
  // QString getValue(void);

public slots:
  void accept(void);

  // Called wehen page combo changes
  void onPageComboChange(int idx);

  // Description buttons
  void addRegister(void);
  void editRegister(void);
  void dupRegister(void);
  void deleteRegister(void);

private:
  Ui::CDlgMdfRegisterList* ui;

  // MDF
  CMDF *m_pmdf;

  // Register page
  uint16_t m_page;

  // Used to get a sorted list of registers
  std::set<uint32_t> m_registersSet ;
};

#endif // CDLGMDFREGISTERLIST_H
