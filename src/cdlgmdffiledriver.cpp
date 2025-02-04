// cdlgmdfdriver.cpp
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
#include "ui_cdlgmdffiledriver.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfFileDriver::CDlgMdfFileDriver(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfFileDriver)
{
  ui->setupUi(this);

  // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfFileDriver::~CDlgMdfFileDriver()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfFileDriver::initDialogData(const CMDF_Object* pmdfobj, mdf_driver_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pdriver = (CMDF_Driver*)pmdfobj;

  ui->editName->setText(m_pdriver->getName().c_str());
  ui->editUrl->setText(m_pdriver->getUrl().c_str());
  ui->editType->setText(m_pdriver->getType().c_str());
  ui->editOs->setText(m_pdriver->getOS().c_str());
  ui->editArchitecture->setText(m_pdriver->getArchitecture().c_str());
  ui->editOsVersion->setText(m_pdriver->getOSVer().c_str());
  ui->editVersion->setText(m_pdriver->getVersion().c_str());

  QDate dd = QDate::fromString(m_pdriver->getDate().c_str(), "YY-MM_DD");
  ui->date->setDate(dd);

  switch (index) {
    case index_file_driver_name:
      ui->editName->setFocus();
      break;

    case index_file_driver_url:
      ui->editUrl->setFocus();
      break;

    case index_file_driver_type:
      ui->editType->setFocus();
      break;

    case index_file_driver_os:
      ui->editOs->setFocus();
      break;

    case index_file_driver_architecture:
      ui->editArchitecture->setFocus();
      break;

    case index_file_driver_os_version:
      ui->editOsVersion->setFocus();
      break;

    case index_file_driver_date:
      ui->date->setFocus();
      break;

    case index_file_driver_version:
      ui->editVersion->setFocus();
      break;

    case index_file_driver_md5:
      ui->editMd5->setFocus();
      break;  

    case index_file_driver_none:
    default:
      ui->editName->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfFileDriver::setInitialFocus(void)
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
CDlgMdfFileDriver::setName(const QString& name)
{
  ui->editName->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileDriver::getName(void)
{
  return ui->editName->text();
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void
CDlgMdfFileDriver::setUrl(const QString& name)
{
  ui->editUrl->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString
CDlgMdfFileDriver::getUrl(void)
{
  return ui->editUrl->text();
}

///////////////////////////////////////////////////////////////////////////////
// setType
//

void
CDlgMdfFileDriver::setType(const QString& name)
{
  ui->editType->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getType
//

QString
CDlgMdfFileDriver::getType(void)
{
  return ui->editType->text();
}

///////////////////////////////////////////////////////////////////////////////
// setOs
//

void
CDlgMdfFileDriver::setOs(const QString& name)
{
  ui->editOs->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getOs
//

QString
CDlgMdfFileDriver::getOs(void)
{
  return ui->editOs->text();
}

///////////////////////////////////////////////////////////////////////////////
// setArchitecture
//

void
CDlgMdfFileDriver::setArchitecture(const QString& name)
{
  ui->editArchitecture->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getArchitecture
//

QString
CDlgMdfFileDriver::getArchitecture(void)
{
  return ui->editArchitecture->text();
}

///////////////////////////////////////////////////////////////////////////////
// setOsVersion
//

void
CDlgMdfFileDriver::setOsVersion(const QString& name)
{
  ui->editOsVersion->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getOsVersion
//

QString
CDlgMdfFileDriver::getOsVersion(void)
{
  return ui->editOsVersion->text();
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

void
CDlgMdfFileDriver::setDate(const QString& strdate)
{
  QDate dd = QDate::fromString(strdate, "YY-MM_DD");
  ui->date->setDate(dd);
}

///////////////////////////////////////////////////////////////////////////////
// getDate
//

QString
CDlgMdfFileDriver::getDate(void)
{
  QDate dd = ui->date->date();
  return dd.toString("YY-MM-DD");
}

///////////////////////////////////////////////////////////////////////////////
// setVersion
//

void
CDlgMdfFileDriver::setVersion(const QString& ver)
{
  ui->editVersion->setText(ver);
}

///////////////////////////////////////////////////////////////////////////////
// getVersion
//

QString
CDlgMdfFileDriver::getVersion(void)
{
  return ui->editVersion->text();
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfFileDriver::accept()
{
  std::string str;
  if (nullptr != m_pdriver) {

    str = ui->editName->text().toStdString();
    m_pdriver->setName(str);

    str = ui->editUrl->text().toStdString();
    m_pdriver->setUrl(str);

    str = ui->editType->text().toStdString();
    m_pdriver->setType(str);

    str = ui->editOs->text().toStdString();
    m_pdriver->setOS(str);

    str = ui->editArchitecture->text().toStdString();
    m_pdriver->setArchitecture(str);

    str = ui->editOsVersion->text().toStdString();
    m_pdriver->setVersion(str);

    str = ui->editMd5->text().toStdString();
    m_pdriver->setMd5(str);

    QDate dd = ui->date->date();
    m_pdriver->setDate(dd.toString("YY-MM-DD").toStdString());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMdfFileDriver::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}