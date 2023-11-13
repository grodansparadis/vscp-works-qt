// cdlgmdffirmware.h
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

#ifndef CDLGMDFFIRMWARE_H
#define CDLGMDFFIRMWARE_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values 
// in correct position among other things.
typedef enum mdf_file_firmware_index {
  index_file_firmware_none = 0,
  index_file_firmware_name,
  index_file_firmware_target,
  index_file_firmware_target_code,
  index_file_firmware_url,
  index_file_firmware_format,
  index_file_firmware_date,
  index_file_firmware_version,
  index_file_firmware_size,
  index_file_firmware_md5,
} mdf_file_firmware_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_FIRMWARE_NAME tr("Name: ")
#define PREFIX_MDF_FIRMWARE_TARGET_CODE tr("Target Code: ")
#define PREFIX_MDF_FIRMWARE_URL tr("URL: ")
#define PREFIX_MDF_FIRMWARE_FORMAT tr("Format: ")
#define PREFIX_MDF_FIRMWARE_DATE tr("Date: ")
#define PREFIX_MDF_FIRMWARE_VERSION tr("Version: ")
#define PREFIX_MDF_FIRMWARE_MAX tr("Size: ")
#define PREFIX_MDF_FIRMWARE_ACCESS tr("MD5: ")

namespace Ui {
class CDlgMdfFileFirmware;
}

class CDlgMdfFileFirmware : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfFileFirmware(QWidget* parent = nullptr);
  ~CDlgMdfFileFirmware();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index The index for the field that will get focus
      
  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_file_firmware_index index = index_file_firmware_none);


  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  /*!
      Name getter/setters
  */
  void setName(const QString& name);
  QString getName(void);

  /*!
      Target getter/setters
  */
  void setTarget(const QString& url);
  QString getTarget(void);

  /*!
      target code getter/setters
  */
  void setTargetCode(uint16_t code);
  uint16_t getTargetCode(void);

  /*!
      URL getter/setters
  */
  void setUrl(const QString& url);
  QString getUrl(void);

  /*!
      Format getter/setters
  */
  void setFormat(const QString& format);
  QString getFormat(void);

  /*!
      Date getter/setters
  */
  void setDate(const QString& strdate);
  QString getDate(void);

  /*!
      Version getter/setters
  */
  void setVersion(const QString& ver);
  QString getVersion(void);

  /*!
      Size getter/setters
  */
  void setSize(size_t size);
  size_t getSize(void);

  /*!
      MD5 getter/setters
  */
  void setMd5(const QString& md5);
  QString getMd5(void);

public slots:
  void accept(void);


private:
  Ui::CDlgMdfFileFirmware* ui;
  CMDF_Firmware* m_pfirmware;
};

#endif // CDLGMDFFIRMWARE_H
