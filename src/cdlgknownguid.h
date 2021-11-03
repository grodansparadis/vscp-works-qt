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

#include <guid.h>

#include <QDialog>  
#include <QTableWidgetItem>

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

    /*!
        Insert one GUID element into table
        @param guid The GUID for the row
        @param name The symbolic name for the GUID
    */
    void insertGuidItem(QString guid, QString name);

    /*!
        An item in the list has been clicked
        @param item The clicked item
    */
    void listItemClicked(QTableWidgetItem *item);

    /*!
        An item in the list has been double clicked
        @param item The double clicked item
    */
    void listItemDoubleClicked(QTableWidgetItem *item);

    /*!
        Show context menu
    */
    void showContextMenu(const QPoint& pos);

    /*!
        Select row that holds the supplied GUID
        @param guid GUID to search for among known GUID's
        @return true if supplied guid found and row is selected
                otherwise false.
    */
    bool selectByGuid(const QString& guid);

    /*!
        Set GUID that will be used next time add is calles/pressed
        @param guid GUID to show in new known node window
    */
    void setAddGuid(const QString& guid) 
                { m_addGuid = guid; };

    /*! 
        Get the GUID that is selected
        @return GUID on string form
    */
    bool getSelectedGuid(cguid& guid);

    /*!
        Enable disable alternative double click
        behaviour
    */
    void enableAccept(bool b) { m_bEnableDblClickAccept = b; };    

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

    /// Open sensor index dialog
    void btnSensorIndex(void);

    /// Handler for load button
    void btnLoad(void); 

    /// Handler for save button
    void btnSave(void); 
    

private:

    /*!
        If true a double click of a row item accepts
        the entry instead of going into edit.
    */
    bool m_bEnableDblClickAccept;

    Ui::CDlgKnownGuid *ui;

    /// GUID to use as default for add
    QString m_addGuid;

};


#endif // CDLGKNOWNGUID_H
