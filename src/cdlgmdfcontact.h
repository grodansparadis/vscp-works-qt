// cdlgmdfcontact.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, Grodans Paradis AB
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
    @param index The index for the field that will get focus
  */
  void initDialogData(const CMDF_Object* pmdfobj);


  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  void setName(const QString& name);
  QString getName(void);

  /*!
      GUID getter/setters
  */
  void setModel(const QString& guid);
  QString getModel(void);

  /*!
      GUID getter/setters
  */
  void setVersion(const QString& guid);
  QString getVersion(void);

  /*!
      GUID getter/setters
  */
  void setChangeDate(const QString& guid);
  QString getChangeDate(void);

  /*!
      GUID getter/setters
  */
  void setBufferSize(int size);
  int getBufferSize(void);

  /*!
      GUID getter/setters
  */
  void setCopyright(const QString& guid);
  QString getCopyright(void);

  /*!
      GUID getter/setters
  */
  void setDescription(const QString& guid);
  QString getDescription(void);

public slots:
  void accept(void);

private:
  Ui::CDlgMdfContact* ui;
  CMDF* m_pmdf;
};

#endif // CDLGMDFCONTACT_H
