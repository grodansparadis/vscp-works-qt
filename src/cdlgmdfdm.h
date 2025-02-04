// cdlgmdfdm.h
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

#ifndef CDLGMDFDM_H
#define CDLGMDFDM_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

namespace Ui {
class CDlgMdfDM;
}

class CDlgMdfDM : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfDM(QWidget* parent = nullptr);
  ~CDlgMdfDM();

  static const int index_level        = 0;
  static const int index_start_page   = 1;
  static const int index_start_offset = 2;
  static const int index_rows         = 3;
  static const int index_row_size     = 4;
  static const int index_actions      = 5;

  static const char pre_str_dm[];

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pmdfobject Pointer to MDF object
    @param index Selected file item

  */
  void initDialogData(CMDF* pmdf, CMDF_DecisionMatrix* pdm, int index = 0);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // level
  int getLevel(void);
  void setLevel(int level);

  // page
  uint16_t getPage(void);
  void setPage(uint16_t page);

  // offset
  uint32_t getOffset(void);
  void setOffset(uint32_t offset);

  // Number of DM rows
  uint16_t getRows(void);
  void setRows(uint16_t rows);

  // DM row size
  uint16_t getSize(void);
  void setSize(uint16_t size);

  // Render available actions
  void renderActions(void);

public slots:

  // Edit action item
  void editAction(void);

  // Add action item
  void addAction(void);

  // Duplicate action item
  void dupAction(void);

  // Delete action item
  void deleteAction(void);

  /*!
    Accept dialog data and write to register
  */
  void accept(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfDM* ui;

  /// Pointer to MDF
  CMDF* m_pmdf;

  CMDF_DecisionMatrix* m_pdm;
};

#endif // CDLGMDFDM_H
