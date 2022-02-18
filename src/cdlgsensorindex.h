// cdlgsensorindex.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGSENSORINDEX_H
#define CDLGSENSORINDEX_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class CDlgSensorIndex;
}


class CDlgSensorIndex : public QDialog
{
    Q_OBJECT

public:
    

public:
    explicit CDlgSensorIndex(QWidget *parent = nullptr, int link_to_guid=-1);
    ~CDlgSensorIndex();

    /*!
        set Initial focus
    */
    void setInitialFocus(void);

    /*!
        Insert one GUID element into table
        @param sensorindex The sensor index for the row
        @param name The symbolic name for the GUID
    */
    void insertSensorIndexItem(QString sensorindex, QString name);

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
    bool selectBySensorIndex(const QString& sensorindex);

    /*!
        Set GUID that will be used next time add is calles/pressed
        @param guid GUID to show in new known node window
    */
    void setAddGuid(const QString& guid) 
                { m_addGuid = guid; };

    /*!
        Set GUID label value
    */
    void setGuid(const QString& strguid );

    /*!
        Get GUID label value
    */
   QString getGuid(void);

   /*!
        Set GUID name label value
    */
    void setGuidName(const QString& strguid );

    /*!
        Get GUID name label value
    */
   QString getGuidName(void);

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

    Ui::CDlgSensorIndex *ui;

    /// GUID to use as default for add
    QString m_addGuid;

    /// Link to the GUID this sensor belongs to
    int m_link_to_guid;
};


#endif // CDLGSENSORINDEX_H
