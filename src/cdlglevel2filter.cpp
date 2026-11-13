// cdlglevel2filter.cpp
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

#include "vscphelper.h"
#include "vscpworks.h"

#include "cdlglevel2filter.h"
#include "ui_cdlglevel2filter.h"

#include <QMessageBox>

#include <set>
#include <QMenu>
#include <QDesktopServices>
#include <QShortcut>
#include <QDebug>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel2Filter::CDlgLevel2Filter(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgLevel2Filter)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_baseIndex       = pworks->m_base;
  ui->comboNumberBase->setCurrentIndex(static_cast<int>(m_baseIndex));
  onBaseChange(static_cast<int>(m_baseIndex));

  connect(ui->btnTransferRight,
          &QPushButton::clicked,
          this,
          &CDlgLevel2Filter::transferToVisual);
  connect(ui->btnTransferLeft,
          &QPushButton::clicked,
          this,
          &CDlgLevel2Filter::transferFromVisual);

  connect(ui->btnClearFilter,
          &QPushButton::clicked,
          this,
          &CDlgLevel2Filter::clearFilter);

  connect(ui->comboNumberBase,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(onBaseChange(int)));

  connect(ui->editVscpPriorityFilter,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedPriorityFilter);

  connect(ui->editVscpPriorityMask,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedPriorityMask);

  connect(ui->editVscpClassFilter,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedVscpClassFilter);

  connect(ui->editVscpClassMask,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedVscpClassMask);

  connect(ui->editVscpTypeFilter,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedVscpTypeFilter);

  connect(ui->editVscpTypeMask,
          &QLineEdit::textChanged,
          this,
          &CDlgLevel2Filter::onTextChangedVscpTypeMask);

  // Open pop up menu on right click on priority listbox
  connect(ui->listPriority,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgLevel2Filter::showPriorityContextMenu);

  // Open pop up menu on right click on VSCP class listbox
  connect(ui->listClass,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgLevel2Filter::showVscpClassContextMenu);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listType,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgLevel2Filter::showVscpTypeContextMenu);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listType,
          &QListWidget::itemClicked,
          this,
          &CDlgLevel2Filter::onVscpTypeItemClicked);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  // Add items to the listboxes
  fillPriorities();
  fillVscpClasses();
  fillVscpTypes();

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgLevel2Filter::~CDlgLevel2Filter()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgLevel2Filter::setInitialFocus(void)
{
  ui->editVscpPriorityFilter->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setNumBaseComboIndex
//

void
CDlgLevel2Filter::setNumBaseComboIndex(uint8_t index)
{
  if (index > 3)
    index = 0;
  ui->comboNumberBase->setCurrentIndex(index);
}

///////////////////////////////////////////////////////////////////////////////
// getNumComboIndex
//

uint8_t
CDlgLevel2Filter::getNumComboIndex(void)
{
  return ui->comboNumberBase->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void
CDlgLevel2Filter::onBaseChange(int index)
{
  int base = 10;
  QString qstr;
  QString prefix;
  numerical_base numbase = static_cast<numerical_base>(index);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  switch (numbase) {
    case numerical_base::HEX:
      prefix = "0x";
      base   = 16;
      break;
    case numerical_base::DECIMAL:
    default:
      prefix = "";
      base   = 10;
      break;
    case numerical_base::OCTAL:
      prefix = "0o";
      base   = 8;
      break;
    case numerical_base::BINARY:
      prefix = "0b";
      base   = 2;
      break;
  }

  m_bSkipAutomaticUpdate = true; // No automatic selectins

  qstr = prefix +
         QString::number(vscp_readStringValue(
                           ui->editVscpPriorityFilter->text().toStdString()),
                         base);
  ui->editVscpPriorityFilter->setText(qstr);

  qstr = prefix +
         QString::number(vscp_readStringValue(
                           ui->editVscpPriorityMask->text().toStdString()),
                         base);
  ui->editVscpPriorityMask->setText(qstr);

  qstr =
    prefix + QString::number(vscp_readStringValue(
                               ui->editVscpClassFilter->text().toStdString()),
                             base);

  ui->editVscpClassFilter->setText(qstr);

  qstr = prefix +
         QString::number(
           vscp_readStringValue(ui->editVscpClassMask->text().toStdString()),
           base);

  ui->editVscpClassMask->setText(qstr);

  qstr = prefix +
         QString::number(
           vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()),
           base);

  ui->editVscpTypeFilter->setText(qstr);

  qstr = prefix +
         QString::number(
           vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()),
           base);

  ui->editVscpTypeMask->setText(qstr);

  m_bSkipAutomaticUpdate = false; // Enable automatic selections again

  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// fillPriorities
//

void
CDlgLevel2Filter::fillPriorities(void)
{
  QStringList priorities = { "Priority 0 Highest", "Priority 1", "Priority 2", "Priority 3 Normal", "Priority 4", "Priority 5", "Priority 6", "Priority 7 Lowest" };
  ui->listPriority->addItems(priorities);

  // Clear selections
  ui->listPriority->setCurrentRow(0, QItemSelectionModel::Clear);
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpClasses
//

void
CDlgLevel2Filter::fillVscpClasses(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Clear selections
  ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);

  int i = 0;
  std::map<uint16_t, QString>::iterator it;
  for (it = pworks->m_mapVscpClassToToken.begin();
       it != pworks->m_mapVscpClassToToken.end();
       ++it) {

    uint16_t classId   = it->first;
    QString classToken = it->second;

    QString listItem =
      vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
    // while (listItem.length() < 30) listItem += " ";
    listItem +=
      vscp_str_format(" -- (%d / 0x%04x)", (int)classId, (int)classId)
        .c_str();
    QListWidgetItem* item = new QListWidgetItem(listItem, ui->listClass);
    // QVariant val(classId);
    item->setData(Qt::UserRole, classId);
    ui->listClass->addItem(item);
    // m_classToIndexVector.push_back(classId);
    // qDebug() << i << " " << classId;
    m_classToIndexMap[classId] = i;
    i++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpTypes
//

void
CDlgLevel2Filter::fillVscpTypes(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Clear selections
  ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);

  int i = 0;
  std::map<uint32_t, QString>::iterator it;
  for (it = pworks->m_mapVscpTypeToToken.begin();
       it != pworks->m_mapVscpTypeToToken.end();
       ++it) {

    uint16_t classId  = (it->first >> 16) & 0xffff;
    uint16_t typeId   = it->first & 0xffff;
    QString typeToken = it->second;

    QString listItem =
      vscp_str_format(
        "%s ",
        /*pworks->mapVscpClassToToken[classId].toStdString().c_str(),*/
        typeToken.toStdString().c_str())
        .c_str();
    // while (listItem.length() < 30) listItem += " ";
    listItem +=
      vscp_str_format(" -- (%d / 0x%04x)", (int)typeId, (int)typeId)
        .c_str();
    QListWidgetItem* item = new QListWidgetItem(listItem, ui->listType);
    // Class in the upper 16 bits, type in the lower
    item->setData(Qt::UserRole, it->first);
    ui->listType->addItem(item);
    // m_classToIndexVector.push_back(classId);
    // qDebug() << i << " " << classId << " " << typeId;
    m_typeToIndexMap[it->first] = i;
    i++;
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showPriorityContextMenu
//

void
CDlgLevel2Filter::showPriorityContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString("Clear selections"), this, SLOT(clrAllPrioritySelections()));
  menu->addAction(QString("Select all"), this, SLOT(selectAllPrioritySelections()));
  menu->addAction(QString("Write left"), this, SLOT(calculatePriorityValues()));

  menu->popup(ui->listPriority->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllPrioritySelections
//

void
CDlgLevel2Filter::clrAllPrioritySelections(void)
{
  for (int i = 0; i < ui->listPriority->count(); ++i) {
    QListWidgetItem* item = ui->listPriority->item(i);
    item->setSelected(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// selectAllPrioritySelections
//

void
CDlgLevel2Filter::selectAllPrioritySelections(void)
{
  for (int i = 0; i < ui->listPriority->count(); ++i) {
    QListWidgetItem* item = ui->listPriority->item(i);
    item->setSelected(true);
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showVscpClassContextMenu
//

void
CDlgLevel2Filter::showVscpClassContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString("Clear selections"), this, SLOT(clrAllVscpClassSelections()));
  menu->addAction(QString("Select all"), this, SLOT(selectAllVscpClassSelections()));
  menu->addAction(QString("Write left"), this, SLOT(calculateVscpClassValues()));

  menu->popup(ui->listClass->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllVscpClassSelections
//

void
CDlgLevel2Filter::clrAllVscpClassSelections(void)
{
  for (int i = 0; i < ui->listClass->count(); ++i) {
    QListWidgetItem* item = ui->listClass->item(i);
    item->setSelected(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// selectAllVscpClassSelections
//

void
CDlgLevel2Filter::selectAllVscpClassSelections(void)
{
  for (int i = 0; i < ui->listClass->count(); ++i) {
    QListWidgetItem* item = ui->listClass->item(i);
    item->setSelected(true);
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showVscpTypeContextMenu
//

void
CDlgLevel2Filter::showVscpTypeContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString("Clear selections"), this, SLOT(clrAllVscpTypeSelections()));
  menu->addAction(QString("Select all"), this, SLOT(selectAllVscpTypeSelections()));
  menu->addAction(QString("Write left"), this, SLOT(calculateVscpTypeValues()));

  menu->popup(ui->listType->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllVscpTypeSelections
//

void
CDlgLevel2Filter::clrAllVscpTypeSelections(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  for (int i = 0; i < ui->listType->count(); ++i) {
    QListWidgetItem* item = ui->listType->item(i);
    item->setSelected(false);
  }
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// selectAllVscpTypeSelections
//

void
CDlgLevel2Filter::selectAllVscpTypeSelections(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  for (int i = 0; i < ui->listType->count(); ++i) {
    QListWidgetItem* item = ui->listType->item(i);
    item->setSelected(true);
  }
  QApplication::restoreOverrideCursor();
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// transferToVisual
//

void
CDlgLevel2Filter::transferToVisual(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  doPrioritySelections();
  doVscpClassSelections();
  doVscpTypeSelections();
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// transferFromVisual
//

void
CDlgLevel2Filter::transferFromVisual(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  addClassesOfSelectedTypes();
  calculatePriorityValues();
  calculateVscpClassValues();
  calculateVscpTypeValues();
  QApplication::restoreOverrideCursor();
  checkFilterCoverage();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityFilter
//

void
CDlgLevel2Filter::onTextChangedPriorityFilter(const QString& text)
{
  if (!m_bSkipAutomaticUpdate) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    doPrioritySelections();
    QApplication::restoreOverrideCursor();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityMask
//

void
CDlgLevel2Filter::onTextChangedPriorityMask(const QString& text)
{
  if (!m_bSkipAutomaticUpdate) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    doPrioritySelections();
    QApplication::restoreOverrideCursor();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassFilter
//

void
CDlgLevel2Filter::onTextChangedVscpClassFilter(const QString& text)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  if (!m_bSkipAutomaticUpdate)
    doVscpClassSelections();
  if (!m_bSkipAutomaticUpdate)
    doVscpTypeSelections();
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassMask
//

void
CDlgLevel2Filter::onTextChangedVscpClassMask(const QString& text)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  if (!m_bSkipAutomaticUpdate)
    doVscpClassSelections();
  if (!m_bSkipAutomaticUpdate)
    doVscpTypeSelections();
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeFilter
//

void
CDlgLevel2Filter::onTextChangedVscpTypeFilter(const QString& text)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  if (!m_bSkipAutomaticUpdate)
    doVscpTypeSelections();
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeMask
//

void
CDlgLevel2Filter::onTextChangedVscpTypeMask(const QString& text)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  if (!m_bSkipAutomaticUpdate)
    doVscpTypeSelections();
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// doPrioritySelections
//

void
CDlgLevel2Filter::doPrioritySelections(void)
{
  uint8_t priority_filter =
    vscp_readStringValue(ui->editVscpPriorityFilter->text().toStdString()) &
    0x07;
  uint8_t priority_mask =
    vscp_readStringValue(ui->editVscpPriorityMask->text().toStdString()) &
    0x07;

  ui->listPriority->setCurrentRow(0, QItemSelectionModel::Clear);
  for (uint8_t i = 0; i < 8; i++) {
    if ((i & priority_mask) == (priority_filter & priority_mask)) {
      ui->listPriority->setCurrentRow(i, QItemSelectionModel::Select);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// doVscpClassSelections
//

void
CDlgLevel2Filter::doVscpClassSelections(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  uint16_t vscpclass_filter =
    vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) &
    0xffff;
  uint16_t vscpclass_mask =
    vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0xffff;

  ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);
  ui->listClass->setCurrentRow(0, QItemSelectionModel::Deselect);

  // qDebug() << "-------------------------------------------------------";
  std::map<uint16_t, QString>::iterator it;
  for (it = pworks->m_mapVscpClassToToken.begin();
       it != pworks->m_mapVscpClassToToken.end();
       ++it) {

    uint16_t classId = it->first;

    if ((classId & vscpclass_mask) ==
        (vscpclass_filter & vscpclass_mask)) {
      ui->listClass->setCurrentRow(m_classToIndexMap[classId],
                                   QItemSelectionModel::Select);
      // qDebug() << "class =" << classId
      //          << " map =" << m_classToIndexMap[classId];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// doVscpTypeSelections
//

void
CDlgLevel2Filter::doVscpTypeSelections(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  uint16_t vscpclass_filter =
    vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) &
    0xffff;
  uint16_t vscpclass_mask =
    vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0xffff;

  uint8_t vscptype_filter =
    vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()) & 0xffff;
  uint8_t vscptype_mask =
    vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()) & 0xffff;

  ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);
  ui->listType->setCurrentRow(0, QItemSelectionModel::Deselect);

  // qDebug() << "-------------------------------------------------------";
  std::map<uint16_t, QString>::iterator itClass;
  for (itClass = pworks->m_mapVscpClassToToken.begin();
       itClass != pworks->m_mapVscpClassToToken.end();
       ++itClass) {

    uint16_t classId = itClass->first;

    if ((classId & vscpclass_mask) ==
        (vscpclass_filter & vscpclass_mask)) {
      ui->listClass->setCurrentRow(m_classToIndexMap[classId],
                                   QItemSelectionModel::Select);
      // qDebug() << "class =" << classId
      //          << " map =" << m_classToIndexMap[classId] << " !";

      std::map<uint32_t, QString>::iterator itType;
      for (itType = pworks->m_mapVscpTypeToToken.begin();
           itType != pworks->m_mapVscpTypeToToken.end();
           ++itType) {

        uint32_t typeId     = itType->first;
        QString typeToken   = itType->second;
        uint16_t vscp_class = typeId >> 16;
        uint8_t vscp_type   = typeId & 0xffff;

        if (classId == (typeId >> 16)) {
          if ((vscp_type & vscptype_mask) ==
              (vscptype_filter & vscptype_mask)) {
            ui->listType->setCurrentRow(
              m_typeToIndexMap[itType->first],
              QItemSelectionModel::Select);
            // qDebug()
            //   << "class =" << vscp_class
            //   << "type =" << vscp_type
            //   << " map =" << m_typeToIndexMap[classId] << " !";
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// calculatePriorityValues
//
// https://www.dcode.fr/boolean-expressions-calculator
// https://www.dcode.fr/boolean-truth-table
//
// (!a * !b * !c) + (a * b * c)
//
// a	b	c	X   m1            m2            Or
// -------------
// 0	0	0	1   11=1 11=1     00=0  00=0    10=1
// 0	0	1	0   11=1 01=0     00=0  10=0    00=0
// 0	1	0	0   10=0 10=0     01=0  00=0    00=0
// 0	1	1	0   10=0 00=0     01=0  10=0    00=0
// 1	0	0	0   01=0 10=0     10=0  00=0    00=0
// 1	0	1	0   01=0 00=0     10=0  10=0    00=0
// 1	1	0	0   00=0 10=0     11=1  01=0    00=0
// 1	1	1	1   00=1 01=0     11=1  11=1    01=1
//
//
// * and
// + or
//
// m1 = !bit and !bit and !bit   ~byte, bit and bit and bi
// m2 = bit and bit and b´it  (FILTER)
// m = m1 or m2
//

void
CDlgLevel2Filter::calculatePriorityValues(void)
{
  uint8_t filter = 0;
  uint8_t mask   = 0;
  uint8_t mask1;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (!ui->listPriority->count()) {
    // QMessageBox::information(this,
    //                          tr(APPNAME),
    //                          tr("From Visual"),
    //                          QMessageBox::Ok);
  }
  else {
    std::vector<uint8_t> m1;
    std::vector<uint8_t> m2;
    for (int i = 0; i < ui->listPriority->count(); ++i) {
      QListWidgetItem* item = ui->listPriority->item(i);
      if (item->isSelected()) {
        m1.push_back(i);
      }
    }

    // Must be some selections to work on
    if (m1.size()) {

      // Build mask (mask2 is same as filter)
      mask1  = ~m1[0];
      filter = m1[0];

      for (int i = 1; i < m1.size(); i++) {
        mask1  = mask1 & ~m1[i];
        filter = filter & m1[i];
      }

      mask = mask1 | filter;

      std::string str = vscp_str_format("F:%02X M:%02X -- M1:%02X M:%02X",
                                        filter,
                                        mask,
                                        mask1,
                                        mask1 | filter);
      qDebug() << str.c_str();
      qDebug() << filter;
    }
  }

  // setVscpPriorityFilter(filter);
  // setVscpPriorityMask(mask);
  ui->editVscpPriorityFilter->setText(pworks->decimalToStringInBase(filter & 0x07, ui->comboNumberBase->currentIndex()));
  ui->editVscpPriorityMask->setText(pworks->decimalToStringInBase(mask & 0x07, ui->comboNumberBase->currentIndex()));
}

///////////////////////////////////////////////////////////////////////////////
// calculateVscpClassValues
//

void
CDlgLevel2Filter::calculateVscpClassValues(void)
{
  uint16_t filter = 0;
  uint16_t mask   = 0;
  uint16_t mask1;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  if (!ui->listClass->count()) {
    // QMessageBox::information(this,
    //                          tr(APPNAME),
    //                          tr("From Visual"),
    //                          QMessageBox::Ok);
  }
  else {
    std::vector<uint16_t> m1;
    for (int i = 0; i < ui->listClass->count(); ++i) {
      QListWidgetItem* item = ui->listClass->item(i);
      if (item->isSelected()) {
        m1.push_back(item->data(Qt::UserRole).toUInt());
      }
    }

    // Must be some selections to work on
    if (m1.size()) {

      // Build mask (mask2 is same as filter)
      mask1  = ~m1[0];
      filter = m1[0];

      for (int i = 1; i < m1.size(); i++) {
        mask1  = mask1 & ~m1[i];
        filter = filter & m1[i];
      }

      mask = mask1 | filter;

      std::string str = vscp_str_format("F:%02X M:%02X -- M1:%02X M:%02X",
                                        filter,
                                        mask,
                                        mask1,
                                        mask1 | filter);
      qDebug() << str.c_str();
      qDebug() << filter;
    }
  }

  ui->editVscpClassFilter->setText(pworks->decimalToStringInBase(filter & 0xffff, ui->comboNumberBase->currentIndex()));
  ui->editVscpClassMask->setText(pworks->decimalToStringInBase(mask & 0xffff, ui->comboNumberBase->currentIndex()));
}

///////////////////////////////////////////////////////////////////////////////
// calculateVscpTypeValues
//

void
CDlgLevel2Filter::calculateVscpTypeValues(void)
{
  uint16_t filter = 0;
  uint16_t mask   = 0;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Type numbers of the selected events
  std::vector<uint16_t> m1;
  for (int i = 0; i < ui->listType->count(); ++i) {
    QListWidgetItem* item = ui->listType->item(i);
    if (item->isSelected()) {
      m1.push_back(item->data(Qt::UserRole).toUInt() & 0xffff);
    }
  }

  // No selected types: filter and mask are zero, all types pass
  if (m1.size()) {
    // Mask bits are the bits that are equal in all selected types
    uint16_t mask1 = ~m1[0];
    filter         = m1[0];
    for (size_t i = 1; i < m1.size(); i++) {
      mask1  = mask1 & ~m1[i];
      filter = filter & m1[i];
    }
    mask = mask1 | filter;
  }

  ui->editVscpTypeFilter->setText(pworks->decimalToStringInBase(filter & 0xffff, ui->comboNumberBase->currentIndex()));
  ui->editVscpTypeMask->setText(pworks->decimalToStringInBase(mask & 0xffff, ui->comboNumberBase->currentIndex()));
}

///////////////////////////////////////////////////////////////////////////////
// calculateNodeIdValues
//

void
CDlgLevel2Filter::onVscpTypeItemClicked(QListWidgetItem* item)
{
  uint16_t classId = (item->data(Qt::UserRole).toUInt() >> 16) & 0xffff;
  uint16_t typeId  = item->data(Qt::UserRole).toUInt() & 0xffff;

  qDebug() << "Class = " << classId << " Type = " << typeId;

  QListWidgetItem* itemClass = ui->listClass->item(m_classToIndexMap[classId]);
  qDebug() << itemClass->text();
  if (!itemClass->isSelected()) {
    if (QMessageBox::Yes == QMessageBox::information(this,
                                                     tr(APPNAME),
                                                     tr("The VSCP class for this type is not currently selected. Should it be selected?"),
                                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {
      itemClass->setSelected(true); // QMessageBox::Question
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// clearFilter
//

void
CDlgLevel2Filter::clearFilter(void)
{
  ui->editVscpPriorityFilter->setText("0x00");
  ui->editVscpPriorityMask->setText("0x00");
  ui->editVscpClassFilter->setText("0x0000");
  ui->editVscpClassMask->setText("0x0000");
  ui->editVscpTypeFilter->setText("0x0000");
  ui->editVscpTypeMask->setText("0x0000");
  ui->editVscpGuidFilter->setText("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  ui->editVscpGuidMask->setText("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// setFilter
//

void
CDlgLevel2Filter::setFilter(const vscp_event_filter_t* pfilter)
{
  QString qstr;

  assert(nullptr != pfilter);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  qstr = pworks->decimalToStringInBase(pfilter->filter_priority & 0x07);
  ui->editVscpPriorityFilter->setText(qstr);

  qstr = pworks->decimalToStringInBase(pfilter->mask_priority & 0x07);
  ui->editVscpPriorityMask->setText(qstr);

  qstr = pworks->decimalToStringInBase(pfilter->filter_class & 0xffff);
  ui->editVscpClassFilter->setText(qstr);

  qstr = pworks->decimalToStringInBase(pfilter->mask_class & 0xffff);
  ui->editVscpClassMask->setText(qstr);

  qstr = pworks->decimalToStringInBase(pfilter->filter_type & 0xffff);
  ui->editVscpTypeFilter->setText(qstr);

  qstr = pworks->decimalToStringInBase(pfilter->mask_type & 0xffff);
  ui->editVscpTypeMask->setText(qstr);

  std::string str;
  vscp_writeGuidArrayToString(str, pfilter->filter_GUID);
  ui->editVscpGuidFilter->setText(str.c_str());

  vscp_writeGuidArrayToString(str, pfilter->mask_GUID);
  ui->editVscpGuidMask->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getFilter
//

void
CDlgLevel2Filter::getFilter(vscp_event_filter_t* pfilter)
{
  QString qstr;

  assert(nullptr != pfilter);

  pfilter->filter_priority =
    vscp_readStringValue(ui->editVscpPriorityFilter->text().toStdString()) & 0x07;

  pfilter->mask_priority =
    vscp_readStringValue(ui->editVscpPriorityMask->text().toStdString()) & 0x07;

  pfilter->filter_class =
    vscp_readStringValue(ui->editVscpClassFilter->text().toStdString());

  pfilter->mask_class =
    vscp_readStringValue(ui->editVscpClassMask->text().toStdString());

  pfilter->filter_type =
    vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString());

  pfilter->mask_type =
    vscp_readStringValue(ui->editVscpTypeMask->text().toStdString());

  vscp_getGuidFromStringToArray(pfilter->mask_GUID,
                                ui->editVscpGuidMask->text().toStdString());

  vscp_getGuidFromStringToArray(pfilter->filter_GUID,
                                ui->editVscpGuidFilter->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgLevel2Filter::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=filter-on-specific-events-or-groups-of-events";
  QDesktopServices::openUrl(QUrl(link));
}

///////////////////////////////////////////////////////////////////////////////
// addClassesOfSelectedTypes
//
// A selected event (VSCP type) also selects its VSCP class, so a specific
// event can be picked in the type list alone.
//

void
CDlgLevel2Filter::addClassesOfSelectedTypes(void)
{
  for (int i = 0; i < ui->listType->count(); ++i) {
    QListWidgetItem* item = ui->listType->item(i);
    if (!item->isSelected()) {
      continue;
    }
    uint16_t eventClass = (item->data(Qt::UserRole).toUInt() >> 16) & 0xffff;
    auto it             = m_classToIndexMap.find(eventClass);
    if (it != m_classToIndexMap.end()) {
      ui->listClass->item(it->second)->setSelected(true);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// checkFilterCoverage
//
// One filter/mask pair per field can only describe one bit pattern, so the
// calculated filter can let more events through than were selected, or block
// some of them. Compare the known events the filter passes with the selection
// and tell the user when they differ. A selected class without selected types
// stands for all its types.
//

void
CDlgLevel2Filter::checkFilterCoverage(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  std::set<uint16_t> classes;
  for (int i = 0; i < ui->listClass->count(); ++i) {
    if (ui->listClass->item(i)->isSelected()) {
      classes.insert(ui->listClass->item(i)->data(Qt::UserRole).toUInt() & 0xffff);
    }
  }
  std::map<uint16_t, std::set<uint16_t>> types; // class -> selected types
  for (int i = 0; i < ui->listType->count(); ++i) {
    if (ui->listType->item(i)->isSelected()) {
      uint32_t key = ui->listType->item(i)->data(Qt::UserRole).toUInt();
      types[(key >> 16) & 0xffff].insert(key & 0xffff);
    }
  }
  if (classes.empty() && types.empty()) {
    return; // Nothing selected: the filter passes everything
  }

  uint16_t classFilter = vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) & 0xffff;
  uint16_t classMask   = vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0xffff;
  uint16_t typeFilter  = vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()) & 0xffff;
  uint16_t typeMask    = vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()) & 0xffff;

  QStringList extra;   // Passed but not selected
  QStringList blocked; // Selected but not passed
  for (auto it = pworks->m_mapVscpTypeToToken.begin(); it != pworks->m_mapVscpTypeToToken.end(); ++it) {
    uint16_t eventClass = (it->first >> 16) & 0xffff;
    uint16_t eventType  = it->first & 0xffff;
    bool wanted = false;
    if (classes.count(eventClass)) {
      auto t = types.find(eventClass);
      wanted = (t == types.end()) || t->second.count(eventType);
    }
    bool passed = ((eventClass & classMask) == (classFilter & classMask)) &&
                  ((eventType & typeMask) == (typeFilter & typeMask));
    if (passed && !wanted) {
      extra.append(it->second);
    }
    else if (!passed && wanted) {
      blocked.append(it->second);
    }
  }

  if (extra.isEmpty() && blocked.isEmpty()) {
    return; // The filter matches the selection exactly
  }

  auto examples = [](const QStringList& list) {
    QString str;
    for (int i = 0; i < list.size() && i < 5; i++) {
      str += "\n    " + list[i];
    }
    if (list.size() > 5) {
      str += "\n    ...";
    }
    return str;
  };

  QString msg = tr("A filter/mask pair can only describe one bit pattern for the class "
                   "and one for the type, so the calculated filter does not match "
                   "exactly the events you selected.");
  if (!extra.isEmpty()) {
    msg += "\n\n" + tr("It also lets through %1 events you did not select, for example:").arg(extra.size()) +
           examples(extra);
  }
  if (!blocked.isEmpty()) {
    msg += "\n\n" + tr("It blocks %1 of the events you selected, for example:").arg(blocked.size()) +
           examples(blocked);
  }
  msg += "\n\n" + tr("Select events that differ in fewer bits, or use several "
                      "connections or session filters for an exact set.");
  QMessageBox::information(this, tr(APPNAME), msg, QMessageBox::Ok);
}
