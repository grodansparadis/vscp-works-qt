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
#include <QInputDialog>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>
#include <QUrl>
#include <QSize>
#include <QVBoxLayout>
#include <QtSerialBus/QCanBus>

namespace {

QString
resolveCanBusBackendName()
{
  const QStringList availablePlugins = QCanBus::instance()->plugins();
  const QStringList preferredBackends = []() {
#if defined(Q_OS_LINUX)
    return QStringList{QStringLiteral("socketcan")};
#elif defined(Q_OS_WIN)
    return QStringList{QStringLiteral("peakcan"), QStringLiteral("virtualcan"), QStringLiteral("socketcan")};
#elif defined(Q_OS_MACOS)
    return QStringList{QStringLiteral("virtualcan"), QStringLiteral("socketcan")};
#else
    return QStringList{QStringLiteral("socketcan")};
#endif
  }();

  for (const QString& backend : preferredBackends) {
    if (availablePlugins.contains(backend)) {
      return backend;
    }
  }

  return preferredBackends.isEmpty() ? QString() : preferredBackends.first();
}

} // namespace

// ----------------------------------------------------------------------------

CFrmRawCanSession::CFrmRawCanSession(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_canDevice(nullptr)
  , m_autoConnectAttempted(false)
  , m_paused(false)
  , m_statusLabel(nullptr)
  , m_comboViewMode(nullptr)
  , m_tableIdFilters(nullptr)
  , m_stackViews(nullptr)
  , m_idFilterDialog(nullptr)
  , m_sendFrameBox(nullptr)
  , m_templatesBox(nullptr)
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
  , m_treeFrames(nullptr)
  , m_tableSummary(nullptr)
  , m_treeTemplates(nullptr)
  , m_menuBar(nullptr)
  , m_toolBar(nullptr)
  , m_actSaveCurrentFrame(nullptr)
  , m_actSendSelectedFrame(nullptr)
  , m_actDeleteSelectedFrame(nullptr)
  , m_actClearFrames(nullptr)
  , m_actLoadFromDisk(nullptr)
  , m_actSaveToDisk(nullptr)
  , m_actShowIdFilters(nullptr)
  , m_actToggleSendFrame(nullptr)
  , m_actToggleSavedFrames(nullptr)
  , m_actPause(nullptr)
  , m_actConnect(nullptr)
  , m_actClearLog(nullptr)
  , m_actHelp(nullptr)
{
  if (nullptr != pconn) {
    m_connObject = *pconn;
  }

  if (m_connObject.contains("device") && m_connObject["device"].is_string()) {
    m_interfaceName = m_connObject["device"].get<std::string>().c_str();
  }
  setupUi();
  loadTemplatesFromDisk();
  refreshTemplatesView();
  setConnectedState(false);
}

CFrmRawCanSession::~CFrmRawCanSession()
{
  saveTemplatesToDisk();
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
    QTimer::singleShot(0, this, &CFrmRawCanSession::connectOrDisconnect);
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setupUi()
{
  setWindowTitle(tr("CAN frame session - %1")
                   .arg(m_interfaceName.isEmpty() ? tr("Unknown device") : m_interfaceName));
  resize(1250, 780);
  setMinimumSize(980, 620);
  setStyleSheet(R"(
    QDialog {
      background: #f7f9fc;
      color: #1f2937;
    }
    QToolBar, QMenuBar {
      background: #ffffff;
      border: 1px solid #dbe3ee;
      padding: 4px;
    }
    QToolBar QToolButton, QPushButton, QComboBox, QLineEdit, QTreeWidget, QTableWidget, QGroupBox {
      border-radius: 6px;
    }
    QPushButton {
      background: #eef4ff;
      border: 1px solid #c9d8f2;
      padding: 6px 10px;
    }
    QPushButton:hover {
      background: #e2ecff;
    }
    QPushButton:pressed {
      background: #d7e6ff;
    }
    QComboBox, QLineEdit {
      border: 1px solid #c9d8f2;
      padding: 5px 7px;
      background: #ffffff;
    }
    QGroupBox {
      border: 1px solid #dbe3ee;
      margin-top: 10px;
      padding-top: 8px;
      background: #ffffff;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 10px;
      padding: 0 6px 0 6px;
      color: #4b5563;
      font-weight: 600;
    }
    QTreeWidget, QTableWidget {
      border: 1px solid #dbe3ee;
      background: #ffffff;
      alternate-background-color: #f8fbff;
    }
    QHeaderView::section {
      background: #eef4ff;
      color: #374151;
      padding: 6px;
      border: 1px solid #dbe3ee;
    }
    QLabel {
      color: #374151;
    }
  )");

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(14, 14, 14, 14);
  mainLayout->setSpacing(10);

  m_menuBar = new QMenuBar(this);
  m_toolBar = new QToolBar(tr("Raw CAN"), this);
  m_toolBar->setMovable(false);
  m_toolBar->setFloatable(false);
  m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_toolBar->setIconSize(QSize(16, 16));
  mainLayout->addWidget(m_menuBar);
  mainLayout->addWidget(m_toolBar);

  QMenu* fileMenu = m_menuBar->addMenu(tr("&Frame"));
  m_actSaveCurrentFrame = fileMenu->addAction(tr("Save current frame"));
  m_actSendSelectedFrame = fileMenu->addAction(tr("Send selected frame"));
  m_actDeleteSelectedFrame = fileMenu->addAction(tr("Delete selected frame"));
  fileMenu->addSeparator();
  m_actClearFrames = fileMenu->addAction(tr("Clear receive log"));
  fileMenu->addSeparator();
  m_actLoadFromDisk = fileMenu->addAction(tr("Reload saved frames"));
  m_actSaveToDisk = fileMenu->addAction(tr("Save frames to disk"));

  m_actShowIdFilters = fileMenu->addAction(tr("ID filters..."));
  m_actShowIdFilters->setToolTip(tr("Open the ID filter dialog"));
  fileMenu->addSeparator();
  m_actToggleSendFrame = fileMenu->addAction(tr("Show send frame"));
  m_actToggleSendFrame->setCheckable(true);
  m_actToggleSendFrame->setChecked(true);
  m_actToggleSavedFrames = fileMenu->addAction(tr("Show saved frames"));
  m_actToggleSavedFrames->setCheckable(true);
  m_actToggleSavedFrames->setChecked(true);

  m_actConnect = m_toolBar->addAction(tr("Connect"));
  m_actClearLog = m_toolBar->addAction(tr("Clear"));
  m_actPause = m_toolBar->addAction(tr("Pause"));
  m_actPause->setCheckable(true);
  m_actHelp = m_toolBar->addAction(tr("Help"));
  m_toolBar->addSeparator();
  m_toolBar->addAction(m_actSaveCurrentFrame);
  m_toolBar->addAction(m_actSendSelectedFrame);
  m_toolBar->addAction(m_actDeleteSelectedFrame);
  m_toolBar->addSeparator();
  m_toolBar->addAction(m_actClearFrames);

  QHBoxLayout* topLayout = new QHBoxLayout;
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(8);
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

  m_stackViews = new QStackedWidget(this);

  m_treeFrames = new QTreeWidget(m_stackViews);
  m_treeFrames->setAlternatingRowColors(true);
  m_treeFrames->setColumnCount(5);
  m_treeFrames->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_treeFrames->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_treeFrames->setHeaderLabels(QStringList() << tr("Time") << tr("ID") << tr("Type") << tr("Format") << tr("Data"));
  m_treeFrames->setUniformRowHeights(true);
  m_treeFrames->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_treeFrames->setRootIsDecorated(false);
  m_treeFrames->setAllColumnsShowFocus(true);
  m_treeFrames->setIndentation(0);
  m_treeFrames->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_treeFrames->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_treeFrames->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_treeFrames->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_treeFrames->header()->setSectionResizeMode(4, QHeaderView::Stretch);
  m_stackViews->addWidget(m_treeFrames);

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

  QVBoxLayout* bottomLayout = new QVBoxLayout;
  bottomLayout->setSpacing(10);
  m_sendFrameBox = new QGroupBox(tr("Send frame"), this);
  QGridLayout* sendLayout = new QGridLayout(m_sendFrameBox);

  m_editFrameId = new QLineEdit(m_sendFrameBox);
  m_editFrameId->setPlaceholderText(tr("Example: 0x123 or 291"));
  m_editPayload = new QLineEdit(m_sendFrameBox);
  m_editPayload->setPlaceholderText(
    tr("Hex bytes: 11 22 33 or 112233, or CSV values: 0x11,0o10,0b11,4"));

  m_chkExtended            = new QCheckBox(tr("Extended ID (29-bit)"), m_sendFrameBox);
  m_chkFd                  = new QCheckBox(tr("CAN FD"), m_sendFrameBox);
  m_chkBitrateSwitch       = new QCheckBox(tr("Bitrate switch (BRS)"), m_sendFrameBox);
  m_chkErrorStateIndicator = new QCheckBox(tr("Error state indicator (ESI)"), m_sendFrameBox);
  m_chkRemoteRequest       = new QCheckBox(tr("Remote request"), m_sendFrameBox);

  m_btnSend = new QPushButton(tr("Send"), m_sendFrameBox);
  m_btnSend->setMinimumHeight(34);

  sendLayout->addWidget(new QLabel(tr("Frame ID"), m_sendFrameBox), 0, 0);
  sendLayout->addWidget(m_editFrameId, 0, 1, 1, 3);
  sendLayout->addWidget(new QLabel(tr("Payload"), m_sendFrameBox), 1, 0);
  sendLayout->addWidget(m_editPayload, 1, 1, 1, 3);
  sendLayout->addWidget(m_chkExtended, 2, 0);
  sendLayout->addWidget(m_chkFd, 2, 1);
  sendLayout->addWidget(m_chkBitrateSwitch, 2, 2);
  sendLayout->addWidget(m_chkErrorStateIndicator, 2, 3);
  sendLayout->addWidget(m_chkRemoteRequest, 3, 0, 1, 2);
  sendLayout->addWidget(m_btnSend, 3, 3);

  m_templatesBox = new QGroupBox(tr("Saved frames"), this);
  QVBoxLayout* templatesLayout = new QVBoxLayout(m_templatesBox);
  m_treeTemplates = new QTreeWidget(m_templatesBox);
  m_treeTemplates->setColumnCount(2);
  m_treeTemplates->setHeaderLabels(QStringList() << tr("Name") << tr("Frame"));
  m_treeTemplates->setAlternatingRowColors(true);
  m_treeTemplates->setUniformRowHeights(true);
  m_treeTemplates->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_treeTemplates->setSelectionMode(QAbstractItemView::SingleSelection);
  templatesLayout->addWidget(m_treeTemplates);
  QHBoxLayout* templateButtons = new QHBoxLayout;
  QPushButton* btnSaveTemplate = new QPushButton(tr("Save current"), m_templatesBox);
  QPushButton* btnLoadTemplate = new QPushButton(tr("Load selected"), m_templatesBox);
  QPushButton* btnDeleteTemplate = new QPushButton(tr("Delete"), m_templatesBox);
  templateButtons->addWidget(btnSaveTemplate);
  templateButtons->addWidget(btnLoadTemplate);
  templateButtons->addWidget(btnDeleteTemplate);
  templatesLayout->addLayout(templateButtons);

  QSplitter* bottomSplitter = new QSplitter(Qt::Horizontal, this);
  bottomSplitter->setChildrenCollapsible(false);
  bottomSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  bottomSplitter->addWidget(m_sendFrameBox);
  bottomSplitter->addWidget(m_templatesBox);
  bottomSplitter->setStretchFactor(0, 1);
  bottomSplitter->setStretchFactor(1, 1);
  bottomLayout->addWidget(bottomSplitter);
  mainLayout->addLayout(bottomLayout);

  connect(m_btnConnect, &QPushButton::clicked, this, &CFrmRawCanSession::connectOrDisconnect);
  connect(m_actConnect, &QAction::triggered, this, &CFrmRawCanSession::connectOrDisconnect);
  connect(m_treeFrames, &QTreeWidget::itemSelectionChanged, this, &CFrmRawCanSession::onFrameSelectionChanged);
  connect(m_btnSend, &QPushButton::clicked, this, &CFrmRawCanSession::sendFrame);
  connect(m_btnClear, &QPushButton::clicked, this, &CFrmRawCanSession::clearLog);
  connect(m_actClearLog, &QAction::triggered, this, &CFrmRawCanSession::clearLog);
  connect(btnHelp, &QPushButton::clicked, this, &CFrmRawCanSession::showHelp);
  connect(m_actHelp, &QAction::triggered, this, &CFrmRawCanSession::showHelp);
  connect(m_comboViewMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &CFrmRawCanSession::onViewModeChanged);
  connect(m_actShowIdFilters, &QAction::triggered, this, &CFrmRawCanSession::showIdFiltersDialog);
  connect(m_actToggleSendFrame, &QAction::toggled, this, &CFrmRawCanSession::setSendFrameVisible);
  connect(m_actToggleSavedFrames, &QAction::toggled, this, &CFrmRawCanSession::setSavedFramesVisible);
  connect(m_actPause, &QAction::toggled, this, &CFrmRawCanSession::setPaused);
  connect(m_actSaveCurrentFrame, &QAction::triggered, this, &CFrmRawCanSession::saveCurrentFrameAsTemplate);
  connect(m_actSendSelectedFrame, &QAction::triggered, this, &CFrmRawCanSession::sendSelectedTemplate);
  connect(m_actDeleteSelectedFrame, &QAction::triggered, this, &CFrmRawCanSession::deleteSelectedTemplate);
  connect(m_actClearFrames, &QAction::triggered, this, &CFrmRawCanSession::clearLog);
  connect(m_actLoadFromDisk, &QAction::triggered, this, &CFrmRawCanSession::loadTemplatesFromDisk);
  connect(m_actSaveToDisk, &QAction::triggered, this, &CFrmRawCanSession::saveTemplatesToDisk);
  connect(btnSaveTemplate, &QPushButton::clicked, this, &CFrmRawCanSession::saveCurrentFrameAsTemplate);
  connect(btnLoadTemplate, &QPushButton::clicked, this, &CFrmRawCanSession::sendSelectedTemplate);
  connect(btnDeleteTemplate, &QPushButton::clicked, this, &CFrmRawCanSession::deleteSelectedTemplate);
  connect(m_treeTemplates, &QTreeWidget::itemSelectionChanged, this, &CFrmRawCanSession::onTemplateSelectionChanged);

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
                         tr("No CAN interface is configured for this connection."),
                         QMessageBox::Ok);
    return;
  }

  const QString backendName = resolveCanBusBackendName();
  if (backendName.isEmpty()) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("No Qt CAN bus backend is available on this platform."),
                         QMessageBox::Ok);
    setConnectedState(false);
    return;
  }

  QString errorString;
  m_canDevice = QCanBus::instance()->createDevice(backendName,
                                                  m_interfaceName,
                                                  &errorString);
  if (nullptr == m_canDevice) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Failed to create %1 CAN device: %2")
                           .arg(backendName)
                           .arg(errorString.isEmpty() ? tr("no backend available") : errorString),
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
                         tr("Not connected to a CAN interface."),
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

  appendFrame(frame, FrameDirection::Tx);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::processReceivedFrames()
{
  if (m_paused) {
    return;
  }

  while ((nullptr != m_canDevice) && m_canDevice->framesAvailable()) {
    appendFrame(m_canDevice->readFrame(), FrameDirection::Rx);
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
  m_treeFrames->clear();
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
  if (nullptr == m_tableIdFilters) {
    return;
  }

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
  if (nullptr == m_tableIdFilters) {
    return;
  }

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
CFrmRawCanSession::showIdFiltersDialog()
{
  if (nullptr != m_idFilterDialog) {
    m_idFilterDialog->raise();
    m_idFilterDialog->activateWindow();
    return;
  }

  QDialog* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowTitle(tr("CAN ID filters"));
  dialog->resize(420, 260);
  m_idFilterDialog = dialog;

  QVBoxLayout* layout = new QVBoxLayout(dialog);
  m_tableIdFilters = new QTableWidget(dialog);
  m_tableIdFilters->setColumnCount(3);
  m_tableIdFilters->setHorizontalHeaderLabels(QStringList() << tr("Use") << tr("From ID") << tr("To ID"));
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_tableIdFilters->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_tableIdFilters->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableIdFilters->setAlternatingRowColors(true);
  layout->addWidget(m_tableIdFilters);

  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  m_btnAddFilter = new QPushButton(tr("Add range"), dialog);
  m_btnRemoveFilter = new QPushButton(tr("Remove selected"), dialog);
  QPushButton* btnClose = new QPushButton(tr("Close"), dialog);
  buttonsLayout->addWidget(m_btnAddFilter);
  buttonsLayout->addWidget(m_btnRemoveFilter);
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(btnClose);
  layout->addLayout(buttonsLayout);

  connect(m_btnAddFilter, &QPushButton::clicked, this, &CFrmRawCanSession::addIdFilter);
  connect(m_btnRemoveFilter, &QPushButton::clicked, this, &CFrmRawCanSession::removeSelectedIdFilter);
  connect(m_tableIdFilters, &QTableWidget::itemChanged, this, &CFrmRawCanSession::onFilterTableChanged);
  connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
  connect(dialog, &QDialog::finished, this, [this, dialog](int) {
    if (m_idFilterDialog == dialog) {
      refreshFilterModelFromTable();
      refreshViews();
      m_idFilterDialog = nullptr;
      m_tableIdFilters = nullptr;
      m_btnAddFilter = nullptr;
      m_btnRemoveFilter = nullptr;
    }
  });

  populateFilterTableFromModel();
  if (m_tableIdFilters->rowCount() == 0) {
    addIdFilter();
  }
  dialog->show();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setSendFrameVisible(bool visible)
{
  if (nullptr != m_sendFrameBox) {
    m_sendFrameBox->setVisible(visible);
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setSavedFramesVisible(bool visible)
{
  if (nullptr != m_templatesBox) {
    m_templatesBox->setVisible(visible);
  }
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::appendFrame(const QCanBusFrame& frame, FrameDirection direction)
{
  FrameRecord rec;
  rec.timestamp = QDateTime::currentDateTime();
  rec.direction = direction;
  rec.frame     = frame;
  m_frameHistory.push_back(rec);

  refreshViews();
}

void
CFrmRawCanSession::saveCurrentFrameAsTemplate()
{
  bool ok = false;
  QString name = QInputDialog::getText(this,
                                        tr("Save frame"),
                                        tr("Template name"),
                                        QLineEdit::Normal,
                                        QString(),
                                        &ok);
  if (!ok || name.trimmed().isEmpty()) {
    return;
  }

  SavedFrameTemplate templateItem;
  templateItem.name = name.trimmed();
  templateItem.frameId = m_editFrameId->text().trimmed();
  templateItem.payload = m_editPayload->text().trimmed();
  templateItem.extended = m_chkExtended->isChecked();
  templateItem.fd = m_chkFd->isChecked();
  templateItem.bitrateSwitch = m_chkBitrateSwitch->isChecked();
  templateItem.errorStateIndicator = m_chkErrorStateIndicator->isChecked();
  templateItem.remoteRequest = m_chkRemoteRequest->isChecked();

  m_savedTemplates.push_back(templateItem);
  refreshTemplatesView();
  saveTemplatesToDisk();
}

void
CFrmRawCanSession::deleteSelectedTemplate()
{
  const auto selectedItems = m_treeTemplates->selectedItems();
  if (selectedItems.isEmpty()) {
    return;
  }

  const int selectedIndex = m_treeTemplates->indexOfTopLevelItem(selectedItems.first());
  if (selectedIndex < 0) {
    return;
  }

  m_savedTemplates.removeAt(selectedIndex);
  refreshTemplatesView();
  saveTemplatesToDisk();
}

void
CFrmRawCanSession::clearTemplates()
{
  m_savedTemplates.clear();
  refreshTemplatesView();
  saveTemplatesToDisk();
}

void
CFrmRawCanSession::sendSelectedTemplate()
{
  const auto selectedItems = m_treeTemplates->selectedItems();
  if (selectedItems.isEmpty()) {
    return;
  }

  const int index = m_treeTemplates->indexOfTopLevelItem(selectedItems.first());
  if (index < 0 || index >= m_savedTemplates.size()) {
    return;
  }

  applyTemplateToInputs(m_savedTemplates.at(index));
  sendFrame();
}

void
CFrmRawCanSession::loadTemplatesFromDisk()
{
  const QString settingsKey = QStringLiteral("rawcan/templates/%1").arg(m_interfaceName.isEmpty() ? QStringLiteral("default") : m_interfaceName);
  QSettings settings;
  const int count = settings.value(settingsKey + "/count", 0).toInt();
  m_savedTemplates.clear();
  for (int i = 0; i < count; ++i) {
    SavedFrameTemplate item;
    item.name = settings.value(QStringLiteral("%1/%2/name").arg(settingsKey).arg(i), QString()).toString();
    item.frameId = settings.value(QStringLiteral("%1/%2/frameId").arg(settingsKey).arg(i), QString()).toString();
    item.payload = settings.value(QStringLiteral("%1/%2/payload").arg(settingsKey).arg(i), QString()).toString();
    item.extended = settings.value(QStringLiteral("%1/%2/extended").arg(settingsKey).arg(i), false).toBool();
    item.fd = settings.value(QStringLiteral("%1/%2/fd").arg(settingsKey).arg(i), false).toBool();
    item.bitrateSwitch = settings.value(QStringLiteral("%1/%2/bitrateSwitch").arg(settingsKey).arg(i), false).toBool();
    item.errorStateIndicator = settings.value(QStringLiteral("%1/%2/errorStateIndicator").arg(settingsKey).arg(i), false).toBool();
    item.remoteRequest = settings.value(QStringLiteral("%1/%2/remoteRequest").arg(settingsKey).arg(i), false).toBool();
    if (!item.name.isEmpty()) {
      m_savedTemplates.push_back(item);
    }
  }
  refreshTemplatesView();
}

void
CFrmRawCanSession::saveTemplatesToDisk()
{
  const QString settingsKey = QStringLiteral("rawcan/templates/%1").arg(m_interfaceName.isEmpty() ? QStringLiteral("default") : m_interfaceName);
  QSettings settings;
  settings.remove(settingsKey);
  settings.setValue(settingsKey + "/count", m_savedTemplates.size());
  for (int i = 0; i < m_savedTemplates.size(); ++i) {
    const SavedFrameTemplate& item = m_savedTemplates.at(i);
    settings.setValue(QStringLiteral("%1/%2/name").arg(settingsKey).arg(i), item.name);
    settings.setValue(QStringLiteral("%1/%2/frameId").arg(settingsKey).arg(i), item.frameId);
    settings.setValue(QStringLiteral("%1/%2/payload").arg(settingsKey).arg(i), item.payload);
    settings.setValue(QStringLiteral("%1/%2/extended").arg(settingsKey).arg(i), item.extended);
    settings.setValue(QStringLiteral("%1/%2/fd").arg(settingsKey).arg(i), item.fd);
    settings.setValue(QStringLiteral("%1/%2/bitrateSwitch").arg(settingsKey).arg(i), item.bitrateSwitch);
    settings.setValue(QStringLiteral("%1/%2/errorStateIndicator").arg(settingsKey).arg(i), item.errorStateIndicator);
    settings.setValue(QStringLiteral("%1/%2/remoteRequest").arg(settingsKey).arg(i), item.remoteRequest);
  }
}

void
CFrmRawCanSession::refreshTemplatesView()
{
  m_treeTemplates->clear();
  for (const SavedFrameTemplate& item : m_savedTemplates) {
    QTreeWidgetItem* treeItem = new QTreeWidgetItem(m_treeTemplates);
    treeItem->setText(0, item.name);
    treeItem->setText(1, QStringLiteral("%1 %2").arg(item.frameId).arg(item.payload));
    treeItem->setToolTip(0, tr("ID: %1\nPayload: %2").arg(item.frameId).arg(item.payload));
  }
}

void
CFrmRawCanSession::applyTemplateToInputs(const SavedFrameTemplate& templateItem)
{
  m_editFrameId->setText(templateItem.frameId);
  m_editPayload->setText(templateItem.payload);
  m_chkExtended->setChecked(templateItem.extended);
  m_chkFd->setChecked(templateItem.fd);
  m_chkBitrateSwitch->setChecked(templateItem.bitrateSwitch);
  m_chkErrorStateIndicator->setChecked(templateItem.errorStateIndicator);
  m_chkRemoteRequest->setChecked(templateItem.remoteRequest);
}

void
CFrmRawCanSession::onTemplateSelectionChanged()
{
  const auto selectedItems = m_treeTemplates->selectedItems();
  if (selectedItems.isEmpty()) {
    return;
  }

  const int index = m_treeTemplates->indexOfTopLevelItem(selectedItems.first());
  if (index < 0 || index >= m_savedTemplates.size()) {
    return;
  }

  applyTemplateToInputs(m_savedTemplates.at(index));
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::refreshViews()
{
  if (m_stackViews->currentWidget() == m_treeFrames) {
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
  const int previousTopIndex = m_treeFrames->topLevelItemCount();
  m_treeFrames->clear();

  int visibleIndex = 0;
  for (const FrameRecord& rec : m_frameHistory) {
    if (!isFrameVisibleByFilter(rec.frame)) {
      continue;
    }

    const QString frameTypeText = (QCanBusFrame::RemoteRequestFrame == rec.frame.frameType())
                                    ? tr("Remote")
                                    : tr("Data");
    const QString payloadPreview = formatPayload(rec.frame.payload());
    const QString compactPayload = (payloadPreview.size() > 64) ? payloadPreview.left(64) + QStringLiteral("…") : payloadPreview;
    const QString mainRowText = tr("%1 %2 | %3 | %4 | DLC %5 | %6")
                                  .arg(directionText(rec.direction))
                                  .arg(rec.timestamp.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")))
                                  .arg(frameTypeText)
                                  .arg(rec.frame.hasExtendedFrameFormat() ? tr("EXT") : tr("STD"))
                                  .arg(rec.frame.payload().size())
                                  .arg(compactPayload.isEmpty() ? tr("<empty>") : compactPayload);

    QTreeWidgetItem* frameItem = new QTreeWidgetItem(m_treeFrames);
    frameItem->setData(0, Qt::UserRole, visibleIndex);
    frameItem->setText(0, rec.timestamp.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")));
    frameItem->setText(1, formatId(rec.frame.frameId(), rec.frame.hasExtendedFrameFormat()));
    frameItem->setText(2, frameTypeText);
    frameItem->setText(3, rec.frame.hasExtendedFrameFormat() ? tr("EXT") : tr("STD"));
    frameItem->setText(4, compactPayload.isEmpty() ? tr("<empty>") : compactPayload);
    frameItem->setToolTip(0, tr("Frame ID: %1\nPayload: %2\nFlags: %3")
                                 .arg(formatId(rec.frame.frameId(), rec.frame.hasExtendedFrameFormat()))
                                 .arg(formatPayload(rec.frame.payload()))
                                 .arg(frameFlagsToString(rec.frame)));

    const QColor rowBgColor = rowBackgroundColorForDirection(rec.direction);
    const QColor rowFgColor = rowForegroundColorForDirection(rec.direction);
    frameItem->setBackground(0, rowBgColor);
    frameItem->setBackground(1, rowBgColor);
    frameItem->setForeground(0, rowFgColor);
    frameItem->setForeground(1, rowFgColor);

    ++visibleIndex;
  }

  if (m_treeFrames->topLevelItemCount() > 0) {
    const int lastRow = m_treeFrames->topLevelItemCount() - 1;
    m_treeFrames->scrollToItem(m_treeFrames->topLevelItem(lastRow), QAbstractItemView::PositionAtBottom);
  }

  updateStatusLabel();
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
    if (FrameDirection::Rx != rec.direction) {
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

QString
CFrmRawCanSession::directionText(FrameDirection direction) const
{
  switch (direction) {
    case FrameDirection::Tx:
      return tr("TX");
    case FrameDirection::Rx:
      return tr("RX");
    default:
      return tr("?");
  }
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::rowBackgroundColorForDirection(FrameDirection direction) const
{
  if (FrameDirection::Tx == direction) {
    return QColor(235, 244, 255);
  }
  if (FrameDirection::Rx == direction) {
    return QColor(236, 248, 236);
  }

  return QColor(Qt::white);
}

// ----------------------------------------------------------------------------

QColor
CFrmRawCanSession::rowForegroundColorForDirection(FrameDirection direction) const
{
  if (FrameDirection::Tx == direction) {
    return QColor(13, 71, 161);
  }
  if (FrameDirection::Rx == direction) {
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
  if (nullptr == m_tableIdFilters) {
    return;
  }

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

void
CFrmRawCanSession::populateFilterTableFromModel()
{
  if (nullptr == m_tableIdFilters) {
    return;
  }

  const QSignalBlocker blocker(m_tableIdFilters);
  m_tableIdFilters->setRowCount(0);

  for (const IdFilterRange& filter : m_idFilters) {
    const int row = m_tableIdFilters->rowCount();
    m_tableIdFilters->insertRow(row);

    QTableWidgetItem* useItem = new QTableWidgetItem;
    useItem->setFlags(useItem->flags() | Qt::ItemIsUserCheckable);
    useItem->setCheckState(filter.enabled ? Qt::Checked : Qt::Unchecked);
    m_tableIdFilters->setItem(row, 0, useItem);

    m_tableIdFilters->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(filter.idFrom, 3, 16, QChar('0')).toUpper()));
    m_tableIdFilters->setItem(row, 2, new QTableWidgetItem(QString("0x%1").arg(filter.idTo, 3, 16, QChar('0')).toUpper()));
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
CFrmRawCanSession::updateStatusLabel()
{
  QString text;
  if (nullptr != m_canDevice) {
    text = m_paused ? tr("Paused - connected to %1").arg(m_interfaceName) : tr("Connected to %1").arg(m_interfaceName);
  }
  else {
    text = tr("Disconnected");
  }

  if ((nullptr != m_treeFrames) && (m_treeFrames->selectedItems().size() == 2)) {
    QList<QTreeWidgetItem*> selectedItems = m_treeFrames->selectedItems();
    QVector<qint64> selectedIndices;
    selectedIndices.reserve(selectedItems.size());
    for (QTreeWidgetItem* item : selectedItems) {
      const QVariant indexData = item->data(0, Qt::UserRole);
      const int index = indexData.isValid() ? indexData.toInt() : -1;
      if (index >= 0 && index < m_frameHistory.size()) {
        selectedIndices.push_back(index);
      }
    }

    if (selectedIndices.size() == 2) {
      const qint64 firstIndex = selectedIndices.at(0);
      const qint64 secondIndex = selectedIndices.at(1);
      const qint64 deltaUs = qAbs(m_frameHistory.at(firstIndex).timestamp.msecsTo(m_frameHistory.at(secondIndex).timestamp)) * 1000;
      text = tr("%1 | Δt %2 µs").arg(text).arg(deltaUs);
    }
  }

  m_statusLabel->setText(text);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setConnectedState(bool connected)
{
  Q_UNUSED(connected);
  m_btnConnect->setText(nullptr != m_canDevice ? tr("Disconnect") : tr("Connect"));
  m_btnSend->setEnabled(nullptr != m_canDevice);
  m_actConnect->setText(nullptr != m_canDevice ? tr("Disconnect") : tr("Connect"));
  updateStatusLabel();
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::setPaused(bool paused)
{
  m_paused = paused;
  if (nullptr != m_actPause) {
    m_actPause->setText(paused ? tr("Resume") : tr("Pause"));
  }
  setConnectedState(nullptr != m_canDevice);
}

// ----------------------------------------------------------------------------

void
CFrmRawCanSession::onFrameSelectionChanged()
{
  updateStatusLabel();
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
