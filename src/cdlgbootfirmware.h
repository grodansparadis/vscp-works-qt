// cdlgbootfirmware.h
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



#ifndef CDLGBOOTFIRMWARE_H
#define CDLGBOOTFIRMWARE_H

#include <mdf.h>
#include <vscpworks.h>

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgBootFirmware;
}

class CDlgBootFirmware : public QDialog {
  Q_OBJECT

public:
  explicit CDlgBootFirmware(QWidget* parent = nullptr);
  ~CDlgBootFirmware();

  /*!
    Init dialog data
    @param pmdfobject Pointer to MDF object
    @param index The index for the field that will get focus

  */
  void initDialogData(CMDF *pmdf);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------


  /*!
    Check if user selected a local file
    @return true if local file selected
  */
  bool isLocalFile(void);

  /*!
    Get local file path
    @return path to set local file
  */
  QString
  getLocalFilePath(void);

  /*!
    Get local file type
  */
  QString
  getFileType(void);

  /*!
    Get selected firmware info
    @return pointer to current selected firmware object
  */
  CMDF_Firmware* getSelectedItem(void);

public slots:

  void selectLocalFile(void);
  void chkLocalFile(bool checked);
  void selectFirmwareFile(QListWidgetItem *item);

  void accept(void);

private:
  Ui::CDlgBootFirmware* ui;

  /// MDF object
  CMDF *m_pmdf;
};

#endif // CDLGBOOTFIRMWARE_H
