// cdlglogviewer.cpp
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

#include "cdlglogviewer.h"
#include "ui_cdlglogviewer.h"
#include "vscpworks.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFont>
#include <QScrollBar>
#include <QRegularExpression>
#include <QApplication>

///////////////////////////////////////////////////////////////////////////////
// CDlgLogViewer
//

CDlgLogViewer::CDlgLogViewer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CDlgLogViewer)
    , m_autoRefreshTimer(new QTimer(this))
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    // Table setup
    ui->tableLog->setColumnCount(2);
    ui->tableLog->horizontalHeader()->setStretchLastSection(true);
    ui->tableLog->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableLog->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableLog->verticalHeader()->setVisible(false);
    ui->tableLog->setShowGrid(false);

    QFont monoFont("Monospace");
    monoFont.setStyleHint(QFont::TypeWriter);
    monoFont.setPointSize(9);
    ui->tableLog->setFont(monoFont);

    // Dark background for the table
    ui->tableLog->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1e1e1e;"
        "  gridline-color: #2d2d2d;"
        "  color: #d4d4d4;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #264f78;"
        "  color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "  background-color: #252526;"
        "  color: #cccccc;"
        "  border: 1px solid #3e3e42;"
        "  padding: 4px;"
        "  font-weight: bold;"
        "}"
    );

    // Auto-refresh timer
    m_autoRefreshTimer->setInterval(5000);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &CDlgLogViewer::onAutoRefreshTick);

    // Connections
    connect(ui->btnRefresh,      &QPushButton::clicked,          this, &CDlgLogViewer::onRefresh);
    connect(ui->btnClear,        &QPushButton::clicked,          this, &CDlgLogViewer::onClearDisplay);
    connect(ui->btnSave,         &QPushButton::clicked,          this, &CDlgLogViewer::onSave);
    connect(ui->chkAutoRefresh,  &QCheckBox::toggled,            this, &CDlgLogViewer::onAutoRefreshToggled);
    connect(ui->comboLevel,      QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                  this, &CDlgLogViewer::onFilterChanged);
    connect(ui->editSearch,      &QLineEdit::textChanged,        this, &CDlgLogViewer::onSearchChanged);

    loadLogFile();
}

CDlgLogViewer::~CDlgLogViewer()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// levelColor  – foreground text colour for a given level string
//

QColor
CDlgLogViewer::levelColor(const QString &level) const
{
    QString l = level.toLower();
    if (l == "trace")    return QColor(0x88, 0x88, 0x88);   // dim grey
    if (l == "debug")    return QColor(0x56, 0xd3, 0xef);   // cyan
    if (l == "info")     return QColor(0x6a, 0xd9, 0x6f);   // green
    if (l == "warn" || l == "warning")
                         return QColor(0xff, 0xd7, 0x00);   // golden yellow
    if (l == "err" || l == "error")
                         return QColor(0xff, 0x55, 0x55);   // red
    if (l == "critical") return QColor(0xff, 0x00, 0x7f);   // hot pink
    return QColor(0xd4, 0xd4, 0xd4);                        // default white
}

///////////////////////////////////////////////////////////////////////////////
// levelBackgroundColor  – subtle tinted row background
//

QColor
CDlgLogViewer::levelBackgroundColor(const QString &level) const
{
    QString l = level.toLower();
    if (l == "trace")    return QColor(0x22, 0x22, 0x22);
    if (l == "debug")    return QColor(0x1a, 0x26, 0x2e);
    if (l == "info")     return QColor(0x1a, 0x26, 0x1a);
    if (l == "warn" || l == "warning")
                         return QColor(0x2e, 0x28, 0x10);
    if (l == "err" || l == "error")
                         return QColor(0x30, 0x14, 0x14);
    if (l == "critical") return QColor(0x38, 0x08, 0x1a);
    return QColor(0x1e, 0x1e, 0x1e);
}

///////////////////////////////////////////////////////////////////////////////
// parseLevelEnum
//

CDlgLogViewer::LogLevel
CDlgLogViewer::parseLevelEnum(const QString &level) const
{
    QString l = level.toLower();
    if (l == "trace")    return LogLevel::TRACE;
    if (l == "debug")    return LogLevel::DEBUG;
    if (l == "info")     return LogLevel::INFO;
    if (l == "warn" || l == "warning")  return LogLevel::WARNING;
    if (l == "err" || l == "error")     return LogLevel::ERR;
    if (l == "critical") return LogLevel::CRITICAL;
    return LogLevel::ALL;
}

///////////////////////////////////////////////////////////////////////////////
// loadLogFile  – reads the spdlog rotating file and parses entries
//

void
CDlgLogViewer::loadLogFile()
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

    QString path = QString::fromStdString(pworks->m_fileLogPath);

    m_entries.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // If file not found, show a placeholder entry
        LogEntry e;
        e.level   = "info";
        e.message = QString("Log file not found: %1").arg(path);
        m_entries.append(e);
        applyFilter();
        return;
    }

    // spdlog pattern: "[%^%l%$] %v"
    // The colour markers %^ / %$ are stripped in file sinks, so
    // lines look like:  [info] some message text
    // They may also contain ANSI escape sequences if the pattern uses
    // a colour sink accidentally – strip those too.
    static const QRegularExpression ansiEscape("\x1B\\[[0-9;]*m");
    // Match   [level] message   with optional leading ANSI colour codes
    static const QRegularExpression lineRe(R"(^\[([a-zA-Z]+)\]\s+(.*)$)");

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // Strip ANSI escapes
        line.remove(ansiEscape);
        line = line.trimmed();
        if (line.isEmpty()) continue;

        auto match = lineRe.match(line);
        if (match.hasMatch()) {
            LogEntry e;
            e.level   = match.captured(1);
            e.message = match.captured(2);
            m_entries.append(e);
        }
        else {
            // Continuation / unparsed line – append as info entry
            LogEntry e;
            e.level   = "info";
            e.message = line;
            m_entries.append(e);
        }
    }

    file.close();
    applyFilter();
}

///////////////////////////////////////////////////////////////////////////////
// applyFilter  – populates the table based on current level + search criteria
//

void
CDlgLogViewer::applyFilter()
{
    QString search      = ui->editSearch->text().trimmed();
    int     levelIndex  = ui->comboLevel->currentIndex();
    LogLevel filterLevel = static_cast<LogLevel>(levelIndex);

    // Minimum numeric severity for >= filtering
    auto levelSeverity = [](LogLevel l) -> int {
        switch (l) {
            case LogLevel::TRACE:    return 0;
            case LogLevel::DEBUG:    return 1;
            case LogLevel::INFO:     return 2;
            case LogLevel::WARNING:  return 3;
            case LogLevel::ERR:      return 4;
            case LogLevel::CRITICAL: return 5;
            default:                 return -1;
        }
    };
    int minSeverity = levelSeverity(filterLevel);

    ui->tableLog->setUpdatesEnabled(false);
    ui->tableLog->clearContents();
    ui->tableLog->setRowCount(0);

    int visibleCount = 0;
    for (const LogEntry &e : qAsConst(m_entries)) {
        // Severity filter
        if (filterLevel != LogLevel::ALL) {
            int sev = levelSeverity(parseLevelEnum(e.level));
            if (sev < minSeverity) continue;
        }
        // Text search
        if (!search.isEmpty() &&
            !e.message.contains(search, Qt::CaseInsensitive) &&
            !e.level.contains(search, Qt::CaseInsensitive)) {
            continue;
        }

        int row = ui->tableLog->rowCount();
        ui->tableLog->insertRow(row);

        QColor fg = levelColor(e.level);
        QColor bg = levelBackgroundColor(e.level);

        // Level column
        auto *levelItem = new QTableWidgetItem(e.level.toUpper());
        levelItem->setForeground(fg);
        levelItem->setBackground(bg);
        QFont boldFont = ui->tableLog->font();
        boldFont.setBold(true);
        levelItem->setFont(boldFont);
        levelItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableLog->setItem(row, 0, levelItem);

        // Message column
        auto *msgItem = new QTableWidgetItem(e.message);
        msgItem->setForeground(fg);
        msgItem->setBackground(bg);
        ui->tableLog->setItem(row, 1, msgItem);

        ++visibleCount;
    }

    ui->tableLog->setUpdatesEnabled(true);

    // Scroll to last entry
    if (ui->tableLog->rowCount() > 0) {
        ui->tableLog->scrollToBottom();
    }

    ui->labelEntries->setText(
        QString("Entries: %1 / %2").arg(visibleCount).arg(m_entries.size()));
}

///////////////////////////////////////////////////////////////////////////////
// onRefresh
//

void
CDlgLogViewer::onRefresh()
{
    loadLogFile();
}

///////////////////////////////////////////////////////////////////////////////
// onClearDisplay
//

void
CDlgLogViewer::onClearDisplay()
{
    m_entries.clear();
    ui->tableLog->clearContents();
    ui->tableLog->setRowCount(0);
    ui->labelEntries->setText("Entries: 0 / 0");
}

///////////////////////////////////////////////////////////////////////////////
// onSave  – save visible rows to a text file
//

void
CDlgLogViewer::onSave()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Log"),
        QString(),
        tr("Text files (*.txt);;All files (*)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < ui->tableLog->rowCount(); ++row) {
        QString level = ui->tableLog->item(row, 0)
                            ? ui->tableLog->item(row, 0)->text()
                            : QString();
        QString msg   = ui->tableLog->item(row, 1)
                            ? ui->tableLog->item(row, 1)->text()
                            : QString();
        out << QString("[%1] %2\n").arg(level, -8).arg(msg);
    }
}

///////////////////////////////////////////////////////////////////////////////
// onAutoRefreshToggled
//

void
CDlgLogViewer::onAutoRefreshToggled(bool checked)
{
    if (checked) {
        m_autoRefreshTimer->start();
    }
    else {
        m_autoRefreshTimer->stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
// onAutoRefreshTick
//

void
CDlgLogViewer::onAutoRefreshTick()
{
    loadLogFile();
}

///////////////////////////////////////////////////////////////////////////////
// onFilterChanged
//

void
CDlgLogViewer::onFilterChanged()
{
    applyFilter();
}

///////////////////////////////////////////////////////////////////////////////
// onSearchChanged
//

void
CDlgLogViewer::onSearchChanged(const QString &)
{
    applyFilter();
}
