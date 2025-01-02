// cdlgmdfcontact.h
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

#ifndef CDLGMDFCONTACT_H
#define CDLGMDFCONTACT_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

typedef enum mdf_dlg_contact_type {
  dlg_type_contact_none,
  dlg_type_contact_phone,
  dlg_type_contact_fax,
  dlg_type_contact_email,
  dlg_type_contact_web,
  dlg_type_contact_social,
} mdf_dlg_contact_type;


namespace Ui {
class CDlgMdfContact;
}

class CDlgMdfContact : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfContact(QWidget* parent = nullptr);
  ~CDlgMdfContact();

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
  void initDialogData(const CMDF_Object* pitemobj, 
                        mdf_dlg_contact_type type,
                        QString title);

  /*!
    Fill in contact descriptions
  */
  void fillDescription(void);

  /*!
    Fill in contact info URL's
  */
  void fillInfoUrl(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  void setValue(const QString& name);
  QString getValue(void);

public slots:
  void accept(void);

  // Description buttons
  void addDesc(void);
  void editDesc(void);
  void dupDesc(void);
  void deleteDesc(void);

  // Info URL buttons
  void addInfo(void);
  void editInfo(void);
  void dupInfo(void);
  void deleteInfo(void);

private:
  Ui::CDlgMdfContact* ui;
  CMDF_Item* m_pitemobj;
};

#endif // CDLGMDFCONTACT_H
