// cdlglevel1filter.h
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

#ifndef CDLGLEVEL1FILTER_H
#define CDLGLEVEL1FILTER_H

#include <vscpworks.h>

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgLevel1Filter;
}


class CDlgLevel1Filter : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgLevel1Filter(QWidget *parent = nullptr);
    ~CDlgLevel1Filter();

    /*!
        Set inital focus to description
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

    // ----------------------------------------------------------------------------
    //                             Getters & Setters
    // ----------------------------------------------------------------------------

    /*!
        Priority filter getter/setters
    */
    void setDescription(const std::string& description);
    std::string getDescription(void);

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
    
    /*!
        Set value for hardcoded checkbox
    */
    void setHardcoded(bool val);
    bool getHardcoded(void);

    /*!
        Set/get value for inverted checkbox
    */
    void setInverted(bool val);
    bool getInverted(void);

    /*!
        Enable disable the description field
    */
    void disableDescriptionField(void);
    void enableDescriptionField(void);

    

 public slots:

    /*!
        Wizard button has been clicked
    */
    void onWizard(void);

    /*!
        Numerical base changed -recalc
    */
    void onBaseChange(int index);

    // void onBaseChange(const QString&);

    /*!
        ID and MASK should be entered
    */
    void onIdMask(void);

private:

    Ui::CDlgLevel1Filter *ui;

    numerical_base m_baseIndex;
    // void createMenu();
    // void createHorizontalGroupBox();
    // void createGridGroupBox();
    // void createFormGroupBox();

};


#endif // CDLGLEVEL1FILTER_H
