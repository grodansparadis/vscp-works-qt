// cdlgknownguid.cpp
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

#include <guid.h>
#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "cdlgknownguid.h"
#include "cdlgsensorindex.h"
#include "mainwindow.h"
#include "ui_cdlgknownguid.h"

#include "cdlgeditguid.h"

#include <QMenu>
#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgKnownGuid::CDlgKnownGuid(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgKnownGuid)
{
  ui->setupUi(this);
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  m_bShowOnlyInterfaces = false; // All GUID's are shown by default

  // edit on row double click
  m_bEnableDblClickAccept = false;

  ui->btnSave->setVisible(false);
  ui->btnLoad->setVisible(false);

  connect(ui->listGuid, &QTableWidget::itemClicked, this, &CDlgKnownGuid::listItemClicked);
  connect(ui->listGuid, &QTableWidget::itemDoubleClicked, this, &CDlgKnownGuid::listItemDoubleClicked);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listGuid,
          &QTableWidget::customContextMenuRequested,
          this,
          &CDlgKnownGuid::showContextMenu);

  connect(ui->btnSearch, &QPushButton::clicked, this, &CDlgKnownGuid::btnSearch);
  connect(ui->btnAdd, &QPushButton::clicked, this, &CDlgKnownGuid::btnAdd);
  connect(ui->btnEdit, &QPushButton::clicked, this, &CDlgKnownGuid::btnEdit);
  connect(ui->btnClone, &QPushButton::clicked, this, &CDlgKnownGuid::btnClone);
  connect(ui->btnDelete, &QPushButton::clicked, this, &CDlgKnownGuid::btnDelete);
  connect(ui->btnSensorIndex, &QPushButton::clicked, this, &CDlgKnownGuid::btnSensorIndex);
  connect(ui->btnLoad, &QPushButton::clicked, this, &CDlgKnownGuid::btnLoad);
  connect(ui->btnSave, &QPushButton::clicked, this, &CDlgKnownGuid::btnSave);

  connect(ui->checkShowInterfaces, &QCheckBox::clicked, this, &CDlgKnownGuid::showOnlyInterfaces);

  ui->textDescription->acceptRichText();

  QStringList headers(
    QString(tr("GUID, Name")).split(','));
  ui->listGuid->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu
  ui->listGuid->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->listGuid->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->listGuid->setColumnCount(2);
  ui->listGuid->setColumnWidth(0, 450); // GUID
  ui->listGuid->setColumnWidth(1, 200); // Name
  ui->listGuid->horizontalHeader()->setStretchLastSection(true);
  ui->listGuid->setHorizontalHeaderLabels(headers);

  // Fill in GUID's
  fillGuidFromDb();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgKnownGuid::~CDlgKnownGuid()
{
  delete ui;
}

void
CDlgKnownGuid::setInterfaceShow(bool b)
{
  ui->checkShowInterfaces->setChecked(b);
  showOnlyInterfaces();
}

///////////////////////////////////////////////////////////////////////////////
// fillGuidFromDb
//

bool
CDlgKnownGuid::fillGuidFromDb(void)
{
  int rv;
  sqlite3_stmt* ppStmt;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  pworks->m_mutexGuidMap.lock();

  //  Query known GUID's
  if (SQLITE_OK !=
      (rv = sqlite3_prepare(pworks->m_db_vscp_works,
                            "SELECT * FROM guid order by name",
                            -1,
                            &ppStmt,
                            NULL))) {
    spdlog::error("cdlgknownguid: Failed to query GUID's. rv={0} {1}", rv, sqlite3_errmsg(pworks->m_db_vscp_works));
    pworks->m_mutexGuidMap.unlock();
    return false;
  }

  while (SQLITE_ROW == sqlite3_step(ppStmt)) {
    QString guid = QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 1));
    QString name = QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 2));

    insertGuidItem(guid, name);
  }
  sqlite3_finalize(ppStmt);

  pworks->m_mutexGuidMap.unlock();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// insertGuidItem
//

void
CDlgKnownGuid::insertGuidItem(QString strguid, QString name)
{
  int row = ui->listGuid->rowCount();

  // Show only interfaces (two LSB's are zero)
  if (m_bShowOnlyInterfaces) {
    cguid ifguid(strguid.toStdString());
    if (ifguid[14] || ifguid[15]) {
      return;
    }
  }

  // Insert new row
  ui->listGuid->insertRow(row);

  // * * * GUID
  QTableWidgetItem* itemGuid = new QTableWidgetItem(strguid);

  // Set fixed font to make GUID's easier to read
  QFont font = QFont();
  font.setFamily("Courier");
  itemGuid->setFont(font);

  // Not editable
  itemGuid->setFlags(itemGuid->flags() & ~Qt::ItemIsEditable);

  ui->listGuid->setItem(ui->listGuid->rowCount() - 1, 0, itemGuid);

  // * * * Name
  QTableWidgetItem* itemName = new QTableWidgetItem(name);

  // Not editable
  itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);

  ui->listGuid->setItem(ui->listGuid->rowCount() - 1, 1, itemName);

  // Make all rows equal height
  ui->listGuid->setUpdatesEnabled(false);
  for (int i = 0; i < ui->listGuid->rowCount(); i++) {
    ui->listGuid->setRowHeight(i, 10);
  }
  ui->listGuid->setUpdatesEnabled(true);

  ui->listGuid->resizeRowsToContents();
}

///////////////////////////////////////////////////////////////////////////////
// listItemClicked
//

bool
CDlgKnownGuid::listItemClicked(QTableWidgetItem* item)
{
  int rv;
  sqlite3_stmt* ppStmt;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  int currentRow = ui->listGuid->selectionModel()->currentIndex().row();
  if (-1 == currentRow) {
    currentRow = 0; // First row
  }

  QTableWidgetItem* itemGuid = ui->listGuid->item(currentRow, 0);
  QString strguid            = itemGuid->text();

  // Search db record for description
  pworks->m_mutexGuidMap.lock();

  // Query known GUID's
  QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';").arg(strguid);
  if (SQLITE_OK !=
      (rv = sqlite3_prepare(pworks->m_db_vscp_works,
                            strQuery.toStdString().c_str(),
                            -1,
                            &ppStmt,
                            NULL))) {
    pworks->m_mutexGuidMap.unlock();
    spdlog::error("Unable to find guid {0}. rv={1} {2}", strguid.toStdString(), rv, sqlite3_errmsg(pworks->m_db_vscp_works));
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Unable to find GUID %1.\n\n rv=%2 %3").arg(strguid).arg(rv).arg(sqlite3_errmsg(pworks->m_db_vscp_works)),
                             QMessageBox::Ok);
    return false;
  }

  while (SQLITE_ROW == sqlite3_step(ppStmt)) {
#if QT_VERSION >= 0x050E00
    ui->textDescription->setMarkdown(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 3)));
#else
    ui->textDescription->setText(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 3)));
#endif
  }
  sqlite3_finalize(ppStmt);

  pworks->m_mutexGuidMap.unlock();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// listItemDoubleClicked
//

void
CDlgKnownGuid::listItemDoubleClicked(QTableWidgetItem* item)
{
  if (!m_bEnableDblClickAccept) {
    btnEdit();
  }
  else {
    accept();
  }
}

///////////////////////////////////////////////////////////////////////////////
// showContextMenu
//

void
CDlgKnownGuid::showContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString(tr("Add...")), this, SLOT(btnAdd()));
  menu->addAction(QString(tr("Edit...")), this, SLOT(btnEdit()));
  menu->addAction(QString(tr("Clone...")), this, SLOT(btnClone()));
  menu->addAction(QString(tr("Delete")), this, SLOT(btnDelete()));
  menu->addSeparator();
  menu->addAction(QString(tr("Sensor...")), this, SLOT(btnSensorIndex()));

  // menu->addAction(QString(tr("Load from file file...")), this, SLOT(btnLoad()));
  // menu->addAction(QString(tr("Save to file...")), this, SLOT(btnSave()));

  menu->popup(ui->listGuid->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// selectByGuid
//

bool
CDlgKnownGuid::selectByGuid(const QString& guid)
{
  for (int i = 0; i < ui->listGuid->rowCount(); i++) {

    QTableWidgetItem* itemGuid = ui->listGuid->item(i, 0);
    QTableWidgetItem* itemName = ui->listGuid->item(i, 1);

    if (itemGuid->text() == guid) {
      ui->listGuid->selectRow(i);
      return true;
    }
    else {
      ui->listGuid->clearSelection();
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void
CDlgKnownGuid::done(int rv)
{
  if (QDialog::Accepted == rv) { // ok was pressed

    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    // Session window
    // pworks->m_session_maxEvents = ui->editMaxSessionEvents->text().toInt();
  }
  QDialog::done(rv);
}

///////////////////////////////////////////////////////////////////////////////
// btnSearch
//

void
CDlgKnownGuid::btnSearch(void)
{
  /*!
      Search for GUID if ":" is in search term other wise search for name
      which must start with a letter.
  */
  int searchType = ui->comboSearchType->currentIndex(); // 0-exact, 1=start, 2=contains

  int currentRow = ui->listGuid->selectionModel()->currentIndex().row();
  if (-1 == currentRow) {
    currentRow = 0; // First row
  }
  else {
    currentRow++; // Row after the selected one
  }

  QString strsearch = ui->editSearch->text();

  for (int i = currentRow; i < ui->listGuid->rowCount(); i++) {

    QTableWidgetItem* itemGuid = ui->listGuid->item(i, 0);
    QTableWidgetItem* itemName = ui->listGuid->item(i, 1);

    // GUID exact match
    if (0 == searchType) {
      if (itemGuid->text() == ui->editSearch->text()) {
        // itemGuid->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
    // GUID starts with
    else if (1 == searchType) {
      if (itemGuid->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
        // itemGuid->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
    // GUID contains
    else if (2 == searchType) {
      if (itemGuid->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
        itemGuid->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
    // Name Exact match
    else if (3 == searchType) {
      if (itemName->text() == ui->editSearch->text()) {
        // itemName->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
    // Name starts with
    else if (4 == searchType) {
      if (itemName->text().startsWith(ui->editSearch->text(), Qt::CaseInsensitive)) {
        // itemName->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
    // Name contains
    else if (5 == searchType) {
      if (itemName->text().contains(ui->editSearch->text(), Qt::CaseInsensitive)) {
        // itemName->setSelected(true);
        ui->listGuid->selectRow(i);
        break;
      }
      else {
        ui->listGuid->clearSelection();
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// btnAdd
//

void
CDlgKnownGuid::btnAdd(void)
{
  int rv;
  sqlite3_stmt* ppStmt;

  CDlgEditGuid dlg;
  dlg.setWindowTitle(tr("Add new known GUID"));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  dlg.setGuid(m_addGuid);
  m_addGuid = "";

again:
  if (QDialog::Accepted == dlg.exec()) {

    QString strguid = dlg.getGuid();
    strguid         = strguid.trimmed();

    // Validate length
    if (47 < strguid.length()) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Invalid GUID. Length is wrong."),
                               QMessageBox::Ok);
      goto again;
    }

    // Validate format
    int cntColon = 0;
    for (int i = 0; i < strguid.length(); i++) {
      if (':' == strguid[i])
        cntColon++;
    }

    // Validate # of colons
    if (15 != cntColon) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Invalid GUID. Format is wrong. Should be like 'FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:0A:00:02:00:05'"),
                               QMessageBox::Ok);
      goto again;
    }

    // Add to the db table
    pworks->m_mutexGuidMap.lock();
    QString strQuery = tr("INSERT INTO guid (guid, name, description) VALUES ('%1', '%2', '%3');")
                         .arg(strguid)
                         .arg(dlg.getName())
                         .arg(dlg.getDescription());
    if (SQLITE_OK != (rv = sqlite3_exec(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), NULL, NULL, NULL))) {
      pworks->m_mutexGuidMap.unlock();
      spdlog::error(std::string(tr("Unable to save GUID into database (duplicate?). Err =").toStdString()) +
                    sqlite3_errmsg(pworks->m_db_vscp_works));
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Unable to save GUID into database (duplicate?).\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                               QMessageBox::Ok);
      goto again;
    }

    // Add to the internal table
    pworks->m_mapGuidToSymbolicName[strguid] = dlg.getName();
    pworks->m_mutexGuidMap.unlock();

    // Add to dialog List
    insertGuidItem(strguid, dlg.getName());

#if QT_VERSION >= 0x050E00
    ui->textDescription->setMarkdown(dlg.getDescription());
#else
    ui->textDescription->setText(dlg.getDescription());
#endif
    ui->listGuid->sortItems(0, Qt::AscendingOrder);

    // Select added item
    for (int i = 0; i < ui->listGuid->rowCount(); i++) {

      QTableWidgetItem* itemGuid = ui->listGuid->item(i, 0);

      if (itemGuid->text() == strguid) {
        ui->listGuid->selectRow(i);
        break;
        ;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// btnEdit
//

void
CDlgKnownGuid::btnEdit(void)
{
  int rv;
  sqlite3_stmt* ppStmt;
  CDlgEditGuid dlg;
  dlg.setWindowTitle(tr("Edit known GUID"));
  dlg.setEditMode();

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  int row = ui->listGuid->currentRow();
  if (-1 == row) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("No GUID is selected"),
                             QMessageBox::Ok);
    return;
  }
  QTableWidgetItem* itemGuid = ui->listGuid->item(row, 0);
  QString strguid            = itemGuid->text();
  strguid                    = strguid.trimmed();

  pworks->m_mutexGuidMap.lock();

  QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';").arg(strguid);
  if (SQLITE_OK != (rv = sqlite3_prepare(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), -1, &ppStmt, NULL))) {
    pworks->m_mutexGuidMap.unlock();
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Unable to find record in database.\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                             QMessageBox::Ok);
    spdlog::error(std::string(tr("Unable to find record in database. Err =").toStdString()) +
                  sqlite3_errmsg(pworks->m_db_vscp_works));
    return;
  }

  if (SQLITE_ROW == sqlite3_step(ppStmt)) {
    dlg.setGuid(QString::fromUtf8(sqlite3_column_text(ppStmt, 1)));
    dlg.setName(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 2)));
    dlg.setDescription(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 3)));
  }
  sqlite3_finalize(ppStmt);

  pworks->m_mutexGuidMap.unlock();

again:
  if (QDialog::Accepted == dlg.exec()) {

    QString strname        = dlg.getName();
    QString strdescription = dlg.getDescription();

    // Add to the db table
    pworks->m_mutexGuidMap.lock();
    QString strQuery = tr("UPDATE guid SET name='%1', description='%2' WHERE guid='%3';")
                         .arg(strname)
                         .arg(strdescription)
                         .arg(strguid);

    if (SQLITE_OK != (rv = sqlite3_exec(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), NULL, NULL, NULL))) {
      pworks->m_mutexGuidMap.unlock();
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Unable to save edited GUID into database.\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                               QMessageBox::Ok);
      spdlog::error(std::string(tr("Unable to save edited GUID into database. Err =").toStdString()) +
                    sqlite3_errmsg(pworks->m_db_vscp_works));
      goto again;
    }

    pworks->m_mutexGuidMap.unlock();

    // Add to the internal table
    pworks->m_mapGuidToSymbolicName[strguid] = strname;

    // Add to dialog List
    QTableWidgetItem* itemName = ui->listGuid->item(row, 1);
    itemName->setText(dlg.getName());
#if QT_VERSION >= 0x050E00
    ui->textDescription->setMarkdown(dlg.getDescription());
#else
    ui->textDescription->setText(dlg.getDescription());
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////
// btnClone
//

void
CDlgKnownGuid::btnClone(void)
{
  int rv;
  sqlite3_stmt* ppStmt;
  CDlgEditGuid dlg;
  dlg.setWindowTitle(tr("Clone GUID"));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  int row = ui->listGuid->currentRow();
  if (-1 == row) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("No GUID is selected"),
                             QMessageBox::Ok);
    return;
  }
  QTableWidgetItem* itemGuid = ui->listGuid->item(row, 0);
  QString strguid            = itemGuid->text();
  strguid                    = strguid.trimmed();

  QString strQuery = tr("SELECT * FROM guid WHERE guid='%1';").arg(strguid);
  pworks->m_mutexGuidMap.lock();

  if (SQLITE_OK != (rv = sqlite3_prepare(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), -1, &ppStmt, NULL))) {
    pworks->m_mutexGuidMap.unlock();
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Unable to find record in database.\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                             QMessageBox::Ok);
    spdlog::error(std::string(tr("Unable to find record in database. Err =").toStdString()) +
                  sqlite3_errmsg(pworks->m_db_vscp_works));
    return;
  }

  if (SQLITE_ROW == sqlite3_step(ppStmt)) {
    dlg.setGuid(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 1)));
    dlg.setName(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 2)));
    dlg.setDescription(QString::fromUtf8((const char*)sqlite3_column_text(ppStmt, 3)));
  }
  sqlite3_finalize(ppStmt);

  pworks->m_mutexGuidMap.unlock();

again:
  if (QDialog::Accepted == dlg.exec()) {

    QString strguid = dlg.getGuid();
    strguid         = strguid.trimmed();

    // Validate length
    if (47 != strguid.length()) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Invalid GUID. Length is wrong."),
                               QMessageBox::Ok);
      goto again;
    }

    // Validate format
    int cntColon = 0;
    for (int i = 0; i < strguid.length(); i++) {
      if (':' == strguid[i])
        cntColon++;
    }

    // Validate # of colons
    if (15 != cntColon) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Invalid GUID. Format is wrong. Should be like 'FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:0A:00:02:00:05'"),
                               QMessageBox::Ok);
      goto again;
    }

    QString strQuery = tr("INSERT INTO guid (guid, name, description) VALUES ('%1', '%2', '%3');")
                         .arg(strguid)
                         .arg(dlg.getName())
                         .arg(dlg.getDescription());
    pworks->m_mutexGuidMap.lock();

    if (SQLITE_OK != (rv = sqlite3_exec(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), NULL, NULL, NULL))) {
      pworks->m_mutexGuidMap.unlock();
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Unable to save GUID into database (duplicate?).\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                               QMessageBox::Ok);
      spdlog::error(std::string(tr("Unable to save GUID into database (duplicate?). Err =").toStdString()) +
                    sqlite3_errmsg(pworks->m_db_vscp_works));
      goto again;
    }

    pworks->m_mutexGuidMap.unlock();

    // Add to the internal table
    pworks->m_mapGuidToSymbolicName[strguid] = dlg.getName();

    // Add to dialog List
    insertGuidItem(strguid, dlg.getName());
  }
}

///////////////////////////////////////////////////////////////////////////////
// btnDelete
//

void
CDlgKnownGuid::btnDelete(void)
{
  int rv;
  sqlite3_stmt* ppStmt;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  int row = ui->listGuid->currentRow();
  if (-1 == row) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("No GUID is selected"),
                             QMessageBox::Ok);
    return;
  }

  rv = QMessageBox::warning(this,
                            tr(APPNAME),
                            tr("Are you sure?"),
                            QMessageBox::Yes | QMessageBox::Cancel);

  if (QMessageBox::Yes == rv) {
    QTableWidgetItem* item = ui->listGuid->item(row, 0);
    QString strguid        = item->text();
    strguid                = strguid.trimmed();

    QString strQuery = tr("DELETE FROM guid WHERE guid='%1';").arg(strguid);
    pworks->m_mutexGuidMap.lock();

    if (SQLITE_OK != (rv = sqlite3_exec(pworks->m_db_vscp_works, strQuery.toStdString().c_str(), NULL, NULL, NULL))) {
      pworks->m_mutexGuidMap.unlock();
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Unable to delete GUID.\n\n Error =") + sqlite3_errmsg(pworks->m_db_vscp_works),
                               QMessageBox::Ok);
      spdlog::error(std::string(tr("Unable to delete GUID. Err =").toStdString()) +
                    sqlite3_errmsg(pworks->m_db_vscp_works));
      return;
    }
    else {
      // Delete row
      ui->listGuid->removeRow(row);

      // Delete from internal table
      pworks->m_mapGuidToSymbolicName.erase(strguid);
    }

    pworks->m_mutexGuidMap.unlock();
  }
}

///////////////////////////////////////////////////////////////////////////////
// btnSensorIndex
//

void
CDlgKnownGuid::btnSensorIndex(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Must be a selected GUID
  int row = ui->listGuid->currentRow();
  if (-1 == row) {
    QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("No GUID is selected"),
                             QMessageBox::Ok);
    return;
  }

  QTableWidgetItem* item = ui->listGuid->item(row, 0);
  QString strguid        = item->text();
  strguid                = strguid.trimmed();

  item            = ui->listGuid->item(row, 1);
  QString strname = item->text();

  CDlgSensorIndex dlg(nullptr, pworks->getIdxForGuidRecord(strguid));

  dlg.setGuid(strguid);
  dlg.setGuidName(strname);

  if (QDialog::Accepted == dlg.exec()) {
  }
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedGuid
//

bool
CDlgKnownGuid::getSelectedGuid(cguid& guid)
{
  int row = ui->CDlgKnownGuid::listGuid->currentRow();
  if (-1 == row)
    return false;
  QTableWidgetItem* itemGuid = ui->listGuid->item(row, 0);
  QString strguid            = itemGuid->text();
  strguid                    = strguid.trimmed();
  guid.getFromString(strguid.toStdString());
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// btnLoad
//

void
CDlgKnownGuid::btnLoad(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// btnSave
//

void
CDlgKnownGuid::btnSave(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// showOnlyInterfaces
//

void
CDlgKnownGuid::showOnlyInterfaces(void)
{
  m_bShowOnlyInterfaces = ui->checkShowInterfaces->isChecked();
  // ui->listGuid->removeRows
  ui->listGuid->clearContents();
  ui->listGuid->model()->removeRows(0, ui->listGuid->rowCount());
  fillGuidFromDb();
}
