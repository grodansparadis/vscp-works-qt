// CFrmMdf.cpp
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
#ifndef CFRMMDF_H
#define CFRMMDF_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <mdf.h>
#include <register.h>
#include <vscp.h>
#include <vscp_client_base.h>

#include <set>

#include <QDialog>
#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
class QAction;
class QIcon;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QTextBrowser;
class QToolBar;
class QVBoxLayout;
class QAction;
class QTreeWidgetItem;
class QTreeWidget;
class QToolBox;
QT_END_NAMESPACE

#include <QMainWindow>
#include <QTableWidgetItem>

namespace Ui {
class CFrmMdf;
}

// #define TREE_LIST_FOUND_NODE_TYPE (QTreeWidgetItem::UserType + 1)

// ----------------------------------------------------------------------------

/*!
    Class that represent a row in the TX list
*/
class QMdfTreeWidgetItem : public QTreeWidgetItem {

public:
  QMdfTreeWidgetItem(const QString& text);
  QMdfTreeWidgetItem(mdf_record_type objtype);
  QMdfTreeWidgetItem(CMDF_Object* pobj, mdf_record_type objtype, uint16_t index = 0);
  QMdfTreeWidgetItem(QTreeWidgetItem* parent, mdf_record_type objtype);
  QMdfTreeWidgetItem(QTreeWidgetItem* parent, CMDF_Object* pobj, mdf_record_type objtype, uint16_t index = 0);
  virtual ~QMdfTreeWidgetItem();

  /*!
    Get MDF object type
    @return Return MDF object type
  */
  mdf_record_type getObjectType(void) { return m_objType; };

  /*!
    Set MDF object type
    @param type MDF object type
  */
  void setObjectType(mdf_record_type objtype) { m_objType = objtype; };

  /*!
    Get index for element
    @return Return element index
  */
  uint16_t getElementIndex(void) { return m_fieldIndex; };

  /*!
    Set index for element
    @param idx Element index used or direct editing
  */
  void setElementIndex(uint16_t idx) { m_fieldIndex = idx; };

  /*!
    Get MDF object
    @return Return pointer to MDF object
  */
  CMDF_Object* getObject(void) { return m_pMdfRecord; };

  /*!
    Set pointer to MDF object
    @param pobj Pointer to MDF object
  */
  void setObject(CMDF_Object* pobj) { m_pMdfRecord = pobj; };

private:
  /*!
    This is a pointer to a special object for certain types
  */
  // void *m_pSpecialObj;

  /*!
    This is the index for fields of a record.
  */
  uint16_t m_fieldIndex;

  /*!
    Pointer to MDF record
  */
  CMDF_Object* m_pMdfRecord;

  /*!
    Pointer to MDF record
  */
  mdf_record_type m_objType;
};

// ----------------------------------------------------------------------------

/*!
  The session window
*/

class CFrmMdf : public QMainWindow {
  Q_OBJECT

public:
  explicit CFrmMdf(QWidget* parent = nullptr, const char* path = nullptr);
  virtual ~CFrmMdf();

  /*!
      set Initial focus
  */
  void setInitialFocus(void);

public slots:

  /// Dialog return
  void done(int r);

  /// Open settings dialog
  void menu_open_main_settings(void);

  /// Show find nodes context menu
  void showMdfContextMenu(const QPoint& pos);

  /// Item has been clicked
  void onItemClicked(QTreeWidgetItem* item, int column);

  /// Item has been double clicked
  void onItemDoubleClicked(QTreeWidgetItem* item, int column);

  /// Edit MDF data
  void editData(void);

  /// Edit MDF module data
  void editModuleData(void);

  /// Edit MDF descriptions
  void editDescription(void);

  /// Edit MDF info URL's
  void editInfoUrl(void);

  /// Edit MDF manufacturer data
  void editManufacturerData(void);

  /*!
    Fill in data from info map as children to parent item

    @param parent Pointer to parent treewidget (or description head)
    @param pobj MDF object
    @param pObjMap Pointer to map holding descriptions in different languages
    @param bChildIsKnown Set to true of pParent points to description head
  */
  void
  fillDescriptionItems(QTreeWidgetItem* pParent, 
                          CMDF_Object* pobj, 
                          std::map<std::string, std::string>* pObjMap, 
                          bool bChildIsKnown = false);

  /*!
    Fill in data from help URL map as children to parent item

    @param parent Pointer to parent treewidget (or ino URL head)
    @param pobj MDF object
    @param pObjMap Pointer to map holding info URL's in different languages
    @param bChildIsKnown Set to true of pParent points to info URL head
  */
  void
  fillHelpUrlItems(QTreeWidgetItem* pParent, 
                      CMDF_Object* pobj, 
                      std::map<std::string, std::string>* pObjMap, 
                      bool bChildIsKnown = false);

  /*!
    Fill bit list info
    @param parent Pointer to parent treewidget
    @param dequebits Reference for std:deque holding bit infor objects
  */
  void
  fillBitInfo(QTreeWidgetItem* pParent, std::deque<CMDF_Bit*>& dequebits);

  /*!
    Fill value list info

    @param parent Pointer to parent treewidget
    @param dequevalues Reference for std:deque holding bit infor objects
  */
  void
  fillValueInfo(QTreeWidgetItem* pParent, std::deque<CMDF_Value*>& dequevalues);

  /*!
    Fill in register info

    @parent Pointer to parent treewidget
    @preg Pointer to register definition
  */
  void
  fillRegisterInfo(QTreeWidgetItem* pParent, CMDF_Register* preg);

  /// Do the new operation
  void newMdf(void);

  /// Do the open operation
  void openMdf(void);

  /// Load MDF for selected node
  void loadMdf(void);

  /*!
    Find next MDF type item in tree
    @param Pointer to MDF widget item
    @param type MDF item type to search for
    @return Pointer to item if found or
      null pointer if not found.
  */
  QMdfTreeWidgetItem* findMdfWidgetItem(QMdfTreeWidgetItem* pItem, mdf_record_type type);

  /*!
    Delete all child items of a specific MDF type
    @param Pointer to MDF widget item
    @param type MDF item type to search for
    @return Return number of items that been deleted.
  */

  int deleteMdfWidgetChildItems(QMdfTreeWidgetItem* pItem, mdf_record_type type);

signals:

private:
  // The UI definition
  Ui::CFrmMdf* ui;

  // Object that holds mdf definition in parsed form
  CMDF m_mdf;

  // The statusbar
  QStatusBar* m_bar;
};

#endif // CFrmMdf_H
