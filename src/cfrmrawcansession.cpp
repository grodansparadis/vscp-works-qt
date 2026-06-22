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
#include <QMessageBox>
#include <QUrl>
#include <QVBoxLayout>
#include <QtSerialBus/QCanBus>

// ----------------------------------------------------------------------------

CFrmRawCanSession::CFrmRawCanSession(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_canDevice(nullptr)
  , m_statusLabel(nullptr)
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
  , m_tableFrames(nullptr)
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
  m_statusLabel          = new QLabel(this);
  m_statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  topLayout->addWidget(m_btnConnect);
  topLayout->addWidget(m_btnClear);
  topLayout->addWidget(btnHelp);
  topLayout->addStretch(1);
  topLayout->addWidget(m_statusLabel, 1);
  mainLayout->addLayout(topLayout);

  m_tableFrames = new QTableWidget(this);
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
  mainLayout->addWidget(m_tableFrames, 1);

  QGroupBox* sendBox     = new QGroupBox(tr("Send frame"), this);
  QGridLayout* sendLayout = new QGridLayout(sendBox);

  m_editFrameId = new QLineEdit(sendBox);
  m_editFrameId->setPlaceholderText(tr("Example: 0x123 or 291"));
  m_editPayload = new QLineEdit(sendBox);
  m_editPayload->setPlaceholderText(tr("Hex bytes: 11 22 33 or 112233"));

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
  m_tableFrames->setRowCount(0);
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
CFrmRawCanSession::appendFrame(const QCanBusFrame& frame, const QString& direction)
{
  const int row = m_tableFrames->rowCount();
  m_tableFrames->insertRow(row);

  const QString ts = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
  const QString frameType = (QCanBusFrame::DataFrame == frame.frameType())
                              ? tr("Data")
                              : (QCanBusFrame::RemoteRequestFrame == frame.frameType())
                                  ? tr("Remote")
                                  : tr("Other");
  const QString id = frame.hasExtendedFrameFormat()
                       ? QString("0x%1").arg(frame.frameId(), 8, 16, QChar('0')).toUpper()
                       : QString("0x%1").arg(frame.frameId(), 3, 16, QChar('0')).toUpper();

  m_tableFrames->setItem(row, 0, new QTableWidgetItem(ts));
  m_tableFrames->setItem(row, 1, new QTableWidgetItem(direction));
  m_tableFrames->setItem(row, 2, new QTableWidgetItem(id));
  m_tableFrames->setItem(row, 3, new QTableWidgetItem(frame.hasExtendedFrameFormat() ? tr("EXT") : tr("STD")));
  m_tableFrames->setItem(row, 4, new QTableWidgetItem(frameType + " " + frameFlagsToString(frame)));
  m_tableFrames->setItem(row, 5, new QTableWidgetItem(QString::number(frame.payload().size())));
  m_tableFrames->setItem(row, 6, new QTableWidgetItem(formatPayload(frame.payload())));
  m_tableFrames->scrollToBottom();
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
  QString str = m_editFrameId->text().trimmed();
  if (str.isEmpty()) {
    QMessageBox::warning(this, tr("VSCP Works"), tr("Frame ID is required."), QMessageBox::Ok);
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
  if (!ok) {
    QMessageBox::warning(this,
                         tr("VSCP Works"),
                         tr("Frame ID is not a valid number."),
                         QMessageBox::Ok);
    return false;
  }

  id = static_cast<uint32_t>(val);
  return true;
}

// ----------------------------------------------------------------------------

bool
CFrmRawCanSession::parsePayload(QByteArray& payload)
{
  payload.clear();
  QString str = m_editPayload->text().trimmed();
  if (str.isEmpty()) {
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
