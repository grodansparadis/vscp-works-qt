// cdlgselectclass.h
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

#ifndef CDLGSELECTCLASS_H
#define CDLGSELECTCLASS_H


#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSelectClass;
}


class CDlgSelectClass : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgSelectClass(QWidget *parent = nullptr);
    ~CDlgSelectClass();

public:

    /// Get selected VSCP classes
    std::deque<uint16_t> getSelectedClasses(void);
    
    /// Get selected VSCP types
    std::deque<uint32_t> getSelectedTypes(void);

    /// Select classes from configured list
    void selectClasses(const std::deque<uint16_t>& mapClass);

    /// Select types from configured list
    void selectTypes(const std::deque<uint32_t>& mapType);

    /// Fill classes into list
    void fillVscpClasses(void);

    /// Fill type into list
    void fillVscpTypes(void);

private slots:

    /// Clear selections in class list
    void clearClassSelections(void);

    /// Clear selections in type list
    void clearTypeSelections(void);

    /// Item in class listbox clicked
    void itemClassClicked(QListWidgetItem *item);

    /// Item in type listbox clicked
    void itemTypeClicked(QListWidgetItem *item);

private:

    Ui::CDlgSelectClass *ui;

};


#endif // CDLGSELECTCLASS_H
