// cdlgmdfmanufacturer.h
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

#ifndef CDLGMDFMANUFACTURER_H
#define CDLGMDFMANUFACTURER_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values
// in correct position among other things.
typedef enum mdf_manufacturer_index {
  index_manufacturer_name = 0,
  index_manufacturer_street,
  index_manufacturer_city,
  index_manufacturer_town,
  index_manufacturer_post_code,
  index_manufacturer_region,
  index_manufacturer_state,
  index_manufacturer_country,
  index_manufacturer_phone,
  index_manufacturer_fax,
  index_manufacturer_email,
  index_manufacturer_web,
  index_manufacturer_social,
} mdf_manufacturer_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_MANUFACTURER_NAME      tr("Name : ")
#define PREFIX_MDF_MANUFACTURER_STREET    tr("Street: ")
#define PREFIX_MDF_MANUFACTURER_CITY      tr("City: ")
#define PREFIX_MDF_MANUFACTURER_TOWN      tr("Town: ")
#define PREFIX_MDF_MANUFACTURER_POST_CODE tr("Post Code: ")
#define PREFIX_MDF_MANUFACTURER_REGION    tr("Region: ")
#define PREFIX_MDF_MANUFACTURER_STATE     tr("State: ")
#define PREFIX_MDF_MANUFACTURER_COUNTRY   tr("Country: ")
#define PREFIX_MDF_MANUFACTURER_PHONE     tr("phone: ")
#define PREFIX_MDF_MANUFACTURER_FAX       tr("fax: ")
#define PREFIX_MDF_MANUFACTURER_EMAIL     tr("email: ")
#define PREFIX_MDF_MANUFACTURER_WEB       tr("web: ")
#define PREFIX_MDF_MANUFACTURER_SOCIAL    tr("social: ")

namespace Ui {
class CDlgMdfManufacturer;
}

class CDlgMdfManufacturer : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfManufacturer(QWidget* parent = nullptr);
  ~CDlgMdfManufacturer();

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
  void initDialogData(const CMDF_Object* pmdfobj, mdf_manufacturer_index index = index_manufacturer_name);

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
  void setName(const QString& str);
  QString getName(void);

  /*!
      Street getter/setters
  */
  void setStreet(const QString& str);
  QString getStreet(void);

  /*!
      City getter/setters
  */
  void setCity(const QString& str);
  QString getCity(void);

  /*!
      Town getter/setters
  */
  void setTown(const QString& str);
  QString getTown(void);

  /*!
      Post code getter/setters
  */
  void setPostCode(const QString& str);
  QString getPostCode(void);

  /*!
      Region getter/setters
  */
  void setRegion(const QString& str);
  QString getRegion(void);

  /*!
      State getter/setters
  */
  void setState(const QString& str);
  QString getState(void);

  /*!
      Country getter/setters
  */
  void setCountry(const QString& str);
  QString getCountry(void);

public slots:
  void accept(void);

  // Contact buttons
  void addContact(void);
  void editContact(void);
  void dupContact(void);
  void deleteContact(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfManufacturer* ui;
  CMDF_Manufacturer* m_pmanufacturer;
};

#endif // CDLGMDFMMANUFACTURER_H
