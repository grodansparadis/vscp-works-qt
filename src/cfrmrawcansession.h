// cfrmrawcansession.h
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

#ifndef CFRMRAWCANSESSION_H
#define CFRMRAWCANSESSION_H

#ifndef WIN32

#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanBusFrame>

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class CFrmRawCanSession : public QDialog
{
  Q_OBJECT

public:
  explicit CFrmRawCanSession(QWidget* parent, json* pconn);
  ~CFrmRawCanSession();

private slots:
  void connectOrDisconnect();
  void sendFrame();
  void processReceivedFrames();
  void processError(QCanBusDevice::CanBusError error);
  void clearLog();
  void showHelp();

private:
  void setupUi();
  void appendFrame(const QCanBusFrame& frame, const QString& direction);
  void setConnectedState(bool connected);
  bool parseFrameId(uint32_t& id);
  bool parsePayload(QByteArray& payload);
  QString formatPayload(const QByteArray& payload) const;
  QString frameFlagsToString(const QCanBusFrame& frame) const;

  json m_connObject;
  QString m_interfaceName;

  QCanBusDevice* m_canDevice;

  QLabel* m_statusLabel;
  QLineEdit* m_editFrameId;
  QLineEdit* m_editPayload;
  QCheckBox* m_chkExtended;
  QCheckBox* m_chkFd;
  QCheckBox* m_chkBitrateSwitch;
  QCheckBox* m_chkErrorStateIndicator;
  QCheckBox* m_chkRemoteRequest;
  QPushButton* m_btnConnect;
  QPushButton* m_btnSend;
  QPushButton* m_btnClear;
  QTableWidget* m_tableFrames;
};

#endif // !WIN32

#endif // CFRMRAWCANSESSION_H
