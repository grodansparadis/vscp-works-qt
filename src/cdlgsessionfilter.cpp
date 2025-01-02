// cdlgsessionfilter.cpp
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"
#include "cdlgeditsessionfilter.h"

#include "cdlgsessionfilter.h"
#include "ui_cdlgsessionfilter.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QInputDialog>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSessionFilter::CDlgSessionFilter(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSessionFilter)
{
    ui->setupUi(this);

    // Enable context menu
    ui->listFilter->setContextMenuPolicy(
        Qt::CustomContextMenu); 
    ui->listFilter->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listFilter->setAlternatingRowColors(true);    

    connect(ui->btnAddFilter, &QPushButton::clicked, this, &CDlgSessionFilter::addFilter );
    connect(ui->btnEditFilter, &QPushButton::clicked, this, &CDlgSessionFilter::editFilter ); 
    connect(ui->btnCloneFilter, &QPushButton::clicked, this, &CDlgSessionFilter::cloneFilter ); 
    connect(ui->btnDeleteFilter, &QPushButton::clicked, this, &CDlgSessionFilter::deleteFilter ); 
    connect(ui->btnLoadFilter, &QPushButton::clicked, this, &CDlgSessionFilter::loadFilter );  
    connect(ui->btnSaveFilter, &QPushButton::clicked, this, &CDlgSessionFilter::saveFilter );  

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listFilter,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgSessionFilter::showTxContextMenu);

           
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSessionFilter::~CDlgSessionFilter()
{
    delete ui;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// showTxContextMenu
//

void
CDlgSessionFilter::showTxContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    menu->addAction(QString(tr("Add filter...")),
                    this,
                    SLOT(addFilter()));

    menu->addAction(QString(tr("Edit filter...")),
                    this,
                    SLOT(editFilter()));

    menu->addAction(QString(tr("Clone filter")),
                    this,
                    SLOT(cloneFilter()));

    menu->addAction(QString(tr("Delete filter")),
                    this,
                    SLOT(deleteFilter()));

    menu->addSeparator();                    

    menu->addAction(QString(tr("Load filters...")),
                    this,
                    SLOT(loadFilter()));

    menu->addAction(QString(tr("Save filters...")),
                    this,
                    SLOT(saveFilter()));

    menu->popup(ui->listFilter->viewport()->mapToGlobal(pos));
}


///////////////////////////////////////////////////////////////////////////////
// addFilter
//

void
CDlgSessionFilter::addFilter()
{
    CDlgEditSessionFilter dlg;
    if (QDialog::Accepted == dlg.exec()) {
        
    }
}


///////////////////////////////////////////////////////////////////////////////
// editFilter
//

void
CDlgSessionFilter::editFilter()
{
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("editFilter"),
          QMessageBox::Ok);
}


///////////////////////////////////////////////////////////////////////////////
// cloneFilter
//

void
CDlgSessionFilter::cloneFilter()
{
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("cloneFilter"),
          QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// deleteFilter
//

void
CDlgSessionFilter::deleteFilter()
{
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("deleteFilter"),
          QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// loadFilter
//

void
CDlgSessionFilter::loadFilter()
{
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("loadFilter"),
          QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// saveFilter
//

void
CDlgSessionFilter::saveFilter()
{
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("saveFilter"),
          QMessageBox::Ok);
}

