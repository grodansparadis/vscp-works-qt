// cdlgmdfregisterbitlist.h
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

#ifndef CDLGMDFBITLIST_H
#define CDLGMDFBITLIST_H

#include <mdf.h>
#include <vscpworks.h>

#include "cdlgmdfregister.h"

#include <QDialog>

namespace Ui {
class CDlgMdfBitList;
}

class CDlgMdfBitList : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfBitList(QWidget* parent = nullptr);
  ~CDlgMdfBitList();

  /*!
      Set edit mode.
      GUID will be READ ONLY
  */
  void setEditMode();

  /*!
    Init dialog data
    @param pmdf Pointer to MDF object
  */
  void initDialogData(CMDF_Object* pobj, mdf_record_type type = mdf_type_register);

  /*!
    Get bit list for set type
    @return pointer to bitlist or nullpointer if invalid type
  */
  std::deque<CMDF_Bit*>* getBits(void);

  /*!
    Fill in register bit items
  */
  void renderBitItems(void);

  /*!
    Get a pointer to the bitlist for the type
    @return Pointer to bitlist or nullptr if invalid
  */
  std::deque<CMDF_Bit*>* getBitList(void);

  /*!
    Get pointer to bit object
    @param pos The bits start position
    @return Pointer to bit object or null pointer if pos is invalid
  */
  CMDF_Bit* getBitObj(uint8_t pos);

  /*!
    Check if bits overlap
    @param pbit2test pointer to bit to test
    @param bEdit Edit mode. The bit is already in the list.
    @return Overlapping bits or zero iof no bits overlap
  */
  uint8_t checkIfBitsOverlap(CMDF_Bit* pbit2test, bool bEdit = false);

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

  // Description buttons
  void addRegisterBit(void);
  void editRegisterBit(void);
  void dupRegisterBit(void);
  void deleteRegisterBit(void);

private:
  Ui::CDlgMdfBitList* ui;

  // MDF
  CMDF_Object* m_pobj;

  // Object type
  mdf_record_type m_type;

  // Render data
  std::set<uint8_t> m_bitset;
  std::map<uint8_t, CMDF_Bit*> m_bitmap;
};

#endif // CDLGMDFREGISTERBITLIST_H
