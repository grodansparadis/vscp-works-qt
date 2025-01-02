// cdlgmdffile.cpp
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgmdffiledriver.h"
#include "cdlgmdffilefirmware.h"
#include "cdlgmdffilemanual.h"
#include "cdlgmdffilepicture.h"
#include "cdlgmdffilesetup.h"
#include "cdlgmdffilevideo.h"

#include "cdlgmdffile.h"
#include "ui_cdlgmdffile.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfFile:: pre_str_picture[] = "Picture: ";
const char CDlgMdfFile:: pre_str_video[] = "Video: ";
const char CDlgMdfFile:: pre_str_firmware[] = "Firmware: ";
const char CDlgMdfFile:: pre_str_driver[] = "Driver: ";
const char CDlgMdfFile:: pre_str_setup[] = "Setup script: ";
const char CDlgMdfFile:: pre_str_manual[] = "Manual: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfFile::CDlgMdfFile(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfFile)
{
  ui->setupUi(this);

  m_type = mdf_type_unknown;
  m_pmdf = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfFile::~CDlgMdfFile()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfFile::initDialogData(const CMDF_Object* pmdfobj, mdf_record_type type, int index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_type = type;
  m_pmdf = (CMDF*)pmdfobj;

  connect(ui->btnAddFile, &QToolButton::clicked, this, &CDlgMdfFile::addFileItem);
  connect(ui->btnEditFile, &QToolButton::clicked, this, &CDlgMdfFile::editFileItem);
  connect(ui->btnDupFile, &QToolButton::clicked, this, &CDlgMdfFile::dupFileItem);
  connect(ui->btnDelFile, &QToolButton::clicked, this, &CDlgMdfFile::deleteFileItem);

  switch (type) {

    case mdf_type_picture: {
      setWindowTitle(tr("Picture files"));
      ui->comboFileType->setCurrentIndex(index_picture);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getPictureCount()) {
        return;
      }

      int index = 0;
      CMDF_Picture* pPictureObj;

      do {
        pPictureObj = m_pmdf->getPictureObj(index);
        if (nullptr != pPictureObj) {
          ui->listFile->addItem(QString("Picture: - ") + pPictureObj->getName().c_str());
        }

        index++;
      } while (nullptr != pPictureObj);
    } break;

    case mdf_type_picture_item:
      setWindowTitle(tr("Picture files"));
      break;

    case mdf_type_video: {
      setWindowTitle(tr("Video files"));
      ui->comboFileType->setCurrentIndex(index_video);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getVideoCount()) {
        return;
      }

      int index = 0;
      CMDF_Video* pVideoObj;

      do {
        pVideoObj = m_pmdf->getVideoObj(index);
        if (nullptr != pVideoObj) {
          ui->listFile->addItem(QString("Video - ") + pVideoObj->getName().c_str());
        }

        index++;
      } while (nullptr != pVideoObj);
    } break;

    case mdf_type_video_item: {
      setWindowTitle(tr("Video files"));
    } break;

    case mdf_type_firmware: {
      setWindowTitle(tr("Firmware files"));
      ui->comboFileType->setCurrentIndex(index_firmware);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getFirmwareCount()) {
        return;
      }

      int index = 0;
      CMDF_Firmware* pFirmwareObj;

      do {
        pFirmwareObj = m_pmdf->getFirmwareObj(index);
        if (nullptr != pFirmwareObj) {
          ui->listFile->addItem(QString("Firmware - ") + pFirmwareObj->getName().c_str());
        }

        index++;
      } while (nullptr != pFirmwareObj);
    } break;

    case mdf_type_firmware_item:
      setWindowTitle(tr("Firmware files"));
      break;

    case mdf_type_driver: {
      setWindowTitle(tr("Driver files"));
      ui->comboFileType->setCurrentIndex(index_driver);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getDriverCount()) {
        return;
      }

      int index = 0;
      CMDF_Driver* pDriverObj;

      do {
        pDriverObj = m_pmdf->getDriverObj(index);
        if (nullptr != pDriverObj) {
          ui->listFile->addItem(QString("Driver - ") + pDriverObj->getName().c_str());
        }

        index++;
      } while (nullptr != pDriverObj);
    } break;

    case mdf_type_driver_item:
      setWindowTitle(tr("Driver files"));
      break;

    case mdf_type_setup: {
      setWindowTitle(tr("Setup script files"));
      ui->comboFileType->setCurrentIndex(index_setup);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getSetupCount()) {
        return;
      }

      int index = 0;
      CMDF_Setup* pSetupObj;

      do {
        pSetupObj = m_pmdf->getSetupObj(index);
        if (nullptr != pSetupObj) {
          ui->listFile->addItem(QString("Setup - ") + pSetupObj->getName().c_str());
        }

        index++;
      } while (nullptr != pSetupObj);
    } break;

    case mdf_type_setup_item:
      setWindowTitle(tr("Setup script files"));
      break;

    case mdf_type_manual: {
      setWindowTitle(tr("Manual files"));
      ui->comboFileType->setCurrentIndex(index_manual);
      ui->comboFileType->setEnabled(false);

      // Return if noting to render
      if (!m_pmdf->getManualCount()) {
        return;
      }

      int index = 0;
      CMDF_Manual* pManualObj;

      do {
        pManualObj = m_pmdf->getManualObj(index);
        if (nullptr != pManualObj) {
          ui->listFile->addItem(QString("Manual - ") + pManualObj->getName().c_str());
        }

        index++;
      } while (nullptr != pManualObj);
    } break;

    case mdf_type_manual_item:
      setWindowTitle(tr("Manual files"));
      break;

    default:
    break;  
  }

  // ui->editName->setText(m_pfile->getName().c_str());
  // ui->editUrl->setText(m_pfile->getUrl().c_str());
  // ui->editType->setText(m_pfile->getType().c_str());
  // ui->editOs->setText(m_pfile->getOS().c_str());
  // ui->editArchitecture->setText(m_pfile->getArchitecture().c_str());
  // ui->editOsVersion->setText(m_pfile->getOSVer().c_str());
  // ui->editVersion->setText(m_pfile->getVersion().c_str());

  // QDate dd = QDate::fromString(m_pfile->getDate().c_str(),"YY-MM_DD");
  // ui->date->setDate(dd);

  // switch (index) {
  //   case index_file_name:
  //     ui->editName->setFocus();
  //     break;

  //   case index_file_url:
  //     ui->editUrl->setFocus();
  //     break;

  //   case index_file_type:
  //     ui->editType->setFocus();
  //     break;

  //   case index_file_os:
  //     ui->editOs->setFocus();
  //     break;

  //   case index_file_architecture:
  //     ui->editArchitecture->setFocus();
  //     break;

  //   case index_file_os_version:
  //     ui->editOsVersion->setFocus();
  //     break;

  //   case index_file_date:
  //     ui->date->setFocus();
  //     break;

  //   case index_file_version:
  //     ui->editVersion->setFocus();
  //     break;

  //   case index_file_none:
  //   default:
  //     ui->editName->setFocus();
  //     break;
  // }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfFile::setInitialFocus(void)
{
  //ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// addFileItem
//

void
CDlgMdfFile::addFileItem(void)
{
  switch (m_type) {

    case mdf_type_picture: {

      CMDF_Picture* pobjnew = new CMDF_Picture();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CDlgMdfFilePicture dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getPictureObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_picture) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    case mdf_type_video: {

      CMDF_Video* pobjnew = new CMDF_Video();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CMDF_Video* pobj = m_pmdf->getVideoObj(ui->listFile->currentRow());
      CDlgMdfFileVideo dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getVideoObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_video) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    case mdf_type_firmware: {

      CMDF_Firmware* pobjnew = new CMDF_Firmware();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CDlgMdfFileFirmware dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getFirmwareObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_firmware) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    case mdf_type_driver: {

      CMDF_Driver* pobjnew = new CMDF_Driver();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CDlgMdfFileDriver dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getDriverObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_driver) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    case mdf_type_setup: {

      CMDF_Setup* pobjnew = new CMDF_Setup();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CDlgMdfFileSetup dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getSetupObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_setup) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    case mdf_type_manual: {

      CMDF_Manual* pobjnew = new CMDF_Manual();
      if (nullptr == pobjnew) {
        QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
         return;
      }

      CDlgMdfFileManual dlg(this);
      dlg.initDialogData(pobjnew);
      if (QDialog::Accepted == dlg.exec()) {
        m_pmdf->getManualObjList()->push_back(pobjnew);
        ui->listFile->addItem(QString(pre_str_manual) + QString(" ") + pobjnew->getName().c_str());
      }
      else {
        delete pobjnew;
      }
    } break;

    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editFileItem
//

void
CDlgMdfFile::editFileItem(void)
{
  if (-1 != ui->listFile->currentRow()) {

    // Save the row
    int idx = ui->listFile->currentRow();

    switch (m_type) {

      case mdf_type_picture: {
        CMDF_Picture* pobj = m_pmdf->getPictureObj(ui->listFile->currentRow());
        CDlgMdfFilePicture dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          ui->listFile->currentItem()->setText(QString(pre_str_picture) + QString("- ") + pobj->getName().c_str());
        }
      } break;

      case mdf_type_video: {
        CMDF_Video* pobj = m_pmdf->getVideoObj(ui->listFile->currentRow());
        CDlgMdfFileVideo dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          ui->listFile->currentItem()->setText(QString(pre_str_video) + QString(" ") + pobj->getName().c_str());
        }
      } break;

      case mdf_type_firmware: {
        CMDF_Firmware* pobj = m_pmdf->getFirmwareObj(ui->listFile->currentRow());
        CDlgMdfFileFirmware dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          ui->listFile->currentItem()->setText(QString(pre_str_firmware) + QString(" ") + pobj->getName().c_str());
        }
      } break;

      case mdf_type_driver: {
        CMDF_Driver* pobj = m_pmdf->getDriverObj(ui->listFile->currentRow());
        CDlgMdfFileDriver dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          ui->listFile->currentItem()->setText(QString(pre_str_driver) + QString(" ") + pobj->getName().c_str());
        }
      } break;

      case mdf_type_setup: {
        CMDF_Setup* pobj = m_pmdf->getSetupObj(ui->listFile->currentRow());
        CDlgMdfFileSetup dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          // ui->listDescription->clear();
          // fillDescription();
        }
      } break;

      case mdf_type_manual: {
        CMDF_Manual* pobj = m_pmdf->getManualObj(ui->listFile->currentRow());
        CDlgMdfFileManual dlg(this);
        dlg.initDialogData(pobj /*, &selstr*/);
        if (QDialog::Accepted == dlg.exec()) {
          ui->listFile->currentItem()->setText(QString(pre_str_manual) + QString(" ") + pobj->getName().c_str());
        }
      } break;

      default:
      break;
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupFileItem
//

void
CDlgMdfFile::dupFileItem(void)
{
  if (-1 != ui->listFile->currentRow()) {

    // Save the row
    int idx = ui->listFile->currentRow();

    switch (m_type) {

      case mdf_type_picture: {

        CMDF_Picture* pobj = m_pmdf->getPictureObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Picture* pobjnew = new CMDF_Picture();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFilePicture dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getPictureObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_picture) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }

      } break;

      case mdf_type_video: {
        CMDF_Video* pobj = m_pmdf->getVideoObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Video* pobjnew = new CMDF_Video();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFileVideo dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getVideoObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_video) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }
      } break;

      case mdf_type_firmware: {
        CMDF_Firmware* pobj = m_pmdf->getFirmwareObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Firmware* pobjnew = new CMDF_Firmware();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFileFirmware dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getFirmwareObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_firmware) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }
      } break;

      case mdf_type_driver: {
        CMDF_Driver* pobj = m_pmdf->getDriverObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Driver* pobjnew = new CMDF_Driver();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFileDriver dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getDriverObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_driver) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }
      } break;

      case mdf_type_setup: {
        CMDF_Setup* pobj = m_pmdf->getSetupObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Setup* pobjnew = new CMDF_Setup();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFileSetup dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getSetupObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_setup) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }
      } break;

      case mdf_type_manual: {
        CMDF_Manual* pobj = m_pmdf->getManualObj(ui->listFile->currentRow());
        if (nullptr == pobj) {
          QMessageBox::warning(this, tr(APPNAME), tr("Invalid object"), QMessageBox::Ok);
          return;
        }

        CMDF_Manual* pobjnew = new CMDF_Manual();
        if (nullptr == pobjnew) {
          QMessageBox::warning(this, tr("APPNAME"), tr("Out of memory problem"), QMessageBox::Ok);
          return;
        }

        *pobjnew = *pobj;

        CDlgMdfFileManual dlg(this);
        dlg.initDialogData(pobjnew);
        if (QDialog::Accepted == dlg.exec()) {
          m_pmdf->getManualObjList()->push_back(pobjnew);
          ui->listFile->addItem(QString(pre_str_manual) + QString(" ") + pobjnew->getName().c_str());
        }
        else {
          delete pobjnew;
        }
      } break;

      default:
      break;
    } // switch
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteFileItem
//

void
CDlgMdfFile::deleteFileItem(void)
{
  if (-1 != ui->listFile->currentRow()) {

    // Save the row
    int idx = ui->listFile->currentRow();

    switch (m_type) {

      case mdf_type_picture: {
        std::deque<CMDF_Picture*>* pPictureList = m_pmdf->getPictureObjList();
        pPictureList->erase(pPictureList->begin() + idx);
        ui->listFile->takeItem(idx);
      } break;

      case mdf_type_video: {
        std::deque<CMDF_Video*>* pVideoList = m_pmdf->getVideoObjList();
        pVideoList->erase(pVideoList->begin() + idx);
        ui->listFile->takeItem(idx);

      } break;

      case mdf_type_firmware: {
        std::deque<CMDF_Firmware*>* pFirmwareList = m_pmdf->getFirmwareObjList();
        pFirmwareList->erase(pFirmwareList->begin() + idx);
        ui->listFile->takeItem(idx);
      } break;

      case mdf_type_driver: {
        std::deque<CMDF_Driver*>* pDriverList = m_pmdf->getDriverObjList();
        pDriverList->erase(pDriverList->begin() + idx);
        ui->listFile->takeItem(idx);
      } break;

      case mdf_type_setup: {
        std::deque<CMDF_Setup*>* pSetupList = m_pmdf->getSetupObjList();
        pSetupList->erase(pSetupList->begin() + idx);
        ui->listFile->takeItem(idx);
      } break;

      case mdf_type_manual: {
        std::deque<CMDF_Manual*>* pManualList = m_pmdf->getManualObjList();
        pManualList->erase(pManualList->begin() + idx);
        ui->listFile->takeItem(idx);
      } break;

      default:
      break;
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfFile::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

    /*  str = ui->editBlockSize->text().toStdString();
     m_pbootinfo->setBlocksize(vscp_readStringValue(str));

     str = ui->editBlockCount->text().toStdString();
     m_pbootinfo->setBlockCount(vscp_readStringValue(str));

     // m_pbootinfo->setModuleLevel(ui->comboModuleLevel->currentIndex());

     int idx = ui->comboBoxAlgorithm->currentIndex(); //  ->text().toStdString();
     m_pbootinfo->setAlgorithm(idx); */
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
