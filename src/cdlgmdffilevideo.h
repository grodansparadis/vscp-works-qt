// cdlgmdffilevideo.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGMDFFILEVIDEO_H
#define CDLGMDFFILEVIDEO_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values
// in correct position among other things.
typedef enum mdf_file_video_index {
  index_file_video_none = 0,
  index_file_video_name,
  index_file_video_url,
  index_file_video_format,
  index_file_video_date,
} mdf_file_video_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_FILE_VIDEO_NAME   tr("Name: ")
#define PREFIX_MDF_FILE_VIDEO_URL    tr("URL: ")
#define PREFIX_MDF_FILE_VIDEO_FORMAT tr("Format: ")
#define PREFIX_MDF_FILE_VIDEO_DATE   tr("Date: ")

namespace Ui {
class CDlgMdfFileVideo;
}

class CDlgMdfFileVideo : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfFileVideo(QWidget* parent = nullptr);
  ~CDlgMdfFileVideo();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index The index for the field that will get focus

  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_file_video_index index = index_file_video_name);

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
      Format getter/setters
  */
  void setFormat(const QString& format);
  QString getFormat(void);

  /*!
      Date getter/setters
  */
  void setDate(const QString& strdate);
  QString getDate(void);

public slots:
  void accept(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgMdfFileVideo* ui;
  CMDF_Video* m_pvideo;
};

#endif // CDLGMDFFILEVIDEO_H
