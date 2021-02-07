// cdlglevel1filter.h
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
    */
    void setVscpPriorityFilter(uint8_t value);


    //     QString qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpPriorityFilter->text().toStdString()), base);
    //     int base = 10;
    //     QString qstr;
    //     QString prefix;

    //     vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    //     switch (pworks->m_base) {
    //     case HEX:
    //         prefix = "0x";
    //         base = 16;
    //         break;
    //     case DECIMAL:
    //     default:
    //         prefix = "";
    //         base = 10;
    //         break;
    //     case OCTAL:
    //         prefix = "0o";
    //         base = 8;
    //         break;
    //     case BINARY:
    //         prefix = "0b";
    //         base = 2;
    //         break;
    // }
    //     QString str = 
    //     qstr = prefix + QString::number( vscp_readStringValue( ui->editVscpPriorityFilter->text().toStdString()), base);
    //     ui->editVscpPriorityFilter->setText(qstr);
    
    uint8_t getVscpPriorityFilter(void);

    void setVscpPriorityMask(uint8_t value) { };
    uint8_t getVscpPriorityMask(void);

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

    int m_baseIndex;
    // void createMenu();
    // void createHorizontalGroupBox();
    // void createGridGroupBox();
    // void createFormGroupBox();

};


#endif // CDLGLEVEL1FILTER_H
