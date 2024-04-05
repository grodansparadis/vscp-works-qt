// cdlgselectmqtttopic.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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
#include <vscp_client_mqtt.h>

#include "cdlgselectmqtttopics.h"
#include "ui_cdlgselectmqtttopics.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectMqttTopics::CDlgSelectMqttTopics(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgSelectMqttTopics)
{
  ui->setupUi(this);
  this->setFixedSize(this->size().width(), this->size().height());

  connect(
    this,
    &QDialog::accepted,
    this,
    &CDlgSelectMqttTopics::accepted);

  // connect(
  //   ui->tableTopics,
  //   &QTableWidget::cellDoubleClicked,
  //   this,
  //   &CDlgSelectMqttTopics::onRowDoubleClicked);

  m_type        = CDlgSelectMqttTopics::SUBSCRIBE; // Subscription topics is default
  m_pvscpClient = nullptr;                         // No client yet


}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectMqttTopics::~CDlgSelectMqttTopics()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// fillSubscriptionTopics
//

void
CDlgSelectMqttTopics::fillSubscriptionTopics(void)
{
  QTableWidgetItem* pitem;
  // QIcon icon(":/check-mark-red.png");
  int idx = 0; // Row index

  // Must be a client
  if (nullptr == m_pvscpClient) {
    return;
  }

  std::list<subscribeTopic*>* plist = m_pvscpClient->getSubscribeList();
  for (std::list<subscribeTopic*>::iterator it = plist->begin(); it != plist->end(); ++it) {
    subscribeTopic* psub = *it;
    std::cout << psub->getTopic() << std::endl;
    ui->tableTopics->insertRow(idx);
    pitem = new QTableWidgetItem(psub->getTopic().c_str());
    if (nullptr != pitem) {
      // pitem->setIcon(icon);
      // pitem->setCheckState(Qt::Checked);
      pitem->setCheckState(psub->isActive() ? Qt::Checked : Qt::Unchecked);
      pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      ui->tableTopics->setItem(idx, 0, pitem);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillPublishTopics
//

void
CDlgSelectMqttTopics::fillPublishTopics(void)
{
  QTableWidgetItem* pitem;
  // QIcon icon(":/check-mark-red.png");
  int idx = 0; // Row index

  // Must be a client
  if (nullptr == m_pvscpClient) {
    return;
  }

  std::list<publishTopic*>* plist = m_pvscpClient->getPublishList();
  for (std::list<publishTopic*>::iterator it = plist->begin(); it != plist->end(); ++it) {
    publishTopic* ppub = *it;
    std::cout << ppub->getTopic() << std::endl;
    ui->tableTopics->insertRow(idx);
    pitem = new QTableWidgetItem(ppub->getTopic().c_str());
    if (nullptr != pitem) {
      // pitem->setIcon(icon);
      // pitem->setCheckState(Qt::Checked);
      pitem->setCheckState(ppub->isActive() ? Qt::Checked : Qt::Unchecked);
      pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      ui->tableTopics->setItem(idx, 0, pitem);
    }
    idx++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
CDlgSelectMqttTopics::init(CDlgSelectMqttTopics::dlgtype type, const CVscpClient* pvscpClient)
{
  m_type        = type;

  // Check pointer
  if (nullptr == pvscpClient)  {
    return VSCP_ERROR_INVALID_POINTER;
  }

  m_pvscpClient = (vscpClientMqtt*)pvscpClient;

  // Fill the table with data
  if (CDlgSelectMqttTopics::SUBSCRIBE == type) {
    setWindowTitle("MQTT Subscription topics");
    fillSubscriptionTopics();
  }
  else if (CDlgSelectMqttTopics::PUBLISH == type) {
    setWindowTitle("MQTT Publish topics");
    fillPublishTopics();
  }
  else if (CDlgSelectMqttTopics::CLRRETAIN == type) {
    setWindowTitle("MQTT clear retain publish topics");
    fillPublishTopics();
  }

  QIcon icon(":/check-mark-red.png");
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isActive
//

bool
CDlgSelectMqttTopics::isActive(int pos)
{
  if (pos >= ui->tableTopics->rowCount()) {
    return false;
  }

  QTableWidgetItem* pitem = ui->tableTopics->item(pos, 0);
  if (nullptr == pitem) {
    return false;
  }

  return (Qt::Checked == pitem->checkState()) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// accepted
//

void
CDlgSelectMqttTopics::accepted(void)
{
  int pos = 0;
  if (CDlgSelectMqttTopics::SUBSCRIBE == m_type) {
    std::list<subscribeTopic*>* plist = m_pvscpClient->getSubscribeList();
    for (std::list<subscribeTopic*>::iterator it = plist->begin(); it != plist->end(); ++it) {
      subscribeTopic* psub = *it;
      psub->setActive(isActive(pos));
      pos++;
    }
  }
  else if (CDlgSelectMqttTopics::PUBLISH == m_type) {
    pos                             = 0;
    std::list<publishTopic*>* plist = m_pvscpClient->getPublishList();
    for (std::list<publishTopic*>::iterator it = plist->begin(); it != plist->end(); ++it) {
      publishTopic* ppub = *it;
      std::string str = ppub->getTopic();
      m_pvscpClient->clearRetain4Topic(str);
      pos++;
    }
  }
}
