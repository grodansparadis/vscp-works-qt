// cdlgmdfcontact.cpp
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgmdfcontact.h"
#include "ui_cdlgmdfcontact.h"

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

CDlgMdfContact::CDlgMdfContact(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfContact)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfContact::editDesc);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfContact::~CDlgMdfContact()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfContact::initDialogData(const CMDF_Object* pmdfobj)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = (CMDF*)pmdfobj;

  // ui->editName->setText(m_pmdf->getModuleName().c_str());
  // ui->editModel->setText(m_pmdf->getModuleModel().c_str());
  // ui->comboModuleLevel->setCurrentIndex(m_pmdf->getModuleLevel());
  // ui->editVersion->setText(m_pmdf->getModuleVersion().c_str());
  // str = m_pmdf->getModuleChangeDate().c_str();
  // ui->editDate->setDate(QDate::fromString(str, Qt::ISODate));
  // ui->editBufferSize->setValue(m_pmdf->getModuleBufferSize());
  // ui->editCopyright->setText(m_pmdf->getModuleCopyright().c_str());

  // switch (index) {
  //   case index_module_model:
  //     ui->editModel->setFocus();
  //     break;

  //   case index_module_version:
  //     ui->editVersion->setFocus();
  //     break;

  //   case index_module_level:
  //     ui->comboModuleLevel->setFocus();
  //     break;

  //   case index_module_change_date:
  //     ui->editDate->setFocus();
  //     break;

  //   case index_module_buffer_size:
  //     ui->editBufferSize->setFocus();
  //     break;

  //   case index_module_copyright:
  //     ui->editCopyright->setFocus();
  //     break;

  //   case index_module_name:
  //   default:
  //     ui->editName->setFocus();
  //     break;
  // }

  // Fill in descriptions
  //fillDescription();

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
  //fillInfoUrl();
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
// setInitialFocus
//

void
CDlgMdfContact::setInitialFocus(void)
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
CDlgMdfContact::getName(void)
{
  return (ui->editValue->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgMdfContact::setName(const QString& str)
{
  ui->editValue->setText(str);
}


///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfContact::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

    // str = ui->editName->text().toStdString();
    // m_pmdf->setModuleName(str);

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
