// cdlglevel1filterwizard.cpp
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
// https://blog.csdn.net/weixin_30251587/article/details/96939409
//

#ifdef WIN32
#include <pch.h>
#endif

#include "vscphelper.h"
#include "vscpworks.h"

#include "cdlglevel1filterwizard.h"
#include "ui_cdlglevel1filterwizard.h"

#include <QMessageBox>
#include <QMenu>
#include <QDebug>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel1FilterWizard::CDlgLevel1FilterWizard(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgLevel1FilterWizard)
{
    m_bSkipAutomaticUpdate = false;
    ui->setupUi(this);

    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    m_baseIndex       = pworks->m_base;
    ui->comboNumberBase->setCurrentIndex(static_cast<int>(m_baseIndex));
    onBaseChange(static_cast<int>(m_baseIndex));

    connect(ui->btnTransferRight,
            &QPushButton::clicked,
            this,
            &CDlgLevel1FilterWizard::transferToVisual);
    connect(ui->btnTransferLeft,
            &QPushButton::clicked,
            this,
            &CDlgLevel1FilterWizard::transferFromVisual);

    connect(ui->comboNumberBase,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(onBaseChange(int)));

    connect(ui->editVscpPriorityFilter,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedPriorityFilter);

    connect(ui->editVscpPriorityMask,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedPriorityMask);

    connect(ui->editVscpClassFilter,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpClassFilter);

    connect(ui->editVscpClassMask,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpClassMask);

    connect(ui->editVscpTypeFilter,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpTypeFilter);

    connect(ui->editVscpTypeMask,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpTypeMask);

    connect(ui->editVscpNodeIdFilter,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpNodeIdFilter);

    connect(ui->editVscpNodeIdMask,
            &QLineEdit::textChanged,
            this,
            &CDlgLevel1FilterWizard::onTextChangedVscpNodeIdMask);

    // Open pop up menu on right click on priority listbox
    connect(ui->listPriority,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgLevel1FilterWizard::showPriorityContextMenu);

    // Open pop up menu on right click on VSCP class listbox
    connect(ui->listClass,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgLevel1FilterWizard::showVscpClassContextMenu);

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listType,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgLevel1FilterWizard::showVscpTypeContextMenu);

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listType,
            &QListWidget::itemClicked,
            this,
            &CDlgLevel1FilterWizard::onVscpTypeItemClicked);

    // Open pop up menu on right click on node-id listbox
    connect(ui->listNodeId,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgLevel1FilterWizard::showNodeIdContextMenu);                                               

    // Add items to the listboxes
    fillPriorities();
    fillVscpClasses();
    fillVscpTypes();
    fillVscpNodeIds();

    setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgLevel1FilterWizard::~CDlgLevel1FilterWizard()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgLevel1FilterWizard::setInitialFocus(void)
{
    ui->editVscpPriorityFilter->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setNumBaseComboIndex
//

void
CDlgLevel1FilterWizard::setNumBaseComboIndex(uint8_t index)
{
    if (index > 3)
        index = 0;
    ui->comboNumberBase->setCurrentIndex(index);
}

///////////////////////////////////////////////////////////////////////////////
// getNumComboIndex
//

uint8_t
CDlgLevel1FilterWizard::getNumComboIndex(void)
{
    return ui->comboNumberBase->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////
// onBaseChange
//

void
CDlgLevel1FilterWizard::onBaseChange(int index)
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

    m_bSkipAutomaticUpdate = true;  // No automatic selectins

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

    qstr = prefix +
           QString::number(vscp_readStringValue(
                             ui->editVscpNodeIdFilter->text().toStdString()),
                           base);

    ui->editVscpNodeIdFilter->setText(qstr);

    qstr = prefix +
           QString::number(
             vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()),
             base);

    ui->editVscpNodeIdMask->setText(qstr);

    m_bSkipAutomaticUpdate = false; // Enable automatic selections again

    QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// fillPriorities
//

void
CDlgLevel1FilterWizard::fillPriorities(void)
{
    QStringList priorities = { "Priority 0 Highest", "Priority 1",
                               "Priority 2",         "Priority 3 Normal",
                               "Priority 4",         "Priority 5",
                               "Priority 6",         "Priority 7 Lowest" };
    ui->listPriority->addItems(priorities);

    // Clear selections
    ui->listPriority->setCurrentRow(0, QItemSelectionModel::Clear);
}

///////////////////////////////////////////////////////////////////////////////
// fillVscpClasses
//

void
CDlgLevel1FilterWizard::fillVscpClasses(void)
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
        // Only level I events
        if (classId >= 512) break;

        QString listItem =
            vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
        // while (listItem.length() < 30) listItem += " ";
        listItem +=
            vscp_str_format(" -- (%d / 0x%03x)", (int)classId, (int)classId)
            .c_str();
        QListWidgetItem *item = new QListWidgetItem(listItem, ui->listClass);
        QVariant val(classId);
        item->setData(Qt::UserRole, val);
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
CDlgLevel1FilterWizard::fillVscpTypes(void)
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
        uint16_t typeId   = it->first & 0xfff;
        QString typeToken = it->second;

        // Only level I events
        if (classId >= 512) break;

        QString listItem =
            vscp_str_format(
            "%s ",
            /*pworks->mapVscpClassToToken[classId].toStdString().c_str(),*/
            typeToken.toStdString().c_str())
            .c_str();
        // while (listItem.length() < 30) listItem += " ";
        listItem +=
            vscp_str_format(" -- (%d / 0x%03x)", (int)typeId, (int)typeId)
            .c_str();
        QListWidgetItem *item = new QListWidgetItem(listItem, ui->listType);
        QVariant val(it->first);
        qDebug() << ((it->first >> 16) & 0x1ff);
        item->setData(Qt::UserRole, val);    
        ui->listType->addItem(item);
        // m_classToIndexVector.push_back(classId);
        // qDebug() << i << " " << classId << " " << typeId;
        m_typeToIndexMap[it->first] = i;
        i++;
    }

}

///////////////////////////////////////////////////////////////////////////////
// fillVscpNodeIds
//

void
CDlgLevel1FilterWizard::fillVscpNodeIds(void)
{
    // Clear selections
    ui->listNodeId->setCurrentRow(0, QItemSelectionModel::Clear);
    for (int i = 0; i < 256; i++) {
        QString listItem = vscp_str_format("%d  --  0x%02X", i, i).c_str();
        ui->listNodeId->addItem(listItem);
    }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showPriorityContextMenu
//

void CDlgLevel1FilterWizard::showPriorityContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("Clear selections"), this, SLOT(clrAllPrioritySelections()));
    menu->addAction(QString("Select all"), this, SLOT(selectAllPrioritySelections()));
    menu->addAction(QString("Write left"), this, SLOT(calculatePriorityValues()));

    menu->popup(ui->listPriority->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllPrioritySelections
//

void CDlgLevel1FilterWizard::clrAllPrioritySelections(void) 
{
    for (int i = 0; i < ui->listPriority->count(); ++i) {
        QListWidgetItem* item = ui->listPriority->item(i);
        item->setSelected(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// selectAllPrioritySelections
//

void CDlgLevel1FilterWizard::selectAllPrioritySelections(void) 
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

void CDlgLevel1FilterWizard::showVscpClassContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("Clear selections"), this, SLOT(clrAllVscpClassSelections()));
    menu->addAction(QString("Select all"), this, SLOT(selectAllVscpClassSelections()));
    menu->addAction(QString("Write left"), this, SLOT(calculateVscpClassValues()));

    menu->popup(ui->listClass->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllVscpClassSelections
//

void CDlgLevel1FilterWizard::clrAllVscpClassSelections(void) 
{
    for (int i = 0; i < ui->listClass->count(); ++i) {
        QListWidgetItem* item = ui->listClass->item(i);
        item->setSelected(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// selectAllVscpClassSelections
//

void CDlgLevel1FilterWizard::selectAllVscpClassSelections(void) 
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

void CDlgLevel1FilterWizard::showVscpTypeContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("Clear selections"), this, SLOT(clrAllVscpTypeSelections()));
    menu->addAction(QString("Select all"), this, SLOT(selectAllVscpTypeSelections()));
    menu->addAction(QString("Write left"), this, SLOT(calculateVscpTypeValues()));

    menu->popup(ui->listType->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllVscpTypeSelections
//

void CDlgLevel1FilterWizard::clrAllVscpTypeSelections(void) 
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

void CDlgLevel1FilterWizard::selectAllVscpTypeSelections(void) 
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
// showNodeIdContextMenu
//

void CDlgLevel1FilterWizard::showNodeIdContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("Clear selections"), this, SLOT(clrAllNodeIdSelections()));
    menu->addAction(QString("Select all"), this, SLOT(selectAllNodeIdSelections()));
    menu->addAction(QString("Write left"), this, SLOT(calculateNodeIdValues()));

    menu->popup(ui->listNodeId->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// clrAllNodeIdSelections
//

void CDlgLevel1FilterWizard::clrAllNodeIdSelections(void) 
{
    for (int i = 0; i < ui->listNodeId->count(); ++i) {
        QListWidgetItem* item = ui->listNodeId->item(i);
        item->setSelected(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// selectAllNodeIdSelections
//

void CDlgLevel1FilterWizard::selectAllNodeIdSelections(void) 
{
    for (int i = 0; i < ui->listNodeId->count(); ++i) {
        QListWidgetItem* item = ui->listNodeId->item(i);
        item->setSelected(true);
    }
}

// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// transferToVisual
//

void
CDlgLevel1FilterWizard::transferToVisual(void)
{
    doPrioritySelections();
    doVscpClassSelections();
    doVscpTypeSelections();
    doVscpNodeIdSelections();
}

///////////////////////////////////////////////////////////////////////////////
// transferFromVisual
//

void
CDlgLevel1FilterWizard::transferFromVisual(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    calculatePriorityValues();
    calculateVscpClassValues();
    calculateVscpTypeValues();
    calculateNodeIdValues();
    QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityFilter
//

void
CDlgLevel1FilterWizard::onTextChangedPriorityFilter(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doPrioritySelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityMask
//

void
CDlgLevel1FilterWizard::onTextChangedPriorityMask(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doPrioritySelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpClassFilter(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpClassSelections();
    if (!m_bSkipAutomaticUpdate)  doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpClassMask(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpClassSelections();
    if (!m_bSkipAutomaticUpdate)  doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpTypeFilter(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpTypeMask(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpNodeIdFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpNodeIdFilter(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpNodeIdSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpNodeIdMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpNodeIdMask(const QString& text)
{
    if (!m_bSkipAutomaticUpdate)  doVscpNodeIdSelections();
}

///////////////////////////////////////////////////////////////////////////////
// doPrioritySelections
//

void
CDlgLevel1FilterWizard::doPrioritySelections(void)
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
CDlgLevel1FilterWizard::doVscpClassSelections(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    uint16_t vscpclass_filter =
      vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) &
      0x1ff;
    uint16_t vscpclass_mask =
      vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0x1ff;

    ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);
    ui->listClass->setCurrentRow(0, QItemSelectionModel::Deselect);

    //qDebug() << "-------------------------------------------------------";
    std::map<uint16_t, QString>::iterator it;
    for (it = pworks->m_mapVscpClassToToken.begin();
         it != pworks->m_mapVscpClassToToken.end();
         ++it) {

        uint16_t classId = it->first;
        // QString classToken = it->second;
        if (classId >= 512) break;

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
CDlgLevel1FilterWizard::doVscpTypeSelections(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    uint16_t vscpclass_filter =
      vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) &
      0x1ff;
    uint16_t vscpclass_mask =
      vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) & 0x1ff;

    uint8_t vscptype_filter =
      vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()) & 0xff;
    uint8_t vscptype_mask =
      vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()) & 0xff;

    ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);
    ui->listType->setCurrentRow(0, QItemSelectionModel::Deselect);

    //qDebug() << "-------------------------------------------------------";
    std::map<uint16_t, QString>::iterator itClass;
    for (itClass = pworks->m_mapVscpClassToToken.begin();
         itClass != pworks->m_mapVscpClassToToken.end();
         ++itClass) {

        uint16_t classId = itClass->first;
        // QString classToken = it->second;
        if (classId >= 512) break;

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
                uint8_t vscp_type   = typeId & 0xff;

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
// doVscpNodeIdSelections
//

void
CDlgLevel1FilterWizard::doVscpNodeIdSelections(void)
{
    uint8_t nodeid_filter =
      vscp_readStringValue(ui->editVscpNodeIdFilter->text().toStdString()) &
      0xff;
    uint8_t nodeid_mask =
      vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()) & 0xff;

    ui->listNodeId->setCurrentRow(0, QItemSelectionModel::Clear);

    for (int i = 0; i < 256; i++) {
        if ((i & nodeid_mask) == (nodeid_filter & nodeid_mask)) {
            ui->listNodeId->setCurrentRow(i, QItemSelectionModel::Select);
        }
    }
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityFilter
//

void
CDlgLevel1FilterWizard::setVscpPriorityFilter(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityFilter
//

uint8_t
CDlgLevel1FilterWizard::getVscpPriorityFilter(void)
{
    return (
      vscp_readStringValue(ui->editVscpPriorityFilter->text().toStdString()) &
      0x07);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpPriorityMask
//

void
CDlgLevel1FilterWizard::setVscpPriorityMask(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value & 0x07);
    ui->editVscpPriorityMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpPriorityMask
//

uint8_t
CDlgLevel1FilterWizard::getVscpPriorityMask(void)
{
    return (
      vscp_readStringValue(ui->editVscpPriorityMask->text().toStdString()) &
      0x07);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpClassFilter
//

void
CDlgLevel1FilterWizard::setVscpClassFilter(uint16_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassFilter
//

uint16_t
CDlgLevel1FilterWizard::getVscpClassFilter(void)
{
    return (
      vscp_readStringValue(ui->editVscpClassFilter->text().toStdString()) &
      0x1ff);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpClassMask
//

void
CDlgLevel1FilterWizard::setVscpClassMask(uint16_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value & 0x1ff);
    ui->editVscpClassMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpClassMask
//

uint16_t
CDlgLevel1FilterWizard::getVscpClassMask(void)
{
    return (vscp_readStringValue(ui->editVscpClassMask->text().toStdString()) &
            0x1ff);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpTypeFilter
//

void
CDlgLevel1FilterWizard::setVscpTypeFilter(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value);
    ui->editVscpTypeFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeFilter
//

uint8_t
CDlgLevel1FilterWizard::getVscpTypeFilter(void)
{
    return (vscp_readStringValue(ui->editVscpTypeFilter->text().toStdString()) &
            0xff);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpTypeMask
//

void
CDlgLevel1FilterWizard::setVscpTypeMask(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value);
    ui->editVscpTypeMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpTypeMask
//

uint8_t
CDlgLevel1FilterWizard::getVscpTypeMask(void)
{
    return (vscp_readStringValue(ui->editVscpTypeMask->text().toStdString()) &
            0xff);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdFilter
//

void
CDlgLevel1FilterWizard::setVscpNodeIdFilter(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdFilter->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdFilter
//

uint8_t
CDlgLevel1FilterWizard::getVscpNodeIdFilter(void)
{
    return (
      vscp_readStringValue(ui->editVscpNodeIdFilter->text().toStdString()) &
      0xff);
}

///////////////////////////////////////////////////////////////////////////////
// setVscpNodeIdMask
//

void
CDlgLevel1FilterWizard::setVscpNodeIdMask(uint8_t value)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString qstr      = pworks->decimalToStringInBase(value);
    ui->editVscpNodeIdMask->setText(qstr);
}

///////////////////////////////////////////////////////////////////////////////
// getVscpNodeIdMask
//

uint8_t
CDlgLevel1FilterWizard::getVscpNodeIdMask(void)
{
    return (vscp_readStringValue(ui->editVscpNodeIdMask->text().toStdString()) &
            0xff);
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
CDlgLevel1FilterWizard::calculatePriorityValues(void)
{
    uint8_t filter = 0;
    uint8_t mask = 0;
    uint8_t mask1;

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    if (!ui->listPriority->count()) {
        // QMessageBox::information(this,
        //                          tr("vscpworks+"),
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
            mask1 = ~m1[0];
            filter = m1[0];

            for (int i = 1; i < m1.size(); i++) {
                mask1 = mask1 & ~m1[i];
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

    //setVscpPriorityFilter(filter);
    //setVscpPriorityMask(mask);
    ui->editVscpPriorityFilter->setText(pworks->decimalToStringInBase(filter & 0x07, ui->comboNumberBase->currentIndex()));
    ui->editVscpPriorityMask->setText(pworks->decimalToStringInBase(mask & 0x07, ui->comboNumberBase->currentIndex()));
}

///////////////////////////////////////////////////////////////////////////////
// calculateVscpClassValues
//

void
CDlgLevel1FilterWizard::calculateVscpClassValues(void)
{
    uint16_t filter = 0;
    uint16_t mask = 0;
    uint16_t mask1;

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    if (!ui->listClass->count()) {
        // QMessageBox::information(this,
        //                          tr("vscpworks+"),
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
            mask1 = ~m1[0];
            filter = m1[0];

            for (int i = 1; i < m1.size(); i++) {
                mask1 = mask1 & ~m1[i];
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

    ui->editVscpClassFilter->setText(pworks->decimalToStringInBase(filter & 0x1ff, ui->comboNumberBase->currentIndex()));
    ui->editVscpClassMask->setText(pworks->decimalToStringInBase(mask & 0x1ff, ui->comboNumberBase->currentIndex()));
}

///////////////////////////////////////////////////////////////////////////////
// calculateVscpTypeValues
//

void
CDlgLevel1FilterWizard::calculateVscpTypeValues(void)
{
    
}

///////////////////////////////////////////////////////////////////////////////
// calculateNodeIdValues
//

void
CDlgLevel1FilterWizard::calculateNodeIdValues(void)
{
    uint8_t filter = 0;
    uint8_t mask = 0;
    uint8_t mask1;

    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    if (!ui->listNodeId->count()) {
        QMessageBox::information(this,
                                  tr("vscpworks+"),
                                  tr("Nothing selected. Filter and mask will be set to zero."),
                                  QMessageBox::Ok);
    }
    else {
        std::vector<uint8_t> m1;
        for (int i = 0; i < ui->listNodeId->count(); ++i) {
            QListWidgetItem* item = ui->listNodeId->item(i);
            if (item->isSelected()) {
                m1.push_back(i);
            }
        }

        // Must be some selections to work on
        if (m1.size()) {

            // Build mask (mask2 is same as filter)
            mask1 = ~m1[0];
            filter = m1[0];

            for (int i = 1; i < m1.size(); i++) {
                mask1 = mask1 & ~m1[i];
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

    ui->editVscpNodeIdFilter->setText(pworks->decimalToStringInBase(filter & 0xff, ui->comboNumberBase->currentIndex()));
    ui->editVscpNodeIdMask->setText(pworks->decimalToStringInBase(mask & 0xff, ui->comboNumberBase->currentIndex()));    
}


///////////////////////////////////////////////////////////////////////////////
// calculateNodeIdValues
//

void
CDlgLevel1FilterWizard::onVscpTypeItemClicked(QListWidgetItem *item)
{    
    uint16_t classId = (item->data(Qt::UserRole).toUInt() >> 16) & 0x1fff;
    uint16_t typeId = item->data(Qt::UserRole).toUInt() & 0xff;

    qDebug() << "Class = " << classId << " Type = " << typeId;

    QListWidgetItem* itemClass = ui->listClass->item(m_classToIndexMap[classId]);
    qDebug() << itemClass->text();
    if (!itemClass->isSelected()) {
        if (QMessageBox::Yes == QMessageBox::information(this,
                                  tr("vscpworks+"),
                                 tr("The VSCP class for this type is not currently selected. Should it be selected?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {                                      
            itemClass->setSelected(true); // QMessageBox::Question
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// disablePriorityFields
//

void CDlgLevel1FilterWizard::disablePriorityFields(void)
{
    ui->editVscpPriorityFilter->setEnabled(false);
    ui->editVscpPriorityMask->setEnabled(false);
    ui->listPriority->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// enablePriorityFields
//

void CDlgLevel1FilterWizard::enablePriorityFields(void)
{
    ui->editVscpPriorityFilter->setEnabled(true);
    ui->editVscpPriorityMask->setEnabled(true);
    ui->listPriority->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// disableNodeIdFields
//

void CDlgLevel1FilterWizard::disableNodeIdFields(void)
{
    ui->editVscpNodeIdFilter->setEnabled(false);
    ui->editVscpNodeIdMask->setEnabled(false);
    ui->listNodeId->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// enablePriorityFields
//

void CDlgLevel1FilterWizard::enableNodeIdFields(void)
{
    ui->editVscpNodeIdFilter->setEnabled(true);
    ui->editVscpNodeIdMask->setEnabled(true);
    ui->listNodeId->setEnabled(true);
}