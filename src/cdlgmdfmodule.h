// cdlgmdfmodule.h
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

#ifndef CDLGMDFMODULE_H
#define CDLGMDFMODULE_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

// Id for fields to identify them in the UI.
// Use to set focus on edit and to write changed values 
// in correct position among other things.
typedef enum mdf_module_index {
  index_module_name = 0,
  index_module_model,
  index_module_level,
  index_module_version,
  index_module_change_date,
  index_module_buffer_size,
  index_module_copyright
} mdf_module_index;

// Tree prefixes (Text before value in tree table)
#define PREFIX_MDF_MODULE_NAME tr("Name : ")
#define PREFIX_MDF_MODULE_MODEL tr("Model: ")
#define PREFIX_MDF_MODULE_LEVEL tr("VSCP Level: ")
#define PREFIX_MDF_MODULE_VERSION tr("Version: ")
#define PREFIX_MDF_MODULE_CHANGE_DATE tr("Change Date: ")
#define PREFIX_MDF_MODULE_BUFFER_SIZE tr("Buffer Size: ")
#define PREFIX_MDF_MODULE_COPYRIGHT tr("Copyright: ")

namespace Ui {
class CDlgMdfModule;
}

class CDlgMdfModule : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgMdfModule(QWidget* parent = nullptr);
  ~CDlgMdfModule();

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
      0 - Module name
      1 - Module name
      2 - Module version
      4 - Module date
      5 - Module buffer size
      6 - Module copyright
      
  */
  void initDialogData(const CMDF_Object* pmdfobj, mdf_module_index index = index_module_name);

  /*!
    Fill in description items
  */
  void fillDescription(void);

  /*!
    Fill in Info URL items
  */
  void fillInfoUrl(void);

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
  Ui::CDlgMdfModule* ui;
  CMDF* m_pmdf;
};

#endif // CDLGMDFMODULE_H
