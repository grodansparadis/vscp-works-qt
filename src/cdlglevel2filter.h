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

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgLevel2Filter;
}


class CDlgLevel2Filter : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgLevel2Filter(QWidget *parent = nullptr);
    ~CDlgLevel2Filter();

private:

    Ui::CDlgLevel2Filter *ui;

    // void createMenu();
    // void createHorizontalGroupBox();
    // void createGridGroupBox();
    // void createFormGroupBox();

};


#endif // CDLGLEVEL2FILTER_H