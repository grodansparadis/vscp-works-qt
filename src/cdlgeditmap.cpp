// cdlgeditmap.cpp
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

#include "cdlgeditmap.h"
#include "ui_cdlgeditmap.h"

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

CDlgEditMap::CDlgEditMap(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgEditMap)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgEditMap::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  connect(ui->btnAddMapItem, &QToolButton::clicked, this, &CDlgEditMap::addMapItem);
  connect(ui->btnEditMapItem, &QToolButton::clicked, this, &CDlgEditMap::editMapItem);
  connect(ui->btnDupMapItem, &QToolButton::clicked, this, &CDlgEditMap::dupMapItem);
  connect(ui->btnDelMapItem, &QToolButton::clicked, this, &CDlgEditMap::deleteMapItem);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgEditMap::~CDlgEditMap()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgEditMap::initDialogData(std::map<std::string, std::string>* pmap, descriptionMap type, const QString* pselstr)
{
  QString str;

  if (nullptr == pmap) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pMap = pmap;

  fillMapData(pselstr);
}

///////////////////////////////////////////////////////////////////////////////
// fillMapData
//

void
CDlgEditMap::fillMapData(const QString* pselstr)
{
  QString str;

  // Fill in map data
  std::map<std::string, std::string>::iterator itMap = m_pMap->begin();
  while (itMap != m_pMap->end()) {
    std::string key   = itMap->first; // key
    std::string value = itMap->second;
    str               = key.c_str() + tr(" - ") + value.c_str();
    ui->listMap->addItem(str);
    itMap++;
  }

  // Select row as of MDF frame selection
  if ((nullptr != pselstr) && pselstr->length()) {
    for (int i = 0; i < ui->listMap->count(); i++) {
      QString selstr = ui->listMap->item(i)->text().left(2);
      qDebug() << selstr << " - " << *pselstr;
      if (selstr == *pselstr) {
        ui->listMap->setCurrentRow(i);
        ui->listMap->item(i)->setSelected(true);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillInfoUrl
//

// void
// CDlgEditMap::fillInfoUrl()
// {
//   QString str;

//   // Fill in help URL's
//   std::map<std::string, std::string>* pmapHelpUrl     = m_pmdf->getModuleHelpUrlMap();
//   std::map<std::string, std::string>::iterator itInfo = pmapHelpUrl->begin();
//   while (itInfo != pmapHelpUrl->end()) {
//     std::string lang = itInfo->first; // key
//     std::string info = itInfo->second;
//     str              = lang.c_str() + tr(" - ") + info.c_str();
//     ui->listInfo->addItem(str);
//     itInfo++;
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgEditMap::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgEditMap::accept()
{
  std::string str;
  if (nullptr != m_pMap) {

  }
  else {
    spdlog::error("MDF map information - Invalid map object (accept)");
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// addMapItem
//

void
CDlgEditMap::addMapItem(void)
{
  QString selstr = "en"; // Default language

  CDlgMdfDescription dlg(this);
  dlg.initDialogData(m_pMap);
  if (QDialog::Accepted == dlg.exec()) {
    ui->listMap->clear();
    fillMapData();
  }
}

///////////////////////////////////////////////////////////////////////////////
// editMapItem
//

void
CDlgEditMap::editMapItem(void)
{
  if (-1 != ui->listMap->currentRow()) {

    // Save the row
    int idx = ui->listMap->currentRow();

    QListWidgetItem* pitem = ui->listMap->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pMap, &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listMap->clear();
      fillMapData();
      ui->listMap->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupMapItem
//

void
CDlgEditMap::dupMapItem(void)
{
  if (-1 != ui->listMap->currentRow()) {
    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pMap);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listMap->clear();
      fillMapData();
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteMapItem
//

void
CDlgEditMap::deleteMapItem(void)
{
  if (-1 != ui->listMap->currentRow()) {

    // Save the row
    int idx = ui->listMap->currentRow();

    QListWidgetItem* pitem = ui->listMap->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    m_pMap->erase(selstr.toStdString());
    ui->listMap->clear();
    fillMapData();
  }
}

///////////////////////////////////////////////////////////////////////////////
// addInfo
//

// void
// CDlgEditMap::addInfo(void)
// {
//   QString selstr = "en"; // Default language

//   CDlgMdfInfoUrl dlg(this);
//   dlg.initDialogData(m_pmdf->getModuleHelpUrlMap()/*, &selstr*/);
//   if (QDialog::Accepted == dlg.exec()) {
//     ui->listInfo->clear();
//     fillInfoUrl();
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// editInfo
//

// void
// CDlgEditMap::editInfo(void)
// {
//   if (-1 != ui->listInfo->currentRow()) {

//     // Save the row
//     int idx = ui->listInfo->currentRow();

//     QListWidgetItem* pitem = ui->listInfo->currentItem();
//     QString selstr         = pitem->text().split('_').first().left(2);

//     CDlgMdfInfoUrl dlg(this);
//     dlg.initDialogData(m_pmdf->getModuleHelpUrlMap(), &selstr);
//     if (QDialog::Accepted == dlg.exec()) {
//       ui->listInfo->clear();
//       fillInfoUrl();
//       ui->listInfo->setCurrentRow(idx);
//     }
//   }
//   else {
//     QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// dupInfo
//

// void
// CDlgEditMap::dupInfo(void)
// {
//   if (-1 != ui->listInfo->currentRow()) {
//     CDlgMdfInfoUrl dlg(this);
//     dlg.initDialogData(m_pmdf->getModuleHelpUrlMap());
//     if (QDialog::Accepted == dlg.exec()) {
//       ui->listInfo->clear();
//       fillInfoUrl();
//     }
//   }
//   else {
//     QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// deleteInfo
//

// void
// CDlgEditMap::deleteInfo(void)
// {
//     if (-1 != ui->listInfo->currentRow()) {

//     // Save the row
//     int idx = ui->listInfo->currentRow();

//     QListWidgetItem* pitem = ui->listInfo->currentItem();
//     QString selstr         = pitem->text().split('_').first().left(2);

//     m_pmdf->getModuleHelpUrlMap()->erase(selstr.toStdString());
//     ui->listInfo->clear();
//     fillInfoUrl();

//   }
// }