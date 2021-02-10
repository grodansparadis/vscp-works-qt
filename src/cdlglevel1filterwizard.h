// cdlglevel1filterwizard.h
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

#ifndef CDLGLEVEL1FILTERWIZARD_H
#define CDLGLEVEL1FILTERWIZARD_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgLevel1FilterWizard;
}


class CDlgLevel1FilterWizard : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgLevel1FilterWizard(QWidget *parent = nullptr);
    ~CDlgLevel1FilterWizard();

    /*!
        set Initial focus
    */
    void setInitialFocus(void);

    /*!
        set combo numbase index
    */
    void setNumBaseComboIndex(uint8_t index);

    /*!
        Get combo numbase index
    */
    uint8_t getNumComboIndex(void);

public slots:
    /*!
        Numerical base changed -recalc
    */
    void onBaseChange(int index);

    /*!
        Add priorities to the list box
    */
    void fillPriorities(void);

    /*!
        Add VSCP classes to the list box
    */
    void fillVscpClasses(void);

    /*!
        Add VSCP types to the list box
    */
    void fillVscpTypes(void);

    /*!
        Add node-id's to the list box
    */
    void fillVscpNodeIds(void);

    /*!
        Fill in selected priorities from
        set priority filter/mask
    */
    void doPrioritySelections(void);

    /*!
        Fill in selected VSCP classes from
        set class filter/mask
    */
    void doVscpClassSelections(void);

    /*!
        Fill in selected VSCP types from
        set type filter/mask
    */
    void doVscpTypeSelections(void);

    /*!
        Fill in selected node-id's from
        set node id filter/mask
    */
    void doVscpNodeIdSelections(void);

    /*!
        Transfer info to visual side
    */
    void transferToVisual(void);

    /*!
        Transfer info to id/mask
    */
    void transferFromVisual(void);

    /*!
        Calculate and set priority filter and
        mask values from listbox selection
    */
    void calculatePriorityValues(void);

    /*!
        Calculate and set VSCP class filter and
        mask values from listbox selection
    */
    void calculateVscpClassValues(void);

    /*!
        Calculate and set VSCP type filter and
        mask values from listbox selection
    */
    void calculateVscpTypeValues(void);

    /*!
        Calculate and set node-id filter and
        mask values from listbox selection
    */
    void calculateNodeIdValues(void);


    // ----------------------------------------------------------------------------
    //                             Getters & Setters
    // ----------------------------------------------------------------------------

    /*!
        Priority filter getter/setters
    */
    void setVscpPriorityFilter(uint8_t value);
    uint8_t getVscpPriorityFilter(void);

    /*!
        Priority mask getter/setters
    */
    void setVscpPriorityMask(uint8_t value);
    uint8_t getVscpPriorityMask(void);

    /*!
        Class filter getter/setters
    */
    void setVscpClassFilter(uint16_t value);
    uint16_t getVscpClassFilter(void);

    /*!
        Class mask getter/setters
    */
    void setVscpClassMask(uint16_t value);
    uint16_t getVscpClassMask(void);

    /*!
        Type filter getter/setters
    */
    void setVscpTypeFilter(uint8_t value);
    uint8_t getVscpTypeFilter(void);

    /*!
        Type mask getter/setters
    */
    void setVscpTypeMask(uint8_t value);
    uint8_t getVscpTypeMask(void);

    /*!
        Nodeid filter getter/setters
    */
    void setVscpNodeIdFilter(uint8_t value);
    uint8_t getVscpNodeIdFilter(void);

    /*!
        Nodeid mask getter/setters
    */
    void setVscpNodeIdMask(uint8_t value);
    uint8_t getVscpNodeIdMask(void);    

private slots:
    
    void showPriorityContextMenu(const QPoint& pos);
    void showVscpClassContextMenu(const QPoint& pos);
    void showVscpTypeContextMenu(const QPoint& pos);
    void showNodeIdContextMenu(const QPoint& pos);

    void clrAllPrioritySelections(void);
    void selectAllPrioritySelections(void);

    void clrAllVscpClassSelections(void);
    void selectAllVscpClassSelections(void);

    void clrAllVscpTypeSelections(void);
    void selectAllVscpTypeSelections(void);

    void clrAllNodeIdSelections(void);
    void selectAllNodeIdSelections(void);

    void onTextChangedPriorityFilter(const QString &text);
    void onTextChangedPriorityMask(const QString &text);  

    void onTextChangedVscpClassFilter(const QString &text);
    void onTextChangedVscpClassMask(const QString &text);

    void onTextChangedVscpTypeFilter(const QString &text);
    void onTextChangedVscpTypeMask(const QString &text);

    void onTextChangedVscpNodeIdFilter(const QString &text);
    void onTextChangedVscpNodeIdMask(const QString &text);

    void onVscpTypeItemClicked(QListWidgetItem *item);

private:

    Ui::CDlgLevel1FilterWizard *ui;

    numerical_base m_baseIndex;

    // Vector that maps VSCP class to list index
    std::map<uint16_t,uint16_t> m_classToIndexMap;

    // Vector that maps VSCP type to list index
    std::map<uint32_t,uint16_t> m_typeToIndexMap;

    /*!
        Used by update mechanism to prevent automatic
        sections when base is changed.
    */
    bool m_bSkipAutomaticUpdate;

};


#endif // CDLGLEVEL1FILTERWIZARD_H
