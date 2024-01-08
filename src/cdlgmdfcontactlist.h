// cdlgmdfcontactlist.h
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

#ifndef CDLGMDFCONTACTLIST_H
#define CDLGMDFCONTACTLIST_H

#include <vscpworks.h>
#include <mdf.h>

#include "cdlgmdfcontact.h"

#include <QDialog>

// typedef enum mdf_dlg_contact_type {
//   dlg_type_contact_none,
//   dlg_type_contact_phone,
//   dlg_type_contact_fax,
//   dlg_type_contact_email,
//   dlg_type_contact_web,
//   dlg_type_contact_social,
// } mdf_dlg_contact_type;


namespace Ui {
class CDlgMdfContactList;
}

class CDlgMdfContactList : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfContactList(QWidget* parent = nullptr);
  ~CDlgMdfContactList();

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
    @param pmdfobject Pointer to MDF object
    @param type Contact type
    @param title Dialog title
  */
  void initDialogData(CMDF_Manufacturer* pManufacturer,
                        mdf_dlg_contact_type type,
                        QString title);

  /*!
    Fill in contact items
  */
  void fillContactItems(void);


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
  void addContact(void);
  void editContact(void);
  void dupContact(void);
  void deleteContact(void);

private:
  Ui::CDlgMdfContactList* ui;

  /// Type of contact item in this list
  mdf_dlg_contact_type m_type;

  /// Holds pointer to manufacturer item
  CMDF_Manufacturer* m_pManufacturer;

  /// Hold pointer to contact item
  std::deque<CMDF_Item *> *m_pContactList;
};

#endif // CDLGMDFCONTACTList_H
