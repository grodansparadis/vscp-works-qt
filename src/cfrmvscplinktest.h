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

QT_BEGIN_NAMESPACE
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
  bool stepVerifyLevel2(QString& details);
  bool stepDisconnect(QString& details);
  bool ensureClientCreated(QString& details);
  bool resetClient();
  void setupUi();
  void addStepRows();
  void setStepResult(int row, StepResult result, const QString& details);
  void appendLog(const QString& text);

  json m_connObject;
  CVscpClient::connType m_connType;
  CVscpClient* m_vscpClient;

  QTableWidget* m_stepTable;
  QPlainTextEdit* m_logEdit;
  QSpinBox* m_cycleCount;

  QPushButton* m_runStepButton;
  QPushButton* m_runAllButton;
  QPushButton* m_reliabilityButton;
  QPushButton* m_clearLogButton;
};

#endif // CFRMVSCPLINKTEST_H
