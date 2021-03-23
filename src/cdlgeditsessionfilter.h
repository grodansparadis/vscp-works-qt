// cdlgeditsessionfilter.h
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

#ifndef CDLGEDITSESSIONFILTER_H
#define CDLGEDITSESSIONFILTER_H

#include "sessionfilter.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgEditSessionFilter;
}


class CDlgEditSessionFilter : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgEditSessionFilter(QWidget *parent = nullptr);
    ~CDlgEditSessionFilter();

    // Use to store constraint type
    const uint16_t role_constraint_type = Qt::UserRole;

public:
    
    /// Set focus on
    void setInitialFocus(void);

    /*!
        Check if a constraint is already defined
        @param chk Constraint to check for
        @return True if allready defined
    */
    bool isConstraintDefined(uint8_t chk);

    /*! 
        Get pointer to session filter
        @return Pointer to session filter
    */
    CSessionFilter *getSessionFilter(void) { return &m_sessionFilter; };

private slots:

    /// Add filter constraint
    void addConstraint(void);
    
    /// Edit filter constraint
    void editConstraint(void);

    /// Delete filter constraint
    void deleteConstraint(void);

    /// Context menu
    void showContextMenu(const QPoint& pos);
    
private:

    Ui::CDlgEditSessionFilter *ui;

    // Settings for the filter
    CSessionFilter m_sessionFilter;

};


#endif // CDLGEDITSESSIONFILTER_H
