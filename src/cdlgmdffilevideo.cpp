// cdlgmdffilevideo.cpp
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

#include "cdlgmdffilevideo.h"
#include "ui_cdlgmdffilevideo.h"

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

CDlgMdfFileVideo::CDlgMdfFileVideo(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfFileVideo)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Fill combo with predefined bootloader algorithms (vscp.h)
  // ui->comboBoxAlgorithm->addItem("VSCP", VSCP_BOOTLOADER_VSCP); // VSCP boot loader algorithm

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfFileVideo::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfFileVideo::~CDlgMdfFileVideo()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfFileVideo::initDialogData(const CMDF_Object* pmdfobj, mdf_file_video_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pvideo = (CMDF_Video*)pmdfobj;

  ui->editName->setText(m_pvideo->getName().c_str());
  ui->editUrl->setText(m_pvideo->getUrl().c_str());
  ui->editFormat->setText(m_pvideo->getFormat().c_str());

  QDate dd = QDate::fromString(m_pvideo->getDate().c_str(), "YY-MM_DD");
  ui->date->setDate(dd);

  switch (index) {
    case index_file_video_url:
      ui->editUrl->setFocus();
      break;

    case index_file_video_format:
      ui->editFormat->setFocus();
      break;

    case index_file_video_date:
      ui->date->setFocus();
      break;

    case index_file_video_name:
    default:
      ui->editName->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfFileVideo::setInitialFocus(void)
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
CDlgMdfFileVideo::setName(const QString& name)
{
  ui->editName->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileVideo::getName(void)
{
  return ui->editName->text();
}

///////////////////////////////////////////////////////////////////////////////
// setUrl
//

void
CDlgMdfFileVideo::setUrl(const QString& name)
{
  ui->editUrl->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getUrl
//

QString
CDlgMdfFileVideo::getUrl(void)
{
  return ui->editUrl->text();
}

///////////////////////////////////////////////////////////////////////////////
// setFormat
//

void
CDlgMdfFileVideo::setFormat(const QString& name)
{
  ui->editFormat->setText(name);
}

///////////////////////////////////////////////////////////////////////////////
// getFormat
//

QString
CDlgMdfFileVideo::getFormat(void)
{
  return ui->editFormat->text();
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

void
CDlgMdfFileVideo::setDate(const QString& strdate)
{
  QDate dd = QDate::fromString(strdate, "YY-MM_DD");
  ui->date->setDate(dd);
}

///////////////////////////////////////////////////////////////////////////////
// getDate
//

QString
CDlgMdfFileVideo::getDate(void)
{
  QDate dd = ui->date->date();
  return dd.toString("YY-MM-DD");
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfFileVideo::accept()
{
  std::string str;
  if (nullptr != m_pvideo) {

    str = ui->editName->text().toStdString();
    m_pvideo->setName(str);

    str = ui->editUrl->text().toStdString();
    m_pvideo->setUrl(str);

    str = ui->editFormat->text().toStdString();
    m_pvideo->setFormat(str);

    QDate dd = ui->date->date();
    m_pvideo->setDate(dd.toString("YY-MM-DD").toStdString());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
