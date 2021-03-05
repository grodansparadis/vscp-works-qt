// cdlgknownguid.h
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

#ifndef CDLGKNOWNGUID_H
#define CDLGKNOWNGUID_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgKnownGuid;
}


class CDlgKnownGuid : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgKnownGuid(QWidget *parent = nullptr);
    ~CDlgKnownGuid();

    /*!
        set Initial focus
    */
    void setInitialFocus(void);

public slots:
    
    /// Dialog return
    void done(int r);   

    /// Handler for search button
    void btnSearch(void); 

    /// Handler for add button
    void btnAdd(void); 

    /// Handler for edit button
    void btnEdit(void); 

    /// Handler for clone button
    void btnClone(void); 

    /// Handler for delete button
    void btnDelete(void); 

    /// Handler for load button
    void btnLoad(void); 

    /// Handler for save button
    void btnSave(void); 

private:

    Ui::CDlgKnownGuid *ui;

};


#endif // CDLGKNOWNGUID_H
