// cdlgeditdm.h
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

#ifndef CDLGEDITDM_H
#define CDLGEDITDM_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

namespace Ui {
class CDlgEditDm;
}


class CDlgEditDm : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgEditDm(QWidget *parent = nullptr);
    ~CDlgEditDm();

    /*!
      Set inital focus to description
    */
    void setInitialFocus(void);

    /*!
      Setters/getters for origin
    */
    uint8_t getDmAddressOrigin(void);
    void setDmAddressOrigin(const std::string& str);

    /*!
      Setters/getters for flags
    */
    uint8_t getDmFlags(void);
    void setDmFlags(const std::string& str);

    /*!
      Setters/getters for Class Mask
    */
    uint8_t getDmClassMask(void);
    void setDmClassMask(const std::string& str);

    /*!
      Setters/getters for Class Filter
    */
    uint8_t getDmClassFilter(void);
    void setDmClassFilter(const std::string& str);

    /*!
      Setters/getters for Type Mask
    */
    uint8_t getDmTypeMask(void);
    void setDmTypeMask(const std::string& str);

    /*!
      Setters/getters for Type filter
    */
    uint8_t getDmTypeFilter(void);
    void setDmTypeFilter(const std::string& str);

    /*!
        Setters/getters for action
    */
    uint8_t getDmAction(void);
    void setDmAction(int idx);
    void setDmActionFromCode(uint8_t code);

    /*!
      Setters/getters for action paramter
    */
    uint8_t getDmActionParameter(void);
    void setDmActionParameter(const std::string& str);

    /*!
      Calculate flags value for checkboxes
    */
    void flagsToValue(void);

    /*!
      Flags value to checkboxes
    */
    void valueToFlags(void);

    /*!
      Set DM definitions
      @param pDM Pointer to DM definition object
    */
    //void setDm(CMDF_DecisionMatrix *pDM);
    void setMDF(CMDF *pMDF);

    /*!
      Fill in HTML info
    */
    void fillHtmlInfo(void);


public slots:
    void enableRow_stateChanged(int state);
    void originAddressMatch_stateChanged(int state);
    void originHardcoded_stateChanged(int state);
    void matchZone_stateChanged(int state);
    void matchSubzone_stateChanged(int state);
    void classMaskBit8_stateChanged(int state);
    void classFilterBit8_stateChanged(int state);

    void currentIndexChangedActions(int index);

    void filterWizard(void);
    void actionParameterWizard(void);

    void accept(void);
    //void reject(void);

private:

    Ui::CDlgEditDm *ui;

    /*!
      Pointer to decision matrix object
    */
    //CMDF_DecisionMatrix *m_pDM;
    CMDF *m_pMDF;

    /*!
      Data block for DM row transfere
    */
    uint8_t m_data[8];
};



#endif // CDlgEditDm_H