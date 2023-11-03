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
// tableWidget->resizeRowsToContents();

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <stdlib.h>

#include <mdf.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpworks.h>

#include <mustache.hpp>

#include "cfrmmdf.h"
#include "ui_cfrmmdf.h"

#include "cdlgeditmap.h"
#include "cdlgmdfbootloader.h"
#include "cdlgmdfcontact.h"
#include "cdlgmdfcontactlist.h"
#include "cdlgmdfdescription.h"
#include "cdlgmdfmanufacturer.h"
#include "cdlgmdfmodule.h"
#include "cdlgmdffile.h"
#include "cdlgmdffilepicture.h"
#include "cdlgmdffilevideo.h"
#include "cdlgmdffilemanual.h"
#include "cdlgmdffiledriver.h"
#include "cdlgmdffilefirmware.h"
#include "cdlgmdffilesetup.h"

#include <QClipboard>
#include <QFile>
#include <QJSEngine>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

QMdfTreeWidgetItem::QMdfTreeWidgetItem(const QString& text)
  : QTreeWidgetItem(0)
{
  m_objType    = mdf_type_unknown;
  m_fieldIndex = 0;
  m_pMdfRecord = nullptr;
}

QMdfTreeWidgetItem::QMdfTreeWidgetItem(mdf_record_type objtype)
  : QTreeWidgetItem(QTreeWidgetItem::Type)
{
  m_objType    = objtype;
  m_fieldIndex = 0;
  m_pMdfRecord = nullptr;
}

QMdfTreeWidgetItem::QMdfTreeWidgetItem(CMDF_Object* pobj, mdf_record_type objtype, uint16_t index)
  : QTreeWidgetItem(QTreeWidgetItem::Type)
{
  m_objType    = objtype;
  m_fieldIndex = index;
  m_pMdfRecord = pobj;
}

QMdfTreeWidgetItem::QMdfTreeWidgetItem(QTreeWidgetItem* parent, mdf_record_type objtype)
  : QTreeWidgetItem(parent, QTreeWidgetItem::Type)
{
  m_objType    = objtype;
  m_fieldIndex = 0;
  m_pMdfRecord = nullptr;
}

QMdfTreeWidgetItem::QMdfTreeWidgetItem(QTreeWidgetItem* parent,
                                       CMDF_Object* pobj,
                                       mdf_record_type objtype,
                                       uint16_t index)
  : QTreeWidgetItem(parent, QTreeWidgetItem::Type)
{
  m_objType    = objtype;
  m_fieldIndex = index;
  m_pMdfRecord = pobj;
}

QMdfTreeWidgetItem::~QMdfTreeWidgetItem()
{
  ;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CFrmMdf
//

CFrmMdf::CFrmMdf(QWidget* parent, const char* path)
  : QMainWindow(parent)
  , ui(new Ui::CFrmMdf)
{
  QString str;

  ui->setupUi(this);

  // QStatusBar *m_bar = new QStatusBar(this);
  // ui->statusbar->addWidget(m_bar);

  ui->treeMDF->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeMDF->setEditTriggers(QAbstractItemView::NoEditTriggers);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(std::string(tr("Node configuration module opened").toStdString()));

  if (nullptr == path) {
    //   spdlog::trace(std::string(tr("path is null").toStdString()));
    //   QMessageBox::information(this,
    //                            tr("vscpworks+"),
    //                            tr("Can't open node configuration window - "
    //                               "application configuration data is missing"),
    //                            QMessageBox::Ok);
    //   return;
  }

  setWindowTitle(str);

  // Initial default size of window
  int nWidth  = ui->centralwidget->width();
  int nHeight = ui->centralwidget->height();

  // Load MDF has been selected in the menu
  // connect(ui->actionLoadMdf, SIGNAL(triggered()), this, SLOT(loadSelectedMdf()));

  // Open pop up menu on right click on register list
  connect(ui->treeMDF,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmMdf::showMdfContextMenu);

  // New has been selected in the menu - Create new MDF file
  connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newMdf()));

  // Open has been selected in the menu - Open new MDF file
  connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openMdf()));

  // Open has been selected in the menu - Edit Module info
  connect(ui->actionEdit_item, SIGNAL(triggered()), this, SLOT(editData()));

  // Register row has been clicked.
  connect(ui->treeMDF,
          &QTreeWidget::itemClicked,
          this,
          &CFrmMdf::onItemClicked);

  // Register row has been double clicked.
  connect(ui->treeMDF,
          &QTreeWidget::itemDoubleClicked,
          this,
          &CFrmMdf::onItemDoubleClicked);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmMdf::~CFrmMdf()
{
}

///////////////////////////////////////////////////////////////////////////////
// newMdf
//

void
CFrmMdf::newMdf(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // getSaveFileName()
}

///////////////////////////////////////////////////////////////////////////////
// openMdf
//

void
CFrmMdf::openMdf(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open Module Description File (MDF)"),
                                                  "/usr/local/src/VSCP/vscp-works-qt/mdf/beijing_2.xml",
                                                  tr("MDF Files (*.mdf *.json *.xml);;All Files (*.*)"));

  if (fileName.length()) {
    qInfo() << "Selected MDF filename: " << fileName;
    int rv = m_mdf.parseMDF(fileName.toStdString());
    if (VSCP_ERROR_SUCCESS != rv) {
      spdlog::error("Failed to parse MDF file {0}", fileName.toStdString());
      QMessageBox::warning(this, APPNAME, tr("Failed to parse MDF file."));
      return;
    }

    // Load the tree with parsed objects6
    loadMdf();
  }
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void
CFrmMdf::done(int rv)
{
  if (QDialog::Accepted == rv) { // ok was pressed
                                 // vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
                                 // Session window
                                 // pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
  }
  // QMainWindow::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// menu_open_main_settings
//

void
CFrmMdf::menu_open_main_settings(void)
{
  // CDlgMainSettings* dlg = new CDlgMainSettings(this);
  // dlg->exec();
}

///////////////////////////////////////////////////////////////////////////////
// showFindNodesContextMenu
//

void
CFrmMdf::showMdfContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  if (nullptr == pItem) {
    menu->addAction(QString(tr("Fetch MDF")), this, SLOT(loadSelectedMdf()));
    menu->addAction(QString(tr("Fetch ALL MDF")), this, SLOT(loadAllMdf()));
    menu->addSeparator();
    menu->addAction(QString(tr("Rescan")), this, SLOT(doScan()));
    menu->addSeparator();
    menu->addAction(QString(tr("Configure")), this, SLOT(goConfig()));
    menu->addAction(QString(tr("Session")), this, SLOT(goSession()));
  }
  else {
    switch (pItem->getObjectType() /*pItem->type() - QTreeWidgetItem::UserType*/) {
      case mdf_type_unknown:
        menu->addAction(QString(tr("Unknown")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_redirection:
        menu->addAction(QString(tr("redirection")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_mdf:
        menu->addAction(QString(tr("Edit")), this, SLOT(editModuleData()));
        break;

      case mdf_type_mdf_item:
        menu->addAction(QString(tr("Edit")), this, SLOT(editModuleData()));
        break;

      case mdf_type_value:
        menu->addAction(QString(tr("Value")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_bit:
        menu->addAction(QString(tr("Bit")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_bit_item:
        menu->addAction(QString(tr("Bit item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_register_page:
        menu->addAction(QString(tr("Register Page")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_register:
        menu->addAction(QString(tr("Register")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_register_item:
        menu->addAction(QString(tr("Register item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_remotevar:
        menu->addAction(QString(tr("Remote Variable")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_remotevar_item:
        menu->addAction(QString(tr("Remote Variable Item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_action_param:
        menu->addAction(QString(tr("Action Param")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_action:
        menu->addAction(QString(tr("Action")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_action_item:
        menu->addAction(QString(tr("Action item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_decision_matrix:
        menu->addAction(QString(tr("DM")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_event_data:
        menu->addAction(QString(tr("Event Data")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_event_data_item:
        menu->addAction(QString(tr("Event Data item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_event:
        menu->addAction(QString(tr("Event")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_event_item:
        menu->addAction(QString(tr("Event item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_value_item:
        menu->addAction(QString(tr("Value Item")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_bootloader:
        // menu->addAction(QString(tr("Bootloader")), this, SLOT(editBootlLoader()));
        menu->addAction(QString(tr("Edit bootloader")), this, SLOT(editBootLoader()));
        break;

      case mdf_type_bootloader_item:
        // menu->addAction(QString(tr("Bootloader")), this, SLOT(editBootlLoader()));
        menu->addAction(QString(tr("Edit bootloader item")), this, SLOT(editBootLoader()));
        break;

      case mdf_type_alarm:
        menu->addAction(QString(tr("Alarm")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_address:
        menu->addAction(QString(tr("Address")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_manufacturer:
        // menu->addAction(QString(tr("Manufacturer")), this, SLOT(loadSelectedMdf()));
        menu->addAction(QString(tr("Edit manufacturer")), this, SLOT(editManufacturerData()));
        break;

      case mdf_type_manufacturer_item:
        // menu->addAction(QString(tr("Manufacturer")), this, SLOT(loadSelectedMdf()));
        menu->addAction(QString(tr("Edit")), this, SLOT(editManufacturerData()));
        break;

      case mdf_type_email:
        menu->addAction(QString(tr("Edit email")), this, SLOT(editContact()));
        break;

      case mdf_type_email_item:
        menu->addAction(QString(tr("Edit email item")), this, SLOT(editContact()));
        break;

      case mdf_type_phone:
        menu->addAction(QString(tr("Edit phone")), this, SLOT(editContact()));
        break;

      case mdf_type_phone_item:
        menu->addAction(QString(tr("Edit phone item")), this, SLOT(editContact()));
        break;

      case mdf_type_fax:
        menu->addAction(QString(tr("Edit fax")), this, SLOT(editContact()));
        break;

      case mdf_type_fax_item:
        menu->addAction(QString(tr("Edit fax item")), this, SLOT(editContact()));
        break;

      case mdf_type_web:
        menu->addAction(QString(tr("Edit web")), this, SLOT(editContact()));
        break;

      case mdf_type_web_item:
        menu->addAction(QString(tr("Edit web item")), this, SLOT(editContact()));
        break;

      case mdf_type_social:
        menu->addAction(QString(tr("Edit social")), this, SLOT(editContact()));
        break;

      case mdf_type_social_item:
        menu->addAction(QString(tr("Edit social item")), this, SLOT(editContact()));
        break;

      case mdf_type_file:
        menu->addAction(QString(tr("Edit file items")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_picture:
        menu->addAction(QString(tr("Edit pictures")), this, SLOT(editFileList()));
        break;

      case mdf_type_picture_item:
        menu->addAction(QString(tr("Edit picture items")), this, SLOT(editFile()));
        break;

      case mdf_type_video:
        menu->addAction(QString(tr("Edit video items")), this, SLOT(editFileList()));
        break;

      case mdf_type_video_item:
        menu->addAction(QString(tr("Edit video")), this, SLOT(editFile()));
        break;

      case mdf_type_firmware:
        menu->addAction(QString(tr("Edit firmware items")), this, SLOT(editFileList()));
        break;

      case mdf_type_firmware_item:
        menu->addAction(QString(tr("Edit firmware")), this, SLOT(editFile()));
        break;

      case mdf_type_driver:
        menu->addAction(QString(tr("Edit driver items")), this, SLOT(editFileList()));
        break;

      case mdf_type_driver_item:
        menu->addAction(QString(tr("Edit driver")), this, SLOT(editFile()));
        break;

      case mdf_type_setup:
        menu->addAction(QString(tr("Edit setup items")), this, SLOT(editFileList()));
        break;

      case mdf_type_setup_item:
        menu->addAction(QString(tr("Edit Setup")), this, SLOT(editFile()));
        break;

      case mdf_type_manual:
        menu->addAction(QString(tr("Edit manual items")), this, SLOT(editFileList()));
        break;

      case mdf_type_manual_item:
        menu->addAction(QString(tr("Edit manual")), this, SLOT(editFile()));
        break;

      case mdf_type_generic_string:
        menu->addAction(QString(tr("string")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_generic_number:
        menu->addAction(QString(tr("number")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_generic_url:
        menu->addAction(QString(tr("url")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_generic_date:
        menu->addAction(QString(tr("date")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_generic_access:
        menu->addAction(QString(tr("access")), this, SLOT(loadSelectedMdf()));
        break;

      case mdf_type_generic_description:
        menu->addAction(QString(tr("Edit Descriptions")), this, SLOT(editDescription(void)));
        break;

      case mdf_type_generic_description_item:
        menu->addAction(QString(tr("Edit Description")), this, SLOT(editDescription(void)));
        break;

      case mdf_type_generic_help_url:
        menu->addAction(QString(tr("Edit Info URL's")), this, SLOT(editInfoUrl(void)));
        break;

      case mdf_type_generic_help_url_item:
        menu->addAction(QString(tr("Edit Info URL")), this, SLOT(editInfoUrl(void)));
        break;

      default:
        menu->addAction(QString(tr("Default")), this, SLOT(loadSelectedMdf()));
        break;
    }
  }

  menu->popup(ui->treeMDF->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// fillDescriptionItems
//

void
CFrmMdf::fillDescriptionItems(QTreeWidgetItem* pParent,
                              CMDF_Object* pobj,
                              std::map<std::string, std::string>* pObjMap,
                              bool bChildIsKnown)
{
  QString str;
  QMdfTreeWidgetItem* pItemModuleDescription;
  QMdfTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent) {
    return;
  }

  if (nullptr == pobj) {
    return;
  }

  if (nullptr == pObjMap) {
    return;
  }

  if (!bChildIsKnown) {
    pItemModuleDescription = new QMdfTreeWidgetItem(pParent, pobj, mdf_type_generic_description);
    pItemModuleDescription->setText(0, "Description");
    pParent->addChild(pItemModuleDescription);
  }
  else {
    pItemModuleDescription = (QMdfTreeWidgetItem*)pParent;
  }

  // Must be items to fill in childs
  if (!pObjMap->size()) {
    return;
  }

  int idx                                  = 0;
  std::map<std::string, std::string>* pmap = pObjMap;
  for (auto const& x : *pmap) {
    str   = x.first.c_str() + QString(": ") + x.second.c_str();
    pItem = new QMdfTreeWidgetItem(pItemModuleDescription, pobj, mdf_type_generic_description_item, idx);
    pItem->setText(0, str);
    pParent->addChild(pItem);
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillInfoUrlItems
//

void
CFrmMdf::fillInfoUrlItems(QTreeWidgetItem* pParent,
                          CMDF_Object* pobj,
                          std::map<std::string, std::string>* pObjMap,
                          bool bChildIsKnown)
{
  QString str;
  QMdfTreeWidgetItem* pItemModuleHelpUrl;
  QMdfTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent) {
    return;
  }

  if (nullptr == pobj) {
    return;
  }

  if (nullptr == pObjMap) {
    return;
  }

  if (!bChildIsKnown) {
    pItemModuleHelpUrl = new QMdfTreeWidgetItem(pParent, pobj, mdf_type_generic_help_url);
    pItemModuleHelpUrl->setText(0, "Help/Info URL");
    pParent->addChild(pItemModuleHelpUrl);
  }
  else {
    pItemModuleHelpUrl = (QMdfTreeWidgetItem*)pParent;
  }

  // Must be items to fill in childs
  if (!pObjMap->size()) {
    return;
  }

  int idx                                  = 0;
  std::map<std::string, std::string>* pmap = pObjMap;
  for (auto const& x : *pmap) {
    str   = x.first.c_str() + QString(": ") + x.second.c_str();
    pItem = new QMdfTreeWidgetItem(pItemModuleHelpUrl, pobj, mdf_type_generic_help_url_item, idx);
    pItem->setText(0, str);
    pParent->addChild(pItem);
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillRegisterInfo
//

void
CFrmMdf::fillBitInfo(QTreeWidgetItem* pParent, std::deque<CMDF_Bit*>& dequebits)
{
  QString str;
  QMdfTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent) {
    return;
  }

  QMdfTreeWidgetItem* pItemBitDefs = new QMdfTreeWidgetItem(pParent, mdf_type_bit);
  pItemBitDefs->setText(0, "Bit definitions");
  pParent->addChild(pItemBitDefs);

  // Must be items to fill in childs
  if (!dequebits.size()) {
    return;
  }

  for (int i = 0; i < dequebits.size(); i++) {

    CMDF_Bit* pbit = dequebits[i];

    str = QString("Bitfield no %1 Bits:{").arg(/*dequebits.size()-*/ i);
    for (int j = pbit->getPos(); j < qMin(8, pbit->getPos() + pbit->getWidth()); j++) {
      str += QString(" %1 ").arg(j);
    }
    str += "}";
    QMdfTreeWidgetItem* pItemParent = new QMdfTreeWidgetItem(pItemBitDefs, mdf_type_bit_item);
    pItemParent->setText(0, str);
    pItemBitDefs->addChild(pItemParent);

    str   = QString("Name: %1").arg(pbit->getName().c_str());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_string);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    str   = QString("Pos: %1").arg(pbit->getPos());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    str   = QString("Width: %1").arg(pbit->getWidth());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    str   = QString("Default: %1").arg(pbit->getDefault());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    str   = QString("Min: %1").arg(pbit->getMin());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    str   = QString("Max: %1").arg(pbit->getMax());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemBitDefs->addChild(pItem);

    // str   = QString("Access rights: %1").arg(pbit->getAccess());
    str = QString("Access: %1 (").arg(pbit->getAccess());
    if (2 & pbit->getAccess()) {
      str += "r";
    }
    if (1 & pbit->getAccess()) {
      str += "w";
    }
    str += ")";
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_access);
    pItem->setText(0, str);
    pItemParent->addChild(pItem);

    str   = QString("Mask: 0b%1").arg(pbit->getMask(), 1, 2);
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_string);
    pItem->setText(0, str);
    pItemParent->addChild(pItem);

    // Valid values
    fillValueInfo(pItemParent, *pbit->getListValues());

    // Descriptions
    fillDescriptionItems(pItemParent, pbit, pbit->getMapDescription());

    // Info URL's
    fillInfoUrlItems(pItemParent, pbit, pbit->getMapDescription());
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillValueInfo
//

void
CFrmMdf::fillValueInfo(QTreeWidgetItem* pParent, std::deque<CMDF_Value*>& dequevalues)
{
  QString str;
  QMdfTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent) {
    return;
  }

  QMdfTreeWidgetItem* pItemValue = new QMdfTreeWidgetItem(pParent, mdf_type_value);
  pItemValue->setText(0, "Valid values");
  pParent->addChild(pItemValue);

  // Must be items to fill in childs
  if (!dequevalues.size()) {
    return;
  }

  for (int i = 0; i < dequevalues.size(); i++) {

    CMDF_Value* pvalue = dequevalues[i];

    str = QString("Value %1 (%2)").arg(i).arg(pvalue->getValue().c_str());
    // for (int j = pvalue->getPos(); j < qMin(8, pvalue->getPos() + pvalue->getWidth()); j++) {
    //   str += QString(" %1 ").arg(j);
    // }
    // str += "}";
    QMdfTreeWidgetItem* pItemParent = new QMdfTreeWidgetItem(pItemValue, mdf_type_value_item);
    pItemParent->setText(0, str);
    pItemValue->addChild(pItemParent);

    str   = QString("Name: %1").arg(pvalue->getName().c_str());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_string);
    pItem->setText(0, str);
    pItemValue->addChild(pItem);

    str   = QString("Value: %1").arg(pvalue->getValue().c_str());
    pItem = new QMdfTreeWidgetItem(pItemParent, mdf_type_generic_number);
    pItem->setText(0, str);
    pItemValue->addChild(pItem);

    // Descriptions
    fillDescriptionItems(pItemParent, pvalue, pvalue->getMapDescription());

    // Info URL's
    fillInfoUrlItems(pItemParent, pvalue, pvalue->getMapDescription());
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillRegisterInfo
//

void
CFrmMdf::fillRegisterInfo(QTreeWidgetItem* pParent, CMDF_Register* preg)
{
  QString str;
  QMdfTreeWidgetItem* pItem;

  // Check pointers
  if (nullptr == pParent) {
    return;
  }
  if (nullptr == preg) {
    return;
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_string);
  if (nullptr != pItem) {
    str = QString("Name: %1").arg(preg->getName().c_str());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Page: %1").arg(preg->getPage());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Offset: %1").arg(preg->getOffset());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Default value: %1").arg(preg->getDefault().c_str());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Min value: %1").arg(preg->getMin());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Max value: %1").arg(preg->getMax());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Register span: %1").arg(preg->getSpan());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Register width: %1").arg(preg->getWidth());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Size: %1").arg(preg->getSize());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_access);
  if (nullptr != pItem) {
    str = QString("Access: %1 (").arg(preg->getAccess());
    if (2 & preg->getAccess()) {
      str += "r";
    }
    if (1 & preg->getAccess()) {
      str += "w";
    }
    str += ")";
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_number);
  if (nullptr != pItem) {
    str = QString("Type: %1").arg(preg->getType());
    switch (preg->getType()) {

      case MDF_REG_TYPE_STANDARD:
        str += " - (std)";
        break;

      case MDF_REG_TYPE_DMATRIX1:
        str += " - (dm1)";
        break;

      case MDF_REG_TYPE_BLOCK:
        str += " - (block)";
        break;
    }
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_string);
  if (nullptr != pItem) {
    str = QString("Foreground color: 0x%1").arg(preg->getForegroundColor(), 1, 16);
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QMdfTreeWidgetItem(pParent, mdf_type_generic_string);
  if (nullptr != pItem) {
    str = QString("Background color: 0x%1").arg(preg->getBackgroundColor(), 1, 16);
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  // Fill in bit field info
  fillBitInfo(pParent, *preg->getListBits());

  // Fill in valid values
  fillValueInfo(pParent, *preg->getListValues());

  // Descriptions
  fillDescriptionItems(pParent, preg, preg->getMapDescription());

  // Info URL's
  fillInfoUrlItems(pParent, preg, preg->getMapDescription());
}

///////////////////////////////////////////////////////////////////////////////
// renderManufacturerEmail
//

void
CFrmMdf::renderManufacturerEmail(QMdfTreeWidgetItem* pItemEmailHead)
{
  int index = 0;
  QMdfTreeWidgetItem* pItem;
  CMDF_Item* pItemEmail;

  do {
    pItemEmail = m_mdf.getManufacturer()->getEmailObj(index);
    if (nullptr != pItemEmail) {
      pItem = new QMdfTreeWidgetItem(pItemEmailHead, pItemEmail, mdf_type_email_item, index);
      pItem->setText(0, pItemEmail->getName().c_str());
      pItemEmailHead->addChild(pItem);
      fillDescriptionItems(pItem,
                           pItemEmail,
                           pItemEmail->getMapDescription());
      fillInfoUrlItems(pItem,
                       pItemEmail,
                       pItemEmail->getMapInfoUrl());
      index++;
    }
  } while (nullptr != pItemEmail);
}

///////////////////////////////////////////////////////////////////////////////
// renderManufacturerPhone
//

void
CFrmMdf::renderManufacturerPhone(QMdfTreeWidgetItem* pItemPhoneHead)
{
  int index = 0;
  QMdfTreeWidgetItem* pItem;
  CMDF_Item* pItemPhone;

  do {
    pItemPhone = m_mdf.getManufacturer()->getPhoneObj(index);
    if (nullptr != pItemPhone) {
      pItem = new QMdfTreeWidgetItem(pItemPhoneHead, pItemPhone, mdf_type_phone_item, index);
      pItem->setText(0, pItemPhone->getName().c_str());
      pItemPhoneHead->addChild(pItem);
      fillDescriptionItems(pItem,
                           pItemPhone,
                           pItemPhone->getMapDescription());
      fillInfoUrlItems(pItem,
                       pItemPhone,
                       pItemPhone->getMapInfoUrl());
      index++;
    }
  } while (nullptr != pItemPhone);
}

///////////////////////////////////////////////////////////////////////////////
// renderManufacturerFax
//

void
CFrmMdf::renderManufacturerFax(QMdfTreeWidgetItem* pItemFaxHead)
{
  int index = 0;
  QMdfTreeWidgetItem* pItem;
  CMDF_Item* pItemFax;

  do {
    pItemFax = m_mdf.getManufacturer()->getFaxObj(index);
    if (nullptr != pItemFax) {
      pItem = new QMdfTreeWidgetItem(pItemFaxHead, pItemFax, mdf_type_phone_item, index);
      pItem->setText(0, pItemFax->getName().c_str());
      pItemFaxHead->addChild(pItem);
      fillDescriptionItems(pItem,
                           pItemFax,
                           pItemFax->getMapDescription());
      fillInfoUrlItems(pItem,
                       pItemFax,
                       pItemFax->getMapInfoUrl());
      index++;
    }
  } while (nullptr != pItemFax);
}

///////////////////////////////////////////////////////////////////////////////
// renderManufacturerWeb
//

void
CFrmMdf::renderManufacturerWeb(QMdfTreeWidgetItem* pItemWebHead)
{
  int index = 0;
  QMdfTreeWidgetItem* pItem;
  CMDF_Item* pItemWeb;

  do {
    pItemWeb = m_mdf.getManufacturer()->getWebObj(index);
    if (nullptr != pItemWeb) {
      pItem = new QMdfTreeWidgetItem(pItemWebHead, pItemWeb, mdf_type_phone_item, index);
      pItem->setText(0, pItemWeb->getName().c_str());
      pItemWebHead->addChild(pItem);
      fillDescriptionItems(pItem,
                           pItemWeb,
                           pItemWeb->getMapDescription());
      fillInfoUrlItems(pItem,
                       pItemWeb,
                       pItemWeb->getMapInfoUrl());
      index++;
    }
  } while (nullptr != pItemWeb);
}

///////////////////////////////////////////////////////////////////////////////
// renderManufacturerSocial
//

void
CFrmMdf::renderManufacturerSocial(QMdfTreeWidgetItem* pItemSocialHead)
{
  int index = 0;
  QMdfTreeWidgetItem* pItem;
  CMDF_Item* pItemSocial;

  do {
    pItemSocial = m_mdf.getManufacturer()->getSocialObj(index);
    if (nullptr != pItemSocial) {
      pItem = new QMdfTreeWidgetItem(pItemSocialHead, pItemSocial, mdf_type_phone_item, index);
      pItem->setText(0, pItemSocial->getName().c_str());
      pItemSocialHead->addChild(pItem);
      fillDescriptionItems(pItem,
                           pItemSocial,
                           pItemSocial->getMapDescription());
      fillInfoUrlItems(pItem,
                       pItemSocial,
                       pItemSocial->getMapInfoUrl());
      index++;
    }
  } while (nullptr != pItemSocial);
}

///////////////////////////////////////////////////////////////////////////////
// renderBootloadInfo
//

void
CFrmMdf::renderBootLoadInfo(QMdfTreeWidgetItem* pItemBootHead, uint16_t selectedIndex)
{
  QString str;
  QMdfTreeWidgetItem* pItem;

  pItemBootHead->setSelected(false);

  CMDF_BootLoaderInfo* pboot = m_mdf.getBootLoaderObj();
  if (nullptr != pboot) {
    str   = QString("Algorithm: %1").arg(pboot->getAlgorithm());
    pItem = new QMdfTreeWidgetItem(pItemBootHead, pboot, mdf_type_bootloader_item, static_cast<mdf_bootloader_index>(index_bootloader_algorithm));
    pItem->setText(0, str);
    pItemBootHead->addChild(pItem);
    // Select item
    if (index_bootloader_algorithm == static_cast<mdf_bootloader_index>(selectedIndex)) {
      pItem->setSelected(true);
    }

    str   = QString("Block Size: %1").arg(pboot->getBlockSize());
    pItem = new QMdfTreeWidgetItem(pItemBootHead, pboot, mdf_type_bootloader_item, static_cast<mdf_bootloader_index>(index_bootloader_blocksize));
    pItem->setText(0, str);
    pItemBootHead->addChild(pItem);
    // Select item
    if (index_bootloader_blocksize == static_cast<mdf_bootloader_index>(selectedIndex)) {
      pItem->setSelected(true);
    }

    str   = QString("Block Count: %1").arg(pboot->getBlockCount());
    pItem = new QMdfTreeWidgetItem(pItemBootHead, pboot, mdf_type_bootloader_item, static_cast<mdf_bootloader_index>(index_bootloader_blockcount));
    pItem->setText(0, str);
    pItemBootHead->addChild(pItem);
    // Select item
    if (index_bootloader_blockcount == static_cast<mdf_bootloader_index>(selectedIndex)) {
      pItem->setSelected(true);
    }
  }
  else {
    spdlog::error("No bootloader object when reading MDF");
  }
}

///////////////////////////////////////////////////////////////////////////////
// renderPictureItems
//

void
CFrmMdf::renderPictureItems(QMdfTreeWidgetItem* pItemPicture, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Picture* pPictureObj;

  // Return if noting to render
  if (!m_mdf.getPictureCount()) {
    return;
  }

  do {
    pPictureObj = m_mdf.getPictureObj(index);
    if (nullptr != pPictureObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemPicture, &m_mdf, mdf_type_picture_item, index);
      str                       = QString("Picture %1 - ").arg(index) + pPictureObj->getName().c_str();
      pItem->setText(0, str);
      pItemPicture->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: ") + pPictureObj->getName().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: ") + pPictureObj->getUrl().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Format: ") + pPictureObj->getFormat().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: %1").arg(pPictureObj->getDate().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pPictureObj, pPictureObj->getMapDescription());
      fillInfoUrlItems(pItem, pPictureObj, pPictureObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pPictureObj);
}

///////////////////////////////////////////////////////////////////////////////
// renderVideoItems
//

void
CFrmMdf::renderVideoItems(QMdfTreeWidgetItem* pItemVideo, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Video* pVideoObj;

  // Return if noting to render
  if (!m_mdf.getVideoCount()) {
    return;
  }

  do {
    pVideoObj = m_mdf.getVideoObj(index);
    if (nullptr != pVideoObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemVideo, &m_mdf, mdf_type_video_item, index);
      str                       = QString("Video %1 - ").arg(index) + pVideoObj->getName().c_str();
      pItem->setText(0, str);
      pItemVideo->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: ") + pVideoObj->getName().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: ") + pVideoObj->getUrl().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Format: ") + pVideoObj->getFormat().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: %1").arg(pVideoObj->getDate().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pVideoObj, pVideoObj->getMapDescription());
      fillInfoUrlItems(pItem, pVideoObj, pVideoObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pVideoObj);
}

///////////////////////////////////////////////////////////////////////////////
// renderManualItems
//

void
CFrmMdf::renderManualItems(QMdfTreeWidgetItem* pItemManual, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Manual* pManualObj;

  // Return if noting to render
  if (!m_mdf.getManualCount()) {
    return;
  }

  do {
    pManualObj = m_mdf.getManualObj(index);
    if (nullptr != pManualObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemManual, &m_mdf, mdf_type_manual_item, index);
      str                       = QString("Manual %1 - %2").arg(index).arg(pManualObj->getName().c_str());
      pItem->setText(0, str);
      pItemManual->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: %1").arg(pManualObj->getName().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: ") + pManualObj->getUrl().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Format: ") + pManualObj->getFormat().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: %1").arg(pManualObj->getDate().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pManualObj, pManualObj->getMapDescription());
      fillInfoUrlItems(pItem, pManualObj, pManualObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pManualObj);
}

///////////////////////////////////////////////////////////////////////////////
// renderDriverItems
//

void
CFrmMdf::renderDriverItems(QMdfTreeWidgetItem* pItemDriver, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Driver* pDriverObj;

  // Return if noting to render
  if (!m_mdf.getDriverCount()) {
    return;
  }

  do {
    pDriverObj = m_mdf.getDriverObj(index);
    if (nullptr != pDriverObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemDriver, &m_mdf, mdf_type_driver_item, index);
      str                       = QString("Driver %1 - %2 %3").arg(index).arg(pDriverObj->getName().c_str()).arg(pDriverObj->getDate().c_str());
      pItem->setText(0, str);
      pItemDriver->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: %1").arg(pDriverObj->getName().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Version: %1.%2.%3").arg(pDriverObj->getVersionMajor()).arg(pDriverObj->getVersionMinor()).arg(pDriverObj->getVersionPatch());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: ") + pDriverObj->getDate().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: ") + pDriverObj->getUrl().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Type: ") + pDriverObj->getType().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Architecture: ") + pDriverObj->getFormat().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("OS: ") + pDriverObj->getOS().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("OS Version: ") + pDriverObj->getOSVer().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("MD5 Checksum: ") + pDriverObj->getMd5().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pDriverObj, pDriverObj->getMapDescription());
      fillInfoUrlItems(pItem, pDriverObj, pDriverObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pDriverObj);
}

///////////////////////////////////////////////////////////////////////////////
// renderSetupItems
//

void
CFrmMdf::renderSetupItems(QMdfTreeWidgetItem* pItemSetup, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Setup* pSetupObj;

  // Return if noting to render
  if (!m_mdf.getSetupCount()) {
    return;
  }

  do {
    pSetupObj = m_mdf.getSetupObj(index);
    if (nullptr != pSetupObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemSetup, &m_mdf, mdf_type_setup_item, index);
      str                       = QString("Script %1 - %2").arg(index).arg(pSetupObj->getName().c_str());
      pItem->setText(0, str);
      pItemSetup->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: %1").arg(pSetupObj->getName().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: ") + pSetupObj->getUrl().c_str();
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Format: ") + pSetupObj->getFormat().c_str();
        // pSubItem->setFont(0, fontTopItem);
        //  pSubItem->setForeground(0, greenBrush);
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: %1").arg(pSetupObj->getDate().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pSetupObj, pSetupObj->getMapDescription());
      fillInfoUrlItems(pItem, pSetupObj, pSetupObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pSetupObj);
}

///////////////////////////////////////////////////////////////////////////////
// renderFirmwareItems
//

void
CFrmMdf::renderFirmwareItems(QMdfTreeWidgetItem* pItemFirmware, uint16_t selectedIndex)
{
  int index = 0;
  QString str;
  CMDF_Firmware* pFirmwareObj;

  // Return if noting to render
  if (!m_mdf.getFirmwareCount()) {
    return;
  }

  do {
    pFirmwareObj = m_mdf.getFirmwareObj(index);
    if (nullptr != pFirmwareObj) {
      QMdfTreeWidgetItem* pItem = new QMdfTreeWidgetItem(pItemFirmware, &m_mdf, mdf_type_firmware_item, index);
      str                       = QString("Firmware %1 - %2 %3").arg(index).arg(pFirmwareObj->getDate().c_str()).arg(pFirmwareObj->getName().c_str());
      pItem->setText(0, str);
      pItemFirmware->addChild(pItem);

      QMdfTreeWidgetItem* pSubItem;

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Name: %1").arg(pFirmwareObj->getName().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Version: %1.%2.%3").arg(pFirmwareObj->getVersionMajor()).arg(pFirmwareObj->getVersionMinor()).arg(pFirmwareObj->getVersionPatch());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_url);
      if (nullptr != pSubItem) {
        str = QString("URL: %1").arg(pFirmwareObj->getUrl().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_date);
      if (nullptr != pSubItem) {
        str = QString("Date: %1").arg(pFirmwareObj->getDate().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_number);
      if (nullptr != pSubItem) {
        str = QString("Target Code: %1").arg(pFirmwareObj->getTargetCode());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Target: %1").arg(pFirmwareObj->getTarget().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("Format: %1").arg(pFirmwareObj->getFormat().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_number);
      if (nullptr != pSubItem) {
        str = QString("Size: %1").arg(pFirmwareObj->getSize());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_generic_string);
      if (nullptr != pSubItem) {
        str = QString("MD5 Checksum: %1").arg(pFirmwareObj->getMd5().c_str());
        pSubItem->setText(0, str);
        pItem->addChild(pSubItem);
      }

      fillDescriptionItems(pItem, pFirmwareObj, pFirmwareObj->getMapDescription());
      fillInfoUrlItems(pItem, pFirmwareObj, pFirmwareObj->getMapInfoUrl());
    }

    index++;

  } while (nullptr != pFirmwareObj);
}

///////////////////////////////////////////////////////////////////////////////
// removeSubItems
//

void
CFrmMdf::removeSubItems(QMdfTreeWidgetItem* pItem, mdf_record_type type)
{
  // Save expansion state
  bool bExpanded = pItem->isExpanded();

  // Expand to make traversion possible
  pItem->setExpanded(true);

  QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
  while (type == piter->getObjectType()) {
    pItem->removeChild(piter);
    delete piter;
    piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
  }

  pItem->setExpanded(bExpanded);
}

///////////////////////////////////////////////////////////////////////////////
// loadMdf
//

void
CFrmMdf::loadMdf(void)
{
  QString str;
  size_t index = 0;
  QMdfTreeWidgetItem* pItem;

  // Clear the treeview
  ui->treeMDF->clear();

  QBrush greenBrush(QColor("green"));
  QBrush blueBrush(QColor("blue"));
  QBrush redBrush(QColor("red"));
  QBrush yellowBrush(QColor("yellow"));
  QBrush cyanBrush(QColor("cyan"));
  QBrush magentaBrush(QColor("magenta"));
  QBrush greyBrush(QColor("grey"));
  QBrush darkGreyBrush(QColor("darkGrey"));
  QBrush darkMagentaBrush(QColor("darkMagenta"));

  QFont fontTopItem;
  fontTopItem.setBold(true);

  pItem = new QMdfTreeWidgetItem(&m_mdf, mdf_type_redirection);
  pItem->setText(0, tr("Redirection"));
  // pItem->setDisabled(true);
  ui->treeMDF->addTopLevelItem(pItem);

  // Module
  QMdfTreeWidgetItem* pItemModule = new QMdfTreeWidgetItem(&m_mdf, mdf_type_mdf);
  pItemModule->setObject(&m_mdf);
  pItemModule->setFont(0, fontTopItem);
  pItemModule->setForeground(0, blueBrush);
  pItemModule->setText(0, tr("Module"));
  // pItemModule->setObject(&m_mdf);
  ui->treeMDF->addTopLevelItem(pItemModule);
  pItemModule->setExpanded(true);

  // Module information
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_name);
  pItem->setText(0, PREFIX_MDF_MODULE_NAME + m_mdf.getModuleName().c_str());
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_MODEL + QString(m_mdf.getModuleModel().c_str());
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_model);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_LEVEL + QString::number(m_mdf.getModuleLevel() + 1);
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_level);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_VERSION + QString(m_mdf.getModuleVersion().c_str());
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_version);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_CHANGE_DATE + m_mdf.getModuleChangeDate().c_str();
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_change_date);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_BUFFER_SIZE + QString::number(m_mdf.getModuleBufferSize());
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_buffer_size);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = PREFIX_MDF_MODULE_COPYRIGHT + m_mdf.getModuleCopyright().c_str();
  pItem = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_mdf_item, index_module_copyright);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  fillDescriptionItems(pItemModule, &m_mdf, m_mdf.getMapDescription());
  fillInfoUrlItems(pItemModule, &m_mdf, m_mdf.getHelpUrlMap());

  // * * * Manufacturer info * * *

  QMdfTreeWidgetItem* pItemManufacturer = new QMdfTreeWidgetItem(pItemModule,
                                                                 m_mdf.getManufacturer(),
                                                                 mdf_type_manufacturer);
  pItemManufacturer->setFont(0, fontTopItem);
  pItemManufacturer->setForeground(0, greenBrush);
  pItemManufacturer->setText(0, tr("Manufacturer"));
  ui->treeMDF->addTopLevelItem(pItemManufacturer);

  CMDF_Manufacturer* pManufacturer = m_mdf.getManufacturer();
  if (nullptr != pManufacturer) {

    str   = QString("Name: ") + m_mdf.getManufacturerName().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_name);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Street: ") + m_mdf.getManufacturerStreetAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_street);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("City: ") + m_mdf.getManufacturerCityAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_city);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Town: ") + m_mdf.getManufacturerTownAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_town);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Post Code: ") + m_mdf.getManufacturerPostCodeAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_post_code);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Region: ") + m_mdf.getManufacturerRegionAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_region);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("State: ") + m_mdf.getManufacturerStateAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_state);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Country: ") + m_mdf.getManufacturerCountryAddress().c_str();
    pItem = new QMdfTreeWidgetItem(pItemManufacturer,
                                   m_mdf.getManufacturer(),
                                   mdf_type_manufacturer_item,
                                   index_manufacturer_country);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    // * * * Manufacturer - email * * *

    QMdfTreeWidgetItem* pItemEmailHead = new QMdfTreeWidgetItem(pItemManufacturer,
                                                                m_mdf.getManufacturer(),
                                                                mdf_type_email);
    // pItemEmailHead->setFont(0, fontTopItem);
    // pItemEmailHead->setForeground(0, darkMagentaBrush);
    pItemEmailHead->setText(0, tr("Email"));
    pItemModule->addChild(pItemEmailHead);

    renderManufacturerEmail(pItemEmailHead);

    QMdfTreeWidgetItem* pItemPhoneHead = new QMdfTreeWidgetItem(pItemManufacturer,
                                                                m_mdf.getManufacturer(),
                                                                mdf_type_phone);
    // pItemPhoneHead->setFont(0, fontTopItem);
    // pItemEmailHead->setForeground(0, darkMagentaBrush);
    pItemPhoneHead->setText(0, tr("Phone"));
    pItemModule->addChild(pItemPhoneHead);

    renderManufacturerPhone(pItemPhoneHead);

    QMdfTreeWidgetItem* pItemFaxHead = new QMdfTreeWidgetItem(pItemManufacturer,
                                                              m_mdf.getManufacturer(),
                                                              mdf_type_fax);
    // pItemFaxHead->setFont(0, fontTopItem);
    // pItemEmailHead->setForeground(0, darkMagentaBrush);
    pItemFaxHead->setText(0, tr("Fax"));
    pItemModule->addChild(pItemFaxHead);

    renderManufacturerFax(pItemFaxHead);

    QMdfTreeWidgetItem* pItemWebHead = new QMdfTreeWidgetItem(pItemManufacturer,
                                                              m_mdf.getManufacturer(),
                                                              mdf_type_web);
    // pItemWebHead->setFont(0, fontTopItem);
    // pItemEmailHead->setForeground(0, darkMagentaBrush);
    pItemWebHead->setText(0, tr("Web"));
    pItemModule->addChild(pItemWebHead);

    renderManufacturerWeb(pItemWebHead);

    QMdfTreeWidgetItem* pItemSocialHead = new QMdfTreeWidgetItem(pItemManufacturer,
                                                                 m_mdf.getManufacturer(),
                                                                 mdf_type_social);
    // pItemSocialHead->setFont(0, fontTopItem);
    // pItemEmailHead->setForeground(0, darkMagentaBrush);
    pItemSocialHead->setText(0, tr("Social"));
    pItemModule->addChild(pItemSocialHead);

    renderManufacturerSocial(pItemSocialHead);
  }
  else {
    spdlog::error("No manufacturer object when reading MDF");
  }

  // * * * Boot info * * *

  QMdfTreeWidgetItem* pItemBoot = new QMdfTreeWidgetItem(pItemModule,
                                                         m_mdf.getBootLoaderObj(),
                                                         mdf_type_bootloader,
                                                         static_cast<mdf_bootloader_index>(index_bootloader_algorithm));
  pItemBoot->setFont(0, fontTopItem);
  pItemBoot->setForeground(0, greenBrush);
  pItemBoot->setText(0, tr("Boot"));
  ui->treeMDF->addTopLevelItem(pItemBoot);

  renderBootLoadInfo(pItemBoot);

  // * * * Files * * *

  QMdfTreeWidgetItem* pItemFiles = new QMdfTreeWidgetItem(pItemModule, &m_mdf, mdf_type_file);
  pItemFiles->setFont(0, fontTopItem);
  pItemFiles->setForeground(0, greenBrush);
  pItemFiles->setText(0, tr("Files"));
  ui->treeMDF->addTopLevelItem(pItemFiles);


  // Picture file(s)

  QMdfTreeWidgetItem* pItemPicture = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_picture);
  pItemPicture->setFont(0, fontTopItem);
  pItemPicture->setText(0, tr("Pictures"));
  pItemFiles->addChild(pItemPicture);

  renderPictureItems(pItemPicture);

  // Manuals

  QMdfTreeWidgetItem* pItemManual = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_manual);
  pItemManual->setFont(0, fontTopItem);
  pItemManual->setText(0, tr("Manuals"));
  pItemFiles->addChild(pItemManual);

  renderManualItems(pItemManual);

  // Video file(s)

  QMdfTreeWidgetItem* pItemVideo = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_video);
  pItemVideo->setFont(0, fontTopItem);
  pItemVideo->setText(0, tr("Videos"));
  pItemFiles->addChild(pItemVideo);

  renderVideoItems(pItemVideo);

  // Firmware file(s)

  QMdfTreeWidgetItem* pItemFirmware = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_firmware);
  pItemFirmware->setFont(0, fontTopItem);
  pItemFirmware->setText(0, tr("Firmware"));
  pItemFiles->addChild(pItemFirmware);

  renderFirmwareItems(pItemFirmware);

  // Drivers

  QMdfTreeWidgetItem* pItemDriver = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_driver);
  pItemDriver->setFont(0, fontTopItem);
  pItemDriver->setText(0, tr("Drivers"));
  pItemFiles->addChild(pItemDriver);

  renderDriverItems(pItemDriver);

  // Setup script

  QMdfTreeWidgetItem* pItemSetup = new QMdfTreeWidgetItem(pItemFiles, &m_mdf, mdf_type_setup);
  pItemSetup->setFont(0, fontTopItem);
  // pItemSetup->setForeground(0, greenBrush);
  pItemSetup->setText(0, tr("Setup scripts"));
  pItemFiles->addChild(pItemSetup);

  renderSetupItems(pItemSetup);

  // Registers

  uint32_t nPages;
  std::set<uint16_t> pages;
  QMdfTreeWidgetItem* pSubItem;

  QMdfTreeWidgetItem* pItemRegister = new QMdfTreeWidgetItem(pItemModule, mdf_type_register);
  pItemRegister->setFont(0, fontTopItem);
  pItemRegister->setForeground(0, greenBrush);
  pItemRegister->setText(0, tr("Registers"));
  ui->treeMDF->addTopLevelItem(pItemRegister);

  // Fetch number of pages and pages
  nPages                           = m_mdf.getPages(pages);
  std::deque<CMDF_Register*>* regs = m_mdf.getRegisterObjList();

  // If we have pages separate registers in pages
  if (nPages > 1) {
    for (auto itr : pages) {

      pItem = new QMdfTreeWidgetItem(pItemRegister, mdf_type_register_page);
      if (nullptr != pItem) {

        str = QString("Register page: %1").arg(itr);
        pItem->setText(0, str);
        pItemRegister->addChild(pItem);

        // Add registers for page
        for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
          if (itr == (*it)->getPage()) {
            pSubItem = new QMdfTreeWidgetItem(pItem, mdf_type_register_item);
            if (nullptr != pSubItem) {
              str = QString("Register  %1 %2").arg((*it)->getOffset()).arg((*it)->getName().c_str());
              pSubItem->setText(0, str);
              pItem->addChild(pSubItem);
              fillRegisterInfo(pSubItem, *it);
            }
          }
        }
      }
    }
  }
  // We only have one page
  else {
    std::map<uint32_t, CMDF_Register*> mapRegs;
    m_mdf.getRegisterMap(0, mapRegs);
    fillRegisterInfo(pItem, mapRegs[0]);
  }

  // * * * Remote variables * * *

  QMdfTreeWidgetItem* pItemRemoteVar = new QMdfTreeWidgetItem(pItemModule, mdf_type_remotevar);
  pItemRemoteVar->setFont(0, fontTopItem);
  pItemRemoteVar->setForeground(0, greenBrush);
  pItemRemoteVar->setText(0, tr("Remote Variables"));
  ui->treeMDF->addTopLevelItem(pItemRemoteVar);

  std::deque<CMDF_RemoteVariable*>* pRemoteVarList = m_mdf.getRemoteVariableList();
  if (nullptr != pRemoteVarList) {

    QString str;
    QMdfTreeWidgetItem* pItem;

    for (int i = 0; i < pRemoteVarList->size(); i++) {
      CMDF_RemoteVariable* pvar = (*pRemoteVarList)[i];
      pSubItem                  = new QMdfTreeWidgetItem(pItemRemoteVar, mdf_type_remotevar_item);
      if (nullptr != pSubItem) {
        str = QString("Remote variable: %1 -- %2").arg(i).arg(pvar->getName().c_str());
        pSubItem->setText(0, str);
        pItemRemoteVar->addChild(pSubItem);

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
        if (nullptr != pItem) {
          str = QString("Name: %1").arg(pvar->getName().c_str());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
        if (nullptr != pItem) {
          str = QString("Type: %1 (%2)").arg(pvar->getType()).arg(pvar->getTypeString().c_str());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
        if (nullptr != pItem) {
          str = QString("Byte count: %1").arg(pvar->getTypeByteCount());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
        if (nullptr != pItem) {
          str = QString("Default: %1").arg(pvar->getDefault().c_str());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
        if (nullptr != pItem) {
          str = QString("Offset: %1").arg(pvar->getOffset());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
        if (nullptr != pItem) {
          str = QString("Page: %1").arg(pvar->getPage());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
        if (nullptr != pItem) {
          str = QString("Bit position: %1").arg(pvar->getBitPos());
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_access);
        if (nullptr != pItem) {
          str = QString("Access: %1 (").arg(pvar->getAccess());
          if (2 & pvar->getAccess()) {
            str += "r";
          }
          if (1 & pvar->getAccess()) {
            str += "w";
          }
          str += ")";

          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
        if (nullptr != pItem) {
          str = QString("Foreground color: 0x%1").arg(pvar->getForegroundColor(), 1, 16);
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
        if (nullptr != pItem) {
          str = QString("Background color: 0x%1").arg(pvar->getBackgroundColor(), 1, 16);
          pItem->setText(0, str);
          pSubItem->addChild(pItem);
        }

        // Fill in bit field info
        fillBitInfo(pSubItem, *pvar->getListBits());

        // Fill in valid values
        fillValueInfo(pSubItem, *pvar->getListValues());

        // Descriptions
        fillDescriptionItems(pSubItem, pvar, pvar->getMapDescription());

        // Info URL's
        fillInfoUrlItems(pSubItem, pvar, pvar->getMapDescription());
      }
    }
  }

  // * * * Alarm * * *

  QMdfTreeWidgetItem* pItemAlarm = new QMdfTreeWidgetItem(pItemModule, mdf_type_alarm);
  pItemAlarm->setFont(0, fontTopItem);
  pItemAlarm->setForeground(0, greenBrush);
  pItemAlarm->setText(0, tr("Alarm"));
  ui->treeMDF->addTopLevelItem(pItemAlarm);

  std::deque<CMDF_Bit*>* palarm = m_mdf.getAlarmList();

  // Fill in bit field info
  fillBitInfo(pItemAlarm, *palarm);

  // * * * Decision Matrix * * *

  QMdfTreeWidgetItem* pItemDM = new QMdfTreeWidgetItem(pItemModule, mdf_type_decision_matrix);
  pItemDM->setFont(0, fontTopItem);
  pItemDM->setForeground(0, greenBrush);
  pItemDM->setText(0, tr("Decision Matrix"));
  ui->treeMDF->addTopLevelItem(pItemDM);

  CMDF_DecisionMatrix* pdm = m_mdf.getDM();
  if (nullptr != pdm) {

    pItem = new QMdfTreeWidgetItem(pItemDM, mdf_type_generic_number);
    if (nullptr != pItem) {
      str = QString("Level: %1").arg(pdm->getLevel());
      pItem->setText(0, str);
      pItemDM->addChild(pItem);
    }

    pItem = new QMdfTreeWidgetItem(pItemDM, mdf_type_generic_number);
    if (nullptr != pItem) {
      str = QString("Start page: %1").arg(pdm->getStartPage());
      pItem->setText(0, str);
      pItemDM->addChild(pItem);
    }

    pItem = new QMdfTreeWidgetItem(pItemDM, mdf_type_generic_number);
    if (nullptr != pItem) {
      str = QString("Start offset: %1").arg(pdm->getStartOffset());
      pItem->setText(0, str);
      pItemDM->addChild(pItem);
    }

    pItem = new QMdfTreeWidgetItem(pItemDM, mdf_type_generic_number);
    if (nullptr != pItem) {
      str = QString("Row count: %1 rows").arg(pdm->getRowCount());
      pItem->setText(0, str);
      pItemDM->addChild(pItem);
    }

    pItem = new QMdfTreeWidgetItem(pItemDM, mdf_type_generic_number);
    if (nullptr != pItem) {
      str = QString("Row size: %1 bytes").arg(pdm->getRowSize());
      pItem->setText(0, str);
      pItemDM->addChild(pItem);
    }

    std::deque<CMDF_Action*>* pActionList = pdm->getActionList();

    if (nullptr != pActionList) {

      QString str;
      QMdfTreeWidgetItem* pSubItem;

      QMdfTreeWidgetItem* pItemAction = new QMdfTreeWidgetItem(pItemDM, mdf_type_action);
      if (nullptr != pItemAction) {
        str = QString("Actions").arg(pdm->getRowSize());
        pItemAction->setText(0, str);
        pItemDM->addChild(pItemAction);
      }

      for (int i = 0; i < pActionList->size(); i++) {

        CMDF_Action* paction               = (*pActionList)[i];
        QMdfTreeWidgetItem* pSubItemAction = new QMdfTreeWidgetItem(pItemAction, mdf_type_action_item);

        if (nullptr != pSubItemAction) {

          str = QString("Action: %1 %2").arg(paction->getCode()).arg(paction->getName().c_str());
          pSubItemAction->setText(0, str);
          pItemAction->addChild(pSubItemAction);

          pItem = new QMdfTreeWidgetItem(pSubItemAction, mdf_type_generic_number);
          if (nullptr != pItem) {
            str = QString("Code: %1").arg(paction->getCode());
            pItem->setText(0, str);
            pSubItemAction->addChild(pItem);
          }

          pItem = new QMdfTreeWidgetItem(pSubItemAction, mdf_type_generic_string);
          if (nullptr != pItem) {
            str = QString("Name: %1").arg(paction->getName().c_str());
            pItem->setText(0, str);
            pSubItemAction->addChild(pItem);
          }

          std::deque<CMDF_ActionParameter*>* pActionParamList = paction->getListActionParameter();
          if (nullptr != pActionParamList) {

            QMdfTreeWidgetItem* pActionParamItem = new QMdfTreeWidgetItem(pSubItemAction, mdf_type_generic_number);
            str                                  = QString("Action Parameters");
            pActionParamItem->setText(0, str);
            pSubItemAction->addChild(pActionParamItem);

            for (int j = 0; j < pActionParamList->size(); j++) {

              CMDF_ActionParameter* pactionparam = (*pActionParamList)[j];
              if (nullptr != pSubItemAction) {

                QString str;

                pItem = new QMdfTreeWidgetItem(pActionParamItem, mdf_type_generic_string);
                if (nullptr != pItem) {
                  str = QString("Name: %1 - %2").arg(j).arg(pactionparam->getName().c_str());
                  pItem->setText(0, str);
                  pActionParamItem->addChild(pItem);
                }

                pItem = new QMdfTreeWidgetItem(pActionParamItem, mdf_type_generic_number);
                if (nullptr != pItem) {
                  str = QString("Offset: %1").arg(pactionparam->getOffset());
                  pItem->setText(0, str);
                  pActionParamItem->addChild(pItem);
                }

                pItem = new QMdfTreeWidgetItem(pActionParamItem, mdf_type_generic_number);
                if (nullptr != pItem) {
                  str = QString("Min: %1").arg(pactionparam->getMin());
                  pItem->setText(0, str);
                  pActionParamItem->addChild(pItem);
                }

                pItem = new QMdfTreeWidgetItem(pActionParamItem, mdf_type_generic_number);
                if (nullptr != pItem) {
                  str = QString("Max: %1").arg(pactionparam->getMax());
                  pItem->setText(0, str);
                  pActionParamItem->addChild(pItem);
                }

                // Fill in bit field info
                fillBitInfo(pActionParamItem, *pactionparam->getListBits());

                // Fill in valid values
                fillValueInfo(pActionParamItem, *pactionparam->getListValues());

                // Descriptions
                fillDescriptionItems(pActionParamItem, pactionparam, pactionparam->getMapDescription());

                // Info URL's
                fillInfoUrlItems(pActionParamItem, pactionparam, pactionparam->getMapDescription());
              }
            }
          }

          // Descriptions
          fillDescriptionItems(pSubItemAction, paction, paction->getMapDescription());

          // Info URL's
          fillInfoUrlItems(pSubItemAction, paction, paction->getMapDescription());
        }
      }

      // * * * Events * * *

      QMdfTreeWidgetItem* pItemEvent = new QMdfTreeWidgetItem(pItemModule, mdf_type_event);
      pItemEvent->setFont(0, fontTopItem);
      pItemEvent->setForeground(0, greenBrush);
      pItemEvent->setText(0, tr("Events"));
      ui->treeMDF->addTopLevelItem(pItemEvent);

      std::deque<CMDF_Event*>* pEventList = m_mdf.getEventList();

      if (nullptr != pEventList) {

        QString str;
        QMdfTreeWidgetItem* pItem;

        for (int i = 0; i < pEventList->size(); i++) {

          CMDF_Event* pevent = (*pEventList)[i];
          pSubItem           = new QMdfTreeWidgetItem(pItemEvent, mdf_type_event_item);
          if (nullptr != pSubItem) {

            str = QString("Event: %1 -- %2").arg(i).arg(pevent->getName().c_str());
            pSubItem->setText(0, str);
            pItemEvent->addChild(pSubItem);

            pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
            if (nullptr != pItem) {
              str = QString("Name: %1").arg(pevent->getName().c_str());
              pItem->setText(0, str);
              pSubItem->addChild(pItem);
            }

            pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
            if (nullptr != pItem) {
              str = QString("VSCP Class: %1").arg(pevent->getClass());
              pItem->setText(0, str);
              pSubItem->addChild(pItem);
            }

            pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
            if (nullptr != pItem) {
              str = QString("VSCP Type: %1").arg(pevent->getType());
              pItem->setText(0, str);
              pSubItem->addChild(pItem);
            }

            pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_number);
            if (nullptr != pItem) {
              str = QString("VSCP Priority: %1").arg(pevent->getPriority());
              pItem->setText(0, str);
              pSubItem->addChild(pItem);
            }

            pItem = new QMdfTreeWidgetItem(pSubItem, mdf_type_generic_string);
            if (nullptr != pItem) {
              str = QString("Direction: %1 (%2)").arg((MDF_EVENT_DIR_IN == pevent->getDirection()) ? "In" : "Out").arg(pevent->getDirection());
              pItem->setText(0, str);
              pSubItem->addChild(pItem);
            }

            // Event Data
            QMdfTreeWidgetItem* pItemEventData = new QMdfTreeWidgetItem(pSubItem, mdf_type_event_data);
            // pItemEvent->setFont(0, fontTopItem);
            // pItemEvent->setForeground(0, greenBrush);
            pItemEventData->setText(0, tr("Event data"));
            pSubItem->addChild(pItemEventData);

            std::deque<CMDF_EventData*>* pEventDataList = pevent->getListEventData();
            if (nullptr != pEventDataList) {

              QString str;
              QMdfTreeWidgetItem* pEventSubItem;
              QMdfTreeWidgetItem* pItem;

              for (int j = 0; j < pEventDataList->size(); j++) {

                CMDF_EventData* pEventData = (*pEventDataList)[j];

                pEventSubItem = new QMdfTreeWidgetItem(pItemEventData, mdf_type_event_data_item);
                if (nullptr != pEventSubItem) {

                  // Event data
                  str = QString("Event data: %1 -- %2").arg(j).arg(pEventData->getName().c_str());
                  pEventSubItem->setText(0, str);
                  pItemEventData->addChild(pEventSubItem);

                  pItem = new QMdfTreeWidgetItem(pEventSubItem, mdf_type_generic_string);
                  if (nullptr != pItem) {
                    str = QString("Name: %1").arg(pEventData->getName().c_str());
                    pItem->setText(0, str);
                    pEventSubItem->addChild(pItem);
                  }

                  pItem = new QMdfTreeWidgetItem(pEventSubItem, mdf_type_generic_number);
                  if (nullptr != pItem) {
                    str = QString("Offset: %1").arg(pEventData->getOffset());
                    pItem->setText(0, str);
                    pEventSubItem->addChild(pItem);
                  }

                  // Fill in bit field info
                  fillBitInfo(pEventSubItem, *pEventData->getListBits());

                  // Fill in valid values
                  fillValueInfo(pEventSubItem, *pEventData->getListValues());

                  // Descriptions
                  fillDescriptionItems(pEventSubItem, pEventData, pEventData->getMapDescription());

                  // Info URL's
                  fillInfoUrlItems(pEventSubItem, pEventData, pEventData->getMapDescription());
                }
              } // EventDataList
            }   // list exist

            // Descriptions
            fillDescriptionItems(pSubItem, pevent, pevent->getMapDescription());

            // Info URL's
            fillInfoUrlItems(pSubItem, pevent, pevent->getMapDescription());
          }
        }
      } // EventList
    }
  }

  // Recipes
  // QMdfTreeWidgetItem *pItemRecipes = new QMdfTreeWidgetItem(pItemModule, 1);
  // pItemRecipes->setFont(0, fontTopItem);
  // pItemRecipes->setForeground(0, blueBrush);
  // pItemRecipes->setText(0, tr("Events"));
  // ui->treeMDF->addTopLevelItem(pItemRecipes);

  // ui->btnScan->setEnabled(false);
  // CFoundNodeWidgetItem *pItem;
  // QMdfTreeWidgetItemIterator it(ui->treeFound);
  // while (*it) {
  //   pItem = (CFoundNodeWidgetItem *)(*it);
  //   doLoadMdf(pItem->m_nodeid);
  //   ++it;
  // }
  // ui->btnScan->setEnabled(true);
  // QString fileName = QFileDialog::getOpenFileName(this,
  //   tr("Open Module Description File (MDF)"), ".", tr("MDF Files (*.mdf *.json *.xml)"));

  // ui->treeMDF->expandAll();
  //}

  ///////////////////////////////////////////////////////////////////////////////
  // onFindNodesTreeWidgetItemClicked
  //

  // void
  // CFrmMdf::onFindNodesTreeWidgetItemClicked(QMdfTreeWidgetItem* item, int column)
  //{
  //  CFoundNodeWidgetItem *pItem = (CFoundNodeWidgetItem *)item;
  //  if (nullptr == pItem) {
  //    return;
  //  }

  // if ((pItem->type() == TREE_LIST_FOUND_NODE_TYPE) && pItem->m_bMdf && !pItem->m_bStdRegs) {

  //   // Set the HTML
  //   std::string html = vscp_getDeviceInfoHtml(pItem->m_mdf, pItem->m_stdregs);
  //   ui->infoArea->setHtml(html.c_str());
  // }
  // else {
  //   ui->infoArea->setText(tr("MDF info should be loaded before device info can be viewed"));
  // }
} // loadMdf

///////////////////////////////////////////////////////////////////////////////
// onItemClicked
//

void
CFrmMdf::onItemClicked(QTreeWidgetItem* item, int column)
{
  QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)item;
  if (nullptr == pItem) {
    return;
  }

  QMessageBox msgBox;
  msgBox.setText("Item has been Clicked.");
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Save);
  // int ret = msgBox.exec();
  if (nullptr != pItem->getObject()) {
    // ui->statusbar->showMessage(/*pItem->text(0)*/pItem->getObject()->getObjectTypeString().c_str(), 5000);
    ui->statusbar->showMessage("object", 1000);
  }
}

///////////////////////////////////////////////////////////////////////////////
// onItemDoubleClicked
//

void
CFrmMdf::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  QString str;
  QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)item;
  if (nullptr == pItem) {
    return;
  }

  // QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  switch (pItem->getObjectType()) {
    case mdf_type_unknown:
      str = tr("Unknown");
      break;

    case mdf_type_redirection:
      str = tr("redirection");
      break;

    case mdf_type_mdf:
      str = tr("MDF");
      break;

    case mdf_type_mdf_item:
      str = tr("MDF item");
      break;

    case mdf_type_value:
      str = tr("Value");
      break;

    case mdf_type_bit:
      str = tr("Bit");
      break;

    case mdf_type_bit_item:
      str = tr("Bit item");
      break;

    case mdf_type_register_page:
      str = tr("Register Page");
      break;

    case mdf_type_register:
      str = tr("Register");
      break;

    case mdf_type_register_item:
      str = tr("Register item");
      break;

    case mdf_type_remotevar:
      str = tr("Remote Variable");
      break;

    case mdf_type_remotevar_item:
      str = tr("Remote Variable Item");
      break;

    case mdf_type_action_param:
      str = tr("Action Param");
      break;

    case mdf_type_action:
      str = tr("Action");
      break;

    case mdf_type_action_item:
      str = tr("Action item");
      break;

    case mdf_type_decision_matrix:
      str = tr("DM");
      break;

    case mdf_type_event_data:
      str = tr("Event Data");
      break;

    case mdf_type_event_data_item:
      str = tr("Event Data item");
      break;

    case mdf_type_event:
      str = tr("Event");
      break;

    case mdf_type_event_item:
      str = tr("Event item");
      break;

    case mdf_type_value_item:
      str = tr("Value Item");
      break;

    case mdf_type_bootloader:
      str = tr("Bootloader");
      break;

    case mdf_type_alarm:
      str = tr("Alarm");
      break;

    case mdf_type_address:
      str = tr("Address");
      break;

    case mdf_type_manufacturer:
      str = tr("Manufacturer");
      break;

    case mdf_type_file:
      str = tr("File");
      break;

    case mdf_type_picture:
      str = tr("Picture");
      break;

    case mdf_type_picture_item:
      str = tr("Picture item");
      break;

    case mdf_type_video:
      str = tr("Video");
      break;

    case mdf_type_video_item:
      str = tr("Video item");
      break;

    case mdf_type_firmware:
      str = tr("Firmware");
      break;

    case mdf_type_firmware_item:
      str = tr("Firmware item");
      break;

    case mdf_type_driver:
      str = tr("Driver");
      break;

    case mdf_type_driver_item:
      str = tr("Driver");
      break;

    case mdf_type_setup:
      str = tr("Setup");
      break;

    case mdf_type_setup_item:
      str = tr("Setup");
      break;

    case mdf_type_manual:
      str = tr("Manual");
      break;

    case mdf_type_manual_item:
      str = tr("Manual item");
      break;

    case mdf_type_email:
      str = tr("email");
      break;

    case mdf_type_phone:
      str = tr("phone");
      break;

    case mdf_type_fax:
      str = tr("fax");
      break;

    case mdf_type_web:
      str = tr("web");
      break;

    case mdf_type_social:
      str = tr("social");
      break;

    case mdf_type_generic_string:
      str = tr("string");
      break;

    case mdf_type_generic_number:
      str = tr("number");
      break;

    case mdf_type_generic_url:
      str = tr("url");
      break;

    case mdf_type_generic_date:
      str = tr("date");
      break;

    case mdf_type_generic_access:
      str = tr("access");
      break;

    case mdf_type_generic_description:
      str = tr("description");
      break;

    case mdf_type_generic_description_item:
      str = tr("description item");
      break;

    case mdf_type_generic_help_url:
      str = tr("help url");
      break;

    case mdf_type_generic_help_url_item:
      str = tr("help url item");
      break;

    default:
      str = tr("Default");
      break;
  }

  // QMessageBox msgBox;
  // msgBox.setText(str);
  // msgBox.exec();
  ui->statusbar->showMessage(str, 1000);
}

///////////////////////////////////////////////////////////////////////////////
// findMdfWidgetItem
//

QMdfTreeWidgetItem*
CFrmMdf::findMdfWidgetItem(QMdfTreeWidgetItem* pItem, mdf_record_type type)
{
  pItem->setExpanded(true);
  QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
  while (nullptr != piter) {

    if (type == piter->getObjectType()) {
      return piter;
    }

    piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(piter);
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////////
// findDocumentType
//

int
CFrmMdf::deleteMdfWidgetChildItems(QMdfTreeWidgetItem* pItem, mdf_record_type type)
{
  bool bExpanded;
  int cnt = 0;

  // Check pointer
  if (nullptr == pItem) {
    return 0;
  }

  // Save expansion state
  bExpanded = pItem->isExpanded();

  // Expand to make traversion possible
  pItem->setExpanded(true);

  QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
  while (type == piter->getObjectType()) {
    pItem->removeChild(piter);
    delete piter;
    cnt++;
    piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
  }

  // Restore expansion
  pItem->setExpanded(bExpanded);

  return cnt;
}

///////////////////////////////////////////////////////////////////////////////
// editData
//

void
CFrmMdf::editData()
{
  QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  switch (pItem->getObjectType() /*pItem->type() - QTreeWidgetItem::UserType*/) {
    case mdf_type_unknown:

      break;

    case mdf_type_redirection:

      break;

    case mdf_type_mdf:
      editModuleData();
      break;

    case mdf_type_mdf_item:
      editModuleData();
      break;

    case mdf_type_value:
      break;

    case mdf_type_bit:
      break;

    case mdf_type_bit_item:
      break;

    case mdf_type_register_page:
      break;

    case mdf_type_register:
      break;

    case mdf_type_register_item:
      break;

    case mdf_type_remotevar:
      break;

    case mdf_type_remotevar_item:
      break;

    case mdf_type_action_param:
      break;

    case mdf_type_action:
      break;

    case mdf_type_action_item:
      break;

    case mdf_type_decision_matrix:
      break;

    case mdf_type_event_data:
      break;

    case mdf_type_event_data_item:
      break;

    case mdf_type_event:
      break;

    case mdf_type_event_item:
      break;

    case mdf_type_value_item:
      break;

    case mdf_type_bootloader:
      break;

    case mdf_type_alarm:
      break;

    case mdf_type_address:
      break;

    case mdf_type_manufacturer:
      editManufacturerData();
      break;

    case mdf_type_manufacturer_item:
      editManufacturerData();
      break;

    case mdf_type_email:
      editContact();
      break;

    case mdf_type_email_item:
      editContact();
      break;

    case mdf_type_phone:
      editContact();
      break;

    case mdf_type_phone_item:
      editContact();
      break;

    case mdf_type_fax:
      editContact();
      break;

    case mdf_type_fax_item:
      editContact();
      break;

    case mdf_type_web:
      editContact();
      break;

    case mdf_type_web_item:
      editContact();
      break;

    case mdf_type_social:
      editContact();
      break;

    case mdf_type_social_item:
      editContact();
      break;

    case mdf_type_file:
      break;

    case mdf_type_picture:
      break;

    case mdf_type_picture_item:
      break;

    case mdf_type_video:
      break;

    case mdf_type_video_item:
      break;

    case mdf_type_firmware:
      break;

    case mdf_type_firmware_item:
      break;

    case mdf_type_driver:
      break;

    case mdf_type_driver_item:
      break;

    case mdf_type_setup:
      break;

    case mdf_type_setup_item:
      break;

    case mdf_type_manual:
      break;

    case mdf_type_manual_item:
      break;

    case mdf_type_generic_string:
      break;

    case mdf_type_generic_number:
      break;

    case mdf_type_generic_url:
      break;

    case mdf_type_generic_date:
      break;

    case mdf_type_generic_access:
      break;

    case mdf_type_generic_description:
      break;

    case mdf_type_generic_description_item:
      break;

    case mdf_type_generic_help_url:
      break;

    case mdf_type_generic_help_url_item:
      break;

    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editModuleData
//

void
CFrmMdf::editModuleData()
{
  QMdfTreeWidgetItem* pItemModule      = nullptr; // Pointer to module top item
  QMdfTreeWidgetItem* pItemDescription = nullptr; // Pointer to description top item
  QMdfTreeWidgetItem* pItemItemInfoURL = nullptr; // Pointer to info URL top item
  QMdfTreeWidgetItem* pItem            = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("No MDF module item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Internal error: Invalid module object"));
    spdlog::error("MDF module edit - object has nullptr");
    return;
  }

  // Must be correct object type
  if ((mdf_type_mdf != pItem->getObjectType()) && (mdf_type_mdf_item != pItem->getObjectType())) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("This is not a module item"));
    spdlog::error("MDF module edit - Not a module item");
    return;
  }

  // Set pointer to module top item
  if (mdf_type_mdf == pItem->getObjectType()) {

    pItemModule = pItem;

    // Make sure it is expanded
    pItem->setExpanded(true);
  }
  else if (mdf_type_mdf_item == pItem->getObjectType()) {
    pItemModule = (QMdfTreeWidgetItem*)pItem->parent();
  }
  else {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Invalid module top item object type"));
    spdlog::error("MDF module edit - Invalid module top item object type");
    return;
  }

  //     piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItemModule);
  //     do {
  //       if (nullptr == piter)
  //         return nullptr;
  //   if( (mdf_type_generic_description == piter->getObjectType()) {
  //         return piter;
  //   }
  //   piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(piter);
  // while (mdf_type_mdf_item == piter->getObjectType());

  pItemDescription = findMdfWidgetItem(pItemModule, mdf_type_generic_description); // findDocumentItem(pItemModule);
  pItemItemInfoURL = findMdfWidgetItem(pItemModule, mdf_type_generic_help_url);    // findInfoUrlItem(pItemModule);

  CDlgMdfModule dlg(this);
  dlg.initDialogData(pItem->getObject(), static_cast<mdf_module_index>(pItem->getElementIndex()));

  if (QDialog::Accepted == dlg.exec()) {
    // Update Module items

    QMdfTreeWidgetItem* piter = nullptr;
    // if (nullptr != pItem->parent()) {

    //   // Module sub item selected
    //   piter = (QMdfTreeWidgetItem*)pItem->parent();

    //   // QMdfTreeWidgetItem* pitemBelow = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem->parent());
    //   // if (nullptr != pitemBelow) {
    //   //   ui->statusbar->showMessage(pitemBelow->text(0), 3000);
    //   //   piter = (QMdfTreeWidgetItem*)pItem->parent();
    //   // }
    // }
    // else {

    //   // Module top item selected
    //   // ui->statusbar->showMessage(pItem->text(0), 3000);
    //   piter = (QMdfTreeWidgetItem*)pItem;

    //   // QMdfTreeWidgetItem* pitemBelow = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
    //   // if (nullptr != pitemBelow) {
    //   //   ui->statusbar->showMessage(pitemBelow->text(0), 3000);
    //   //   piter = (QMdfTreeWidgetItem*)pItem->parent();
    //   // }
    // }

    // Protect against a null pointer
    // if (nullptr == piter) {
    //   int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Invalid NULL module item"));
    //   spdlog::error("MDF module edit - module item has nullptr");
    //   return;
    // }

    // Iterate over module items and update ui
    piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItemModule);
    do {

      switch (piter->getElementIndex()) {

        case index_module_name:
          piter->setText(0, PREFIX_MDF_MODULE_NAME + m_mdf.getModuleName().c_str());
          break;

        case index_module_model:
          piter->setText(0, PREFIX_MDF_MODULE_MODEL + m_mdf.getModuleModel().c_str());
          break;

        case index_module_level: {
          QString str;
          str = PREFIX_MDF_MODULE_LEVEL + str.arg(m_mdf.getModuleLevel() + 1);
          piter->setText(0, str);
        } break;

        case index_module_version:
          piter->setText(0, PREFIX_MDF_MODULE_VERSION + m_mdf.getModuleVersion().c_str());
          break;

        case index_module_change_date:
          piter->setText(0, PREFIX_MDF_MODULE_CHANGE_DATE + m_mdf.getModuleChangeDate().c_str());
          break;

        case index_module_buffer_size: {
          QString str;
          str = PREFIX_MDF_MODULE_BUFFER_SIZE + str.arg(m_mdf.getModuleBufferSize());
          piter->setText(0, str);
        } break;

        case index_module_copyright: // Copyright
          piter->setText(0, PREFIX_MDF_MODULE_COPYRIGHT + m_mdf.getModuleCopyright().c_str());
          break;

        default: // Hmmmm.-.. unkown field index
          break;
      }

      piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(piter);
    } while (mdf_type_mdf_item == piter->getObjectType());

    // Find module descriptions
    // int n = piter->childCount();
    // if (mdf_type_generic_description == piter->getObjectType()) {
    //   piter->setExpanded(false);
    //   n = ui->treeMDF->indexOfTopLevelItem(piter);
    //   n = ui->treeMDF->topLevelItemCount();
    //   n = n + 1;
    // }

    deleteMdfWidgetChildItems(pItemDescription, mdf_type_generic_description_item);
    fillDescriptionItems(pItemDescription, &m_mdf, m_mdf.getMapDescription(), true);

    deleteMdfWidgetChildItems(pItemItemInfoURL, mdf_type_generic_help_url_item);
    fillInfoUrlItems(pItemItemInfoURL, &m_mdf, m_mdf.getHelpUrlMap(), true);

  } // accept
}

///////////////////////////////////////////////////////////////////////////////
// editDescription
//

void
CFrmMdf::editDescription(void)
{
  int index                                = 0;
  QString selstr                           = "";
  QMdfTreeWidgetItem* pItemDescription     = nullptr; // Pointer to description top item
  QMdfTreeWidgetItem* pItem                = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();
  std::map<std::string, std::string>* pmap = nullptr;

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("No MDF description item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    QMessageBox::critical(this, tr("MDF module edit"), tr("Internal error: Invalid module object"));
    spdlog::error("MDF module edit - object has nullptr");
    return;
  }

  // Set pointer to module top item
  if (mdf_type_generic_description == pItem->getObjectType()) {

    pItemDescription = pItem;

    // Make sure it is expanded
    pItemDescription->setExpanded(true);
  }
  else if (mdf_type_generic_description_item == pItem->getObjectType()) {
    pItemDescription = (QMdfTreeWidgetItem*)pItem->parent();
    selstr           = pItem->text(0).split('_').first().left(2);
  }
  else {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Invalid module top item object type"));
    spdlog::error("MDF module edit - Invalid module top item object type");
    return;
  }

  QMdfTreeWidgetItem* pParentToItemDescription = (QMdfTreeWidgetItem*)pItemDescription->parent();
  if (nullptr == pParentToItemDescription) {
    return;
  }

  // Get correct pointer to description map
  switch (pParentToItemDescription->getObjectType()) {

    case mdf_type_mdf:
      pmap = m_mdf.getMapDescription();
      break;

    case mdf_type_manufacturer:
      pmap = m_mdf.getManufacturer()->getMapDescription();
      break;

    case mdf_type_phone_item:
    case mdf_type_fax_item:
    case mdf_type_email_item:
    case mdf_type_web_item:
    case mdf_type_social_item: {

      qDebug() << pParentToItemDescription->getObjectType() << pItem->getObjectType() << pItem->text(0) << pParentToItemDescription->text(0) << pParentToItemDescription->parent()->text(0);

      if (mdf_type_generic_description == pItem->getObjectType()) {
        pmap = pItem->getObject()->getMapDescription();
      }
      else if (mdf_type_generic_description_item == pItem->getObjectType()) {
        pmap = pItem->getObject()->getMapDescription();
      }
      else {
        QMessageBox::critical(this, tr("Cintact description edit"), tr("Internal error: Invalid object type"));
        spdlog::error("MDF contact description edit - objttype is not mdf_type_generic_description or mdf_type_generic_description_item");
      }
    } break;
  }

  if (mdf_type_generic_description == pItem->getObjectType()) {
    CDlgEditMap dlg(this);
    dlg.setWindowTitle(tr("Description"));
    dlg.initDialogData(pmap, map_type_description, &selstr);

    if (QDialog::Accepted == dlg.exec()) {
      // Update Module items
      deleteMdfWidgetChildItems(pItemDescription, mdf_type_generic_description_item);
      fillDescriptionItems(pItemDescription, pItem->getObject(), pmap, true);

      QMdfTreeWidgetItem* piter = nullptr;
    }
  }
  else { // item
    CDlgMdfDescription dlg(this);
    dlg.setWindowTitle(tr("MDF Description"));
    selstr = pItem->text(0).split('_').first().left(2);
    dlg.initDialogData(pmap, &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      // Update Module items
      deleteMdfWidgetChildItems(pItemDescription, mdf_type_generic_description_item);
      fillDescriptionItems(pItemDescription, pItem->getObject(), pmap, true);

      // QMdfTreeWidgetItem* piter = nullptr;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editInfoUrl
//

void
CFrmMdf::editInfoUrl(void)
{
  int index                                = 0;
  QString selstr                           = "";
  QMdfTreeWidgetItem* pItemInfoUrl         = nullptr; // Pointer to info url top item
  QMdfTreeWidgetItem* pItem                = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();
  std::map<std::string, std::string>* pmap = nullptr;

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("No MDF info url item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Internal error: Invalid module object"));
    spdlog::error("MDF module edit - object has nullptr");
    return;
  }

  // Set pointer to module top item
  if (mdf_type_generic_help_url == pItem->getObjectType()) {

    pItemInfoUrl = pItem;

    // Make sure it is expanded
    pItemInfoUrl->setExpanded(true);
  }
  else if (mdf_type_generic_help_url_item == pItem->getObjectType()) {
    pItemInfoUrl = (QMdfTreeWidgetItem*)pItem->parent();
    selstr       = pItem->text(0).split('_').first().left(2);
  }
  else {
    int ret = QMessageBox::critical(this, tr("MDF module edit"), tr("Invalid module top item object type"));
    spdlog::error("MDF module edit - Invalid module top item object type");
    return;
  }

  QMdfTreeWidgetItem* pParentToItemInfoUrl = (QMdfTreeWidgetItem*)pItemInfoUrl->parent();
  if (nullptr == pParentToItemInfoUrl) {
    return;
  }

  // Get correct pointer to description map
  switch (pParentToItemInfoUrl->getObjectType()) {

    case mdf_type_mdf:
      pmap = m_mdf.getMapInfoUrl();
      break;

    case mdf_type_manufacturer:
      pmap = m_mdf.getManufacturer()->getMapInfoUrl();
      break;

    case mdf_type_phone_item:
    case mdf_type_fax_item:
    case mdf_type_email_item:
    case mdf_type_web_item:
    case mdf_type_social_item: {

      qDebug() << pParentToItemInfoUrl->getObjectType() << pItem->getObjectType() << pItem->text(0) << pParentToItemInfoUrl->text(0) << pParentToItemInfoUrl->parent()->text(0);

      if (mdf_type_generic_help_url == pItem->getObjectType()) {
        pmap = pItem->getObject()->getMapInfoUrl();
      }
      else if (mdf_type_generic_help_url_item == pItem->getObjectType()) {
        pmap = pItem->getObject()->getMapInfoUrl();
      }
      else {
        QMessageBox::critical(this, tr("Cintact description edit"), tr("Internal error: Invalid object type"));
        spdlog::error("MDF contact description edit - objttype is not mdf_type_generic_description or mdf_type_generic_description_item");
      }
    } break;
  }

  if (mdf_type_generic_help_url == pItem->getObjectType()) {
    CDlgEditMap dlg(this);
    dlg.setWindowTitle(tr("Info URL"));
    dlg.initDialogData(pmap, map_type_info_url, &selstr);

    if (QDialog::Accepted == dlg.exec()) {
      // Update Module items
      deleteMdfWidgetChildItems(pItemInfoUrl, mdf_type_generic_help_url_item);
      fillInfoUrlItems(pItemInfoUrl, pItem->getObject(), pmap, true);

      QMdfTreeWidgetItem* piter = nullptr;
    }
  }
  else { // item
    CDlgMdfDescription dlg(this);
    dlg.setWindowTitle(tr("MDF Info URL"));
    selstr = pItem->text(0).split('_').first().left(2);
    dlg.initDialogData(pmap, &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      // Update Module items
      deleteMdfWidgetChildItems(pItemInfoUrl, mdf_type_generic_help_url_item);
      fillDescriptionItems(pItemInfoUrl, pItem->getObject(), pmap, true);

      // QMdfTreeWidgetItem* piter = nullptr;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editManufacturerData
//

void
CFrmMdf::editManufacturerData()
{
  QMdfTreeWidgetItem* pItemManufacturer = nullptr; // Pointer to module top item
  QMdfTreeWidgetItem* pItemEmail        = nullptr; // Pointer to description top item
  QMdfTreeWidgetItem* pItemWeb          = nullptr; // Pointer to info URL top item
  QMdfTreeWidgetItem* pItem             = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF manufacturer edit"), tr("No MDF manufacturer item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF manufacturer edit"), tr("Internal error: Invalid manufacturer object"));
    spdlog::error("MDF manufacturer edit - object has nullptr");
    return;
  }

  // Must be correct object type
  if ((mdf_type_manufacturer != pItem->getObjectType()) && (mdf_type_manufacturer_item != pItem->getObjectType())) {
    int ret = QMessageBox::critical(this, tr("MDF manufacturer edit"), tr("This is not a manufacturer item"));
    spdlog::error("MDF manufacturer edit - Not a manufacturer item");
    return;
  }

  // Set pointer to module top item
  if (mdf_type_manufacturer == pItem->getObjectType()) {

    pItemManufacturer = pItem;

    // Make sure it is expanded
    pItem->setExpanded(true);
  }
  else if (mdf_type_manufacturer_item == pItem->getObjectType()) {
    pItemManufacturer = (QMdfTreeWidgetItem*)pItem->parent();
  }
  else {
    int ret = QMessageBox::critical(this, tr("MDF manufacturer edit"), tr("Invalid manufacturer top item object type"));
    spdlog::error("MDF manufacturer edit - Invalid manufacturer top item object type");
    return;
  }

  CDlgMdfManufacturer dlg(this);
  dlg.initDialogData(pItem->getObject(), static_cast<mdf_manufacturer_index>(pItem->getElementIndex()));

  if (QDialog::Accepted == dlg.exec()) {

    // Update Module items

    CMDF_Manufacturer* pmanufacturer = (CMDF_Manufacturer*)pItem->getObject();
    pmanufacturer->setName(dlg.getName().toStdString());
    pmanufacturer->getAddressObj()->setStreet(dlg.getStreet().toStdString());
    pmanufacturer->getAddressObj()->setTown(dlg.getTown().toStdString());
    pmanufacturer->getAddressObj()->setCity(dlg.getCity().toStdString());
    pmanufacturer->getAddressObj()->setPostCode(dlg.getPostCode().toStdString());
    pmanufacturer->getAddressObj()->setState(dlg.getState().toStdString());
    pmanufacturer->getAddressObj()->setRegion(dlg.getRegion().toStdString());
    pmanufacturer->getAddressObj()->setCountry(dlg.getCountry().toStdString());

    QMdfTreeWidgetItem* piter = nullptr;
    piter                     = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItemManufacturer);
    do {

      switch (piter->getElementIndex()) {

        case index_manufacturer_name:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_NAME + pmanufacturer->getName().c_str());
          break;

        case index_manufacturer_street:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_STREET + pmanufacturer->getAddressObj()->getStreet().c_str());
          break;

        case index_manufacturer_town:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_TOWN + pmanufacturer->getAddressObj()->getTown().c_str());
          break;

        case index_manufacturer_city:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_CITY + pmanufacturer->getAddressObj()->getCity().c_str());
          break;

        case index_manufacturer_post_code:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_POST_CODE + pmanufacturer->getAddressObj()->getPostCode().c_str());
          break;

        case index_manufacturer_state:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_STATE + pmanufacturer->getAddressObj()->getState().c_str());
          break;

        case index_manufacturer_region:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_REGION + pmanufacturer->getAddressObj()->getRegion().c_str());
          break;

        case index_manufacturer_country:
          piter->setText(0, PREFIX_MDF_MANUFACTURER_COUNTRY + pmanufacturer->getAddressObj()->getCountry().c_str());
          break;
      }

      piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(piter);

    } while (mdf_type_manufacturer_item == piter->getObjectType());
  }
}

///////////////////////////////////////////////////////////////////////////////
// showContactDialog
//

void
CFrmMdf::showContactDialog(QMdfTreeWidgetItem* pItem, mdf_dlg_contact_type type, QString title)
{
  CMDF_Item* pContactObj = (CMDF_Item*)pItem->getObject();
  if (nullptr == pContactObj) {
    int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
    return;
  }

  CDlgMdfContact dlg(this);
  dlg.setWindowTitle(title);
  dlg.initDialogData(pContactObj, type, title);
  if (QDialog::Accepted == dlg.exec()) {

    pItem->setText(0, dlg.getValue());
    QMdfTreeWidgetItem* pItemDescription = findMdfWidgetItem(pItem, mdf_type_generic_description); // findDocumentItem(pItemModule);
    QMdfTreeWidgetItem* pItemItemInfoURL = findMdfWidgetItem(pItem, mdf_type_generic_help_url);    // findInfoUrlItem(pItemModule);

    deleteMdfWidgetChildItems(pItemDescription, mdf_type_generic_description_item);
    fillDescriptionItems(pItemDescription,
                         (CMDF_Item*)pItem->getObject(),
                         ((CMDF_Item*)pItem->getObject())->getMapDescription(),
                         true);
    fillInfoUrlItems(pItemItemInfoURL,
                     (CMDF_Item*)pItem->getObject(),
                     ((CMDF_Item*)pItem->getObject())->getMapInfoUrl(),
                     true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editContact
//

void
CFrmMdf::editContact(void)
{
  QMdfTreeWidgetItem* pItem = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No MDF contact item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("Internal error: Invalid contact object"));
    spdlog::error("MDF contact edit - object has nullptr");
    return;
  }

  switch (pItem->getObjectType()) {

    case mdf_type_phone: {
      CMDF_Manufacturer* pManufacturer = (CMDF_Manufacturer*)pItem->getObject();
      if (nullptr == pManufacturer) {
        int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
        return;
      }

      CDlgMdfContactList dlg(this);
      dlg.initDialogData(pManufacturer, dlg_type_contact_phone, "Edit phone contact items");
      if (QDialog::Accepted == dlg.exec()) {

        // Expand to make traversion possible
        pItem->setExpanded(true);

        QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        while (mdf_type_phone_item == piter->getObjectType()) {
          pItem->removeChild(piter);
          delete piter;
          piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        }

        renderManufacturerPhone(pItem);
      }
    } break;

    case mdf_type_phone_item:
      showContactDialog(pItem, dlg_type_contact_phone, "Edit phone contacts");
      break;

    case mdf_type_fax: {
      CMDF_Manufacturer* pManufacturer = (CMDF_Manufacturer*)pItem->getObject();
      if (nullptr == pManufacturer) {
        int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
        return;
      }

      CDlgMdfContactList dlg(this);
      dlg.initDialogData(pManufacturer, dlg_type_contact_fax, "Edit fax contact items");
      if (QDialog::Accepted == dlg.exec()) {

        // Expand to make traversion possible
        pItem->setExpanded(true);

        QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        while (mdf_type_fax_item == piter->getObjectType()) {
          pItem->removeChild(piter);
          delete piter;
          piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        }

        renderManufacturerFax(pItem);
      }
    } break;

    case mdf_type_fax_item:
      showContactDialog(pItem, dlg_type_contact_fax, "Edit fax contacts");
      break;

    case mdf_type_email: {
      CMDF_Manufacturer* pManufacturer = (CMDF_Manufacturer*)pItem->getObject();
      if (nullptr == pManufacturer) {
        int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
        return;
      }

      CDlgMdfContactList dlg(this);
      dlg.initDialogData(pManufacturer, dlg_type_contact_email, "Edit email contact items");
      if (QDialog::Accepted == dlg.exec()) {

        // Expand to make traversion possible
        pItem->setExpanded(true);

        QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        while (mdf_type_email_item == piter->getObjectType()) {
          pItem->removeChild(piter);
          delete piter;
          piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        }

        renderManufacturerEmail(pItem);
      }
    } break;

    case mdf_type_email_item:
      showContactDialog(pItem, dlg_type_contact_email, "Edit email contacts");
      break;

    case mdf_type_web: {
      CMDF_Manufacturer* pManufacturer = (CMDF_Manufacturer*)pItem->getObject();
      if (nullptr == pManufacturer) {
        int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
        return;
      }

      CDlgMdfContactList dlg(this);
      dlg.initDialogData(pManufacturer, dlg_type_contact_web, "Edit web contact items");
      if (QDialog::Accepted == dlg.exec()) {

        // Expand to make traversion possible
        pItem->setExpanded(true);

        QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        while (mdf_type_web_item == piter->getObjectType()) {
          pItem->removeChild(piter);
          delete piter;
          piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        }

        renderManufacturerWeb(pItem);
      }
    } break;

    case mdf_type_web_item:
      showContactDialog(pItem, dlg_type_contact_web, "Edit fax contacts");
      break;

    case mdf_type_social: {
      CMDF_Manufacturer* pManufacturer = (CMDF_Manufacturer*)pItem->getObject();
      if (nullptr == pManufacturer) {
        int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("No contact item object"));
        return;
      }

      CDlgMdfContactList dlg(this);
      dlg.initDialogData(pManufacturer, dlg_type_contact_social, "Edit social contact items");
      if (QDialog::Accepted == dlg.exec()) {

        // Expand to make traversion possible
        pItem->setExpanded(true);

        QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        while (mdf_type_social_item == piter->getObjectType()) {
          pItem->removeChild(piter);
          delete piter;
          piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pItem);
        }

        renderManufacturerSocial(pItem);
      }
    } break;

    case mdf_type_social_item:
      showContactDialog(pItem, dlg_type_contact_social, "Edit social contacts");
      break;

    default:
      int ret = QMessageBox::critical(this, tr("MDF contact edit"), tr("This is not a contact item"));
      spdlog::error("MDF contact edit - Not a contact item");
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editBootLoader
//

void
CFrmMdf::editBootLoader(void)
{
  QMdfTreeWidgetItem* pItem         = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();
  QMdfTreeWidgetItem* pBootHeadItem = nullptr;
  uint16_t selectedIndex            = pItem->getElementIndex();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("No MDF bootloader item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("Internal error: Invalid bootloader object"));
    spdlog::error("MDF bootloader edit - object has nullptr");
    return;
  }

  CMDF_BootLoaderInfo* pBootInfo = (CMDF_BootLoaderInfo*)pItem->getObject();
  if (nullptr == pBootInfo) {
    int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("No bootloader item object"));
    return;
  }

  CDlgMdfBootLoader dlg(this);

  switch (pItem->getObjectType()) {

    case mdf_type_bootloader:
      pBootHeadItem = pItem;
      pBootHeadItem->setExpanded(true);
      break;

    case mdf_type_bootloader_item: {
      pBootHeadItem = (QMdfTreeWidgetItem*)pItem->parent();
      break;
    }

    default:
      int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("This is not a bootloader item"));
      spdlog::error("MDF bootloader edit - Not a bootloader item");
      break;
  }

  dlg.initDialogData(pBootInfo, static_cast<mdf_bootloader_index>(selectedIndex));
  if (QDialog::Accepted == dlg.exec()) {

    QMdfTreeWidgetItem* piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pBootHeadItem);
    while (mdf_type_bootloader_item == piter->getObjectType()) {
      pItem->removeChild(piter);
      delete piter;
      piter = (QMdfTreeWidgetItem*)ui->treeMDF->itemBelow(pBootHeadItem);
    }

    renderBootLoadInfo(pBootHeadItem, selectedIndex);
  }
}


///////////////////////////////////////////////////////////////////////////////
// editFileList
//

void
CFrmMdf::editFileList(void)
{
  QMdfTreeWidgetItem* pItem         = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();
  QMdfTreeWidgetItem* pFileHeadItem = nullptr;
  uint16_t selectedIndex            = pItem->getElementIndex();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF file list edit"), tr("No MDF file list item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF file list edit"), tr("Internal error: Invalid file list object"));
    spdlog::error("MDF bootloader edit - object has nullptr");
    return;
  }

  CDlgMdfFile dlg(this);
  dlg.initDialogData(pItem->getObject(), pItem->getObjectType());
  if (QDialog::Accepted == dlg.exec()) {
  
  }
}


///////////////////////////////////////////////////////////////////////////////
// editFile
//

void
CFrmMdf::editFile(void)
{
  QMdfTreeWidgetItem* pItem         = (QMdfTreeWidgetItem*)ui->treeMDF->currentItem();
  QMdfTreeWidgetItem* pBootHeadItem = nullptr;
  uint16_t selectedIndex            = pItem->getElementIndex();

  // Item must be selected
  if (nullptr == pItem) {
    int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("No MDF bootloader item selected"));
    return;
  }

  // Must have an object
  if (nullptr == pItem->getObject()) {
    int ret = QMessageBox::critical(this, tr("MDF bootloader edit"), tr("Internal error: Invalid bootloader object"));
    spdlog::error("MDF bootloader edit - object has nullptr");
    return;
  }
}