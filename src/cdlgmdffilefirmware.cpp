// cdlgmdffirmware.cpp
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

#include "cdlgmdffilefirmware.h"
#include "ui_cdlgmdffilefirmware.h"

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

CDlgMdfFileFirmware::CDlgMdfFileFirmware(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfFileFirmware)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfFileFirmware::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfFileFirmware::~CDlgMdfFileFirmware()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfFileFirmware::initDialogData(const CMDF_Object* pmdfobj, mdf_firmware_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pfirmware = (CMDF_Firmware*)pmdfobj;

  ui->editName->setText(m_pfirmware->getName().c_str());
  ui->editTargetCode->setText(QString::number(m_pfirmware->getTargetCode()));
  ui->editUrl->setText(m_pfirmware->getUrl().c_str());
  ui->editFormat->setText(m_pfirmware->getFormat().c_str());

  QDate dd = QDate::fromString(m_pfirmware->getDate().c_str(),"YY-MM_DD");
  ui->date->setDate(dd); 

  ui->editVersion->setText(m_pfirmware->getVersion().c_str());
  ui->editSize->setText(QString::number(m_pfirmware->getSize()));
  ui->editMd5->setText(m_pfirmware->getMd5().c_str());

  switch (index) {
    case index_firmware_name:
      ui->editName->setFocus();
      break;

    case index_firmware_target_code:
      ui->editTargetCode->setFocus();
      break;

    case index_firmware_format:
      ui->editFormat->setFocus();
      break;  

    case index_firmware_date:
      ui->date->setFocus();
      break;  

    case index_firmware_version:
      ui->editVersion->setFocus();
      break;  

    case index_firmware_size:
      ui->editSize->setFocus();
      break;  

    case index_firmware_md5:
      ui->editMd5->setFocus();
      break;  

    case index_firmware_none:
    default:
      ui->editName->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfFileFirmware::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgMdfFileFirmware::setName(const QString& name)
{
  ui->editName->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileFirmware::getName(void)
{
  return ui->editName->text();
}

///////////////////////////////////////////////////////////////////////////////
// setTargetCode
//

void
CDlgMdfFileFirmware::setTargetCode(uint16_t code)
{
  ui->editTargetCode->setText(QString::number(code));
}

///////////////////////////////////////////////////////////////////////////////
// getTargetCode
//

uint16_t
CDlgMdfFileFirmware::getTargetCode(void)
{
  return vscp_readStringValue(ui->editTargetCode->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void
CDlgMdfFileFirmware::setUrl(const QString& name)
{
  ui->editUrl->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString
CDlgMdfFileFirmware::getUrl(void)
{
  return ui->editUrl->text();
}

///////////////////////////////////////////////////////////////////////////////
// setFormat
//

void
CDlgMdfFileFirmware::setFormat(const QString& name)
{
  ui->editFormat->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileFirmware::getFormat(void)
{
  return ui->editFormat->text();
}


///////////////////////////////////////////////////////////////////////////////
// setDate
//

void
CDlgMdfFileFirmware::setDate(const QString& strdate)
{
  QDate dd = QDate::fromString(strdate,"YY-MM_DD");
  ui->date->setDate(dd); 
}

///////////////////////////////////////////////////////////////////////////////
// getDate
//

QString
CDlgMdfFileFirmware::getDate(void)
{
  QDate dd = ui->date->date(); 
  return dd.toString("YY-MM-DD");
}

///////////////////////////////////////////////////////////////////////////////
// setVersion
//

void
CDlgMdfFileFirmware::setVersion(const QString& ver)
{
  ui->editVersion->setText(ver);
}

///////////////////////////////////////////////////////////////////////////////
// getVersion
//

QString
CDlgMdfFileFirmware::getVersion(void)
{
  return ui->editVersion->text();
}

///////////////////////////////////////////////////////////////////////////////
// setSize
//

void
CDlgMdfFileFirmware::setSize(size_t size)
{
  ui->editSize->setText(QString::number(size));
}

///////////////////////////////////////////////////////////////////////////////
// getSize
//

size_t
CDlgMdfFileFirmware::getSize(void)
{
  return vscp_readStringValue(ui->editSize->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setMd5
//

void
CDlgMdfFileFirmware::setMd5(const QString& md5)
{
  ui->editMd5->setText(md5);
}

///////////////////////////////////////////////////////////////////////////////
// getMd5
//

QString
CDlgMdfFileFirmware::getMd5(void)
{
  return ui->editMd5->text();
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfFileFirmware::accept()
{
  std::string str;
  if (nullptr != m_pfirmware) {

    str = ui->editName->text().toStdString();
    m_pfirmware->setName(str);

    str = ui->editTargetCode->text().toStdString();
    m_pfirmware->setTargetCode(vscp_readStringValue(str));

    str = ui->editUrl->text().toStdString();
    m_pfirmware->setUrl(str);

    str = ui->editFormat->text().toStdString();
    m_pfirmware->setFormat(str);

    str = ui->editVersion->text().toStdString();
    m_pfirmware->setVersion(str);

    str = ui->editSize->text().toStdString();
    m_pfirmware->setSize(vscp_readStringValue(str));

    str = ui->editMd5->text().toStdString();
    m_pfirmware->setMd5(str);

    // str = ui->editBlockCount->text().toStdString();
    // m_pbootinfo->setBlockCount(vscp_readStringValue(str));

    // // m_pbootinfo->setModuleLevel(ui->comboModuleLevel->currentIndex());

    // int idx = ui->comboBoxAlgorithm->currentIndex(); //  ->text().toStdString();
    // m_pbootinfo->setAlgorithm(idx);
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
