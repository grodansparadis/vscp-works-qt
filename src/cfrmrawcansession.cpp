// cfrmrawcansession.cpp
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

#ifndef WIN32

#include "cfrmrawcansession.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMap>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>
#include <QtSerialBus/QCanBus>

// ----------------------------------------------------------------------------

CFrmRawCanSession::CFrmRawCanSession(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_canDevice(nullptr)
  , m_autoConnectAttempted(false)
  , m_statusLabel(nullptr)
  , m_comboViewMode(nullptr)
  , m_tableIdFilters(nullptr)
  , m_stackViews(nullptr)
  , m_editFrameId(nullptr)
  , m_editPayload(nullptr)
  , m_chkExtended(nullptr)
  , m_chkFd(nullptr)
  , m_chkBitrateSwitch(nullptr)
  , m_chkErrorStateIndicator(nullptr)
  , m_chkRemoteRequest(nullptr)
  , m_btnConnect(nullptr)
  , m_btnSend(nullptr)
  , m_btnClear(nullptr)
  , m_btnAddFilter(nullptr)
  , m_btnRemoveFilter(nullptr)
  , m_tableFrames(nullptr)
  , m_tableSummary(nullptr)
{
  if (nullptr != pconn) {
    m_connObject = *pconn;
  }

  if (m_connObject.contains("device") && m_connObject["device"].is_string()) {
    m_interfaceName = m_connObject["device"].get<std::string>().c_str();
  }
  setupUi();
  setConnectedState(false);
}

CFrmRawCanSession::~CFrmRawCanSession()
{
  if (nullptr != m_canDevice) {
    m_canDevice->disconnectDevice();
    delete m_canDevice;
    m_canDevice = nullptr;
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);
  if (!m_autoConnectAttempted && (nullptr == m_canDevice)) {
    m_autoConnectAttempted = true;
    connectOrDisconnect();
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setupUi()
{
  setWindowTitle(tr("SocketCAN frame session - %1")
                   .arg(m_interfaceName.isEmpty() ? tr("Unknown device") : m_interfaceName));
  resize(1150, 720);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QHBoxLayout* topLayout = new QHBoxLayout;
  m_btnConnect           = new QPushButton(tr("Connect"), this);
  m_btnClear             = new QPushButton(tr("Clear"), this);
  QPushButton* btnHelp   = new QPushButton(tr("Help"), this);
  m_comboViewMode        = new QComboBox(this);
  m_comboViewMode->addItem(tr("Frame log"));
  m_comboViewMode->addItem(tr("ID statistics"));
  m_statusLabel          = new QLabel(this);
  m_statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  topLayout->addWidget(m_btnConnect);
  topLayout->addWidget(m_btnClear);
  topLayout->addWidget(btnHelp);
  topLayout->addSpacing(10);
  topLayout->addWidget(new QLabel(tr("View:"), this));
  topLayout->addWidget(m_comboViewMode);
  topLayout->addStretch(1);
  topLayout->addWidget(m_statusLabel, 1);
  mainLayout->addLayout(topLayout);

  QGroupBox* filterBox          = new QGroupBox(tr("ID filters"), this);
  QVBoxLayout* filterMainLayout = new QVBoxLayout(filterBox);
  m_tableIdFilters              = new QTableWidget(filterBox);
  m_tableIdFilters->setColumnCount(3);
  m_tableIdFilters->setHorizontalHeaderLabels(
    QStringList() << tr("Use") << tr("From ID") << tr("To ID"));
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_tableIdFilters->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableIdFilters->setAlternatingRowColors(true);
  filterMainLayout->addWidget(m_tableIdFilters);

  QHBoxLayout* filterBtnLayout = new QHBoxLayout;
  m_btnAddFilter               = new QPushButton(tr("Add range"), filterBox);
  m_btnRemoveFilter            = new QPushButton(tr("Remove selected"), filterBox);
  filterBtnLayout->addWidget(m_btnAddFilter);
  filterBtnLayout->addWidget(m_btnRemoveFilter);
  filterBtnLayout->addStretch(1);
  filterMainLayout->addLayout(filterBtnLayout);

  mainLayout->addWidget(filterBox);

  m_stackViews = new QStackedWidget(this);

  m_tableFrames = new QTableWidget(m_stackViews);
  m_tableFrames->setColumnCount(7);
  m_tableFrames->setAlternatingRowColors(true);
  m_tableFrames->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableFrames->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableFrames->setHorizontalHeaderLabels(
    QStringList() << tr("Time")
                  << tr("Dir")
                  << tr("ID")
                  << tr("Format")
                  << tr("Type")
                  << tr("DLC")
                  << tr("Data"));
  m_tableFrames->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
  m_tableFrames->horizontalHeader()->setStretchLastSection(true);
  m_stackViews->addWidget(m_tableFrames);

  m_tableSummary = new QTableWidget(m_stackViews);
  m_tableSummary->setColumnCount(5);
  m_tableSummary->setAlternatingRowColors(true);
  m_tableSummary->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableSummary->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableSummary->setHorizontalHeaderLabels(
    QStringList() << tr("ID")
                  << tr("Received")
                  << tr("Δt last (ms)")
                  << tr("Δt avg (ms)")
                  << tr("Data"));
  m_tableSummary->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_tableSummary->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_tableSummary->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_tableSummary->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_tableSummary->horizontalHeader()->setStretchLastSection(true);
  m_stackViews->addWidget(m_tableSummary);

  mainLayout->addWidget(m_stackViews, 1);

  QGroupBox* sendBox     = new QGroupBox(tr("Send frame"), this);
  QGridLayout* sendLayout = new QGridLayout(sendBox);

  m_editFrameId = new QLineEdit(sendBox);
  m_editFrameId->setPlaceholderText(tr("Example: 0x123 or 291"));
  m_editPayload = new QLineEdit(sendBox);
  m_editPayload->setPlaceholderText(
    tr("Hex bytes: 11 22 33 or 112233, or CSV values: 0x11,0o10,0b11,4"));

  m_chkExtended            = new QCheckBox(tr("Extended ID (29-bit)"), sendBox);
  m_chkFd                  = new QCheckBox(tr("CAN FD"), sendBox);
  m_chkBitrateSwitch       = new QCheckBox(tr("Bitrate switch (BRS)"), sendBox);
  m_chkErrorStateIndicator = new QCheckBox(tr("Error state indicator (ESI)"), sendBox);
  m_chkRemoteRequest       = new QCheckBox(tr("Remote request"), sendBox);

  m_btnSend = new QPushButton(tr("Send"), sendBox);

  sendLayout->addWidget(new QLabel(tr("Frame ID"), sendBox), 0, 0);
  sendLayout->addWidget(m_editFrameId, 0, 1, 1, 3);
  sendLayout->addWidget(new QLabel(tr("Payload"), sendBox), 1, 0);
  sendLayout->addWidget(m_editPayload, 1, 1, 1, 3);
  sendLayout->addWidget(m_chkExtended, 2, 0);
  sendLayout->addWidget(m_chkFd, 2, 1);
  sendLayout->addWidget(m_chkBitrateSwitch, 2, 2);
  sendLayout->addWidget(m_chkErrorStateIndicator, 2, 3);
  sendLayout->addWidget(m_chkRemoteRequest, 3, 0, 1, 2);
  sendLayout->addWidget(m_btnSend, 3, 3);

  mainLayout->addWidget(sendBox);

  connect(m_btnConnect, &QPushButton::clicked, this, &CFrmRawCanSession::connectOrDisconnect);
  connect(m_btnSend, &QPushButton::clicked, this, &CFrmRawCanSession::sendFrame);
  connect(m_btnClear, &QPushButton::clicked, this, &CFrmRawCanSession::clearLog);
  connect(btnHelp, &QPushButton::clicked, this, &CFrmRawCanSession::showHelp);
  connect(m_comboViewMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &CFrmRawCanSession::onViewModeChanged);
  connect(m_btnAddFilter, &QPushButton::clicked, this, &CFrmRawCanSession::addIdFilter);
  connect(m_btnRemoveFilter, &QPushButton::clicked, this, &CFrmRawCanSession::removeSelectedIdFilter);
  connect(m_tableIdFilters, &QTableWidget::itemChanged, this, &CFrmRawCanSession::onFilterTableChanged);

  addIdFilter();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::connectOrDisconnect()
{
  if (nullptr != m_canDevice) {
    m_canDevice->disconnectDevice();
    delete m_canDevice;
    m_canDevice = nullptr;
    setConnectedState(false);
    return;
  }

  if (m_interfaceName.isEmpty()) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("No SocketCAN interface is configured for this connection."),
                         QMessageBox::Ok);
    return;
  }

  QString errorString;
  m_canDevice = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),
                                                  m_interfaceName,
                                                  &errorString);
  if (nullptr == m_canDevice) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Failed to create SocketCAN device: %1").arg(errorString),
                         QMessageBox::Ok);
    setConnectedState(false);
    return;
  }

  m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);
  m_canDevice->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);

  connect(m_canDevice, &QCanBusDevice::framesReceived, this, &CFrmRawCanSession::processReceivedFrames);
  connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &CFrmRawCanSession::processError);

  if (!m_canDevice->connectDevice()) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Failed to connect to interface %1: %2")
                           .arg(m_interfaceName)
                           .arg(m_canDevice->errorString()),
                         QMessageBox::Ok);
    delete m_canDevice;
    m_canDevice = nullptr;
    setConnectedState(false);
    return;
  }

  setConnectedState(true);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::sendFrame()
{
  if (nullptr == m_canDevice) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Not connected to a SocketCAN interface."),
                         QMessageBox::Ok);
    return;
  }

  uint32_t frameId = 0;
  if (!parseFrameId(frameId)) {
    return;
  }

  QByteArray payload;
  if (!parsePayload(payload)) {
    return;
  }

  if (!m_chkExtended->isChecked() && (frameId > 0x7FFU)) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Standard CAN identifiers are limited to 0x7FF."),
                         QMessageBox::Ok);
    return;
  }

  if (m_chkExtended->isChecked() && (frameId > 0x1FFFFFFFU)) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Extended CAN identifiers are limited to 0x1FFFFFFF."),
                         QMessageBox::Ok);
    return;
  }

  if (m_chkFd->isChecked()) {
    const int size = payload.size();
    const bool validFdSize = (size <= 8) || (size == 12) || (size == 16) || (size == 20) ||
                             (size == 24) || (size == 32) || (size == 48) || (size == 64);
    if (!validFdSize) {
      QMessageBox::warning(this,
                           tr("VSCP Works"),
                           tr("CAN FD payload length must be 0-8, 12, 16, 20, 24, 32, 48 or 64 bytes."),
                           QMessageBox::Ok);
      return;
    }
  }
  else if (payload.size() > 8) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Standard CAN payload length must be 0-8 bytes."),
                         QMessageBox::Ok);
    return;
  }

  if (m_chkFd->isChecked() && m_chkRemoteRequest->isChecked()) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Remote request frames are not supported for CAN FD."),
                         QMessageBox::Ok);
    return;
  }

  QCanBusFrame frame(frameId, payload);
  frame.setExtendedFrameFormat(m_chkExtended->isChecked());
  frame.setFlexibleDataRateFormat(m_chkFd->isChecked());
  frame.setBitrateSwitch(m_chkBitrateSwitch->isChecked());
  frame.setErrorStateIndicator(m_chkErrorStateIndicator->isChecked());
  frame.setFrameType(m_chkRemoteRequest->isChecked()
                       ? QCanBusFrame::RemoteRequestFrame
                       : QCanBusFrame::DataFrame);

  if (!m_canDevice->writeFrame(frame)) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Failed to send frame: %1").arg(m_canDevice->errorString()),
                         QMessageBox::Ok);
    return;
  }

  appendFrame(frame, tr("TX"));
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::processReceivedFrames()
{
  while ((nullptr != m_canDevice) && m_canDevice->framesAvailable()) {
    appendFrame(m_canDevice->readFrame(), tr("RX"));
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::processError(QCanBusDevice::CanBusError error)
{
  if ((nullptr == m_canDevice) || (QCanBusDevice::NoError == error)) {
    return;
  }

  m_statusLabel->setText(tr("Error: %1").arg(m_canDevice->errorString()));
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::clearLog()
{
  m_frameHistory.clear();
  m_tableFrames->setRowCount(0);
  m_tableSummary->setRowCount(0);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::showHelp()
{
  const QString link = "https://grodansparadis.github.io/vscp-works-qt/#/rawcan_window";
  QDesktopServices::openUrl(QUrl(link));
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::onViewModeChanged(int index)
{
  m_stackViews->setCurrentIndex(index);
  refreshViews();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::addIdFilter()
{
  const QSignalBlocker blocker(m_tableIdFilters);

  const int row = m_tableIdFilters->rowCount();
  m_tableIdFilters->insertRow(row);

  QTableWidgetItem* useItem = new QTableWidgetItem;
  useItem->setFlags(useItem->flags() | Qt::ItemIsUserCheckable);
  useItem->setCheckState(Qt::Unchecked);
  m_tableIdFilters->setItem(row, 0, useItem);

  m_tableIdFilters->setItem(row, 1, new QTableWidgetItem("0x000"));
  m_tableIdFilters->setItem(row, 2, new QTableWidgetItem("0x7FF"));

  refreshFilterModelFromTable();
  refreshViews();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::removeSelectedIdFilter()
{
  const int row = m_tableIdFilters->currentRow();
  if (row < 0) {
    return;
  }

  m_tableIdFilters->removeRow(row);
  refreshFilterModelFromTable();
  refreshViews();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::onFilterTableChanged(QTableWidgetItem* item)
{
  Q_UNUSED(item);
  refreshFilterModelFromTable();
  refreshViews();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::appendFrame(const QCanBusFrame& frame, const QString& direction)
{
  FrameRecord rec;
  rec.timestamp = QDateTime::currentDateTime();
  rec.direction = direction;
  rec.frame     = frame;
  m_frameHistory.push_back(rec);

  refreshViews();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::refreshViews()
{
  if (m_stackViews->currentWidget() == m_tableFrames) {
    refreshFrameView();
  }
  else {
    refreshSummaryView();
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::refreshFrameView()
{
  m_tableFrames->setRowCount(0);

  for (const FrameRecord& rec : m_frameHistory) {
    if (!isFrameVisibleByFilter(rec.frame)) {
      continue;
    }

    const int row = m_tableFrames->rowCount();
    m_tableFrames->insertRow(row);

    const QString frameType = (QCanBusFrame::DataFrame == rec.frame.frameType())
                                ? tr("Data")
                                : (QCanBusFrame::RemoteRequestFrame == rec.frame.frameType()) ? tr("Remote")
                                                                                                : tr("Other");

    QTableWidgetItem* timeItem = new QTableWidgetItem(rec.timestamp.toString(Qt::ISODateWithMs));
    QTableWidgetItem* dirItem  = new QTableWidgetItem(rec.direction);
    QTableWidgetItem* idItem =
      new QTableWidgetItem(formatId(rec.frame.frameId(), rec.frame.hasExtendedFrameFormat()));
    QTableWidgetItem* formatItem =
      new QTableWidgetItem(rec.frame.hasExtendedFrameFormat() ? tr("EXT") : tr("STD"));
    QTableWidgetItem* typeItem = new QTableWidgetItem(frameType + " " + frameFlagsToString(rec.frame));
    QTableWidgetItem* dlcItem  = new QTableWidgetItem(QString::number(rec.frame.payload().size()));
    QTableWidgetItem* dataItem = new QTableWidgetItem(formatPayload(rec.frame.payload()));

    const QColor rowBgColor = rowBackgroundColorForDirection(rec.direction);
    const QColor rowFgColor = rowForegroundColorForDirection(rec.direction);
    for (QTableWidgetItem* item : { timeItem, dirItem, idItem, formatItem, typeItem, dlcItem, dataItem }) {
      item->setBackground(rowBgColor);
      item->setForeground(rowFgColor);
    }

    typeItem->setBackground(frameTypeBackgroundColor(rec.frame));
    typeItem->setForeground(frameTypeForegroundColor(rec.frame));

    m_tableFrames->setItem(row, 0, timeItem);
    m_tableFrames->setItem(row, 1, dirItem);
    m_tableFrames->setItem(row, 2, idItem);
    m_tableFrames->setItem(row, 3, formatItem);
    m_tableFrames->setItem(row, 4, typeItem);
    m_tableFrames->setItem(row, 5, dlcItem);
    m_tableFrames->setItem(row, 6, dataItem);
  }

  m_tableFrames->scrollToBottom();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::refreshSummaryView()
{
  struct SummaryData {
    bool extended;
    uint32_t id;
    int receivedCount;
    qint64 lastGapMs;
    qint64 sumGapMs;
    int gapCount;
    bool hasPrevious;
    QDateTime previousTs;
    QByteArray lastPayload;
  };

  m_tableSummary->setRowCount(0);
  QMap<QString, SummaryData> summaryMap;

  for (const FrameRecord& rec : m_frameHistory) {
    if (tr("RX") != rec.direction) {
      continue;
    }
    if (!isFrameVisibleByFilter(rec.frame)) {
      continue;
    }

    const bool extended = rec.frame.hasExtendedFrameFormat();
    const uint32_t id   = rec.frame.frameId();
    const QString key   = QString("%1:%2").arg(extended ? "E" : "S").arg(id);

    if (!summaryMap.contains(key)) {
      SummaryData data;
      data.extended     = extended;
      data.id           = id;
      data.receivedCount = 0;
      data.lastGapMs    = -1;
      data.sumGapMs     = 0;
      data.gapCount     = 0;
      data.hasPrevious  = false;
      data.lastPayload  = rec.frame.payload();
      summaryMap.insert(key, data);
    }

    SummaryData data = summaryMap.value(key);
    data.receivedCount++;
    data.lastPayload = rec.frame.payload();

    if (data.hasPrevious) {
      const qint64 gap = data.previousTs.msecsTo(rec.timestamp);
      data.lastGapMs   = gap;
      data.sumGapMs += gap;
      data.gapCount++;
    }

    data.hasPrevious = true;
    data.previousTs  = rec.timestamp;

    summaryMap.insert(key, data);
  }

  for (auto it = summaryMap.constBegin(); it != summaryMap.constEnd(); ++it) {
    const SummaryData data = it.value();
    const int row          = m_tableSummary->rowCount();
    m_tableSummary->insertRow(row);

    const QString avgGap =
      (data.gapCount > 0) ? QString::number(static_cast<double>(data.sumGapMs) / data.gapCount, 'f', 1) : "-";

    m_tableSummary->setItem(row, 0, new QTableWidgetItem(formatId(data.id, data.extended)));
    m_tableSummary->setItem(row, 1, new QTableWidgetItem(QString::number(data.receivedCount)));
    m_tableSummary->setItem(row, 2, new QTableWidgetItem((data.lastGapMs >= 0) ? QString::number(data.lastGapMs) : "-"));
    m_tableSummary->setItem(row, 3, new QTableWidgetItem(avgGap));
    m_tableSummary->setItem(row, 4, new QTableWidgetItem(formatPayload(data.lastPayload)));
  }
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::rowBackgroundColorForDirection(const QString& direction) const
{
  if (direction == tr("TX")) {
    return QColor(235, 244, 255);
  }
  if (direction == tr("RX")) {
    return QColor(236, 248, 236);
  }

  return QColor(Qt::white);
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::rowForegroundColorForDirection(const QString& direction) const
{
  if (direction == tr("TX")) {
    return QColor(13, 71, 161);
  }
  if (direction == tr("RX")) {
    return QColor(27, 94, 32);
  }

  return QColor(Qt::black);
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::frameTypeBackgroundColor(const QCanBusFrame& frame) const
{
  if (QCanBusFrame::RemoteRequestFrame == frame.frameType()) {
    return QColor(243, 229, 245);
  }
  if (frame.hasErrorStateIndicator()) {
    return QColor(255, 235, 238);
  }
  if (frame.hasBitrateSwitch()) {
    return QColor(224, 247, 250);
  }
  if (frame.hasFlexibleDataRateFormat()) {
    return QColor(227, 242, 253);
  }

  return QColor(250, 250, 250);
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::frameTypeForegroundColor(const QCanBusFrame& frame) const
{
  if (QCanBusFrame::RemoteRequestFrame == frame.frameType()) {
    return QColor(74, 20, 140);
  }
  if (frame.hasErrorStateIndicator()) {
    return QColor(183, 28, 28);
  }
  if (frame.hasBitrateSwitch()) {
    return QColor(0, 96, 100);
  }
  if (frame.hasFlexibleDataRateFormat()) {
    return QColor(13, 71, 161);
  }

  return QColor(33, 33, 33);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::refreshFilterModelFromTable()
{
  m_idFilters.clear();

  for (int row = 0; row < m_tableIdFilters->rowCount(); ++row) {
    QTableWidgetItem* useItem  = m_tableIdFilters->item(row, 0);
    QTableWidgetItem* fromItem = m_tableIdFilters->item(row, 1);
    QTableWidgetItem* toItem   = m_tableIdFilters->item(row, 2);
    if ((nullptr == useItem) || (nullptr == fromItem) || (nullptr == toItem)) {
      continue;
    }

    uint32_t idFrom = 0;
    uint32_t idTo   = 0;
    if (!parseIdValue(fromItem->text(), idFrom) || !parseIdValue(toItem->text(), idTo)) {
      continue;
    }
    if (idFrom > idTo) {
      continue;
    }

    IdFilterRange r;
    r.enabled = (Qt::Checked == useItem->checkState());
    r.idFrom  = idFrom;
    r.idTo    = idTo;
    m_idFilters.push_back(r);
  }
}

// ----------------------------------------------------------------------------

bool
CFrmRawCanSession::isFrameVisibleByFilter(const QCanBusFrame& frame) const
{
  bool hasEnabledFilter = false;
  for (const IdFilterRange& r : m_idFilters) {
    if (!r.enabled) {
      continue;
    }
    hasEnabledFilter = true;
    if ((frame.frameId() >= r.idFrom) && (frame.frameId() <= r.idTo)) {
      return true;
    }
  }

  return !hasEnabledFilter;
}

// ----------------------------------------------------------------------------

QString
CFrmRawCanSession::formatId(uint32_t id, bool extended) const
{
  return extended ? QString("0x%1").arg(id, 8, 16, QChar('0')).toUpper() : QString("0x%1").arg(id, 3, 16, QChar('0')).toUpper();
}

// ----------------------------------------------------------------------------

bool
CFrmRawCanSession::parseIdValue(const QString& value, uint32_t& id) const
{
  QString str = value.trimmed();
  if (str.isEmpty()) {
    return false;
  }

  int base = 10;
  if (str.startsWith("0x", Qt::CaseInsensitive)) {
    str  = str.mid(2);
    base = 16;
  }
  else if (str.startsWith("0b", Qt::CaseInsensitive)) {
    str  = str.mid(2);
    base = 2;
  }
  else if (str.startsWith("0o", Qt::CaseInsensitive)) {
    str  = str.mid(2);
    base = 8;
  }

  bool ok            = false;
  const uint64_t val = str.toULongLong(&ok, base);
  if (!ok || (val > 0x1FFFFFFFU)) {
    return false;
  }

  id = static_cast<uint32_t>(val);
  return true;
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setConnectedState(bool connected)
{
  m_btnConnect->setText(connected ? tr("Disconnect") : tr("Connect"));
  m_btnSend->setEnabled(connected);
  m_statusLabel->setText(connected
                           ? tr("Connected to %1").arg(m_interfaceName)
                           : tr("Disconnected"));
}

// ----------------------------------------------------------------------------

bool
CFrmRawCanSession::parseFrameId(uint32_t& id)
{
  const QString str = m_editFrameId->text().trimmed();
  if (str.isEmpty()) {
    QMessageBox::warning(this, tr("VSCP Works"), tr("Frame ID is required."), QMessageBox::Ok);
    return false;
  }

  if (!parseIdValue(str, id)) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Frame ID is not a valid number."),
                         QMessageBox::Ok);
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------

bool
CFrmRawCanSession::parsePayload(QByteArray& payload)
{
  auto parseByteToken = [](const QString& input, uint32_t& value) -> bool {
    QString token = input.trimmed();
    if (token.isEmpty()) {
      return false;
    }

    int base = 10;
    if (token.startsWith("0x", Qt::CaseInsensitive)) {
      token = token.mid(2);
      base  = 16;
    }
    else if (token.startsWith("0o", Qt::CaseInsensitive)) {
      token = token.mid(2);
      base  = 8;
    }
    else if (token.startsWith("0b", Qt::CaseInsensitive) || token.startsWith("ob", Qt::CaseInsensitive)) {
      token = token.mid(2);
      base  = 2;
    }

    bool ok            = false;
    const uint64_t val = token.toULongLong(&ok, base);
    if (!ok || (val > 0xFFU)) {
      return false;
    }

    value = static_cast<uint32_t>(val);
    return true;
  };

  payload.clear();
  QString str = m_editPayload->text().trimmed();
  if (str.isEmpty()) {
    return true;
  }

  if (str.contains(",")) {
    const QStringList parts = str.split(",", Qt::SkipEmptyParts);
    for (const QString& part : parts) {
      uint32_t value = 0;
      if (!parseByteToken(part, value)) {
        QMessageBox::warning(this,
                             tr("VSCP Works"),
                             tr("Payload contains invalid comma-separated value: %1").arg(part.trimmed()),
                             QMessageBox::Ok);
        return false;
      }
      payload.append(static_cast<char>(value));
    }
    return true;
  }

  str.replace(",", " ");
  str.replace("-", " ");
  const QStringList parts = str.split(" ", Qt::SkipEmptyParts);

  if (parts.size() > 1) {
    for (const QString& part : parts) {
      bool ok      = false;
      const int bt = part.toInt(&ok, 16);
      if (!ok || bt < 0 || bt > 255) {
        QMessageBox::warning(this,
                             tr("VSCP Works"),
                             tr("Payload contains invalid hex byte: %1").arg(part),
                             QMessageBox::Ok);
        return false;
      }
      payload.append(static_cast<char>(bt));
    }
    return true;
  }

  str.remove("0x");
  str.remove("0X");
  str.remove(" ");

  if (str.length() % 2) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Payload must contain an even number of hex digits."),
                         QMessageBox::Ok);
    return false;
  }

  const QByteArray data = QByteArray::fromHex(str.toLatin1());
  if ((str.length() > 0) && data.isEmpty()) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Failed to parse payload as hexadecimal data."),
                         QMessageBox::Ok);
    return false;
  }

  payload = data;
  return true;
}

// ----------------------------------------------------------------------------

QString
CFrmRawCanSession::formatPayload(const QByteArray& payload) const
{
  return QString(payload.toHex(' ')).toUpper();
}

// ----------------------------------------------------------------------------

QString
CFrmRawCanSession::frameFlagsToString(const QCanBusFrame& frame) const
{
  QStringList flags;
  if (frame.hasFlexibleDataRateFormat()) {
    flags << tr("FD");
  }
  if (frame.hasBitrateSwitch()) {
    flags << tr("BRS");
  }
  if (frame.hasErrorStateIndicator()) {
    flags << tr("ESI");
  }
  return flags.join("|");
}

#endif // !WIN32
