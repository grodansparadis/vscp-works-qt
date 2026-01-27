// cdlgmdffilesetup.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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

#include "cdlgmdffilesetup.h"
#include "ui_cdlgmdffilesetup.h"

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

CDlgMdfFileSetup::CDlgMdfFileSetup(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfFileSetup)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Fill combo with predefined bootloader algorithms (vscp.h)
  // ui->comboBoxAlgorithm->addItem("VSCP", VSCP_BOOTLOADER_VSCP); // VSCP boot loader algorithm

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfFileSetup::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfFileSetup::~CDlgMdfFileSetup()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfFileSetup::initDialogData(const CMDF_Object* pmdfobj, mdf_file_setup_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_psetup = (CMDF_Setup*)pmdfobj;

  ui->editName->setText(m_psetup->getName().c_str());
  ui->editUrl->setText(m_psetup->getUrl().c_str());
  ui->editFormat->setText(m_psetup->getFormat().c_str());

  QDate dd = QDate::fromString(m_psetup->getDate().c_str(), "YY-MM_DD");
  ui->date->setDate(dd);

  switch (index) {
    case index_file_setup_url:
      ui->editUrl->setFocus();
      break;

    case index_file_setup_format:
      ui->editFormat->setFocus();
      break;

    case index_file_setup_date:
      ui->date->setFocus();
      break;

    case index_file_setup_name:
    default:
      ui->editName->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfFileSetup::setInitialFocus(void)
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
CDlgMdfFileSetup::setName(const QString& name)
{
  ui->editName->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileSetup::getName(void)
{
  return ui->editName->text();
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void
CDlgMdfFileSetup::setUrl(const QString& name)
{
  ui->editUrl->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString
CDlgMdfFileSetup::getUrl(void)
{
  return ui->editUrl->text();
}

///////////////////////////////////////////////////////////////////////////////
// setFormat
//

void
CDlgMdfFileSetup::setFormat(const QString& name)
{
  ui->editFormat->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileSetup::getFormat(void)
{
  return ui->editFormat->text();
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

void
CDlgMdfFileSetup::setDate(const QString& strdate)
{
  QDate dd = QDate::fromString(strdate, "YY-MM_DD");
  ui->date->setDate(dd);
}

///////////////////////////////////////////////////////////////////////////////
// getDate
//

QString
CDlgMdfFileSetup::getDate(void)
{
  QDate dd = ui->date->date();
  return dd.toString("YY-MM-DD");
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfFileSetup::accept()
{
  std::string str;
  if (nullptr != m_psetup) {

    str = ui->editName->text().toStdString();
    m_psetup->setName(str);

    str = ui->editUrl->text().toStdString();
    m_psetup->setUrl(str);

    str = ui->editFormat->text().toStdString();
    m_psetup->setFormat(str);

    QDate dd = ui->date->date();
    m_psetup->setDate(dd.toString("YY-MM-DD").toStdString());
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
CDlgMdfFileSetup::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}