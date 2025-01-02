// CFrmMdf.cpp
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
#ifndef CFRMMDF_H
#define CFRMMDF_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <mdf.h>
#include <register.h>
#include <vscp.h>
#include <vscp-client-base.h>

#include "cdlgmdfcontact.h"

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
  void editItem(void);

  /// Delete MDF data
  void deleteItem(void);

  /// Edit MDF module data
  void editModuleData(void);

  /// Edit MDF descriptions
  void editDescription(void);

  /// Delete MDF descriptions
  void deleteDescription(void);

  /// Edit MDF info URL's
  void editInfoUrl(void);

  /// Delete MDF info URL's
  void deleteInfoUrl(void);

  /// Edit MDF manufacturer data
  void editManufacturerData(void);

  /*!
    Show contact dialog (alow editing)
    @param pItem Pointer to selected item
    @param type Type of contact (phone/fax/email/web/social)
    @param title Title text on dialog
  */
  void showContactDialog(QMdfTreeWidgetItem* pItem, mdf_dlg_contact_type type, QString title);

  /*!
    Get top item MDF type for a sub MDF item
    @param pcurrentitem Pointer to item to test
    @return MDF type for top item or mdf_type_unknown if no top item found.
  */
  mdf_record_type getTopParentType(QTreeWidgetItem* pcurrentitem);

  /// Edit phone/fax/email/web/social data
  void editContact(void);

  /// Delete phone/fax/email/web/social data
  void deleteContact(void);

  /// Edit bootloader data
  void editBootLoader(void);

  /// Edit file list (top item file header)
  void editFileList(void);

  /// Edit file object
  void editFile(void);

  /// Delete file object
  void deleteFile(void);

  /// Edit register info
  void addRegister(void);

  /// Edit register info
  void editRegister(void);

  /// Delete register item
  void deleteRegister(void);

  /// Edit bit
  void editBitDefinition(void);

  /// Delete bit
  void deleteBitDefinition(void);

  /// Edit value
  void editValueDefinition(void);

  /// Delete value
  void deleteValueDefinition(void);

  /// Edit remote variable
  void editRemoteVariable(void);

  /// Delete remote variable
  void deleteRemoteVariable(void);

  /// Edit alarm bits
  void editAlarm(void);

  /// Edit decision matrix
  void editDM(void);

  /// Edit DM action
  void editAction(void);

  /// Edit DM event
  void editEvent(void);

  /// Add DM event
  void addEvent(void);

  /// Delete DM event
  void deleteEvent(void);

  /*!
    Fill in data from info map as children to parent item

    @param parent Pointer to parent treewidget (or description head)
    @param pobj MDF object
    @param pObjMap Pointer to map holding descriptions in different languages
    @param bChildIsKnown Set to true of pParent points to description head
  */
  void
  renderDescriptionItems(QTreeWidgetItem* pParent,
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
  renderInfoUrlItems(QTreeWidgetItem* pParent,
                     CMDF_Object* pobj,
                     std::map<std::string, std::string>* pObjMap,
                     bool bChildIsKnown = false);

  /*!
    Fill bit list info
    @param parent Pointer to parent treewidget
    @param dequebits Reference for std:deque holding bit infor objects
    @param bParentKnown If pParent is the head of the bit definitions this should be set to true
  */
  void
  renderBits(QTreeWidgetItem* pParent, std::deque<CMDF_Bit*>& dequebits, bool bParentKnown = false);

  /*!
    Render info for one bit
    @param pItemParent Pointer to parent item for bit
    @param pbit Object holding bit info
  */
  void
  renderBitItem(QMdfTreeWidgetItem* pItemParent, CMDF_Bit* pbit);

  /*!
    Fill value list info

    @param parent Pointer to parent treewidget
    @param dequevalues Reference for std:deque holding bit infor objects
    @param bParentKnown If pParent is the head of the bit definitions this should be set to true
  */
  void
  renderValues(QTreeWidgetItem* pParent, std::deque<CMDF_Value*>& dequevalues, bool bParentKnown = false);

  /*!
    Render info for one value
    @param pItemParent Pointer to parent item for bit
    @param pvalue Object holding value info
  */
  void
  renderValueItem(QMdfTreeWidgetItem* pItemParent, CMDF_Value* pvalue);

  /*!
    Fill in register info

    @parent Pointer to parent treewidget
    @preg Pointer to register definition
  */
  void
  renderRegisterItem(QTreeWidgetItem* pParent, CMDF_Register* preg);

  /*!
    Render all registers
    @param Header item to draw them as subitems under
  */
  void
  renderRegisters(QTreeWidgetItem* pParent);

  /*!
    Fill in remote variable info

    @parent Pointer to parent treewidget
    @preg Pointer to remote variable definition
  */
  void
  renderRemoteVariableItem(QTreeWidgetItem* pParent, CMDF_RemoteVariable* pvar);

  /*!
      Render all remote variables
      @param Header item to draw them as subitems under
    */
  void
  renderRemoteVariables(QTreeWidgetItem* pParent);

  /*!
    Remove all subitems of a head item that have a speciified type.
    @param pItem Pointer to head item
    @param type Type subitems should have
  */
  void
  removeSubItems(QMdfTreeWidgetItem* pItem, mdf_record_type type);

  /*!
    Render manufacturer email info
    @param pItemEmailHead Pointer to header item
  */

  void
  renderManufacturerEmail(QMdfTreeWidgetItem* pItemEmailHead);

  /*!
    Render manufacturer phone info
    @param pItemPhoneHead Pointer to header item
  */

  void
  renderManufacturerPhone(QMdfTreeWidgetItem* pItemPhoneHead);

  /*!
    Render manufacturer fax info
    @param pItemPhoneHead Pointer to fax item
  */

  void
  renderManufacturerFax(QMdfTreeWidgetItem* pItemFaxHead);

  /*!
    Render manufacturer web info
    @param pItemWebHead Pointer to fax item
  */

  void
  renderManufacturerWeb(QMdfTreeWidgetItem* pItemWebHead);

  /*!
    Render manufacturer social info
    @param pItemSocialHead Pointer to fax item
  */

  void
  renderManufacturerSocial(QMdfTreeWidgetItem* pItemSocialHead);

  /*!
    Render bootloader info
    @param pItemBootHead Pointer to boot info head
  */
  void
  renderBootLoadInfo(QMdfTreeWidgetItem* pItemBootHead, uint16_t selectedIndex = 0);

  /*!
    Render picture items
    @param pItemPicture Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderPictureItems(QMdfTreeWidgetItem* pItemPicture, uint16_t selectedIndex = 0);

  /*!
    Render picture subitems
    @param pItemPicture Pointer to picture item to render
    @param pPictureObj Pointer to picture object
    @param selectedIndex Item that should be selected
  */
  void
  renderPictureSubItems(QMdfTreeWidgetItem* pItemPicture, CMDF_Picture* pPictureObj, uint16_t selectedIndex = 0);

  /*!
    Render video items
    @param pItemVideo Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderVideoItems(QMdfTreeWidgetItem* pItemVideo, uint16_t selectedIndex = 0);

  /*!
    Render video subitems
    @param pItemVideo Pointer to picture item to render
    @param pVideoObj Pointer to picture object
    @param selectedIndex Item that should be selected
  */
  void
  renderVideoSubItems(QMdfTreeWidgetItem* pItemVideo, CMDF_Video* pVideoObj, uint16_t selectedIndex = 0);

  /*!
    Render manual items
    @param pItemManual Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderManualItems(QMdfTreeWidgetItem* pItemManual, uint16_t selectedIndex = 0);

  /*!
    Render manual subitems
    @param pItemManual Pointer to manual item to render
    @param pManualObj Pointer to manaul object
    @param selectedIndex Item that should be selected
  */
  void
  renderManualSubItems(QMdfTreeWidgetItem* pItemManual,
                       CMDF_Manual* pManualObj,
                       uint16_t selectedIndex = 0);

  /*!
    Render driver items
    @param pItemDriver Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderDriverItems(QMdfTreeWidgetItem* pItemDriver, uint16_t selectedIndex = 0);

  /*!
    Render driver subitems
    @param pItemDriver Pointer to driver item to render
    @param pDriverObj Pointer to driver object
    @param selectedIndex Item that should be selected
  */
  void
  renderDriverSubItems(QMdfTreeWidgetItem* pItemDriver,
                       CMDF_Driver* pDriverObj,
                       uint16_t selectedIndex = 0);

  /*!
    Render setup items
    @param pItemSetup Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderSetupItems(QMdfTreeWidgetItem* pItemSetup, uint16_t selectedIndex = 0);

  /*!
    Render setup subitems
    @param pItemSetup Pointer to setup item to render
    @param pSetupObj Pointer to setup object
    @param selectedIndex Item that should be selected
  */
  void
  renderSetupSubItems(QMdfTreeWidgetItem* pItemSetup,
                      CMDF_Setup* pSetupObj,
                      uint16_t selectedIndex = 0);

  /*!
    Render firmware items
    @param pItemFirmware Pointer to header item in tree for sub items
    @param selectedIndex Item that should be selected
  */
  void
  renderFirmwareItems(QMdfTreeWidgetItem* pItemFirmware, uint16_t selectedIndex = 0);

  /*!
    Render firmware subitems
    @param pItemFirmware Pointer to firmware item to render
    @param pFirmwareObj Pointer to firmware object
    @param selectedIndex Item that should be selected
  */
  void
  renderFirmwareSubItems(QMdfTreeWidgetItem* pItemFirmware,
                         CMDF_Firmware* pFirmwareObj,
                         uint16_t selectedIndex = 0);

  /*!
    Render actions
    @param pItemAction Action header
    @param paction Pointer to DM to render
    @param pSubItemAction Item that should be selected
  */
  void
  renderAction(QMdfTreeWidgetItem* pItemAction,
                CMDF_Action* paction);

  /*!
    Render Decision Matrix info
    @param pItemDM Pointer to DM header
    @param pdm Pointer to DM to render
    @param selectedIndex Item that should be selected
  */
  void
  renderDecisionMatrix(QMdfTreeWidgetItem* pItemDM,
                       CMDF_DecisionMatrix* pdm,
                       uint16_t selectedIndex = 0);

  /*!
    Render events
    @param pItemEvent Event header
  */

  void
  renderEvents(QMdfTreeWidgetItem* pItemEvent);

  /*!
    Render one action parameter
    @param pActionParamHeadItem Pointer to header for action parameters head item
    @param pActionParam Pointer to action parameter object that should be rendered
  */
  void
  renderActionParam(QMdfTreeWidgetItem* pActionParamItem,
                           CMDF_ActionParameter* pactionparam);

  /*!
    Render action parameters
    @param pActionParamItem Pointer to header for action parameters
    @param pAction Pointer to action for wich parameters should be renderd
  */
  void
  renderActionParameters(QMdfTreeWidgetItem* pActionParamItem,
                         CMDF_Action* pAction);

  /// Do the new operation
  void newMdf(void);

  /// Do the open operation
  void openMdf(void);

  /// Load MDF for selected node
  void loadMdf(void);

  /// Save MDF to external storage on XML format
  void saveMdf_XML(void);

  /// Save MDF to external storage on JSON format
  void saveMdf_JSON(void);

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

  /*!
    Headers in the tree that are used as reference for updates.
    The following pointers are always valid and set.
  */
  QMdfTreeWidgetItem* m_headModule;
  QMdfTreeWidgetItem* m_headManufacturer;
  QMdfTreeWidgetItem* m_headManufacturerPhone;
  QMdfTreeWidgetItem* m_headManufacturerFax;
  QMdfTreeWidgetItem* m_headManufacturerEmail;
  QMdfTreeWidgetItem* m_headManufacturerWeb;
  QMdfTreeWidgetItem* m_headManufacturerSocial;
  QMdfTreeWidgetItem* m_headBootLoader;
  QMdfTreeWidgetItem* m_headFile;
  QMdfTreeWidgetItem* m_headFilePicture;
  QMdfTreeWidgetItem* m_headFileVideo;
  QMdfTreeWidgetItem* m_headFileManual;
  QMdfTreeWidgetItem* m_headFileFirmware;
  QMdfTreeWidgetItem* m_headFileDriver;
  QMdfTreeWidgetItem* m_headFileSetupScript;
  QMdfTreeWidgetItem* m_headRegister;
  QMdfTreeWidgetItem* m_headRemoteVariabel;
  QMdfTreeWidgetItem* m_headAlarm;
  QMdfTreeWidgetItem* m_headDecisionMatrix;
  QMdfTreeWidgetItem* m_headEvent;

  /// path for last successfully opened MDF
  QString m_last_path;

  /// True if a loadeed file has been edited
  bool m_bChanged;
};

#endif // CFrmMdf_H
