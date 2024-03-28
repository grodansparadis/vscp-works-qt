// cdlgmdfmodule.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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

#include "cdlgmdfmodule.h"
#include "ui_cdlgmdfmodule.h"

#include "cdlgmdfdescription.h"
#include "cdlgmdfinfourl.h"

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

CDlgMdfModule::CDlgMdfModule(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfModule)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  connect(shortcut, &QShortcut::activated, this, &CDlgMdfModule::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  connect(ui->btnAddDesc, &QToolButton::clicked, this, &CDlgMdfModule::addDesc);
  connect(ui->btnEditDesc, &QToolButton::clicked, this, &CDlgMdfModule::editDesc);
  connect(ui->btnDupDesc, &QToolButton::clicked, this, &CDlgMdfModule::dupDesc);
  connect(ui->btnDelDesc, &QToolButton::clicked, this, &CDlgMdfModule::deleteDesc);

  connect(ui->btnAddInfo, &QToolButton::clicked, this, &CDlgMdfModule::addInfo);
  connect(ui->btnEditInfo, &QToolButton::clicked, this, &CDlgMdfModule::editInfo);
  connect(ui->btnDupInfo, &QToolButton::clicked, this, &CDlgMdfModule::dupInfo);
  connect(ui->btnDelInfo, &QToolButton::clicked, this, &CDlgMdfModule::deleteInfo);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfModule::~CDlgMdfModule()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfModule::initDialogData(const CMDF_Object* pmdfobj, mdf_module_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = (CMDF*)pmdfobj;

  ui->editName->setText(m_pmdf->getModuleName().c_str());
  ui->editModel->setText(m_pmdf->getModuleModel().c_str());
  ui->comboModuleLevel->setCurrentIndex(m_pmdf->getModuleLevel());
  ui->editVersion->setText(m_pmdf->getModuleVersion().c_str());
  str = m_pmdf->getModuleChangeDate().c_str();
  ui->editDate->setDate(QDate::fromString(str, Qt::ISODate));
  ui->editBufferSize->setValue(m_pmdf->getModuleBufferSize());
  ui->editCopyright->setText(m_pmdf->getModuleCopyright().c_str());

  switch (index) {
    case index_module_model:
      ui->editModel->setFocus();
      break;

    case index_module_version:
      ui->editVersion->setFocus();
      break;

    case index_module_level:
      ui->comboModuleLevel->setFocus();
      break;

    case index_module_change_date:
      ui->editDate->setFocus();
      break;

    case index_module_buffer_size:
      ui->editBufferSize->setFocus();
      break;

    case index_module_copyright:
      ui->editCopyright->setFocus();
      break;

    case index_module_name:
    default:
      ui->editName->setFocus();
      break;
  }

  // Fill in descriptions
  fillDescription();

  // std::map<std::string, std::string>* pmapDescription = m_pmdf->getModuleDescriptionMap();
  // std::map<std::string, std::string>::iterator itDesc = pmapDescription->begin();
  // while (itDesc != pmapDescription->end()) {
  //   std::string lang        = itDesc->first; // key
  //   std::string description = itDesc->second;
  //   str                     = lang.c_str() + tr(" - ") + description.c_str();
  //   ui->listDescription->addItem(str);
  //   itDesc++;
  // }

  // Fill in help URL's
  fillInfoUrl();
  // std::map<std::string, std::string>* pmapHelpUrl     = m_pmdf->getModuleHelpUrlMap();
  // std::map<std::string, std::string>::iterator itInfo = pmapHelpUrl->begin();
  // while (itInfo != pmapHelpUrl->end()) {
  //   std::string lang = itInfo->first; // key
  //   std::string info = itInfo->second;
  //   str              = lang.c_str() + tr(" - ") + info.c_str();
  //   ui->listInfo->addItem(str);
  //   itInfo++;
  // }
}

///////////////////////////////////////////////////////////////////////////////
// fillDescription
//

void
CDlgMdfModule::fillDescription()
{
  QString str;

  // Fill in descriptions
  std::map<std::string, std::string>* pmapDescription = m_pmdf->getMapDescription();
  std::map<std::string, std::string>::iterator itDesc = pmapDescription->begin();
  while (itDesc != pmapDescription->end()) {
    std::string lang        = itDesc->first; // key
    std::string description = itDesc->second;
    str                     = lang.c_str() + tr(" - ") + description.c_str();
    ui->listDescription->addItem(str);
    itDesc++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillInfoUrl
//

void
CDlgMdfModule::fillInfoUrl()
{
  QString str;

  // Fill in help URL's
  std::map<std::string, std::string>* pmapHelpUrl     = m_pmdf->getHelpUrlMap();
  std::map<std::string, std::string>::iterator itInfo = pmapHelpUrl->begin();
  while (itInfo != pmapHelpUrl->end()) {
    std::string lang = itInfo->first;   // key
    std::string info = itInfo->second;
    str              = lang.c_str() + tr(" - ") + info.c_str();
    ui->listInfo->addItem(str);
    itInfo++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfModule::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setEditMode
//

// void CDlgMdfModule::setEditMode(void)
// {
//     ui->editGuid->setReadOnly(true);
//     ui->btnSetDummyGuid->setVisible(false);
//     ui->editName->setFocus();
// }

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgMdfModule::getName(void)
{
  return (ui->editName->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgMdfModule::setName(const QString& str)
{
  ui->editName->setText(str);
}


///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfModule::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

    str = ui->editName->text().toStdString();
    m_pmdf->setModuleName(str);

    str = ui->editModel->text().toStdString();
    m_pmdf->setModuleModel(str);

    m_pmdf->setModuleLevel(ui->comboModuleLevel->currentIndex());

    str = ui->editVersion->text().toStdString();
    m_pmdf->setModuleVersion(str);

    str = ui->editDate->text().toStdString();
    m_pmdf->setModuleChangeDate(str);

    m_pmdf->setModuleBufferSize(ui->editBufferSize->value());

    str = ui->editCopyright->text().toStdString();
    m_pmdf->setModuleCopyright(str);
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
CDlgMdfModule::addDesc(void)
{
  QString selstr = "en"; // Default language

  CDlgMdfDescription dlg(this);
  dlg.initDialogData(m_pmdf->getMapDescription()/*, &selstr*/);
  if (QDialog::Accepted == dlg.exec()) {
    ui->listDescription->clear();
    fillDescription();
  }
}

///////////////////////////////////////////////////////////////////////////////
// editDesc
//

void
CDlgMdfModule::editDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {

    // Save the row
    int idx = ui->listDescription->currentRow();

    QListWidgetItem* pitem = ui->listDescription->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pmdf->getMapDescription(), &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listDescription->clear();
      fillDescription();
      ui->listDescription->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupDesc
//

void
CDlgMdfModule::dupDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {
    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pmdf->getMapDescription());
    if (QDialog::Accepted == dlg.exec()) {
      ui->listDescription->clear();
      fillDescription();
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteDesc
//

void
CDlgMdfModule::deleteDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {

    // Save the row
    int idx = ui->listDescription->currentRow();

    QListWidgetItem* pitem = ui->listDescription->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    m_pmdf->getMapDescription()->erase(selstr.toStdString());
    ui->listDescription->clear();
    fillDescription();

  }
}

///////////////////////////////////////////////////////////////////////////////
// addInfo
//

void
CDlgMdfModule::addInfo(void)
{
  QString selstr = "en"; // Default language

  CDlgMdfInfoUrl dlg(this);
  dlg.initDialogData(m_pmdf->getHelpUrlMap()/*, &selstr*/);
  if (QDialog::Accepted == dlg.exec()) {
    ui->listInfo->clear();
    fillInfoUrl();
  }
}

///////////////////////////////////////////////////////////////////////////////
// editInfo
//

void
CDlgMdfModule::editInfo(void)
{
  if (-1 != ui->listInfo->currentRow()) {

    // Save the row
    int idx = ui->listInfo->currentRow();

    QListWidgetItem* pitem = ui->listInfo->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    CDlgMdfInfoUrl dlg(this);
    dlg.initDialogData(m_pmdf->getHelpUrlMap(), &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listInfo->clear();
      fillInfoUrl();
      ui->listInfo->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupInfo
//

void
CDlgMdfModule::dupInfo(void)
{
  if (-1 != ui->listInfo->currentRow()) {
    CDlgMdfInfoUrl dlg(this);
    dlg.initDialogData(m_pmdf->getHelpUrlMap());
    if (QDialog::Accepted == dlg.exec()) {
      ui->listInfo->clear();
      fillInfoUrl();
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteInfo
//

void
CDlgMdfModule::deleteInfo(void)
{
    if (-1 != ui->listInfo->currentRow()) {

    // Save the row
    int idx = ui->listInfo->currentRow();

    QListWidgetItem* pitem = ui->listInfo->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    m_pmdf->getHelpUrlMap()->erase(selstr.toStdString());
    ui->listInfo->clear();
    fillInfoUrl();

  }
}