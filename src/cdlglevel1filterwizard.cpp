// cdlglevel1filterwizard.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#include "vscphelper.h"
#include "vscpworks.h"

#include "cdlglevel1filterwizard.h"
#include "ui_cdlglevel1filterwizard.h"

#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgLevel1FilterWizard::CDlgLevel1FilterWizard(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgLevel1FilterWizard)
{
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

    // Add items to the listboxes
    AddPriorities();
    AddVscpClasses();
    AddVscpTypes();
    AddVscpNodeIds();

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
}

///////////////////////////////////////////////////////////////////////////////
// AddPriorities
//

void
CDlgLevel1FilterWizard::AddPriorities(void)
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
// AddVscpClasses
//

void
CDlgLevel1FilterWizard::AddVscpClasses(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear selections
    ui->listClass->setCurrentRow(0, QItemSelectionModel::Clear);

    int i = 0;
    std::map<uint16_t, QString>::iterator it;
    for (it = pworks->mapVscpClassToToken.begin();
         it != pworks->mapVscpClassToToken.end();
         ++it) {

        uint16_t classId   = it->first;
        QString classToken = it->second;
        // Only level I events
        if (classId < 512) {
            QString listItem =
              vscp_str_format("%s ", classToken.toStdString().c_str()).c_str();
            // while (listItem.length() < 30) listItem += " ";
            listItem +=
              vscp_str_format(" -- (%d / 0x%03x)", (int)classId, (int)classId)
                .c_str();
            ui->listClass->addItem(listItem);
            // m_classToIndexVector.push_back(classId);
            qDebug() << i << " " << classId;
            m_classToIndexMap[classId] = i;
            i++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddVscpTypes
//

void
CDlgLevel1FilterWizard::AddVscpTypes(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Clear selections
    ui->listType->setCurrentRow(0, QItemSelectionModel::Clear);

    int i = 0;
    std::map<uint32_t, QString>::iterator it;
    for (it = pworks->mapVscpTypeToToken.begin();
         it != pworks->mapVscpTypeToToken.end();
         ++it) {

        uint16_t classId  = it->first << 16;
        uint16_t typeId   = it->first & 0xfff;
        QString typeToken = it->second;
        // Only level I events
        if (classId < 512) {
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
            ui->listType->addItem(listItem);
            // m_classToIndexVector.push_back(classId);
            qDebug() << i << " " << classId << " " << typeId;
            m_typeToIndexMap[it->first] = i;
            i++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddVscpNodeIds
//

void
CDlgLevel1FilterWizard::AddVscpNodeIds(void)
{
    // Clear selections
    ui->listNodeId->setCurrentRow(0, QItemSelectionModel::Clear);
    for (int i = 0; i < 256; i++) {
        QString listItem = vscp_str_format("%d  --  0x%02X", i, i).c_str();
        ui->listNodeId->addItem(listItem);
    }
}

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
    calculatePriorityValues();    
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityFilter
//

void
CDlgLevel1FilterWizard::onTextChangedPriorityFilter(const QString& text)
{
    doPrioritySelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedPriorityMask
//

void
CDlgLevel1FilterWizard::onTextChangedPriorityMask(const QString& text)
{
    doPrioritySelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpClassFilter(const QString& text)
{
    doVscpClassSelections();
    doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpClassMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpClassMask(const QString& text)
{
    doVscpClassSelections();
    doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpTypeFilter(const QString& text)
{
    doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpTypeMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpTypeMask(const QString& text)
{
    doVscpTypeSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpNodeIdFilter
//

void
CDlgLevel1FilterWizard::onTextChangedVscpNodeIdFilter(const QString& text)
{
    doVscpNodeIdSelections();
}

///////////////////////////////////////////////////////////////////////////////
// onTextChangedVscpNodeIdMask
//

void
CDlgLevel1FilterWizard::onTextChangedVscpNodeIdMask(const QString& text)
{
    doVscpNodeIdSelections();
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

    qDebug() << "-------------------------------------------------------";
    std::map<uint16_t, QString>::iterator it;
    for (it = pworks->mapVscpClassToToken.begin();
         it != pworks->mapVscpClassToToken.end();
         ++it) {

        uint16_t classId = it->first;
        // QString classToken = it->second;
        if (classId < 512) {
            if ((classId & vscpclass_mask) ==
                (vscpclass_filter & vscpclass_mask)) {
                ui->listClass->setCurrentRow(m_classToIndexMap[classId],
                                             QItemSelectionModel::Select);
                qDebug() << "class =" << classId
                         << " map =" << m_classToIndexMap[classId];
            }
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

    qDebug() << "-------------------------------------------------------";
    std::map<uint16_t, QString>::iterator itClass;
    for (itClass = pworks->mapVscpClassToToken.begin();
         itClass != pworks->mapVscpClassToToken.end();
         ++itClass) {

        uint16_t classId = itClass->first;
        // QString classToken = it->second;
        if (classId < 512) {
            if ((classId & vscpclass_mask) ==
                (vscpclass_filter & vscpclass_mask)) {
                ui->listClass->setCurrentRow(m_classToIndexMap[classId],
                                             QItemSelectionModel::Select);
                qDebug() << "class =" << classId
                         << " map =" << m_classToIndexMap[classId] << " !";

                std::map<uint32_t, QString>::iterator itType;
                for (itType = pworks->mapVscpTypeToToken.begin();
                     itType != pworks->mapVscpTypeToToken.end();
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
                            qDebug()
                              << "class =" << vscp_class
                              << "type =" << vscp_type
                              << " map =" << m_typeToIndexMap[classId] << " !";
                        }
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

void
CDlgLevel1FilterWizard::calculatePriorityValues(void)
{
    uint8_t filter = 0xff;
    uint8_t mask = 0x00;
    uint8_t prev_mask;

    if (!ui->listPriority->count()) {
        mask =  0;
        filter = 0;
        // QMessageBox::information(this,
        //                          tr("vscpworks+"),
        //                          tr("From Visual"),
        //                          QMessageBox::Ok);
    }
    else {
        int cnt = 0;
        for(int i = 0; i < ui->listPriority->count(); ++i) {
            QListWidgetItem* item = ui->listPriority->item(i);
            if (item->isSelected()) {
                if (!cnt) {
                    filter = i;
                    mask = 0x00;
                    cnt++;
                }
                else {
                    mask ^= i;                
                    filter &= i;
                    cnt++;
                }
            }            
        }


        std::string str = vscp_str_format("F: %02X M: %02X", (filter & 7), (~mask & 7));
        qDebug() << str.c_str();
        // qDebug() << "filter" << std::hex << ~filter;
        // qDebug() << "mask" << std::hex << ~mask;
    }
}