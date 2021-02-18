// cdlglevel2filter.h
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

#ifndef CDLGLEVEL2FILTER_H
#define CDLGLEVEL2FILTER_H

#include <vscp.h>
#include <vscpworks.h>

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgLevel2Filter;
}


class CDlgLevel2Filter : public QDialog
{
    Q_OBJECT 

public:
    explicit CDlgLevel2Filter(QWidget *parent = nullptr);
    ~CDlgLevel2Filter();

public:

    // Getters setters for filter
    void getFilter(vscpEventFilter *pfilter);
    void setFilter(const vscpEventFilter *pfilter);  

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


private slots:
    
    void showPriorityContextMenu(const QPoint& pos);
    void showVscpClassContextMenu(const QPoint& pos);
    void showVscpTypeContextMenu(const QPoint& pos);

    void clrAllPrioritySelections(void);
    void selectAllPrioritySelections(void);

    void clrAllVscpClassSelections(void);
    void selectAllVscpClassSelections(void);

    void clrAllVscpTypeSelections(void);
    void selectAllVscpTypeSelections(void);

    void onTextChangedPriorityFilter(const QString &text);
    void onTextChangedPriorityMask(const QString &text);  

    void onTextChangedVscpClassFilter(const QString &text);
    void onTextChangedVscpClassMask(const QString &text);

    void onTextChangedVscpTypeFilter(const QString &text);
    void onTextChangedVscpTypeMask(const QString &text);

    void onVscpTypeItemClicked(QListWidgetItem *item);

private:

    Ui::CDlgLevel2Filter *ui;

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


#endif // CDLGLEVEL2FILTER_H
