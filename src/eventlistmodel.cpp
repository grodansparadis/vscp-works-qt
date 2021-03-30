// eventlistmodel.h
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
// https://doc.qt.io/qt-5/qtwidgets-itemviews-fetchmore-example.html
//

#ifdef WIN32
#include "StdAfx.h"
#endif

#include "eventlistmodel.h"

#include <QGuiApplication>
#include <QPalette>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

EventListModel::EventListModel(QObject *parent)
        : QAbstractListModel(parent), m_eventCount(0)
{

}

///////////////////////////////////////////////////////////////////////////////
// setDirPath
//

void EventListModel::setDirPath(const QString &path)
{
    //QDir dir(path);

    beginResetModel();
    //fileList = dir.entryList();
    m_eventCount = 0;
    endResetModel();
}

///////////////////////////////////////////////////////////////////////////////
// canFetchMore
//

bool EventListModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }

    return (m_eventCount < fileList.size());
}

///////////////////////////////////////////////////////////////////////////////
// fetchMore
//

void EventListModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }

    int remainder = fileList.size() - m_eventCount;
    int itemsToFetch = qMin(100, remainder);

    if (itemsToFetch <= 0) {
        return;
    }

    beginInsertRows( QModelIndex(), m_eventCount, m_eventCount + itemsToFetch - 1 );

    m_eventCount += itemsToFetch;

    endInsertRows();

    emit numberPopulated(itemsToFetch);
}

///////////////////////////////////////////////////////////////////////////////
// rowCount
//

int EventListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_eventCount;
}

///////////////////////////////////////////////////////////////////////////////
// data
//

QVariant EventListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= fileList.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return fileList.at(index.row());
    } 
    else if (role == Qt::BackgroundRole) {
        int batch = (index.row() / 100) % 2;
        if (batch == 0) {
            return qApp->palette().base();
        }
        else {
            return qApp->palette().alternateBase();
        }
    }

    return QVariant();
}

