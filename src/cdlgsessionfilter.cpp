// cdlgsessionfilter.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>
#include <QDir>
#include <QFileInfo>

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

    connect(ui->listFilter, &QListWidget::currentRowChanged, this, &CDlgSessionFilter::filterSelected);
    connect(ui->listFilter, &QListWidget::itemDoubleClicked, this, &CDlgSessionFilter::editFilter);
    ui->editDescription->setReadOnly(true);

    // Help
    QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
    shortcut->setAutoRepeat(false);

    QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
    connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

    m_filters = loadDefaultFilters();
    fillList(0);
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

    menu->addAction(QString(tr("Import filters...")),
                    this,
                    SLOT(loadFilter()));

    menu->addAction(QString(tr("Export filters...")),
                    this,
                    SLOT(saveFilter()));

    menu->popup(ui->listFilter->viewport()->mapToGlobal(pos));
}


///////////////////////////////////////////////////////////////////////////////
// defaultFilterPath
//

QString
CDlgSessionFilter::defaultFilterPath(void)
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    return pworks->m_shareFolder + "/sessionfilters.json";
}

///////////////////////////////////////////////////////////////////////////////
// loadDefaultFilters
//

std::vector<CSessionFilter>
CDlgSessionFilter::loadDefaultFilters(void)
{
    std::vector<CSessionFilter> filters;
    QString path = defaultFilterPath();
    if (QFileInfo::exists(path) &&
        !CSessionFilter::readFilters(path.toStdString(), filters)) {
        spdlog::error("Session filters: Failed to read {}", path.toStdString());
    }
    return filters;
}

///////////////////////////////////////////////////////////////////////////////
// saveDefaultFilters
//

bool
CDlgSessionFilter::saveDefaultFilters(const std::vector<CSessionFilter>& filters)
{
    QString path = defaultFilterPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    if (!CSessionFilter::writeFilters(path.toStdString(), filters)) {
        spdlog::error("Session filters: Failed to write {}", path.toStdString());
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// fillList
//

void
CDlgSessionFilter::fillList(int selectRow)
{
    ui->listFilter->clear();
    for (auto const& filter : m_filters) {
        QString name = QString::fromStdString(filter.getName());
        if (!filter.isAllow()) {
            name += tr(" (deny)");
        }
        ui->listFilter->addItem(name);
    }
    if (selectRow >= ui->listFilter->count()) {
        selectRow = ui->listFilter->count() - 1;
    }
    ui->listFilter->setCurrentRow(selectRow);
    filterSelected(selectRow);
}

///////////////////////////////////////////////////////////////////////////////
// filterSelected
//

void
CDlgSessionFilter::filterSelected(int row)
{
    if ((row < 0) || (row >= (int)m_filters.size())) {
        ui->editDescription->clear();
        return;
    }

    const CSessionFilter& filter = m_filters[row];
    QString text = QString::fromStdString(filter.getDescription());
    QStringList constraints;
    for (uint8_t type : filter.getConstraintTypes()) {
        constraints << tr(CSessionFilter::getConstraintTypeName(type).c_str());
    }
    if (!text.isEmpty()) {
        text += "\n\n";
    }
    text += (filter.isAllow() ? tr("Shows events that meet: ") : tr("Hides events that meet: "));
    text += constraints.isEmpty() ? tr("(no constraints, all events)") : constraints.join(", ");
    ui->editDescription->setPlainText(text);
}

///////////////////////////////////////////////////////////////////////////////
// selectFilter
//

void
CDlgSessionFilter::selectFilter(const QString& name)
{
    for (int i = 0; i < (int)m_filters.size(); i++) {
        if (name.toStdString() == m_filters[i].getName()) {
            ui->listFilter->setCurrentRow(i);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// nameExists
//

bool
CDlgSessionFilter::nameExists(const std::string& name, int skip) const
{
    for (int i = 0; i < (int)m_filters.size(); i++) {
        if ((i != skip) && (name == m_filters[i].getName())) {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// addFilter
//

void
CDlgSessionFilter::addFilter()
{
    CDlgEditSessionFilter dlg(this);
    dlg.setWindowTitle(tr("Add session filter"));

    while (QDialog::Accepted == dlg.exec()) {
        CSessionFilter filter = dlg.getFilter();
        if (nameExists(filter.getName())) {
            QMessageBox::information(this, tr(APPNAME), tr("There is already a filter with this name."), QMessageBox::Ok);
            continue;
        }
        m_filters.push_back(filter);
        fillList((int)m_filters.size() - 1);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// editFilter
//

void
CDlgSessionFilter::editFilter()
{
    int row = ui->listFilter->currentRow();
    if ((row < 0) || (row >= (int)m_filters.size())) {
        QMessageBox::information(this, tr(APPNAME), tr("A filter must be selected"), QMessageBox::Ok);
        return;
    }

    CDlgEditSessionFilter dlg(this);
    dlg.setWindowTitle(tr("Edit session filter"));
    dlg.setFilter(m_filters[row]);

    while (QDialog::Accepted == dlg.exec()) {
        CSessionFilter filter = dlg.getFilter();
        if (nameExists(filter.getName(), row)) {
            QMessageBox::information(this, tr(APPNAME), tr("There is already a filter with this name."), QMessageBox::Ok);
            continue;
        }
        m_filters[row] = filter;
        fillList(row);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// cloneFilter
//

void
CDlgSessionFilter::cloneFilter()
{
    int row = ui->listFilter->currentRow();
    if ((row < 0) || (row >= (int)m_filters.size())) {
        QMessageBox::information(this, tr(APPNAME), tr("A filter must be selected"), QMessageBox::Ok);
        return;
    }

    CSessionFilter filter = m_filters[row];
    std::string name      = filter.getName() + " (copy)";
    for (int i = 2; nameExists(name); i++) {
        name = filter.getName() + " (copy " + std::to_string(i) + ")";
    }
    filter.setName(name);
    m_filters.insert(m_filters.begin() + row + 1, filter);
    fillList(row + 1);
}

///////////////////////////////////////////////////////////////////////////////
// deleteFilter
//

void
CDlgSessionFilter::deleteFilter()
{
    int row = ui->listFilter->currentRow();
    if ((row < 0) || (row >= (int)m_filters.size())) {
        QMessageBox::information(this, tr(APPNAME), tr("A filter must be selected"), QMessageBox::Ok);
        return;
    }

    if (QMessageBox::Yes != QMessageBox::question(this,
                                                  tr(APPNAME),
                                                  tr("Delete filter '%1'?").arg(QString::fromStdString(m_filters[row].getName())),
                                                  QMessageBox::Yes | QMessageBox::No)) {
        return;
    }

    m_filters.erase(m_filters.begin() + row);
    fillList(row);
}

///////////////////////////////////////////////////////////////////////////////
// loadFilter
//
// Import filters from a file. Filters with a name that is already used
// replace the existing ones.
//

void
CDlgSessionFilter::loadFilter()
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString fileName  = QFileDialog::getOpenFileName(this,
                                                    tr("Import session filters"),
                                                    pworks->m_shareFolder,
                                                    tr("Filter files (*.json);;All files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    std::vector<CSessionFilter> imported;
    if (!CSessionFilter::readFilters(fileName.toStdString(), imported)) {
        QMessageBox::warning(this, tr(APPNAME), tr("Unable to read session filters from %1").arg(fileName), QMessageBox::Ok);
        return;
    }

    for (auto const& filter : imported) {
        bool bReplaced = false;
        for (auto& existing : m_filters) {
            if (existing.getName() == filter.getName()) {
                existing  = filter;
                bReplaced = true;
                break;
            }
        }
        if (!bReplaced) {
            m_filters.push_back(filter);
        }
    }

    fillList(ui->listFilter->currentRow() < 0 ? 0 : ui->listFilter->currentRow());
    QMessageBox::information(this, tr(APPNAME), tr("%1 filter(s) imported.").arg(imported.size()), QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// saveFilter
//
// Export all filters to a file
//

void
CDlgSessionFilter::saveFilter()
{
    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
    QString fileName  = QFileDialog::getSaveFileName(this,
                                                    tr("Export session filters"),
                                                    pworks->m_shareFolder + "/sessionfilters-export.json",
                                                    tr("Filter files (*.json);;All files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    if (!CSessionFilter::writeFilters(fileName.toStdString(), m_filters)) {
        QMessageBox::warning(this, tr(APPNAME), tr("Unable to write session filters to %1").arg(fileName), QMessageBox::Ok);
    }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgSessionFilter::accept(void)
{
    if (!saveDefaultFilters(m_filters)) {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Unable to save the session filters to %1").arg(defaultFilterPath()),
                             QMessageBox::Ok);
        return;
    }
    QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSessionFilter::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=filter";
  QDesktopServices::openUrl(QUrl(link));
}