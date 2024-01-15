// cdlgmdfregisterbitlist.h
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

#ifndef CDLGMDFREGISTERBITLIST_H
#define CDLGMDFREGISTERBITLIST_H

#include <mdf.h>
#include <vscpworks.h>

#include "cdlgmdfregister.h"

#include <QDialog>

namespace Ui {
class CDlgMdfRegisterBitList;
}

class CDlgMdfRegisterBitList : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfRegisterBitList(QWidget* parent = nullptr);
  ~CDlgMdfRegisterBitList();

  /*!
      Set edit mode.
      GUID will be READ ONLY
  */
  void setEditMode();

  /*!
    Init dialog data
    @param pmdf Pointer to MDF object
  */
  void initDialogData(CMDF_Object* preg, mdf_record_type type = mdf_type_register);

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
    Check if bits overlap
    @param pbit2test pointer to bit to test
    @param bEdit Edit mode. The bit is already in the list.
    @return Overlapping bits or zero iof no bits overlap
  */
  uint8_t checkIfBitsOverlap(CMDF_Bit* pbit2test, bool bEdit = false)
  {
    uint8_t result = 0;
    std::deque<CMDF_Bit*>* pbits;

    // pbits = m_preg->getListBits();
    if (mdf_type_register == m_type) {
      pbits = ((CMDF_Register*)m_pobj)->getListBits();
    }
    else if (mdf_type_remotevar == m_type) {
      pbits = ((CMDF_RemoteVariable*)m_pobj)->getListBits();
    }
    else if (mdf_type_alarm == m_type) {
      pbits = ((CMDF*)m_pobj)->getAlarmListBits();
    }
    else {
      return 0;
    }

    for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
      CMDF_Bit* pbit = *it;
      // Don't test the edited bit of in edit mode
      if (bEdit && (pbit2test == pbit)) {
        continue;
      }
      if (nullptr != pbit) {
        result |= pbit->getMask();
      }
    }

    return (result & pbit2test->getMask());
  };

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
  Ui::CDlgMdfRegisterBitList* ui;

  // MDF
  CMDF_Object* m_pobj;
  // CMDF_Register* m_preg;

  // Object type
  mdf_record_type m_type;
};

#endif // CDLGMDFREGISTERBITLIST_H
