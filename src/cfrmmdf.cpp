// CFrmMdf.cpp
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
// tableWidget->resizeRowsToContents();

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <stdlib.h>

#include <vscp.h>
#include <vscphelper.h>
#include <vscpworks.h>
#include <mdf.h>

#include <mustache.hpp>

#include "cfrmmdf.h"
#include "ui_cfrmmdf.h"

#include <QClipboard>
#include <QFile>
#include <QJSEngine>
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidgetItem>
#include <QXmlStreamReader>
#include <QtSql>
#include <QtWidgets>

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CFrmMdf
//

CFrmMdf::CFrmMdf(QWidget* parent, const char *path)
  : QMainWindow(parent)
  , ui(new Ui::CFrmMdf)
{
  QString str;

  ui->setupUi(this);

  ui->treeMDF->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->treeMDF->setEditTriggers(QAbstractItemView::NoEditTriggers);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  spdlog::debug(std::string(tr("Node configuration module opended").toStdString()));

  if (nullptr == path) {
  //   spdlog::trace(std::string(tr("path is null").toStdString()));
  //   QMessageBox::information(this,
  //                            tr("vscpworks+"),
  //                            tr("Can't open node configuration window - "
  //                               "application configuration data is missing"),
  //                            QMessageBox::Ok);
  //   return;
  }

  setWindowTitle(str);

  // Initial default size of window
  int nWidth  = ui->centralwidget->width();
  int nHeight = ui->centralwidget->height();

  // Load MDF has been selected in the menu
  //connect(ui->actionLoadMdf, SIGNAL(triggered()), this, SLOT(loadSelectedMdf()));

  // Open pop up menu on right click on register list
  connect(ui->treeMDF,
          &QTreeWidget::customContextMenuRequested,
          this,
          &CFrmMdf::showMdfContextMenu);

  // New has been selected in the menu
  connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newMdf()));

  // Open has been selected in the menu
  connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openMdf()));

  // Register row has been clicked.
  // connect(ui->treeMDF,
  //           &QTreeWidget::itemClicked,
  //           this,
  //           &CFrmMdf::onFindNodesTreeWidgetItemClicked);        
}


///////////////////////////////////////////////////////////////////////////////
// DTor
//

CFrmMdf::~CFrmMdf()
{

}

///////////////////////////////////////////////////////////////////////////////
// newMdf
//

void 
CFrmMdf::newMdf(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
}

///////////////////////////////////////////////////////////////////////////////
// openMdf
//

void 
CFrmMdf::openMdf(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void CFrmMdf::done(int rv)
{
    if (QDialog::Accepted == rv) { // ok was pressed        
       // vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
        // Session window
        //pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
        
    }
    //QMainWindow::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// menu_open_main_settings
//

void
CFrmMdf::menu_open_main_settings(void)
{
  // CDlgMainSettings* dlg = new CDlgMainSettings(this);
  // dlg->exec();
}





///////////////////////////////////////////////////////////////////////////////
// showFindNodesContextMenu
// 

void
CFrmMdf::showMdfContextMenu(const QPoint& pos)
{

  QMenu* menu = new QMenu(this);
  menu->addAction(QString(tr("Fetch MDF")), this, SLOT(loadSelectedMdf()));
  menu->addAction(QString(tr("Fetch ALL MDF")), this, SLOT(loadAllMdf()));
  menu->addSeparator();
  menu->addAction(QString(tr("Rescan")), this, SLOT(doScan()));
  menu->addSeparator();
  menu->addAction(QString(tr("Configure")), this, SLOT(goConfig()));
  menu->addAction(QString(tr("Session")), this, SLOT(goSession()));
  menu->popup(ui->treeMDF->viewport()->mapToGlobal(pos));
}



///////////////////////////////////////////////////////////////////////////////
// loadMdf
//

void
CFrmMdf::loadMdf(void)
{
  // ui->btnScan->setEnabled(false);
  // CFoundNodeWidgetItem *pItem;
  // QTreeWidgetItemIterator it(ui->treeFound);
  // while (*it) {
  //   pItem = (CFoundNodeWidgetItem *)(*it);
  //   doLoadMdf(pItem->m_nodeid);
  //   ++it;
  // }
  // ui->btnScan->setEnabled(true);
}


///////////////////////////////////////////////////////////////////////////////
// onFindNodesTreeWidgetItemClicked
//

//void
//CFrmMdf::onFindNodesTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
//{
  // CFoundNodeWidgetItem *pItem = (CFoundNodeWidgetItem *)item;
  // if (nullptr == pItem) {
  //   return;
  // }

  // if ((pItem->type() == TREE_LIST_FOUND_NODE_TYPE) && pItem->m_bMdf && !pItem->m_bStdRegs) {

  //   // Set the HTML
  //   std::string html = vscp_getDeviceInfoHtml(pItem->m_mdf, pItem->m_stdregs);
  //   ui->infoArea->setHtml(html.c_str());
  // }
  // else {
  //   ui->infoArea->setText(tr("MDF info should be loaded before device info can be viewed"));
  // }
//}



