// cdlgmdfdriver.h
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

#ifndef CDLGMDFDRIVER_H
#define CDLGMDFDRIVER_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values 
// in correct position among other things.
typedef enum mdf_driver_index {
  index_file_driver_none = 0,
  index_file_driver_name,
  index_file_driver_url,
  index_file_driver_type,
  index_file_driver_os,
  index_file_driver_architecture,
  index_file_driver_os_version,
  index_file_driver_date,
  index_file_driver_version,
  index_file_driver_md5,
} mdf_file_driver_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_DRIVER_NAME tr("Name: ")
#define PREFIX_MDF_DRIVER_TYPE tr("Type: ")
#define PREFIX_MDF_DRIVER_OS tr("OS: ")
#define PREFIX_MDF_DRIVER_ARCHITECTURE tr("Architecture: ")
#define PREFIX_MDF_DRIVER_DATE tr("Date: ")
#define PREFIX_MDF_DRIVER_OS_VERSION tr("OS Version: ")

namespace Ui {
class CDlgMdfFileDriver;
}

class CDlgMdfFileDriver : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfFileDriver(QWidget* parent = nullptr);
  ~CDlgMdfFileDriver();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index The index for the field that will get focus
      
  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_driver_index index = index_file_driver_none);


  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  void setName(const QString& name);
  QString getName(void);

  /*!
      URL getter/setters
  */
  void setUrl(const QString& url);
  QString getUrl(void);

  /*!
      Type getter/setters
  */
  void setType(const QString& type);
  QString getType(void);

  /*!
      OS getter/setters
  */
  void setOs(const QString& os);
  QString getOs(void);

  /*!
      Architecture getter/setters
  */
  void setArchitecture(const QString& arch);
  QString getArchitecture(void); 

  /*!
      OS Version getter/setters
  */
  void setOsVersion(const QString& version);
  QString getOsVersion(void);

  /*!
      Date getter/setters
  */
  void setDate(const QString& isodate);
  QString getDate(void);

  /*!
      Version getter/setters
  */
  void setVersion(const QString& type);
  QString getVersion(void);

public slots:
  void accept(void);


private:
  Ui::CDlgMdfFileDriver* ui;
  CMDF_Driver* m_pdriver;
};

#endif // CDLGMDFDRIVER_H
