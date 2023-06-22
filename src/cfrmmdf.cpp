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

///////////////////////////////////////////////////////////////////////////////
// CFrmMdf
//

CFrmMdf::CFrmMdf(QWidget* parent, const char* path)
  : QMainWindow(parent)
  , ui(new Ui::CFrmMdf)
{
  QString str;

  ui->setupUi(this);

  ui->treeMDF->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeMDF->setEditTriggers(QAbstractItemView::NoEditTriggers);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(std::string(tr("Node configuration module opended").toStdString()));

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

  // New has been selected in the menu
  connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newMdf()));

  // Open has been selected in the menu
  connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openMdf()));

  // Register row has been clicked.
  // connect(ui->treeMDF,
  //           &QTreeWidget::itemClicked,
  //           this,
  //           &CFrmMdf::onFindNodesTreeWidgetItemClicked);
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
  menu->addAction(QString(tr("Fetch MDF")), this, SLOT(loadSelectedMdf()));
  menu->addAction(QString(tr("Fetch ALL MDF")), this, SLOT(loadAllMdf()));
  menu->addSeparator();
  menu->addAction(QString(tr("Rescan")), this, SLOT(doScan()));
  menu->addSeparator();
  menu->addAction(QString(tr("Configure")), this, SLOT(goConfig()));
  menu->addAction(QString(tr("Session")), this, SLOT(goSession()));
  menu->popup(ui->treeMDF->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// fillDescriptionItems
//

void
CFrmMdf::fillDescriptionItems(QTreeWidgetItem* pParent, std::map<std::string, std::string>* pObjMap)
{
  QString str;
  QTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent)
    return;
  if (nullptr == pObjMap)
    return;

  // Must be items
  if (!pObjMap->size())
    return;

  QTreeWidgetItem* pItemModuleDescription = new QTreeWidgetItem(pParent, 0);
  pItemModuleDescription->setText(0, "Description");
  pParent->addChild(pItemModuleDescription);

  std::map<std::string, std::string>* pmap = pObjMap;
  for (auto const& x : *pmap) {
    str   = x.first.c_str() + QString(": ") + x.second.c_str();
    pItem = new QTreeWidgetItem(pItemModuleDescription, 0);
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillHelpUrlItems
//

void
CFrmMdf::fillHelpUrlItems(QTreeWidgetItem* pParent, std::map<std::string, std::string>* pObjMap)
{
  QString str;
  QTreeWidgetItem* pItem;

  // Must be valid pointer
  if (nullptr == pParent)
    return;
  if (nullptr == pObjMap)
    return;

  // Must be items
  if (!pObjMap->size())
    return;

  QTreeWidgetItem* pItemModuleHelpUrl = new QTreeWidgetItem(pParent, 0);
  pItemModuleHelpUrl->setText(0, "Help/Info URL");
  pParent->addChild(pItemModuleHelpUrl);

  std::map<std::string, std::string>* pmap = pObjMap;
  for (auto const& x : *pmap) {
    str   = x.first.c_str() + QString(": ") + x.second.c_str();
    pItem = new QTreeWidgetItem(pItemModuleHelpUrl, 0);
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillRegisterInfo
//

void
CFrmMdf::fillRegisterInfo(QTreeWidgetItem* pParent, CMDF_Register *preg)
{
  QString str;
  QTreeWidgetItem *pItem;

  // Check pointers
  if (nullptr == pParent) return;
  if (nullptr == preg) return;

  pItem = new QTreeWidgetItem(pParent, 1);
  if (nullptr != pItem) {
    str = QString("Name:  %1").arg(preg->getName().c_str());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QTreeWidgetItem(pParent, 1);
  if (nullptr != pItem) {
    str = QString("Offset:  %1").arg(preg->getOffset());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }

  pItem = new QTreeWidgetItem(pParent, 1);
  if (nullptr != pItem) {
    str = QString("Page:  %1").arg(preg->getPage());
    pItem->setText(0, str);
    pParent->addChild(pItem);
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadMdf
//

void
CFrmMdf::loadMdf(void)
{
  QString str;
  size_t index = 0;
  QTreeWidgetItem* pItem;

  // Clear the treeview
  ui->treeMDF->clear();

  QBrush greenBrush(QColor("green"));
  QBrush blueBrush(QColor("blue"));

  QFont fontTopItem;
  fontTopItem.setBold(true);

  pItem = new QTreeWidgetItem(1);
  pItem->setText(0, tr("Redirection"));
  pItem->setDisabled(true);
  ui->treeMDF->addTopLevelItem(pItem);

  // Module
  QTreeWidgetItem* pItemModule = new QTreeWidgetItem(1);
  pItemModule->setFont(0, fontTopItem);
  pItemModule->setForeground(0, blueBrush);
  pItemModule->setText(0, tr("Module"));
  ui->treeMDF->addTopLevelItem(pItemModule);

  // Module information
  // std::string str = m_mdf.getModuleName().c_str();
  QTreeWidgetItem* pItemModuleInfo = new QTreeWidgetItem(pItemModule, 1);
  pItemModuleInfo->setText(0, QString("Name: ") + m_mdf.getModuleName().c_str());
  pItemModule->addChild(pItemModuleInfo);

  str   = QString("Change Date: ") + m_mdf.getModuleChangeDate().c_str();
  pItem = new QTreeWidgetItem(pItemModule, 0);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = "VSCP Level: " + QString::number(m_mdf.getModuleLevel() + 1);
  pItem = new QTreeWidgetItem(pItemModule, 0);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = "Model: " + QString(m_mdf.getModuleModel().c_str());
  pItem = new QTreeWidgetItem(pItemModule, 0);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = "Version: " + QString(m_mdf.getModuleVersion().c_str());
  pItem = new QTreeWidgetItem(pItemModule, 0);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  str   = "Buffer size: " + QString::number(m_mdf.getModuleBufferSize());
  pItem = new QTreeWidgetItem(pItemModule, 0);
  pItem->setText(0, str);
  pItemModule->addChild(pItem);

  fillDescriptionItems(pItemModule, m_mdf.getModuleDescriptionMap());
  // {
  //   QTreeWidgetItem* pItemModuleDescription = new QTreeWidgetItem(pItemModule, 0);
  //   pItemModuleDescription->setText(0, "Description");
  //   pItemModule->addChild(pItemModuleDescription);

  //   std::map<std::string, std::string>* pmap = m_mdf.getModuleDescriptionMap();
  //   for (auto const& x : *pmap) {
  //     str   = x.first.c_str() + QString(": ") + x.second.c_str();
  //     pItem = new QTreeWidgetItem(pItemModuleDescription, 0);
  //     pItem->setText(0, str);
  //     pItemModule->addChild(pItem);
  //   }
  // }

  fillHelpUrlItems(pItemModule, m_mdf.getModuleHelpUrlMap());
  // {
  //   QTreeWidgetItem* pItemModuleHelpUrl = new QTreeWidgetItem(pItemModule, 0);
  //   pItemModuleHelpUrl->setText(0, "Help/Info URL");
  //   pItemModule->addChild(pItemModuleHelpUrl);

  //   std::map<std::string, std::string>* pmap = m_mdf.getModuleHelpUrlMap();
  //   for (auto const& x : *pmap) {
  //     str   = x.first.c_str() + QString(": ") + x.second.c_str();
  //     pItem = new QTreeWidgetItem(pItemModuleHelpUrl, 0);
  //     pItem->setText(0, str);
  //     pItemModule->addChild(pItem);
  //   }
  // }

  // * * * Manufacturer info * * *

  QTreeWidgetItem* pItemManufacturer = new QTreeWidgetItem(pItemModule, 1);
  pItemManufacturer->setFont(0, fontTopItem);
  pItemManufacturer->setForeground(0, greenBrush);
  pItemManufacturer->setText(0, tr("Manufacturer"));
  ui->treeMDF->addTopLevelItem(pItemManufacturer);

  CMDF_Manufacturer* pManufacturer = m_mdf.getManufacturer();
  if (nullptr != pManufacturer) {

    str   = QString("Name: ") + m_mdf.getManufacturerName().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Street: ") + m_mdf.getManufacturerStreetAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("City: ") + m_mdf.getManufacturerCityAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Town: ") + m_mdf.getManufacturerTownAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Post Code: ") + m_mdf.getManufacturerPostCodeAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Region: ") + m_mdf.getManufacturerRegionAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("State: ") + m_mdf.getManufacturerStateAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    str   = QString("Country: ") + m_mdf.getManufacturerCountryAddress().c_str();
    pItem = new QTreeWidgetItem(pItemManufacturer, 0);
    pItem->setText(0, str);
    pItemModule->addChild(pItem);

    index = 0;
    CMDF_Item* pItemEmail;

    do {
      pItemEmail = pManufacturer->getEmailObj(index++);
      if (nullptr != pItemEmail) {
        str   = QString("Email ") + QString::number(index) + ": " + pItemEmail->getName().c_str();
        pItem = new QTreeWidgetItem(pItemManufacturer, 0);
        pItem->setText(0, str);
        pItemModule->addChild(pItem);
        fillDescriptionItems(pItem, pManufacturer->getEmailObj()->getDescriptionMap());
        fillHelpUrlItems(pItem, pManufacturer->getEmailObj()->getInfoUrlMap());
      }
    } while (nullptr != pItemEmail);

    CMDF_Item* pItemPhone;
    index = 0;

    do {
      pItemPhone = pManufacturer->getPhoneObj(index++);
      if (nullptr != pItemPhone) {
        str   = QString("Phone ") + QString::number(index) + ": " + pItemPhone->getName().c_str();
        pItem = new QTreeWidgetItem(pItemManufacturer, 0);
        pItem->setText(0, str);
        pItemModule->addChild(pItem);
        fillDescriptionItems(pItem, pManufacturer->getPhoneObj()->getDescriptionMap());
        fillHelpUrlItems(pItem, pManufacturer->getPhoneObj()->getInfoUrlMap());
      }
    } while (nullptr != pItemPhone);

    CMDF_Item* pItemFax;
    index = 0;

    do {
      pItemFax = pManufacturer->getFaxObj(index++);
      if (nullptr != pItemFax) {
        str   = QString("Fax ") + QString::number(index) + ": " + pItemFax->getName().c_str();
        pItem = new QTreeWidgetItem(pItemManufacturer, 0);
        pItem->setText(0, str);
        pItemModule->addChild(pItem);
        fillDescriptionItems(pItem, pManufacturer->getFaxObj()->getDescriptionMap());
        fillHelpUrlItems(pItem, pManufacturer->getFaxObj()->getInfoUrlMap());
      }
    } while (nullptr != pItemFax);

    CMDF_Item* pItemWeb;
    index = 0;

    do {
      pItemWeb = pManufacturer->getWebObj(index++);
      if (nullptr != pItemWeb) {
        str   = QString("Web ") + QString::number(index) + ": " + pItemWeb->getName().c_str();
        pItem = new QTreeWidgetItem(pItemManufacturer, 0);
        pItem->setText(0, str);
        pItemModule->addChild(pItem);
        fillDescriptionItems(pItem, pManufacturer->getWebObj()->getDescriptionMap());
        fillHelpUrlItems(pItem, pManufacturer->getWebObj()->getInfoUrlMap());
      }
    } while (nullptr != pItemWeb);

    CMDF_Item* pItemSocial;
    index = 0;

    do {
      pItemSocial = pManufacturer->getSocialObj(index++);
      if (nullptr != pItemSocial) {
        str   = QString("Social ") + QString::number(index) + ": " + pItemSocial->getName().c_str();
        pItem = new QTreeWidgetItem(pItemManufacturer, 0);
        pItem->setText(0, str);
        pItemModule->addChild(pItem);
        fillDescriptionItems(pItem, pManufacturer->getSocialObj()->getDescriptionMap());
        fillHelpUrlItems(pItem, pManufacturer->getSocialObj()->getInfoUrlMap());
      }
    } while (nullptr != pItemSocial);
  }
  else {
    spdlog::error("No manufacturer object when reading MDF");
  }

  // * * * Boot info * * *

  QTreeWidgetItem* pItemBoot = new QTreeWidgetItem(pItemModule, 1);
  pItemBoot->setFont(0, fontTopItem);
  pItemBoot->setForeground(0, greenBrush);
  pItemBoot->setText(0, tr("Boot"));
  ui->treeMDF->addTopLevelItem(pItemBoot);

  CMDF_BootLoaderInfo* pboot = m_mdf.getBootLoaderObj();
  if (nullptr != pboot) {
    str   = QString("Algorithm: %1").arg(pboot->getAlgorithm());
    pItem = new QTreeWidgetItem(pItemBoot, 0);
    pItem->setText(0, str);
    pItemBoot->addChild(pItem);

    str   = QString("Block Size: %1").arg(pboot->getBlockSize());
    pItem = new QTreeWidgetItem(pItemBoot, 0);
    pItem->setText(0, str);
    pItemBoot->addChild(pItem);

    str   = QString("Block Count: %1").arg(pboot->getBlockCount());
    pItem = new QTreeWidgetItem(pItemBoot, 0);
    pItem->setText(0, str);
    pItemBoot->addChild(pItem);
  }
  else {
    spdlog::error("No bootloader object when reading MDF");
  }

  // * * * Files * * *

  QTreeWidgetItem* pItemFiles = new QTreeWidgetItem(pItemModule, 1);
  pItemFiles->setFont(0, fontTopItem);
  pItemFiles->setForeground(0, greenBrush);
  pItemFiles->setText(0, tr("Files"));
  ui->treeMDF->addTopLevelItem(pItemFiles);

  // Manuals

  if (m_mdf.getManualCount()) {

    QTreeWidgetItem* pItemManual = new QTreeWidgetItem(pItemFiles, 1);
    pItemManual->setFont(0, fontTopItem);
    // pItemManual->setForeground(0, greenBrush);
    pItemManual->setText(0, tr("Manuals"));
    pItemFiles->addChild(pItemManual);

    index = 0;
    CMDF_Manual* pManualObj;

    do {
      pManualObj = m_mdf.getManualObj(index);
      if (nullptr != pManualObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemManual, 1);
        str                    = QString("Manual %1 - %2").arg(index).arg(pManualObj->getName().c_str());
        pItem->setText(0, str);
        pItemManual->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: %1").arg(pManualObj->getName().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: ") + pManualObj->getUrl().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Format: ") + pManualObj->getFormat().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pManualObj->getMapDescription());
        fillHelpUrlItems(pItem, pManualObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pManualObj);
  }

  // Picture file(s)

  if (m_mdf.getPictureCount()) {

    QTreeWidgetItem* pItemPicture = new QTreeWidgetItem(pItemFiles, 1);
    pItemPicture->setFont(0, fontTopItem);
    // pItemPicture->setForeground(0, greenBrush);
    pItemPicture->setText(0, tr("Pictures"));
    pItemFiles->addChild(pItemPicture);

    index = 0;
    CMDF_Picture* pPictureObj;

    do {
      pPictureObj = m_mdf.getPictureObj(index);
      if (nullptr != pPictureObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemPicture, 1);
        str                    = QString("Picture %1 - ").arg(index) + pPictureObj->getName().c_str();
        pItem->setText(0, str);
        pItemPicture->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: ") + pPictureObj->getName().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: ") + pPictureObj->getUrl().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Format: ") + pPictureObj->getFormat().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pPictureObj->getMapDescription());
        fillHelpUrlItems(pItem, pPictureObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pPictureObj);
  }

  // Video file(s)

  if (m_mdf.getVideoCount()) {

    QTreeWidgetItem* pItemVideo = new QTreeWidgetItem(pItemFiles, 1);
    pItemVideo->setFont(0, fontTopItem);
    // pItemVideo->setForeground(0, greenBrush);
    pItemVideo->setText(0, tr("Videos"));
    pItemFiles->addChild(pItemVideo);

    index = 0;
    CMDF_Video* pVideoObj;

    do {
      pVideoObj = m_mdf.getVideoObj(index);
      if (nullptr != pVideoObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemVideo, 1);
        str                    = QString("Video %1 - ").arg(index) + pVideoObj->getName().c_str();
        pItem->setText(0, str);
        pItemVideo->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: ") + pVideoObj->getName().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: ") + pVideoObj->getUrl().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Format: ") + pVideoObj->getFormat().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pVideoObj->getMapDescription());
        fillHelpUrlItems(pItem, pVideoObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pVideoObj);
  }

  // Firmware file(s)

  if (m_mdf.getFirmwareCount()) {

    QTreeWidgetItem* pItemFirmware = new QTreeWidgetItem(pItemFiles, 1);
    pItemFirmware->setFont(0, fontTopItem);
    // pItemFirmware->setForeground(0, greenBrush);
    pItemFirmware->setText(0, tr("Firmware"));
    pItemFiles->addChild(pItemFirmware);

    index = 0;
    CMDF_Firmware* pFirmwareObj;

    do {
      pFirmwareObj = m_mdf.getFirmwareObj(index);
      if (nullptr != pFirmwareObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemFirmware, 1);
        str                    = QString("Firmware %1 - %2 %3").arg(index).arg(pFirmwareObj->getDate().c_str()).arg(pFirmwareObj->getName().c_str());
        pItem->setText(0, str);
        pItemFirmware->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: %1").arg(pFirmwareObj->getName().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Version: %1.%2.%3").arg(pFirmwareObj->getVersionMajor()).arg(pFirmwareObj->getVersionMinor()).arg(pFirmwareObj->getVersionPatch());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: %s").arg(pFirmwareObj->getUrl().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Date: %1").arg(pFirmwareObj->getDate().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Target Code: %1").arg(pFirmwareObj->getTargetCode());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Target: %1").arg(pFirmwareObj->getTarget().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Format: %1").arg(pFirmwareObj->getFormat().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Size: %1").arg(pFirmwareObj->getSize());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("MD5 Checksum: %1").arg(pFirmwareObj->getMd5().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pFirmwareObj->getMapDescription());
        fillHelpUrlItems(pItem, pFirmwareObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pFirmwareObj);
  }

  // Drivers

  if (m_mdf.getDriverCount()) {

    QTreeWidgetItem* pItemDriver = new QTreeWidgetItem(pItemFiles, 1);
    pItemDriver->setFont(0, fontTopItem);
    // pItemDriver->setForeground(0, greenBrush);
    pItemDriver->setText(0, tr("Drivers"));
    pItemFiles->addChild(pItemDriver);

    index = 0;
    CMDF_Driver* pDriverObj;

    do {
      pDriverObj = m_mdf.getDriverObj(index);
      if (nullptr != pDriverObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemDriver, 1);
        str                    = QString("Driver %1 - %2 %3").arg(index).arg(pDriverObj->getName().c_str()).arg(pDriverObj->getDate().c_str());
        pItem->setText(0, str);
        pItemDriver->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: %1").arg(pDriverObj->getName().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Version: %1.%2.%3").arg(pDriverObj->getVersionMajor()).arg(pDriverObj->getVersionMinor()).arg(pDriverObj->getVersionPatch());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Date: ") + pDriverObj->getDate().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: ") + pDriverObj->getUrl().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Type: ") + pDriverObj->getType().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Architecture: ") + pDriverObj->getFormat().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("OS: ") + pDriverObj->getOS().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("OS Version: ") + pDriverObj->getOSVer().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("MD5 Checksum: ") + pDriverObj->getMd5().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pDriverObj->getMapDescription());
        fillHelpUrlItems(pItem, pDriverObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pDriverObj);
  }

  // Setup script

  if (m_mdf.getSetupCount()) {

    QTreeWidgetItem* pItemSetup = new QTreeWidgetItem(pItemFiles, 1);
    pItemSetup->setFont(0, fontTopItem);
    // pItemSetup->setForeground(0, greenBrush);
    pItemSetup->setText(0, tr("Setup scripts"));
    pItemFiles->addChild(pItemSetup);

    index = 0;
    CMDF_Setup* pSetupObj;

    do {
      pSetupObj = m_mdf.getSetupObj(index);
      if (nullptr != pSetupObj) {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pItemSetup, 1);
        str                    = QString("Script %1 - %2").arg(index).arg(pSetupObj->getName().c_str());
        pItem->setText(0, str);
        pItemSetup->addChild(pItem);

        QTreeWidgetItem* pSubItem;

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Name: %1").arg(pSetupObj->getName().c_str());
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("URL: ") + pSetupObj->getUrl().c_str();
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        pSubItem = new QTreeWidgetItem(pItem, 1);
        if (nullptr != pSubItem) {
          str = QString("Format: ") + pSetupObj->getFormat().c_str();
          pSubItem->setFont(0, fontTopItem);
          // pSubItem->setForeground(0, greenBrush);
          pSubItem->setText(0, str);
          pItem->addChild(pSubItem);
        }

        fillDescriptionItems(pItem, pSetupObj->getMapDescription());
        fillHelpUrlItems(pItem, pSetupObj->getMapInfoUrl());
      }

      index++;

    } while (nullptr != pSetupObj);
  }

  // Registers

  uint32_t nPages;
  std::set<uint16_t> pages;
  QTreeWidgetItem* pSubItem;

  QTreeWidgetItem* pItemRegister = new QTreeWidgetItem(pItemModule, 1);
  pItemRegister->setFont(0, fontTopItem);
  pItemRegister->setForeground(0, greenBrush);
  pItemRegister->setText(0, tr("Registers"));
  ui->treeMDF->addTopLevelItem(pItemRegister);

  // Fetch number of pages and pages
  nPages                           = m_mdf.getPages(pages);
  std::deque<CMDF_Register*>* regs = m_mdf.getRegisterList();

  // If we have pages separate registers in pages
  if (nPages > 1) {
    for (auto itr : pages) {

      pItem = new QTreeWidgetItem(pItemRegister, 1);
      if (nullptr != pItem) {

        str = QString("Register page: %1").arg(itr);
        pItem->setText(0, str);
        pItemRegister->addChild(pItem);

        // Add registers for page
        for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
          if (itr == (*it)->getPage()) {
            pSubItem = new QTreeWidgetItem(pItem, 1);
            if (nullptr != pSubItem) {
              str = QString("Register  %1").arg((*it)->getOffset());
              pSubItem->setText(0, str);
              pItem->addChild(pSubItem);
              fillRegisterInfo(pSubItem, *it);
            }
          }
        }
      }
    }
  }
  else {
  }

  if (m_mdf.getRegisterCount()) {
  }

  QTreeWidgetItem* pItemRemoteVar = new QTreeWidgetItem(pItemModule, 1);
  pItemRemoteVar->setFont(0, fontTopItem);
  pItemRemoteVar->setForeground(0, greenBrush);
  pItemRemoteVar->setText(0, tr("Remote Variables"));
  ui->treeMDF->addTopLevelItem(pItemRemoteVar);

  QTreeWidgetItem* pItemAlarm = new QTreeWidgetItem(pItemModule, 1);
  pItemAlarm->setFont(0, fontTopItem);
  pItemAlarm->setForeground(0, greenBrush);
  pItemAlarm->setText(0, tr("Alarm"));
  ui->treeMDF->addTopLevelItem(pItemAlarm);

  QTreeWidgetItem* pItemDM = new QTreeWidgetItem(pItemModule, 1);
  pItemDM->setFont(0, fontTopItem);
  pItemDM->setForeground(0, greenBrush);
  pItemDM->setText(0, tr("Decision Matrix"));
  ui->treeMDF->addTopLevelItem(pItemDM);

  // Events
  QTreeWidgetItem* pItemEvent = new QTreeWidgetItem(pItemModule, 1);
  pItemEvent->setFont(0, fontTopItem);
  pItemEvent->setForeground(0, greenBrush);
  pItemEvent->setText(0, tr("Events"));
  ui->treeMDF->addTopLevelItem(pItemEvent);

  // Recipes
  // QTreeWidgetItem *pItemRecipes = new QTreeWidgetItem(pItemModule, 1);
  // pItemRecipes->setFont(0, fontTopItem);
  // pItemRecipes->setForeground(0, blueBrush);
  // pItemRecipes->setText(0, tr("Events"));
  // ui->treeMDF->addTopLevelItem(pItemRecipes);

  // ui->btnScan->setEnabled(false);
  // CFoundNodeWidgetItem *pItem;
  // QTreeWidgetItemIterator it(ui->treeFound);
  // while (*it) {
  //   pItem = (CFoundNodeWidgetItem *)(*it);
  //   doLoadMdf(pItem->m_nodeid);
  //   ++it;
  // }
  // ui->btnScan->setEnabled(true);
  // QString fileName = QFileDialog::getOpenFileName(this,
  //   tr("Open Module Description File (MDF)"), ".", tr("MDF Files (*.mdf *.json *.xml)"));

  // ui->treeMDF->expandAll();
}

///////////////////////////////////////////////////////////////////////////////
// onFindNodesTreeWidgetItemClicked
//

// void
// CFrmMdf::onFindNodesTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
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
//}
