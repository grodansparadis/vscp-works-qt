// cfrmvscplinktest.cpp
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include "cfrmvscplinktest.h"

#include "vscpworks.h"

#include <vscp-client-canal.h>
#include <vscp-client-mqtt.h>
#include <vscp-client-multicast.h>
#ifndef WIN32
#include <vscp-client-socketcan.h>
#endif
#include <vscp-client-tcp.h>
#include <vscp-client-udp.h>
#include <vscp-client-ws1.h>
#include <vscp-client-ws2.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>
#include <deque>

CFrmVscpLinkTest::CFrmVscpLinkTest(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_connType(CVscpClient::connType::NONE)
  , m_vscpClient(nullptr)
  , m_eventsSent(0)
  , m_eventsFailed(0)
  , m_statsBaselineEvents(0)
  , m_connectionCombo(nullptr)
  , m_refreshConnectionsButton(nullptr)
  , m_newTcpipButton(nullptr)
  , m_connNameLabel(nullptr)
  , m_connTypeLabel(nullptr)
  , m_connEndpointLabel(nullptr)
  , m_connUserLabel(nullptr)
  , m_connSslLabel(nullptr)
  , m_connTimeoutLabel(nullptr)
  , m_connProtocolLabel(nullptr)
  , m_connStateLabel(nullptr)
  , m_serverVersionLabel(nullptr)
  , m_serverCapabilitiesLabel(nullptr)
  , m_stepTable(nullptr)
  , m_logEdit(nullptr)
  , m_cycleCount(nullptr)
  , m_retryCount(nullptr)
  , m_stressEventCount(nullptr)
  , m_validateTimeoutCheck(nullptr)
  , m_reliabilityStatsLabel(nullptr)
  , m_runStepButton(nullptr)
  , m_runAllButton(nullptr)
  , m_reliabilityButton(nullptr)
  , m_clearLogButton(nullptr)
{
  setWindowTitle(tr("VSCP Link protocol test tool"));
  resize(1050, 760);

  QString preferredUuid;
  if (nullptr != pconn) {
    m_connObject = *pconn;
    if (m_connObject.contains("uuid") && m_connObject["uuid"].is_string()) {
      preferredUuid = m_connObject["uuid"].get<std::string>().c_str();
    }
  }

  setupUi();
  addStepRows();
  loadAvailableConnections(preferredUuid);
}

CFrmVscpLinkTest::~CFrmVscpLinkTest()
{
  resetClient();
}

void
CFrmVscpLinkTest::setupUi()
{
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  setStyleSheet(
    "QDialog { background-color: #f7fbff; }"
    "QGroupBox { border: 1px solid #9ec5ff; border-radius: 6px; margin-top: 10px; "
    "  background-color: #eef6ff; }"
    "QGroupBox::title { subcontrol-origin: margin; left: 8px; color: #0b5394; font-weight: bold; }"
    "QPushButton { background-color: #d9ecff; border: 1px solid #8fb7e8; border-radius: 4px; padding: 4px 8px; }"
    "QPushButton:hover { background-color: #c8e3ff; }"
    "QHeaderView::section { background-color: #d4e9ff; color: #0b3f76; font-weight: bold; "
    "  border: 1px solid #a9c9eb; padding: 4px; }"
    "QTableWidget { background-color: #ffffff; alternate-background-color: #f3f9ff; gridline-color: #c6dbef; }"
    "QTableWidget::item:selected { background-color: #ffe9a8; color: #1d3557; }");

  QLabel* title = new QLabel(
    tr("Run VSCP Link protocol verification step-by-step, as a full flow, or "
       "as a reliability loop."),
    this);
  mainLayout->addWidget(title);

  QHBoxLayout* connLayout = new QHBoxLayout();
  connLayout->addWidget(new QLabel(tr("Connection:"), this));
  m_connectionCombo = new QComboBox(this);
  m_connectionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
  m_refreshConnectionsButton = new QPushButton(tr("Refresh"), this);
  m_newTcpipButton = new QPushButton(tr("Create TCP/IP interface..."), this);
  connLayout->addWidget(m_connectionCombo, 1);
  connLayout->addWidget(m_refreshConnectionsButton);
  connLayout->addWidget(m_newTcpipButton);
  mainLayout->addLayout(connLayout);

  QGroupBox* connInfoGroup = new QGroupBox(tr("Selected connection information"), this);
  QFormLayout* connInfoLayout = new QFormLayout(connInfoGroup);
  m_connNameLabel         = new QLabel("-", connInfoGroup);
  m_connTypeLabel         = new QLabel("-", connInfoGroup);
  m_connEndpointLabel     = new QLabel("-", connInfoGroup);
  m_connUserLabel         = new QLabel("-", connInfoGroup);
  m_connSslLabel          = new QLabel("-", connInfoGroup);
  m_connTimeoutLabel      = new QLabel("-", connInfoGroup);
  m_connProtocolLabel     = new QLabel("-", connInfoGroup);
  m_connStateLabel        = new QLabel(tr("Disconnected"), connInfoGroup);
  m_serverVersionLabel    = new QLabel("-", connInfoGroup);
  m_serverCapabilitiesLabel = new QLabel("-", connInfoGroup);
  m_connProtocolLabel->setWordWrap(true);
  m_serverCapabilitiesLabel->setWordWrap(true);
  connInfoLayout->addRow(tr("Name:"), m_connNameLabel);
  connInfoLayout->addRow(tr("Type:"), m_connTypeLabel);
  connInfoLayout->addRow(tr("Server / endpoint:"), m_connEndpointLabel);
  connInfoLayout->addRow(tr("Username:"), m_connUserLabel);
  connInfoLayout->addRow(tr("SSL/TLS:"), m_connSslLabel);
  connInfoLayout->addRow(tr("Connection timeout:"), m_connTimeoutLabel);
  connInfoLayout->addRow(tr("Protocol settings:"), m_connProtocolLabel);
  connInfoLayout->addRow(tr("State:"), m_connStateLabel);
  connInfoLayout->addRow(tr("Server version:"), m_serverVersionLabel);
  connInfoLayout->addRow(tr("Server capabilities:"), m_serverCapabilitiesLabel);
  mainLayout->addWidget(connInfoGroup);

  m_stepTable = new QTableWidget(this);
  m_stepTable->setColumnCount(4);
  m_stepTable->setHorizontalHeaderLabels(
    QStringList() << tr("Step") << tr("Command") << tr("Result")
                  << tr("Details"));
  m_stepTable->horizontalHeader()->setStretchLastSection(true);
  m_stepTable->setColumnWidth(0, 48);
  m_stepTable->setColumnWidth(1, 360);
  m_stepTable->setColumnWidth(2, 90);
  m_stepTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_stepTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_stepTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_stepTable->verticalHeader()->setVisible(false);
  m_stepTable->setAlternatingRowColors(true);
  mainLayout->addWidget(m_stepTable, 3);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  m_runStepButton    = new QPushButton(tr("Run selected steps"), this);
  m_runAllButton     = new QPushButton(tr("Run all steps"), this);
  m_reliabilityButton = new QPushButton(tr("Run reliability test"), this);
  m_cycleCount       = new QSpinBox(this);
  m_retryCount       = new QSpinBox(this);
  m_stressEventCount = new QSpinBox(this);
  m_validateTimeoutCheck = new QCheckBox(tr("Validate connect timeout"), this);
  m_cycleCount->setMinimum(1);
  m_cycleCount->setMaximum(10000);
  m_cycleCount->setValue(25);
  m_retryCount->setMinimum(0);
  m_retryCount->setMaximum(20);
  m_retryCount->setValue(2);
  m_stressEventCount->setMinimum(0);
  m_stressEventCount->setMaximum(10000);
  m_stressEventCount->setValue(10);
  m_validateTimeoutCheck->setChecked(true);
  m_clearLogButton = new QPushButton(tr("Clear log"), this);

  buttonLayout->addWidget(m_runStepButton);
  buttonLayout->addWidget(m_runAllButton);
  buttonLayout->addWidget(m_reliabilityButton);
  buttonLayout->addWidget(new QLabel(tr("Cycles:"), this));
  buttonLayout->addWidget(m_cycleCount);
  buttonLayout->addWidget(new QLabel(tr("Retries:"), this));
  buttonLayout->addWidget(m_retryCount);
  buttonLayout->addWidget(new QLabel(tr("Stress events/cycle:"), this));
  buttonLayout->addWidget(m_stressEventCount);
  buttonLayout->addWidget(m_validateTimeoutCheck);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(m_clearLogButton);
  mainLayout->addLayout(buttonLayout);

  m_reliabilityStatsLabel = new QLabel(tr("Reliability stats: not run"), this);
  mainLayout->addWidget(m_reliabilityStatsLabel);

  m_logEdit = new QPlainTextEdit(this);
  m_logEdit->setReadOnly(true);
  mainLayout->addWidget(m_logEdit, 2);

  connect(m_connectionCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &CFrmVscpLinkTest::onConnectionSelectionChanged);
  connect(m_refreshConnectionsButton,
          &QPushButton::clicked,
          this,
          &CFrmVscpLinkTest::refreshConnections);
  connect(m_newTcpipButton,
          &QPushButton::clicked,
          this,
          &CFrmVscpLinkTest::createTcpipConnection);
  connect(m_runStepButton,
          &QPushButton::clicked,
          this,
          &CFrmVscpLinkTest::runSelectedStep);
  connect(
    m_runAllButton, &QPushButton::clicked, this, &CFrmVscpLinkTest::runAllSteps);
  connect(m_reliabilityButton,
          &QPushButton::clicked,
          this,
          &CFrmVscpLinkTest::runReliabilityTest);
  connect(
    m_clearLogButton, &QPushButton::clicked, this, &CFrmVscpLinkTest::clearLog);
}

void
CFrmVscpLinkTest::addStepRows()
{
  const QStringList commands = {
    tr("Initialize client"),
    tr("Connect (wait cursor)"),
    tr("Verify connected state"),
    tr("Query server version"),
    tr("Query server capabilities"),
    tr("Report unavailable commands"),
    tr("Test command responses"),
    tr("Verify full level II support"),
    tr("Disconnect"),
    tr("Verify disconnected state")
  };

  m_stepTable->setRowCount(commands.size());
  for (int i = 0; i < commands.size(); ++i) {
    QTableWidgetItem* stepItem = new QTableWidgetItem(QString::number(i + 1));
    stepItem->setBackground(QColor(242, 248, 255));
    stepItem->setForeground(QBrush(QColor(33, 64, 98)));
    m_stepTable->setItem(i, 0, stepItem);

    QTableWidgetItem* commandItem = new QTableWidgetItem(commands.at(i));
    QFont commandFont = commandItem->font();
    commandFont.setBold(true);
    commandItem->setFont(commandFont);
    commandItem->setBackground(QColor(227, 240, 255));
    commandItem->setForeground(QBrush(QColor(18, 63, 110)));
    m_stepTable->setItem(i, 1, commandItem);
    setStepResult(i, StepResult::NotRun, tr("Not run"));
  }

  m_stepTable->selectRow(0);
}

void
CFrmVscpLinkTest::setStepResult(int row,
                                StepResult result,
                                const QString& details)
{
  QString statusText = tr("Not run");
  QColor statusColor(50, 50, 50);

  if (StepResult::Pass == result) {
    statusText = tr("PASS");
    statusColor = QColor(0, 130, 0);
  }
  else if (StepResult::Fail == result) {
    statusText = tr("FAIL");
    statusColor = QColor(160, 0, 0);
  }

  QTableWidgetItem* statusItem = m_stepTable->item(row, 2);
  if (nullptr == statusItem) {
    statusItem = new QTableWidgetItem();
    m_stepTable->setItem(row, 2, statusItem);
  }

  QTableWidgetItem* detailItem = m_stepTable->item(row, 3);
  if (nullptr == detailItem) {
    detailItem = new QTableWidgetItem();
    m_stepTable->setItem(row, 3, detailItem);
  }

  statusItem->setText(statusText);
  statusItem->setForeground(QBrush(statusColor));
  detailItem->setText(details);
}

void
CFrmVscpLinkTest::appendLog(const QString& text)
{
  m_logEdit->appendPlainText(text);
}

bool
CFrmVscpLinkTest::resetClient()
{
  if (nullptr == m_vscpClient) {
    return true;
  }

  if (m_vscpClient->isConnected()) {
    m_vscpClient->disconnect();
  }

  delete m_vscpClient;
  m_vscpClient = nullptr;
  updateConnectionInfo();

  return true;
}

bool
CFrmVscpLinkTest::isLinkConnection(const json& conn) const
{
  if (!(conn.contains("type") && conn["type"].is_number())) {
    return false;
  }

  const CVscpClient::connType type =
    static_cast<CVscpClient::connType>(conn["type"].get<int>());
  return (CVscpClient::connType::TCPIP == type);
}

void
CFrmVscpLinkTest::loadAvailableConnections(const QString& preferredUuid)
{
  m_connections.clear();
  QSignalBlocker blocker(m_connectionCombo);
  m_connectionCombo->clear();

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  if (nullptr != pworks) {
    QVector<json> sortedConnections;
    QMap<std::string, json>::const_iterator it = pworks->m_mapConn.constBegin();
    while (it != pworks->m_mapConn.constEnd()) {
      const json& conn = it.value();
      if (isLinkConnection(conn)) {
        sortedConnections.push_back(conn);
      }
      ++it;
    }

    std::sort(sortedConnections.begin(),
              sortedConnections.end(),
              [](const json& a, const json& b) {
                QString aname = a.contains("name") && a["name"].is_string()
                                  ? a["name"].get<std::string>().c_str()
                                  : QString();
                QString bname = b.contains("name") && b["name"].is_string()
                                  ? b["name"].get<std::string>().c_str()
                                  : QString();
                return aname.toLower() < bname.toLower();
              });

    m_connections = sortedConnections;
  }

  int selectedIndex = -1;
  for (int i = 0; i < m_connections.size(); ++i) {
    const json& conn = m_connections.at(i);
    const QString name =
      conn.contains("name") && conn["name"].is_string()
        ? conn["name"].get<std::string>().c_str()
        : tr("<unnamed>");
    const CVscpClient::connType type = conn.contains("type") && conn["type"].is_number()
                                         ? static_cast<CVscpClient::connType>(
                                             conn["type"].get<int>())
                                         : CVscpClient::connType::NONE;
    QString itemText =
      tr("%1 (%2)").arg(name, connectionTypeToString(type));
    m_connectionCombo->addItem(itemText);

    if (!preferredUuid.isEmpty() &&
        conn.contains("uuid") && conn["uuid"].is_string()) {
      const QString uuid = conn["uuid"].get<std::string>().c_str();
      if (uuid == preferredUuid) {
        selectedIndex = i;
      }
    }
  }

  if (selectedIndex < 0 && !m_connections.isEmpty()) {
    selectedIndex = 0;
  }

  if (selectedIndex >= 0) {
    m_connectionCombo->setCurrentIndex(selectedIndex);
    QString details;
    applySelectedConnection(selectedIndex, &details);
  }
  else {
    m_connObject = json::object();
    m_connType = CVscpClient::connType::NONE;
    m_serverVersion.clear();
    m_serverCapabilities.clear();
    updateConnectionInfo();
    appendLog(tr("No VSCP Link-capable connections found. Create a TCP/IP "
                 "interface to start testing."));
  }
}

bool
CFrmVscpLinkTest::applySelectedConnection(int index, QString* details)
{
  if (index < 0 || index >= m_connections.size()) {
    if (nullptr != details) {
      *details = tr("Invalid connection selection.");
    }
    return false;
  }

  resetClient();
  m_connObject = m_connections.at(index);
  m_connType = m_connObject.contains("type") && m_connObject["type"].is_number()
                 ? static_cast<CVscpClient::connType>(m_connObject["type"].get<int>())
                 : CVscpClient::connType::NONE;
  m_serverVersion.clear();
  m_serverCapabilities.clear();
  m_eventsSent = 0;
  m_eventsFailed = 0;
  m_statsBaselineEvents = 0;
  resetStepResults();
  updateConnectionInfo();

  if (nullptr != details) {
    const QString name =
      m_connObject.contains("name") && m_connObject["name"].is_string()
        ? m_connObject["name"].get<std::string>().c_str()
        : tr("<unnamed>");
    *details = tr("Selected %1.").arg(name);
  }

  return true;
}

void
CFrmVscpLinkTest::resetStepResults()
{
  for (int row = 0; row < m_stepTable->rowCount(); ++row) {
    setStepResult(row, StepResult::NotRun, tr("Not run"));
  }
}

QString
CFrmVscpLinkTest::connectionTypeToString(CVscpClient::connType type) const
{
  switch (type) {
    case CVscpClient::connType::TCPIP:
      return tr("TCP/IP");
    case CVscpClient::connType::WS1:
      return tr("WebSocket WS1");
    case CVscpClient::connType::WS2:
      return tr("WebSocket WS2");
    case CVscpClient::connType::MQTT:
      return tr("MQTT");
    case CVscpClient::connType::UDP:
      return tr("UDP");
    case CVscpClient::connType::MULTICAST:
      return tr("Multicast");
    case CVscpClient::connType::CANAL:
      return tr("CANAL");
#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      return tr("SocketCAN");
#endif
    default:
      break;
  }
  return tr("Unknown");
}

QString
CFrmVscpLinkTest::connectionEndpoint(const json& conn) const
{
  if (conn.contains("host") && conn["host"].is_string()) {
    return conn["host"].get<std::string>().c_str();
  }
  if (conn.contains("url") && conn["url"].is_string()) {
    return conn["url"].get<std::string>().c_str();
  }
  return tr("-");
}

QString
CFrmVscpLinkTest::connectionProtocolDetails(const json& conn) const
{
  QStringList details;
  if (conn.contains("selected-interface") && conn["selected-interface"].is_string()) {
    details << tr("interface=%1").arg(conn["selected-interface"].get<std::string>().c_str());
  }
  if (conn.contains("bpoll") && conn["bpoll"].is_boolean()) {
    details << tr("poll=%1").arg(conn["bpoll"].get<bool>() ? tr("yes") : tr("no"));
  }
  if (conn.contains("bfull-l2") && conn["bfull-l2"].is_boolean()) {
    details << tr("full-l2=%1").arg(conn["bfull-l2"].get<bool>() ? tr("yes") : tr("no"));
  }

  if (details.isEmpty()) {
    return tr("-");
  }
  return details.join(", ");
}

QString
CFrmVscpLinkTest::failureInsightForStep(int row, const QString& details) const
{
  QStringList hints;
  const QString endpoint = connectionEndpoint(m_connObject);
  const bool hasUser = m_connObject.contains("user") &&
                       m_connObject["user"].is_string() &&
                       !QString(m_connObject["user"].get<std::string>().c_str()).trimmed().isEmpty();
  const bool hasPassword = m_connObject.contains("password") &&
                           m_connObject["password"].is_string() &&
                           !QString(m_connObject["password"].get<std::string>().c_str()).trimmed().isEmpty();
  const int timeout = connectionTimeoutSeconds();

  switch (row) {
    case 0:
      hints << tr("The selected connection JSON may be incomplete or invalid for the TCP/IP client.");
      break;

    case 1:
      if ("-" == endpoint || endpoint.trimmed().isEmpty()) {
        hints << tr("No server endpoint is configured.");
      }
      if (!hasUser) {
        hints << tr("Username is empty; verify whether the server requires authentication.");
      }
      if (!hasPassword) {
        hints << tr("Password is empty; verify whether the server requires authentication.");
      }
      if (timeout > 0 && timeout < 2) {
        hints << tr("Connection timeout is very low (%1 s).").arg(timeout);
      }
      hints << tr("Check daemon reachability, firewall rules, and host/port values.");
      break;

    case 2:
      hints << tr("The connect call may have succeeded partially but transport was dropped immediately.");
      hints << tr("Check server logs for authentication or policy rejection.");
      break;

    case 3:
      hints << tr("The link is up, but the server may not support the version command on this channel.");
      hints << tr("Check protocol compatibility between client library and server.");
      break;

    case 4:
      hints << tr("Capabilities query can fail when interface listing is disabled on the server.");
      hints << tr("Verify server permissions for interface introspection.");
      break;

    case 6:
      hints << tr("A command response failed; check command availability and user privileges.");
      hints << tr("Inspect server-side logs around the same timestamp for protocol errors.");
      break;

    case 7:
      hints << tr("This connection does not report full Level II capability.");
      hints << tr("Verify interface type and server-side full-level-II configuration.");
      break;

    case 8:
      hints << tr("Disconnect command failed; connection state may already be inconsistent.");
      hints << tr("Retry once and inspect server transport logs.");
      break;

    case 9:
      hints << tr("Client still reports connected state after disconnect.");
      hints << tr("Possible stale socket/session state; retry disconnect or reinitialize client.");
      break;

    default:
      break;
  }

  if (details.contains("Error code:")) {
    hints << tr("Use the reported error code to map to VSCP error constants for exact root cause.");
  }

  if (hints.isEmpty()) {
    return QString();
  }

  return hints.join(" ");
}

int
CFrmVscpLinkTest::connectionTimeoutSeconds() const
{
  if (m_connObject.contains("connection-timeout") &&
      m_connObject["connection-timeout"].is_number()) {
    return m_connObject["connection-timeout"].get<int>();
  }
  return 0;
}

void
CFrmVscpLinkTest::updateConnectionInfo()
{
  const QString name =
    m_connObject.contains("name") && m_connObject["name"].is_string()
      ? m_connObject["name"].get<std::string>().c_str()
      : tr("-");
  const QString user =
    m_connObject.contains("user") && m_connObject["user"].is_string()
      ? m_connObject["user"].get<std::string>().c_str()
      : tr("-");
  const QString tls =
    m_connObject.contains("btls") && m_connObject["btls"].is_boolean()
      ? (m_connObject["btls"].get<bool>() ? tr("Enabled") : tr("Disabled"))
      : tr("-");
  const QString timeout =
    m_connObject.contains("connection-timeout") && m_connObject["connection-timeout"].is_number()
      ? tr("%1 s").arg(m_connObject["connection-timeout"].get<int>())
      : tr("-");

  m_connNameLabel->setText(name);
  m_connTypeLabel->setText(connectionTypeToString(m_connType));
  m_connEndpointLabel->setText(connectionEndpoint(m_connObject));
  m_connUserLabel->setText(user);
  m_connSslLabel->setText(tls);
  m_connTimeoutLabel->setText(timeout);
  m_connProtocolLabel->setText(connectionProtocolDetails(m_connObject));
  m_connStateLabel->setText((nullptr != m_vscpClient && m_vscpClient->isConnected())
                              ? tr("Connected")
                              : tr("Disconnected"));
  m_serverVersionLabel->setText(m_serverVersion.isEmpty() ? tr("-") : m_serverVersion);
  m_serverCapabilitiesLabel->setText(
    m_serverCapabilities.isEmpty() ? tr("-") : m_serverCapabilities);
}

void
CFrmVscpLinkTest::onConnectionSelectionChanged(int index)
{
  QString details;
  if (applySelectedConnection(index, &details)) {
    appendLog(details);
  }
}

void
CFrmVscpLinkTest::refreshConnections()
{
  QString preferredUuid;
  if (m_connObject.contains("uuid") && m_connObject["uuid"].is_string()) {
    preferredUuid = m_connObject["uuid"].get<std::string>().c_str();
  }
  loadAvailableConnections(preferredUuid);
  appendLog(tr("Connection list refreshed."));
}

void
CFrmVscpLinkTest::createTcpipConnection()
{
  QWidget* wnd = parentWidget();
  if (nullptr == wnd) {
    appendLog(tr("Unable to create a TCP/IP connection from this context."));
    return;
  }

  const bool invoked = QMetaObject::invokeMethod(wnd, "newTcpipConnection");
  if (!invoked) {
    appendLog(tr("Failed to open TCP/IP connection dialog."));
    return;
  }

  appendLog(tr("TCP/IP connection dialog closed. Refreshing available connections."));
  refreshConnections();
}

bool
CFrmVscpLinkTest::ensureClientCreated(QString& details)
{
  if (nullptr != m_vscpClient) {
    details = tr("Client already initialized.");
    return true;
  }

  if (CVscpClient::connType::NONE == m_connType) {
    details = tr("Connection type is missing.");
    return false;
  }

  switch (m_connType) {

    case CVscpClient::connType::TCPIP:
      m_vscpClient = new vscpClientTcp();
      break;

    case CVscpClient::connType::CANAL:
      m_vscpClient = new vscpClientCanal();
      break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      m_vscpClient = new vscpClientSocketCan();
      break;
#endif

    case CVscpClient::connType::WS1:
      m_vscpClient = new vscpClientWs1();
      break;

    case CVscpClient::connType::WS2:
      m_vscpClient = new vscpClientWs2();
      break;

    case CVscpClient::connType::MQTT:
      m_vscpClient = new vscpClientMqtt();
      break;

    case CVscpClient::connType::UDP:
      m_vscpClient = new vscpClientUdp();
      break;

    case CVscpClient::connType::MULTICAST:
      m_vscpClient = new vscpClientMulticast();
      break;

    default:
      details = tr("Connection type is not supported by this test tool.");
      return false;
  }

  if (nullptr == m_vscpClient) {
    details = tr("Failed to create VSCP client.");
    return false;
  }

  if (!m_vscpClient->initFromJson(m_connObject.dump())) {
    resetClient();
    details = tr("Failed to initialize client from connection settings.");
    return false;
  }

  details = tr("Client initialized from selected connection.");
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepInitializeClient(QString& details)
{
  return ensureClientCreated(details);
}

bool
CFrmVscpLinkTest::stepConnect(QString& details)
{
  if (!ensureClientCreated(details)) {
    return false;
  }

  if (m_vscpClient->isConnected()) {
    details = tr("Already connected.");
    updateConnectionInfo();
    return true;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();
  const int rv = m_vscpClient->connect();
  QApplication::restoreOverrideCursor();
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Connect failed. Error code: %1").arg(rv);
    updateConnectionInfo();
    return false;
  }

  details = tr("Connected successfully.");
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepVerifyConnected(QString& details)
{
  if (nullptr == m_vscpClient) {
    details = tr("Run 'Initialize client' and 'Connect' first.");
    return false;
  }

  if (!m_vscpClient->isConnected()) {
    details = tr("Client is not connected.");
    updateConnectionInfo();
    return false;
  }

  details = tr("Client reports an active connection.");
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepQueryServerVersion(QString& details)
{
  if (nullptr == m_vscpClient || !m_vscpClient->isConnected()) {
    details = tr("Client is not connected.");
    return false;
  }

  uint8_t major_ver = 0;
  uint8_t minor_ver = 0;
  uint8_t release_ver = 0;
  uint8_t build_ver = 0;
  const int rv = m_vscpClient->getversion(&major_ver,
                                          &minor_ver,
                                          &release_ver,
                                          &build_ver);
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Failed to query server version. Error code: %1").arg(rv);
    return false;
  }

  m_serverVersion =
    tr("%1.%2.%3.%4")
      .arg(major_ver)
      .arg(minor_ver)
      .arg(release_ver)
      .arg(build_ver);
  details = tr("Server version: %1").arg(m_serverVersion);
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepQueryServerCapabilities(QString& details)
{
  if (nullptr == m_vscpClient || !m_vscpClient->isConnected()) {
    details = tr("Client is not connected.");
    return false;
  }

  QStringList cap;
  cap << tr("full-level-II=%1").arg(m_vscpClient->isFullLevel2() ? tr("yes") : tr("no"));

  if (CVscpClient::connType::TCPIP == m_connType) {
    vscpClientTcp* ptcp = dynamic_cast<vscpClientTcp*>(m_vscpClient);
    if (nullptr != ptcp) {
      std::deque<std::string> interfaces;
      const int rv = ptcp->getinterfaces(interfaces);
      if (VSCP_ERROR_SUCCESS == rv) {
        cap << tr("interfaces=%1").arg(interfaces.size());
      }
      else {
        cap << tr("interfaces=query-failed(%1)").arg(rv);
      }
    }
  }

  m_serverCapabilities = cap.join(", ");
  details = tr("Capabilities: %1").arg(m_serverCapabilities);
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepReportUnsupportedCommands(QString& details)
{
  details = tr("Unsupported in this environment: send, retr, user, password, noop, help, cdta.");
  return true;
}

bool
CFrmVscpLinkTest::stepTestCommandResponses(QString& details)
{
  if (nullptr == m_vscpClient || !m_vscpClient->isConnected()) {
    details = tr("Client is not connected.");
    return false;
  }

  uint8_t major_ver = 0;
  uint8_t minor_ver = 0;
  uint8_t release_ver = 0;
  uint8_t build_ver = 0;
  int rv = m_vscpClient->getversion(&major_ver,
                                    &minor_ver,
                                    &release_ver,
                                    &build_ver);
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Version command failed in command-response test. Error code: %1").arg(rv);
    return false;
  }

  if (CVscpClient::connType::TCPIP == m_connType) {
    vscpClientTcp* ptcp = dynamic_cast<vscpClientTcp*>(m_vscpClient);
    if (nullptr != ptcp) {
      std::deque<std::string> interfaces;
      rv = ptcp->getinterfaces(interfaces);
      if (VSCP_ERROR_SUCCESS != rv) {
        details = tr("TCP/IP interface listing command failed. Error code: %1").arg(rv);
        return false;
      }
      details = tr("Command responses verified (version + interfaces=%1).")
                  .arg(interfaces.size());
      return true;
    }
  }

  details = tr("Command responses verified for available client API commands.");
  return true;
}

bool
CFrmVscpLinkTest::stepVerifyLevel2(QString& details)
{
  if (nullptr == m_vscpClient) {
    details = tr("Run 'Initialize client' and 'Connect' first.");
    return false;
  }

  if (!m_vscpClient->isConnected()) {
    details = tr("Client is not connected.");
    return false;
  }

  if (!m_vscpClient->isFullLevel2()) {
    details = tr("Connection is not full level II.");
    return false;
  }

  details = tr("Connection reports full level II support.");
  return true;
}

bool
CFrmVscpLinkTest::stepDisconnect(QString& details)
{
  if (nullptr == m_vscpClient) {
    details = tr("Client is not initialized.");
    return false;
  }

  if (!m_vscpClient->isConnected()) {
    details = tr("Already disconnected.");
    updateConnectionInfo();
    return true;
  }

  const int rv = m_vscpClient->disconnect();
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Disconnect failed. Error code: %1").arg(rv);
    updateConnectionInfo();
    return false;
  }

  details = tr("Disconnected successfully.");
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::stepVerifyDisconnected(QString& details)
{
  if (nullptr == m_vscpClient) {
    details = tr("Run 'Initialize client' first.");
    return false;
  }

  if (m_vscpClient->isConnected()) {
    details = tr("Client still reports connected state.");
    updateConnectionInfo();
    return false;
  }

  details = tr("Client reports disconnected state.");
  updateConnectionInfo();
  return true;
}

bool
CFrmVscpLinkTest::runStepByRow(int row)
{
  QString details;
  bool ok = false;

  switch (row) {
    case 0:
      ok = stepInitializeClient(details);
      break;

    case 1:
      ok = stepConnect(details);
      break;

    case 2:
      ok = stepVerifyConnected(details);
      break;

    case 3:
      ok = stepQueryServerVersion(details);
      break;

    case 4:
      ok = stepQueryServerCapabilities(details);
      break;

    case 5:
      ok = stepReportUnsupportedCommands(details);
      break;

    case 6:
      ok = stepTestCommandResponses(details);
      break;

    case 7:
      ok = stepVerifyLevel2(details);
      break;

    case 8:
      ok = stepDisconnect(details);
      break;

    case 9:
      ok = stepVerifyDisconnected(details);
      break;

    default:
      details = tr("Unknown step.");
      ok = false;
      break;
  }

  if (!ok) {
    const QString insight = failureInsightForStep(row, details);
    if (!insight.isEmpty()) {
      details += tr(" Possible cause: %1").arg(insight);
    }
  }

  setStepResult(row, ok ? StepResult::Pass : StepResult::Fail, details);
  appendLog(tr("Step %1: %2 - %3")
              .arg(row + 1)
              .arg(ok ? tr("PASS") : tr("FAIL"))
              .arg(details));

  return ok;
}

void
CFrmVscpLinkTest::runSelectedStep()
{
  const QModelIndexList selectedRows = m_stepTable->selectionModel()->selectedRows();
  if (selectedRows.isEmpty()) {
    appendLog(tr("No steps selected."));
    return;
  }

  QList<int> rows;
  rows.reserve(selectedRows.size());
  for (const QModelIndex& index : selectedRows) {
    rows << index.row();
  }
  std::sort(rows.begin(), rows.end());

  appendLog(tr("Run %1 selected step(s).").arg(rows.size()));
  for (int row : rows) {
    if (!runStepByRow(row)) {
      appendLog(tr("Stopped selected-step execution at step %1 due to failure.").arg(row + 1));
      return;
    }
  }

  appendLog(tr("All selected steps passed."));
}

void
CFrmVscpLinkTest::runAllSteps()
{
  resetStepResults();

  appendLog(tr("Run all protocol verification steps."));
  for (int row = 0; row < m_stepTable->rowCount(); ++row) {
    if (!runStepByRow(row)) {
      appendLog(tr("Stopped at step %1 due to failure.").arg(row + 1));
      return;
    }
  }

  appendLog(tr("All protocol verification steps passed."));
}

void
CFrmVscpLinkTest::runReliabilityTest()
{
  QString details;
  if (!ensureClientCreated(details)) {
    appendLog(tr("Reliability test setup failed: %1").arg(details));
    return;
  }

  if (m_vscpClient->isConnected()) {
    m_vscpClient->disconnect();
  }

  const int cycles      = m_cycleCount->value();
  const int retries     = m_retryCount->value();
  const int stressCount = m_stressEventCount->value();
  const bool validateTimeout = m_validateTimeoutCheck->isChecked();
  const int timeoutSeconds = connectionTimeoutSeconds();

  int cyclePassCount = 0;
  int cycleFailCount = 0;
  int timeoutFailCount = 0;
  uint64_t stressSendOk = 0;
  uint64_t stressSendFail = 0;

  appendLog(tr("Run reliability test (%1 cycles, retries=%2, stress-events/cycle=%3).")
              .arg(cycles)
              .arg(retries)
              .arg(stressCount));

  QProgressDialog progress(tr("Running reliability test..."),
                           tr("Cancel"),
                           0,
                           cycles,
                           this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.setValue(0);

  for (int i = 1; i <= cycles; ++i) {
    QApplication::processEvents();
    if (progress.wasCanceled()) {
      appendLog(tr("Reliability test canceled at cycle %1.").arg(i - 1));
      break;
    }

    bool cycleOk = true;
    bool connected = false;

    for (int attempt = 0; attempt <= retries; ++attempt) {
      QElapsedTimer timer;
      timer.start();
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      const int rv = m_vscpClient->connect();
      QApplication::restoreOverrideCursor();
      const qint64 elapsedMs = timer.elapsed();

      if (VSCP_ERROR_SUCCESS == rv && m_vscpClient->isConnected()) {
        if (validateTimeout && timeoutSeconds > 0 &&
            elapsedMs > (timeoutSeconds * 1000LL)) {
          ++timeoutFailCount;
          cycleOk = false;
          appendLog(tr("Cycle %1 connect timeout validation failed (%2 ms > %3 ms).")
                      .arg(i)
                      .arg(elapsedMs)
                      .arg(timeoutSeconds * 1000LL));
          m_vscpClient->disconnect();
          break;
        }

        connected = true;
        break;
      }

      appendLog(tr("Cycle %1 connect attempt %2/%3 failed. Error code: %4")
                  .arg(i)
                  .arg(attempt + 1)
                  .arg(retries + 1)
                  .arg(rv));
    }

    if (!connected) {
      ++cycleFailCount;
      progress.setValue(i);
      continue;
    }

    if (stressCount > 0) {
      appendLog(tr("Cycle %1: stress event send skipped (%2 requested) because the "
                   "'send' command is not available.")
                  .arg(i)
                  .arg(stressCount));
      stressSendFail += stressCount;
    }

    bool disconnected = false;
    for (int attempt = 0; attempt <= retries; ++attempt) {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      const int rv = m_vscpClient->disconnect();
      QApplication::restoreOverrideCursor();
      if (VSCP_ERROR_SUCCESS == rv && !m_vscpClient->isConnected()) {
        disconnected = true;
        break;
      }

      appendLog(tr("Cycle %1 disconnect attempt %2/%3 failed. Error code: %4")
                  .arg(i)
                  .arg(attempt + 1)
                  .arg(retries + 1)
                  .arg(rv));
    }

    if (!disconnected) {
      cycleOk = false;
      if (m_vscpClient->isConnected()) {
        m_vscpClient->disconnect();
      }
    }

    if (cycleOk) {
      ++cyclePassCount;
    }
    else {
      ++cycleFailCount;
    }

    progress.setValue(i);
    QApplication::processEvents();
  }

  progress.setValue(progress.wasCanceled() ? progress.value() : cycles);
  updateConnectionInfo();

  const QString statsText = tr("Reliability stats: pass=%1 fail=%2 timeout-failures=%3 "
                               "stress-send-ok=%4 stress-send-skipped=%5")
                              .arg(cyclePassCount)
                              .arg(cycleFailCount)
                              .arg(timeoutFailCount)
                              .arg(stressSendOk)
                              .arg(stressSendFail);
  m_reliabilityStatsLabel->setText(statsText);
  appendLog(statsText);
}

void
CFrmVscpLinkTest::clearLog()
{
  m_logEdit->clear();
}
