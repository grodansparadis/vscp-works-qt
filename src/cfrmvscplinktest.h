// cfrmvscplinktest.h
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

#ifndef CFRMVSCPLINKTEST_H
#define CFRMVSCPLINKTEST_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <vscp-client-base.h>

#include <QDialog>
#include <QVector>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
QT_END_NAMESPACE

class CFrmVscpLinkTest : public QDialog {
  Q_OBJECT

public:
  CFrmVscpLinkTest(QWidget* parent, json* pconn);
  ~CFrmVscpLinkTest();

private slots:
  void onConnectionSelectionChanged(int index);
  void refreshConnections();
  void createTcpipConnection();
  void runSelectedStep();
  void runAllSteps();
  void runReliabilityTest();
  void clearLog();

private:
  enum class StepResult { NotRun = 0, Pass, Fail };

  bool runStepByRow(int row);
  bool stepInitializeClient(QString& details);
  bool stepConnect(QString& details);
  bool stepVerifyConnected(QString& details);
  bool stepQueryServerVersion(QString& details);
  bool stepQueryServerCapabilities(QString& details);
  bool stepReportUnsupportedCommands(QString& details);
  bool stepTestCommandResponses(QString& details);
  bool stepVerifyLevel2(QString& details);
  bool stepDisconnect(QString& details);
  bool stepVerifyDisconnected(QString& details);
  bool ensureClientCreated(QString& details);
  bool resetClient();
  bool isLinkConnection(const json& conn) const;
  void loadAvailableConnections(const QString& preferredUuid = QString());
  bool applySelectedConnection(int index, QString* details = nullptr);
  void updateConnectionInfo();
  QString connectionTypeToString(CVscpClient::connType type) const;
  QString connectionEndpoint(const json& conn) const;
  QString connectionProtocolDetails(const json& conn) const;
  QString failureInsightForStep(int row, const QString& details) const;
  int connectionTimeoutSeconds() const;
  void resetStepResults();
  void setupUi();
  void addStepRows();
  void setStepResult(int row, StepResult result, const QString& details);
  void appendLog(const QString& text);

  json m_connObject;
  CVscpClient::connType m_connType;
  CVscpClient* m_vscpClient;
  QVector<json> m_connections;
  uint64_t m_eventsSent;
  uint64_t m_eventsFailed;
  uint64_t m_statsBaselineEvents;
  QString m_serverVersion;
  QString m_serverCapabilities;

  QComboBox* m_connectionCombo;
  QPushButton* m_refreshConnectionsButton;
  QPushButton* m_newTcpipButton;
  QLabel* m_connNameLabel;
  QLabel* m_connTypeLabel;
  QLabel* m_connEndpointLabel;
  QLabel* m_connUserLabel;
  QLabel* m_connSslLabel;
  QLabel* m_connTimeoutLabel;
  QLabel* m_connProtocolLabel;
  QLabel* m_connStateLabel;
  QLabel* m_serverVersionLabel;
  QLabel* m_serverCapabilitiesLabel;
  QTableWidget* m_stepTable;
  QPlainTextEdit* m_logEdit;
  QSpinBox* m_cycleCount;
  QSpinBox* m_retryCount;
  QSpinBox* m_stressEventCount;
  QCheckBox* m_validateTimeoutCheck;
  QLabel* m_reliabilityStatsLabel;

  QPushButton* m_runStepButton;
  QPushButton* m_runAllButton;
  QPushButton* m_reliabilityButton;
  QPushButton* m_clearLogButton;
};

#endif // CFRMVSCPLINKTEST_H
