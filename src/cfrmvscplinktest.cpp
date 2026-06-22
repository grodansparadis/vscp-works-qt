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

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

CFrmVscpLinkTest::CFrmVscpLinkTest(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_connType(CVscpClient::connType::NONE)
  , m_vscpClient(nullptr)
  , m_stepTable(nullptr)
  , m_logEdit(nullptr)
  , m_cycleCount(nullptr)
  , m_runStepButton(nullptr)
  , m_runAllButton(nullptr)
  , m_reliabilityButton(nullptr)
  , m_clearLogButton(nullptr)
{
  setWindowTitle(tr("VSCP Link protocol test tool"));
  resize(950, 620);

  if (nullptr != pconn) {
    m_connObject = *pconn;
    if (m_connObject.contains("type") && m_connObject["type"].is_number()) {
      m_connType =
        static_cast<CVscpClient::connType>(m_connObject["type"].get<int>());
    }
  }

  setupUi();
  addStepRows();
}

CFrmVscpLinkTest::~CFrmVscpLinkTest()
{
  resetClient();
}

void
CFrmVscpLinkTest::setupUi()
{
  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* title = new QLabel(
    tr("Run VSCP Link protocol verification step-by-step, all steps, or as a "
       "reliability loop."),
    this);
  mainLayout->addWidget(title);

  m_stepTable = new QTableWidget(this);
  m_stepTable->setColumnCount(4);
  m_stepTable->setHorizontalHeaderLabels(
    QStringList() << tr("Step") << tr("Command") << tr("Result")
                  << tr("Details"));
  m_stepTable->horizontalHeader()->setStretchLastSection(true);
  m_stepTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_stepTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_stepTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_stepTable->verticalHeader()->setVisible(false);
  mainLayout->addWidget(m_stepTable, 3);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  m_runStepButton = new QPushButton(tr("Run selected step"), this);
  m_runAllButton = new QPushButton(tr("Run all steps"), this);
  m_reliabilityButton = new QPushButton(tr("Run reliability test"), this);
  m_cycleCount = new QSpinBox(this);
  m_cycleCount->setMinimum(1);
  m_cycleCount->setMaximum(10000);
  m_cycleCount->setValue(25);
  m_clearLogButton = new QPushButton(tr("Clear log"), this);

  buttonLayout->addWidget(m_runStepButton);
  buttonLayout->addWidget(m_runAllButton);
  buttonLayout->addWidget(m_reliabilityButton);
  buttonLayout->addWidget(new QLabel(tr("Cycles:"), this));
  buttonLayout->addWidget(m_cycleCount);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(m_clearLogButton);
  mainLayout->addLayout(buttonLayout);

  m_logEdit = new QPlainTextEdit(this);
  m_logEdit->setReadOnly(true);
  mainLayout->addWidget(m_logEdit, 2);

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
    tr("Connect"),
    tr("Verify connected state"),
    tr("Verify full level II support"),
    tr("Disconnect")
  };

  m_stepTable->setRowCount(commands.size());
  for (int i = 0; i < commands.size(); ++i) {
    m_stepTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
    m_stepTable->setItem(i, 1, new QTableWidgetItem(commands.at(i)));
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
  statusItem->setForeground(statusColor);
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

  return true;
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
    return true;
  }

  const int rv = m_vscpClient->connect();
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Connect failed. Error code: %1").arg(rv);
    return false;
  }

  details = tr("Connected successfully.");
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
    return false;
  }

  details = tr("Client reports an active connection.");
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
    return true;
  }

  const int rv = m_vscpClient->disconnect();
  if (VSCP_ERROR_SUCCESS != rv) {
    details = tr("Disconnect failed. Error code: %1").arg(rv);
    return false;
  }

  details = tr("Disconnected successfully.");
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
      ok = stepVerifyLevel2(details);
      break;

    case 4:
      ok = stepDisconnect(details);
      break;

    default:
      details = tr("Unknown step.");
      ok = false;
      break;
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
  const int row = m_stepTable->currentRow();
  if (row < 0) {
    appendLog(tr("No step selected."));
    return;
  }

  runStepByRow(row);
}

void
CFrmVscpLinkTest::runAllSteps()
{
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

  const int cycles = m_cycleCount->value();
  appendLog(tr("Run reliability test (%1 connect/disconnect cycles).")
              .arg(cycles));

  for (int i = 1; i <= cycles; ++i) {

    int rv = m_vscpClient->connect();
    if (VSCP_ERROR_SUCCESS != rv) {
      appendLog(
        tr("Cycle %1 failed at connect. Error code: %2").arg(i).arg(rv));
      return;
    }

    if (!m_vscpClient->isConnected()) {
      appendLog(tr("Cycle %1 failed: client does not report connected state.")
                  .arg(i));
      return;
    }

    rv = m_vscpClient->disconnect();
    if (VSCP_ERROR_SUCCESS != rv) {
      appendLog(
        tr("Cycle %1 failed at disconnect. Error code: %2").arg(i).arg(rv));
      return;
    }
  }

  appendLog(tr("Reliability test passed for %1 cycles.").arg(cycles));
}

void
CFrmVscpLinkTest::clearLog()
{
  m_logEdit->clear();
}
