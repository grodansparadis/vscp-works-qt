// cdlgbootfirmware.cpp
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

#include "cdlgbootfirmware.h"
#include "ui_cdlgbootfirmware.h"

#include <QDate>
#include <QDebug>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgBootFirmware::CDlgBootFirmware(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgBootFirmware)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgBootFirmware::editDesc);

  connect(ui->chkLocal, &QCheckBox::toggled, this, &CDlgBootFirmware::chkLocalFile);
  connect(ui->listFirmware, &QListWidget::itemClicked, this, &CDlgBootFirmware::selectFirmwareFile);
  connect(ui->listFirmware, &QListWidget::itemDoubleClicked, this, &CDlgBootFirmware::accept);
  connect(ui->BtnSelectFile, &QPushButton::clicked, this, &CDlgBootFirmware::selectLocalFile);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgBootFirmware::~CDlgBootFirmware()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgBootFirmware::initDialogData(CMDF* pmdf)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  // Fill MDF firmware info  std::deque<CMDF_Firmware *> *
  std::deque<CMDF_Firmware*>* pfirmwarelst = m_pmdf->getFirmwareObjList();
  std::deque<CMDF_Firmware*>::iterator it  = pfirmwarelst->begin();

  for (auto* pFirmware : *m_pmdf->getFirmwareObjList()) {
    QString itemstr = pFirmware->getName().c_str();
    if (!itemstr.size()) {
      itemstr = "No name";
    }
    std::cout << itemstr.toStdString() << "\n";
    itemstr += " - Target: ";
    itemstr += pFirmware->getTarget().c_str();
    itemstr += " - Target Code: ";
    itemstr += QString::number(pFirmware->getTargetCode()).toStdString().c_str();
    itemstr += " - Version: ";
    itemstr += pFirmware->getVersion().c_str();
    ui->listFirmware->addItem(itemstr);
  }

  // ui->listFirmware->setCurrentRow(0);
}

///////////////////////////////////////////////////////////////////////////////
// chkLocalFile
//

void
CDlgBootFirmware::chkLocalFile(bool checked)
{
  if (checked) {
    ui->editPath->setEnabled(true);
    ui->BtnSelectFile->setEnabled(true);
    ui->comboFormat->setEnabled(true);
    ui->comboFormat->setCurrentIndex(0);
  }
  else {
    ui->editPath->setEnabled(false);
    ui->BtnSelectFile->setEnabled(false);
    ui->comboFormat->setEnabled(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// selectFirmwareFile
//

void
CDlgBootFirmware::selectFirmwareFile(QListWidgetItem* item)
{
  QString str;
  CMDF_Firmware* pFirmware = m_pmdf->getFirmwareObj(ui->listFirmware->currentRow());

  ui->textFirmwareInfo->clear();
  str = "<b>Name:</b> <font color=\"green\">";
  str += pFirmware->getName().c_str();
  str += "</font><br>";
  str += "<b>URL:</b> <a href=\"";
  str += pFirmware->getUrl().c_str();
  str += "\">";
  str += pFirmware->getUrl().c_str();
  str += "</a><br>";
  str += "<b>Version:</b>  <font color=\"green\">";
  str += pFirmware->getVersion().c_str();
  str += "</font><br>";
  str += "<b>Target:</b>  <font color=\"green\">";
  str += pFirmware->getTarget().c_str();
  str += "</font><br>";
  str += "<b>Targetcode</b> =  <font color=\"green\">%0</font>";
  str = str.arg(pFirmware->getTargetCode());
  str += "</font><br>";
  str += "<b>Format:</b>  <font color=\"green\">";
  str += pFirmware->getFormat().c_str();
  str += "</font><br>";
  str += "<b>Date:</b>  <font color=\"green\">";
  str += pFirmware->getDate().c_str();
  str += "</font><br>";
  str += "<b>Size:</b>  <font color=\"green\">%0</font>";
  str = str.arg(pFirmware->getSize());
  str += "</font><br>";
  str += "<b>MD5 Checksum:</b>  <font color=\"green\">";
  str += pFirmware->getMd5().c_str();
  str += "</font><br>";

  ui->textFirmwareInfo->setHtml(str);
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// isLocalFile
//

bool
CDlgBootFirmware::isLocalFile()
{
  return ui->chkLocal->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// getLocalFilePath
//

QString
CDlgBootFirmware::getLocalFilePath(void)
{
  return ui->editPath->text();
}

///////////////////////////////////////////////////////////////////////////////
// getFileType
//

QString
CDlgBootFirmware::getFileType(void)
{
  switch (ui->comboFormat->currentIndex()) {
    
    case 1:
      return "IHEX16";

    case 2:
      return "IHEX32";

    case 0:
    default:   
      return "IHEX8";  
  }
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedItem
//

CMDF_Firmware*
CDlgBootFirmware::getSelectedItem(void)
{
  if (-1 == ui->listFirmware->currentRow()) {
    return nullptr;
  }
  return m_pmdf->getFirmwareObj(ui->listFirmware->currentRow());
}

///////////////////////////////////////////////////////////////////////////////
// selectLocalFile
//

void
CDlgBootFirmware::selectLocalFile(void)
{
  QString path = QFileDialog::getOpenFileName(this,
                                              tr("Set local firmware file"),
                                              ui->editPath->text(),
                                              tr("Firmware (*.hex *.mcs *.int *.ihex *ihe *.ihx);;Text (*.txt);;All (*.*)"));
  ui->editPath->setText(path);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgBootFirmware::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {
    ;
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
