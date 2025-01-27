// cdlgmdfevent.cpp
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

#include "cdlgmdfeventdata.h"

#include "cdlgmdfevent.h"
#include "ui_cdlgmdfevent.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfEvent::pre_str_event[] = "Events: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfEvent::CDlgMdfEvent(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfEvent)
{
  ui->setupUi(this);

  m_pEvent = nullptr;

  // Connect edit action
  connect(ui->btnEdit,
          SIGNAL(clicked()),
          this,
          SLOT(editEventData()));

  // Connect add action
  connect(ui->btnAdd,
          SIGNAL(clicked()),
          this,
          SLOT(addEventData()));

  // Connect dup action
  connect(ui->btnDup,
          SIGNAL(clicked()),
          this,
          SLOT(dupEventData()));

  // Connect dup action
  connect(ui->btnDelete,
          SIGNAL(clicked()),
          this,
          SLOT(deleteEventData()));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfEvent::~CDlgMdfEvent()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfEvent::initDialogData(CMDF* pmdf, CMDF_Event* pevent, bool bAdd, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == pevent) {
    spdlog::error("MDF Event - Invalid MDF register object (initDialogData)");
    return;
  }

  m_pEvent = pevent;

  m_bAdd = bAdd;

  // Connect edit action
  connect(ui->btnEdit,
          SIGNAL(clicked()),
          this,
          SLOT(editAction()));

  // Connect add action
  connect(ui->btnAdd,
          SIGNAL(clicked()),
          this,
          SLOT(addAction()));

  // Connect dup action
  connect(ui->btnDup,
          SIGNAL(clicked()),
          this,
          SLOT(dupAction()));

  // Connect dup action
  connect(ui->btnDelete,
          SIGNAL(clicked()),
          this,
          SLOT(delAction()));

  connect(ui->btnClassHelp, &QToolButton::clicked, this, &CDlgMdfEvent::showVscpClassInfo);
  connect(ui->btnClassHelp, &QToolButton::clicked, this, &CDlgMdfEvent::showVscpTypeInfo);

  // Connect VSCP Class change
  connect(ui->comboClass,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &CDlgMdfEvent::currentVscpClassIndexChanged);

  setName(pevent->getName().c_str());
  setVscpClassType(pevent->getClass(), pevent->getType());
  setPriority(pevent->getPriority());
  setDirection(pevent->getDirection());

  fillVscpClass(pevent->getClass());

  // Render defined event data
  renderEventData();

  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_class:
      ui->comboClass->setFocus();
      break;

    case index_type:
      ui->comboType->setFocus();
      break;

    case index_priority:
      ui->comboPriority->setFocus();
      break;

    case index_direction:
      ui->comboDirection->setFocus();
      break;

    case index_data:
      ui->listEventData->setFocus();
      break;

    default:
      ui->editName->setFocus();
      break;
  }

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// setReadOnly
//

void
CDlgMdfEvent::setReadOnly(void)
{
  ui->editName->setEnabled(false);
  ui->comboClass->setEnabled(false);
  ui->comboType->setEnabled(false);
  ui->comboPriority->setEnabled(false);
  ui->comboDirection->setEnabled(false);
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfEvent::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfEvent::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

uint8_t
CDlgMdfEvent::getPriority(void)
{
  return ui->comboPriority->currentIndex();
};

void
CDlgMdfEvent::setPriority(uint8_t priority)
{
  ui->comboPriority->setCurrentIndex(priority & 7);
};

// -----------------------------------------------------------------------

mdf_event_direction
CDlgMdfEvent::getDirection(void)
{
  return static_cast<mdf_event_direction>(ui->comboDirection->currentIndex());
};

void
CDlgMdfEvent::setDirection(mdf_event_direction dir)
{
  ui->comboDirection->setCurrentIndex(static_cast<int>(dir));
};

///////////////////////////////////////////////////////////////////////////////
// setVscpClassType
//

void
CDlgMdfEvent::setVscpClassType(uint16_t vscpClass, uint16_t vscpType)
{
  ui->comboClass->clear();
  ui->comboType->clear();

  // Select requested VSCP Class item
  for (int i = 0; i < ui->comboClass->count(); i++) {
    if (vscpClass == ui->comboClass->itemData(i).toInt()) {
      ui->comboClass->setCurrentIndex(i);
      break;
    }
  }

  // Select requested VSCP Type item
  for (int i = 0; i < ui->comboType->count(); i++) {
    if (vscpType == ui->comboType->itemData(i).toInt()) {
      ui->comboType->setCurrentIndex(i);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClass
//

uint16_t
CDlgMdfEvent::getVscpClass(void)
{
  int selected = ui->comboClass->currentIndex();
  return ui->comboClass->itemData(selected).toInt();
}

///////////////////////////////////////////////////////////////////////////////
// getVscpType
//

uint16_t
CDlgMdfEvent::getVscpType(void)
{
  int selected = ui->comboType->currentIndex();
  return ui->comboType->itemData(selected).toInt();
}

// -----------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// renderEventData
//

void
CDlgMdfEvent::renderEventData(void)
{
  if (nullptr == m_pEvent) {
    return;
  }

  ui->listEventData->clear();

  std::deque<CMDF_EventData*>* pEventDataList = m_pEvent->getListEventData();

  if (nullptr != pEventDataList) {
    for (int i = 0; i < pEventDataList->size(); ++i) {
      CMDF_EventData* peventdata = (*pEventDataList)[i];
      if (nullptr != peventdata) {
        QString str            = QString("Event data: %1 - %2").arg(peventdata->getOffset()).arg(peventdata->getName().c_str());
        QListWidgetItem* pitem = new QListWidgetItem(str, ui->listEventData);
        if (nullptr != pitem) {
          pitem->setData(QListWidgetItem::UserType, peventdata->getOffset());
          ui->listEventData->addItem(pitem);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// showVscpClassInfo
//

void
CDlgMdfEvent::showVscpClassInfo()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->getHelpUrlForClass(getVscpClass());
  QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}

///////////////////////////////////////////////////////////////////////////////
// showVscpClassInfo
//

void
CDlgMdfEvent::showVscpTypeInfo()
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->getHelpUrlForType(getVscpClass(), getVscpType());
  QDesktopServices::openUrl(QUrl(str, QUrl::TolerantMode));
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpClass
//

void
CDlgMdfEvent::fillVscpClass(uint16_t vscpclass)
{
  int selidx;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Clear all items
  ui->comboType->clear();

  std::map<uint16_t, QString>::iterator it;
  for (it = pworks->m_mapVscpClassToToken.begin();
       it != pworks->m_mapVscpClassToToken.end();
       ++it) {

    uint16_t classId   = it->first;
    QString classToken = it->second;

    QString listItem =
      vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
    listItem +=
      vscp_str_format(" -- (%d / 0x%04x)", (int)classId, (int)classId)
        .c_str();
    ui->comboClass->addItem(listItem, classId);

    // Save index for requested sel
    if (classId == vscpclass) {
      selidx = ui->comboClass->count() - 1;
    }
  }

  // Select the requested item
  ui->comboClass->setCurrentIndex(selidx);
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpType
//

void
CDlgMdfEvent::fillVscpType(uint16_t vscpclass, uint16_t vscptype)
{
  int selidx;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Clear all items
  ui->comboType->clear();

  std::map<uint32_t, QString>::iterator it;
  for (it = pworks->m_mapVscpTypeToToken.begin();
       it != pworks->m_mapVscpTypeToToken.end();
       ++it) {

    uint16_t classId  = (it->first >> 16) & 0xffff;
    uint16_t typeId   = it->first & 0xfff;
    QString typeToken = it->second;

    if (classId == vscpclass) {
      QString listItem =
        vscp_str_format(
          "%s ",
          typeToken.toStdString().c_str())
          .c_str();
      // while (listItem.length() < 30) listItem += " ";
      listItem +=
        vscp_str_format(" -- (%d / 0x%04x)", (int)typeId, (int)typeId)
          .c_str();
      ui->comboType->addItem(listItem, typeId);

      // Save index for requested sel
      if (typeId == vscptype) {
        selidx = ui->comboType->count() - 1;
      }
    }
  }

  // Select the requested item
  ui->comboType->setCurrentIndex(selidx);
}

///////////////////////////////////////////////////////////////////////////////
// currentVscpClassIndexChanged
//

void
CDlgMdfEvent::currentVscpClassIndexChanged(int index)
{
  int selected = ui->comboClass->currentIndex();
  fillVscpType(ui->comboClass->itemData(selected).toInt(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// editEventData
//

void
CDlgMdfEvent::editEventData(void)
{
  // bool ok;
  uint8_t offset; // set to offset of item before edit

  // Save the selected row
  int idx = ui->listEventData->currentRow();

  QListWidgetItem* pitem = ui->listEventData->currentItem();
  if (nullptr == pitem) {
    return;
  }

  CMDF_EventData* pEventData = m_pEvent->getEventData(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == pEventData) {
    return;
  }

  // save origonal offset
  offset = pEventData->getOffset();

  CDlgMdfEventData dlg(this);
adddlg:
  dlg.initDialogData(m_pmdf, pEventData);
  // If DM is level I only offset 0 is allowd
  if (VSCP_LEVEL1 == m_pmdf->getLevel()) {
    dlg.setLevel1();
  }

  if (QDialog::Accepted == dlg.exec()) {

    // If offset has been changed we must make sure its
    // unique
    if (offset != pEventData->getOffset()) {
      // Event data offset must be unique
      if (!m_pEvent->isEventDataOffsetUnique(pEventData)) {
        QMessageBox::warning(this, tr(APPNAME), tr("Offset is already used. Must be unique"));
        goto adddlg;
      }
    }

    ui->listEventData->clear();
    renderEventData();
    ui->listEventData->setCurrentRow(idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
// addEventData
//

void
CDlgMdfEvent::addEventData(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listEventData->currentRow();

  CMDF_EventData* pEventDataNew = new (CMDF_EventData);
  if (nullptr == pEventDataNew) {
    return;
  }

  CDlgMdfEventData dlg(this);
adddlg:
  dlg.initDialogData(m_pmdf, pEventDataNew);
  // If DM is level I only offset 0 is allowd
  if (VSCP_LEVEL1 == m_pmdf->getLevel()) {
    dlg.setLevel1();
  }
  if (QDialog::Accepted == dlg.exec()) {

    // Event data offset must be unique
    if (!m_pEvent->isEventDataOffsetUnique(pEventDataNew->getOffset())) {
      QMessageBox::warning(this, tr(APPNAME), tr("Offset is already used. Must be unique"));
      goto adddlg;
    }

    if (!m_pEvent->addEventData(pEventDataNew)) {
      QMessageBox::critical(this, tr(APPNAME), tr("Unable to add event data."));
    }
    ui->listEventData->clear();
    renderEventData();
    ui->listEventData->setCurrentRow(idx);
  }
  else {
    delete pEventDataNew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupEventData
//

void
CDlgMdfEvent::dupEventData(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listEventData->currentRow();

  QListWidgetItem* pitem = ui->listEventData->currentItem();
  if (nullptr == pitem) {
    return;
  }

  CMDF_EventData* pEventDataNew = new CMDF_EventData();
  if (nullptr == pEventDataNew) {
    return;
  }

  CMDF_EventData* pEventData = m_pEvent->getEventData(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == pEventData) {
    return;
  }

  pEventDataNew->setName(pEventData->getName().c_str());
  pEventDataNew->setOffset(pEventData->getOffset() + 1);

  CDlgMdfEventData dlg(this);
adddlg:
  dlg.initDialogData(m_pmdf, pEventDataNew);
  // If DM is level I only offset 0 is allowd
  if (VSCP_LEVEL1 == m_pmdf->getLevel()) {
    dlg.setLevel1();
  }
  if (QDialog::Accepted == dlg.exec()) {

    // Event data offset must be unique
    if (!m_pEvent->isEventDataOffsetUnique(pEventDataNew->getOffset())) {
      QMessageBox::warning(this, tr(APPNAME), tr("Offset is already used. Must be unique"));
      goto adddlg;
    }

    if (!m_pEvent->addEventData(pEventDataNew)) {
      QMessageBox::critical(this, tr("MDF add new event data"), tr("Unable to add event data."));
    }
    ui->listEventData->clear();
    renderEventData();
    ui->listEventData->setCurrentRow(idx);
  }
  else {
    delete pEventDataNew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteEventData
//

void
CDlgMdfEvent::deleteEventData(void)
{
  // bool ok;

  // Save the selected row
  int idx = ui->listEventData->currentRow();

  QListWidgetItem* pitem     = ui->listEventData->currentItem();
  CMDF_EventData* pEventData = m_pEvent->getEventData(pitem->data(QListWidgetItem::UserType).toUInt());
  if (nullptr == pEventData) {
    return;
  }

  if (QMessageBox::No == QMessageBox::question(this,
                                               tr("MDF delete event data item"),
                                               tr("Delete event data with offset %1.").arg(pEventData->getOffset()))) {
    return;
  }

  if (!m_pEvent->deleteEventData(pEventData)) {
    QMessageBox::warning(this, tr(APPNAME), tr("Failed to remove event data with offset %1.").arg(pEventData->getOffset()));
  }

  renderEventData();
  ui->listEventData->setCurrentRow(idx + 1);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfEvent::accept()
{
  std::string str;

  str = vscp_trim_copy(getName().toStdString()).c_str();
  if (!str.size()) {
    QMessageBox::warning(this, tr(APPNAME), tr("Events must have a name"), QMessageBox::Ok);
    return;
  }

  if (nullptr != m_pEvent) {
    m_pEvent->setName(getName().toStdString());
    m_pEvent->setClass(getVscpClass());
    m_pEvent->setType(getVscpType());
    m_pEvent->setPriority(ui->comboPriority->currentIndex());
    m_pEvent->setDirection(static_cast<mdf_event_direction>(ui->comboDirection->currentIndex()));
    if (m_bAdd) {
      m_pmdf->addEvent(m_pEvent);
    }
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
