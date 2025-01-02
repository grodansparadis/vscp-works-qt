// cdlgmdfmmanufacturer.cpp
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

#include "cdlgmdfmanufacturer.h"
#include "ui_cdlgmdfmanufacturer.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfManufacturer::CDlgMdfManufacturer(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfManufacturer)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfManufacturer::editDesc);

  // // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  // connect(ui->btnAddDesc, &QToolButton::clicked, this, &CDlgMdfManufacturer::addDesc);
  // connect(ui->btnEditDesc, &QToolButton::clicked, this, &CDlgMdfManufacturer::editDesc);
  // connect(ui->btnDupDesc, &QToolButton::clicked, this, &CDlgMdfManufacturer::dupDesc);
  // connect(ui->btnDelDesc, &QToolButton::clicked, this, &CDlgMdfManufacturer::deleteDesc);

  // connect(ui->btnAddInfo, &QToolButton::clicked, this, &CDlgMdfManufacturer::addInfo);
  // connect(ui->btnEditInfo, &QToolButton::clicked, this, &CDlgMdfManufacturer::editInfo);
  // connect(ui->btnDupInfo, &QToolButton::clicked, this, &CDlgMdfManufacturer::dupInfo);
  // connect(ui->btnDelInfo, &QToolButton::clicked, this, &CDlgMdfManufacturer::deleteInfo);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfManufacturer::~CDlgMdfManufacturer()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfManufacturer::initDialogData(const CMDF_Object* pmdfobj, mdf_manufacturer_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmanufacturer = (CMDF_Manufacturer*)pmdfobj;

  ui->editName->setText(m_pmanufacturer->getName().c_str());
  ui->editStreet->setText(m_pmanufacturer->getAddressObj()->getStreet().c_str());
  ui->editCity->setText(m_pmanufacturer->getAddressObj()->getCity().c_str());
  ui->editTown->setText(m_pmanufacturer->getAddressObj()->getTown().c_str());
  ui->editPostCode->setText(m_pmanufacturer->getAddressObj()->getPostCode().c_str());
  ui->editRegion->setText(m_pmanufacturer->getAddressObj()->getRegion().c_str());
  ui->editState->setText(m_pmanufacturer->getAddressObj()->getState().c_str());
  ui->editCountry->setText(m_pmanufacturer->getAddressObj()->getCountry().c_str());

  switch (index) {
    case index_manufacturer_street:
      ui->editStreet->setFocus();
      break;

    case index_manufacturer_city:
      ui->editCity->setFocus();
      break;

    case index_manufacturer_town:
      ui->editTown->setFocus();
      break;  

    case index_manufacturer_post_code:
      ui->editPostCode->setFocus();
      break;   

    case index_manufacturer_region:
      ui->editRegion->setFocus();
      break;   

    case index_manufacturer_state:
      ui->editState->setFocus();
      break;   

    case index_manufacturer_country:
      ui->editCountry->setFocus();
      break;   

    case index_manufacturer_name:
    default:
      ui->editName->setFocus();
      break;
  }
}


///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfManufacturer::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}



// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgMdfManufacturer::getName(void)
{
  return (ui->editName->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgMdfManufacturer::setName(const QString& str)
{
  ui->editName->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getStreet
//

QString
CDlgMdfManufacturer::getStreet(void)
{
  return (ui->editStreet->text());
}

///////////////////////////////////////////////////////////////////////////////
// setStreet
//

void
CDlgMdfManufacturer::setStreet(const QString& str)
{
  ui->editStreet->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getCity
//

QString
CDlgMdfManufacturer::getCity(void)
{
  return (ui->editCity->text());
}

///////////////////////////////////////////////////////////////////////////////
// setCity
//

void
CDlgMdfManufacturer::setCity(const QString& str)
{
  ui->editCity->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getTown
//

QString
CDlgMdfManufacturer::getTown(void)
{
  return (ui->editTown->text());
}

///////////////////////////////////////////////////////////////////////////////
// setTown
//

void
CDlgMdfManufacturer::setTown(const QString& str)
{
  ui->editTown->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPostCode
//

QString
CDlgMdfManufacturer::getPostCode(void)
{
  return (ui->editPostCode->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPostCode
//

void
CDlgMdfManufacturer::setPostCode(const QString& str)
{
  ui->editPostCode->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getRegion
//

QString
CDlgMdfManufacturer::getRegion(void)
{
  return (ui->editRegion->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPostCode
//

void
CDlgMdfManufacturer::setRegion(const QString& str)
{
  ui->editRegion->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getState
//

QString
CDlgMdfManufacturer::getState(void)
{
  return (ui->editState->text());
}

///////////////////////////////////////////////////////////////////////////////
// setState
//

void
CDlgMdfManufacturer::setState(const QString& str)
{
  ui->editState->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getCountry
//

QString
CDlgMdfManufacturer::getCountry(void)
{
  return (ui->editCountry->text());
}

///////////////////////////////////////////////////////////////////////////////
// setCountry
//

void
CDlgMdfManufacturer::setCountry(const QString& str)
{
  ui->editCountry->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfManufacturer::accept()
{
  std::string str;
  if (nullptr != m_pmanufacturer) {

    str = ui->editName->text().toStdString();
    //m_pmdf->setModuleName(str);

    // str = ui->editModel->text().toStdString();
    // m_pmdf->setModuleModel(str);

    // m_pmdf->setModuleLevel(ui->comboModuleLevel->currentIndex());

    // str = ui->editVersion->text().toStdString();
    // m_pmdf->setModuleVersion(str);

    // str = ui->editDate->text().toStdString();
    // m_pmdf->setModuleChangeDate(str);

    // m_pmdf->setModuleBufferSize(ui->editBufferSize->value());

    // str = ui->editCopyright->text().toStdString();
    // m_pmdf->setModuleCopyright(str);
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// addDesc
//

void
CDlgMdfManufacturer::addContact(void)
{
  // QString selstr = "en"; // Default language

  // CDlgMdfDescription dlg(this);
  // dlg.initDialogData(m_pmdf->getModuleDescriptionMap()/*, &selstr*/);
  // if (QDialog::Accepted == dlg.exec()) {
  //   ui->listDescription->clear();
  //   fillDescription();
  // }
}

///////////////////////////////////////////////////////////////////////////////
// editDesc
//

void
CDlgMdfManufacturer::editContact(void)
{
  // if (-1 != ui->listDescription->currentRow()) {

  //   // Save the row
  //   int idx = ui->listDescription->currentRow();

  //   QListWidgetItem* pitem = ui->listDescription->currentItem();
  //   QString selstr         = pitem->text().split('_').first().left(2);

  //   CDlgMdfDescription dlg(this);
  //   dlg.initDialogData(m_pmdf->getModuleDescriptionMap(), &selstr);
  //   if (QDialog::Accepted == dlg.exec()) {
  //     ui->listDescription->clear();
  //     fillDescription();
  //     ui->listDescription->setCurrentRow(idx);
  //   }
  // }
  // else {
  //   QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// dupDesc
//

void
CDlgMdfManufacturer::dupContact(void)
{
  // if (-1 != ui->listDescription->currentRow()) {
  //   CDlgMdfDescription dlg(this);
  //   dlg.initDialogData(m_pmdf->getModuleDescriptionMap());
  //   if (QDialog::Accepted == dlg.exec()) {
  //     ui->listDescription->clear();
  //     fillDescription();
  //   }
  // }
  // else {
  //   QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// deleteDesc
//

void
CDlgMdfManufacturer::deleteContact(void)
{
  // if (-1 != ui->listDescription->currentRow()) {

  //   // Save the row
  //   int idx = ui->listDescription->currentRow();

  //   QListWidgetItem* pitem = ui->listDescription->currentItem();
  //   QString selstr         = pitem->text().split('_').first().left(2);

  //   m_pmdf->getModuleDescriptionMap()->erase(selstr.toStdString());
  //   ui->listDescription->clear();
  //   fillDescription();

  // }
}
