// cdlgselectdate.h
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

#ifndef CDLGSELECTDATE_H
#define CDLGSELECTDATE_H


#include <QDialog>
#include <QListWidgetItem>
#include <QDate>
#include <QCalendarWidget>

namespace Ui {
class CDlgSelectDate;
}


class CDlgSelectDate : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgSelectDate(QWidget *parent = nullptr);
    ~CDlgSelectDate();

public:
    
    // getters/setters for year
    uint16_t getYearValue(void);
    void setYearValue(uint16_t year);
    CSessionFilter::constraint getYearConstraint(void);
    void setYearConstraint(CSessionFilter::constraint op);

    // getters/setters for month
    uint8_t getMonthValue(void);
    void setMonthValue(uint8_t month);
    CSessionFilter::constraint getMonthConstraint(void);
    void setMonthConstraint(CSessionFilter::constraint op);

    // getters/setters for day
    uint8_t getDayValue(void);
    void setDayValue(uint8_t day);
    CSessionFilter::constraint getDayConstraint(void);
    void setDayConstraint(CSessionFilter::constraint op);

    // getters/setters for hour
    uint8_t getHourValue(void);
    void setHourValue(uint8_t hour);
    CSessionFilter::constraint getHourConstraint(void);
    void setHourConstraint(CSessionFilter::constraint op);

    // getters/setters for minute
    uint8_t getMinuteValue(void);
    void setMinuteValue(uint8_t minute);
    CSessionFilter::constraint getMinuteConstraint(void);
    void setMinuteConstraint(CSessionFilter::constraint op);

    // getters/setters for second
    uint8_t getSecondValue(void);
    void setSecondValue(uint8_t second);
    CSessionFilter::constraint getSecondConstraint(void);
    void setSecondConstraint(CSessionFilter::constraint op);


private slots:
    /// Date clicked in the date widget
    void dateClicked(const QDate &date);

private:

    Ui::CDlgSelectDate *ui;

};


#endif // CDLGSELECTDATE_H
