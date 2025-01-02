// cdlgsessionfilter.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGSESSIONFILTER_H
#define CDLGSESSIONFILTER_H


#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSessionFilter;
}


class CDlgSessionFilter : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgSessionFilter(QWidget *parent = nullptr);
    ~CDlgSessionFilter();

public:
    
    

private slots:

    /// Context menu
    void showTxContextMenu(const QPoint& pos);

    /// Add filter
    void addFilter(void);

    /// Edit filter
    void editFilter(void);

    /// Clone filter
    void cloneFilter(void);

    /// Delete filter
    void deleteFilter(void);

    /// Load filter
    void loadFilter(void);

    /// Save filter
    void saveFilter(void);

private:

    Ui::CDlgSessionFilter *ui;

};


#endif // CDLGSESSIONFILTER_H
