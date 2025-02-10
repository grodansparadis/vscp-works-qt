// cdlgconnsettingsmqtt.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#include "vscpworks.h"
#include <vscphelper.h>

#include "cdlgconnsettingsmqtt.h"
#include "ui_cdlgconnsettingsmqtt.h"

#include "cdlgmqttpublish.h"
#include "cdlgmqttsubscribe.h"
#include "cdlgtls.h"

#include <QDebug>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

SubscribeItem::SubscribeItem(const QString& topic, enumMqttMsgFormat fmt, int qos, uint32_t v5_options)
  : QListWidgetItem(topic)
{
  m_topic      = topic;
  m_format     = fmt;
  m_qos        = qos & 3;
  m_v5_options = v5_options;

  m_bActive = true; // Active by default
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

SubscribeItem::~SubscribeItem() {}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

PublishItem::PublishItem(const QString& topic,
                         enumMqttMsgFormat fmt,
                         int qos,
                         bool bretain)
  : QListWidgetItem(vscp_str_format("%s - qos=%d retain=%s",
                                    topic.toStdString().c_str(),
                                    qos,
                                    bretain ? "true" : "false")
                      .c_str())
{
  m_topic   = topic;
  m_format  = fmt;
  m_qos     = qos;
  m_bRetain = bretain;

  m_bActive = true; // Active by default
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

PublishItem::~PublishItem() {}

///////////////////////////////////////////////////////////////////////////////
// setTopic
//

void
PublishItem::setTopic(const QString& topic)
{
  m_topic = topic;
  setText(vscp_str_format("%s - qos=%d retain=%s",
                          topic.toStdString().c_str(),
                          getQos(),
                          getRetain() ? "true" : "false")
            .c_str());
};

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsMqtt::CDlgConnSettingsMqtt(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsMqtt)
{
  ui->setupUi(this);

  // Generate unique client id
  QString clientid;
  int rVal = rand() % 100000;
  clientid.setNum(rVal, 16);
  clientid = "vscpworks-" + clientid;

  // Set defaults
  m_bTLS = false;
  // setConnectionTimeout(TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS);
  // setResponseTimeout(TCPIP_DEFAULT_RESPONSE_TIMEOUT);
  setBroker("tcp://localhost:1883");
  setClientId(clientid);
  setUser("vscp");
  setPassword("secret");
  // Clear filter
  // memset(&m_filter, 0, sizeof(vscpEventFilter));

  connect(ui->btnTLS,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsMqtt::onTLSSettings);
  connect(ui->btnAddSubscription,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsMqtt::onAddSubscription);
  connect(ui->btnAddPublish,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsMqtt::onAddPublish);
  connect(ui->btnTestConnection,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsMqtt::onTestConnection);

  // Help button
  connect(ui->buttonBox,
          &QDialogButtonBox::helpRequested,
          this,
          &CDlgConnSettingsMqtt::onGetHelp);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listSubscribe,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgConnSettingsMqtt::onSubscribeContextMenu);

  // Open edit dialog on item double click
  connect(ui->listSubscribe,
          &QListWidget::doubleClicked,
          this,
          &CDlgConnSettingsMqtt::onEditSubscription);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listPublish,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgConnSettingsMqtt::onPublishContextMenu);

  // Open edit dialog on item double click
  connect(ui->listPublish,
          &QListWidget::doubleClicked,
          this,
          &CDlgConnSettingsMqtt::onEditPublish);

  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsMqtt::~CDlgConnSettingsMqtt()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsMqtt::setInitialFocus(void)
{
  ui->editDescription->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onGetHelp
//

void
CDlgConnSettingsMqtt::onGetHelp()
{
  QUrl helpUrl("https://docs.vscp.org/");
  QDesktopServices::openUrl(helpUrl);
}

//-----------------------------------------------------------------------------
// Getters / Setters
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgConnSettingsMqtt::getName(void)
{
  return (ui->editDescription->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgConnSettingsMqtt::setName(const QString& str)
{
  ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getBroker
//

QString
CDlgConnSettingsMqtt::getBroker(void)
{
  return (ui->editHost->text());
}

///////////////////////////////////////////////////////////////////////////////
// setBroker
//

void
CDlgConnSettingsMqtt::setBroker(const QString& str)
{
  ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getClientId
//

QString
CDlgConnSettingsMqtt::getClientId(void)
{
  return ui->editClientId->text();
}

///////////////////////////////////////////////////////////////////////////////
// setClientId
//

void
CDlgConnSettingsMqtt::setClientId(const QString& clientid)
{
  ui->editClientId->setText(clientid);
}

///////////////////////////////////////////////////////////////////////////////
// getFullL2
//

bool
CDlgConnSettingsMqtt::getFullL2(void)
{
  return ui->chkFullLevel2->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// setFullL2
//

void
CDlgConnSettingsMqtt::setFullL2(bool l2)
{
  ui->chkFullLevel2->setChecked(l2);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString
CDlgConnSettingsMqtt::getUser(void)
{
  return (ui->editUser->text());
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void
CDlgConnSettingsMqtt::setUser(const QString& str)
{
  ui->editUser->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString
CDlgConnSettingsMqtt::getPassword(void)
{
  return (ui->editPassword->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void
CDlgConnSettingsMqtt::setPassword(const QString& str)
{
  ui->editPassword->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
CDlgConnSettingsMqtt::getResponseTimeout(void)
{
  return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
CDlgConnSettingsMqtt::setResponseTimeout(uint32_t timeout)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->decimalToStringInBase(timeout);
  ui->editKeepAlive->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getKeepAlive
//

uint32_t
CDlgConnSettingsMqtt::getKeepAlive(void)
{
  return vscp_readStringValue(ui->editKeepAlive->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setKeepAlive
//

void
CDlgConnSettingsMqtt::setKeepAlive(uint32_t timeout)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->decimalToStringInBase(timeout, 10);
  ui->editKeepAlive->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
CDlgConnSettingsMqtt::getConnectTimeout(void)
{
  uint32_t timeout = vscp_readStringValue(ui->editConnectTimeout->text().toStdString());
  m_client.setConnectionTimeout(timeout);
  return timeout;
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
CDlgConnSettingsMqtt::setConnectTimeout(uint32_t timeout)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->decimalToStringInBase(timeout, 10);
  ui->editConnectTimeout->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// isCleanSessionEnabled
//

bool
CDlgConnSettingsMqtt::isCleanSessionEnabled(void)
{
  return ui->chkCleanSession->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// enableCleanSession
//

void
CDlgConnSettingsMqtt::enableCleanSession(bool clean)
{
  ui->chkCleanSession->setChecked(clean);
}

///////////////////////////////////////////////////////////////////////////////
// isExtendedSecurityEnabled
//

bool
CDlgConnSettingsMqtt::isExtendedSecurityEnabled(void)
{
  return ui->chkExtendedSecurity->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// enableExtendedSecurity
//

void
CDlgConnSettingsMqtt::enableExtendedSecurity(bool extended)
{
  ui->chkExtendedSecurity->setChecked(extended);
}

///////////////////////////////////////////////////////////////////////////////
// isTLSEnabled
//

bool
CDlgConnSettingsMqtt::isTLSEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableTLS
//

void
CDlgConnSettingsMqtt::enableTLS(bool btls)
{
  m_bTLS = btls;
}

///////////////////////////////////////////////////////////////////////////////
// isVerifyPeerEnabled
//

bool
CDlgConnSettingsMqtt::isVerifyPeerEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableVerifyPeer
//

void
CDlgConnSettingsMqtt::enableVerifyPeer(bool bverifypeer)
{
  m_bVerifyPeer = bverifypeer;
}

///////////////////////////////////////////////////////////////////////////////
// getCaFile
//

QString
CDlgConnSettingsMqtt::getCaFile(void)
{
  return m_cafile;
}

///////////////////////////////////////////////////////////////////////////////
// setCaFile
//

void
CDlgConnSettingsMqtt::setCaFile(const QString& str)
{
  m_cafile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCaPath
//

QString
CDlgConnSettingsMqtt::getCaPath(void)
{
  return m_capath;
}

///////////////////////////////////////////////////////////////////////////////
// setCaPath
//

void
CDlgConnSettingsMqtt::setCaPath(const QString& str)
{
  m_capath = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCertFile
//

QString
CDlgConnSettingsMqtt::getCertFile(void)
{
  return m_certfile;
}

///////////////////////////////////////////////////////////////////////////////
// setCertFile
//

void
CDlgConnSettingsMqtt::setCertFile(const QString& str)
{
  m_certfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getKeyFile
//

QString
CDlgConnSettingsMqtt::getKeyFile(void)
{
  return m_keyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setKeyFile
//

void
CDlgConnSettingsMqtt::setKeyFile(const QString& str)
{
  m_keyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getPwKeyFile
//

QString
CDlgConnSettingsMqtt::getPwKeyFile(void)
{
  return m_pwkeyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setPwKeyFile
//

void
CDlgConnSettingsMqtt::setPwKeyFile(const QString& str)
{
  m_pwkeyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsMqtt::getJson(void)
{
  m_jsonConfig["type"]               = static_cast<int>(CVscpClient::connType::MQTT);
  m_jsonConfig["name"]               = getName().toStdString();
  m_jsonConfig["host"]               = getBroker().toStdString();
  m_jsonConfig["clientid"]           = getClientId().toStdString();
  m_jsonConfig["user"]               = getUser().toStdString();
  m_jsonConfig["password"]           = getPassword().toStdString();
  m_jsonConfig["connection-timeout"] = (int)getConnectTimeout();
  m_jsonConfig["response-timeout"]   = (int)getResponseTimeout();
  m_jsonConfig["keepalive"]          = (int)getKeepAlive();
  m_jsonConfig["cleansession"]       = isCleanSessionEnabled();
  m_jsonConfig["extended-security"]  = isExtendedSecurityEnabled();
  m_jsonConfig["bfull-l2"]           = getFullL2();

  m_jsonConfig["btls"]        = isTLSEnabled();
  m_jsonConfig["bverifypeer"] = isVerifyPeerEnabled();
  m_jsonConfig["cafile"]      = getCaFile().toStdString();
  m_jsonConfig["capath"]      = getCaPath().toStdString();
  m_jsonConfig["certfile"]    = getCertFile().toStdString();
  m_jsonConfig["keyfile"]     = getKeyFile().toStdString();
  m_jsonConfig["pwkeyfile"]   = getPwKeyFile().toStdString();

  // Save all subscriptions
  json subscriptionArray = json::array();
  for (int i = 0; i < ui->listSubscribe->count(); i++) {
    json j;
    SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(i);
    j["topic"]           = pitem->getTopic().toStdString();
    j["format"]          = pitem->getFormatInt();
    j["qos"]             = pitem->getQos();
    j["v5_options"]      = pitem->getV5Options();
    subscriptionArray.push_back(j);
  }
  m_jsonConfig["subscribe"] = subscriptionArray;

  // Save all publishing
  json publishingArray = json::array();

  for (int i = 0; i < ui->listPublish->count(); i++) {
    json j;
    PublishItem* pitem = (PublishItem*)ui->listPublish->item(i);
    j["topic"]         = pitem->getTopic().toStdString();
    j["format"]        = pitem->getFormatInt();
    j["qos"]           = pitem->getQos();
    j["bretain"]       = pitem->getRetain();
    publishingArray.push_back(j);
  }
  m_jsonConfig["publish"] = publishingArray;

  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsMqtt::setJson(const json* pobj)
{
  m_jsonConfig = *pobj;

  if (m_jsonConfig.contains("name") && m_jsonConfig["name"].is_string()) {
    setName(m_jsonConfig["name"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("host") && m_jsonConfig["host"].is_string()) {
    setBroker(m_jsonConfig["host"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("clientid") && m_jsonConfig["clientid"].is_string()) {
    setClientId(m_jsonConfig["clientid"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("bfull-l2") && m_jsonConfig["bfull-l2"].is_boolean()) {
    setFullL2(m_jsonConfig["bfull-l2"].get<short>());
  }

  if (m_jsonConfig.contains("user") && m_jsonConfig["user"].is_string()) {
    setUser(m_jsonConfig["user"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("password") && m_jsonConfig["password"].is_string()) {
    setPassword(m_jsonConfig["password"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("connection-timeout") && m_jsonConfig["connection-timeout"].is_number()) {
    setConnectTimeout(m_jsonConfig["connection-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("response-timeout") && m_jsonConfig["response-timeout"].is_number()) {
    setResponseTimeout(m_jsonConfig["response-timeout"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("keepalive") && m_jsonConfig["keepalive"].is_number()) {
    setKeepAlive(m_jsonConfig["keepalive"].get<uint32_t>());
  }

  if (m_jsonConfig.contains("cleansession") && m_jsonConfig["cleansession"].is_boolean()) {
    enableCleanSession(m_jsonConfig["cleansession"].get<bool>());
  }

  if (m_jsonConfig.contains("extended-security") && m_jsonConfig["extended-security"].is_boolean()) {
    enableExtendedSecurity(m_jsonConfig["extended-security"].get<bool>());
  }

  if (m_jsonConfig.contains("btls") && m_jsonConfig["btls"].is_boolean()) {
    enableTLS((short)m_jsonConfig["btls"].get<bool>());
  }

  if (m_jsonConfig.contains("bverifypeer") && m_jsonConfig["bverifypeer"].is_boolean()) {
    enableVerifyPeer(m_jsonConfig["bverifypeer"].get<bool>());
  }

  if (m_jsonConfig.contains("cafile") && m_jsonConfig["cafile"].is_string()) {
    setCaFile(m_jsonConfig["cafile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("capath") && m_jsonConfig["capath"].is_string()) {
    setCaPath(m_jsonConfig["capath"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("certfile") && m_jsonConfig["certfile"].is_string()) {
    setCertFile(m_jsonConfig["certfile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("keyfile") && m_jsonConfig["keyfile"].is_string()) {
    setKeyFile(m_jsonConfig["keyfile"].get<std::string>().c_str());
  }

  if (m_jsonConfig.contains("pwkeyfile") && m_jsonConfig["pwkeyfile"].is_string()) {
    setPwKeyFile(m_jsonConfig["pwkeyfile"].get<std::string>().c_str());
  }

  // Subscriptions
  if (m_jsonConfig.contains("subscribe") && m_jsonConfig["subscribe"].is_array()) {
    json subscribeArray = json::array();
    subscribeArray      = m_jsonConfig["subscribe"];
    spdlog::trace(subscribeArray.dump(4).c_str());

    for (auto v : subscribeArray) {
      int qos             = 0;
      uint32_t v5_options = 0;
      json item           = v;
      if (item.contains("topic") && item.contains("format")) {
        if (item.contains("qos")) {
          qos = item["qos"].get<int>();
        }
        if (item.contains("v5_options")) {
          v5_options = item["v5_options"].get<uint32_t>();
        }
        SubscribeItem* pitem =
          new SubscribeItem(item["topic"].get<std::string>().c_str(),
                            static_cast<enumMqttMsgFormat>(item["format"].get<int>()),
                            qos,
                            v5_options);
        ui->listSubscribe->addItem(pitem);
      }
    }

    ui->listSubscribe->sortItems();
  }

  // Publish
  if (m_jsonConfig.contains("publish") && m_jsonConfig["publish"].is_array()) {
    json interfacesArray = json::array();
    interfacesArray      = m_jsonConfig["publish"];

    for (auto v : interfacesArray) {

      json item = v;
      if (item.contains("topic") && item.contains("format") &&
          item.contains("qos") && item.contains("bretain")) {
        PublishItem* pitem = new PublishItem(
          item["topic"].get<std::string>().c_str(),
          static_cast<enumMqttMsgFormat>(item["format"].get<int>()),
          item["qos"].get<int>(),
          item["bretain"].get<bool>());
        ui->listPublish->addItem(pitem);
      }
    }

    ui->listPublish->sortItems();
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// onTestConnection
//

void
CDlgConnSettingsMqtt::onTestConnection(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  std::string str = getBroker().toStdString();
  m_client.setHost(str);
  str = getUser().toStdString();
  m_client.setUser(str);
  str = getPassword().toStdString();
  m_client.setPassword(str);
  str = getClientId().toStdString();
  m_client.setClientId(str);
  // m_client->setConnectionTimeout(ui->spinConnectionTimeout->value());  // TODO

  // Connect to remote host
  if (VSCP_ERROR_SUCCESS != m_client.connect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to connect to remote MQTT broker"));
    m_client.disconnect();
    return;
  }

  // Get server version
  uint8_t major_ver;
  uint8_t minor_ver;
  uint8_t release_ver;
  uint8_t build_ver;
  QString strVersion;
  if (VSCP_ERROR_SUCCESS == m_client.getversion(&major_ver,
                                                &minor_ver,
                                                &release_ver,
                                                &build_ver)) {

    strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
                                 (int)major_ver,
                                 (int)minor_ver,
                                 (int)release_ver,
                                 (int)build_ver)
                   .c_str();
  }
  else {
    strVersion = tr("Failed to get version from server");
  }

  // Disconnect from remote host
  if (VSCP_ERROR_SUCCESS != m_client.disconnect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(
      this,
      tr(APPNAME),
      tr("Failed to disconnect from remote MQTT broker"));
    return;
  }

  QApplication::restoreOverrideCursor();

  QString msg = tr("Connection test was successful");
  msg += "\n";
  // msg += strVersion;
  QMessageBox::information(this, tr(APPNAME), msg);
}

///////////////////////////////////////////////////////////////////////////////
// onTLSSettings
//

void
CDlgConnSettingsMqtt::onTLSSettings(void)
{
  CDlgTLS dlg;

  dlg.enableTLS(m_bTLS);
  dlg.enableVerifyPeer(m_bVerifyPeer);
  dlg.setCaFile(m_cafile);
  dlg.setCaPath(m_capath);
  dlg.setCertFile(m_certfile);
  dlg.setKeyFile(m_keyfile);
  dlg.setPwKeyFile(m_pwkeyfile);

  if (QDialog::Accepted == dlg.exec()) {
    m_bTLS        = dlg.isTLSEnabled();
    m_bVerifyPeer = dlg.isVerifyPeerEnabled();
    m_cafile      = dlg.getCaFile();
    m_capath      = dlg.getCaPath();
    m_certfile    = dlg.getCertFile();
    m_keyfile     = dlg.getKeyFile();
    m_pwkeyfile   = dlg.getPwKeyFile();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onAddSubscription
//

void
CDlgConnSettingsMqtt::onAddSubscription(void)
{
  // bool ok;
  // QString topic = QInputDialog::getText(this,
  //                                       tr(APPNAME),
  //                                       tr("Subscription topic:"),
  //                                       QLineEdit::Normal,
  //                                       "",
  //                                       &ok);
  // if (ok && !topic.isEmpty()) {
  //     ui->listSubscribe->addItem(new SubscribeItem(topic));
  //     ui->listSubscribe->sortItems();
  // }

  CDlgMqttSubscribe dlg(this);

  if (QDialog::Accepted == dlg.exec()) {

    ui->listSubscribe->addItem(new SubscribeItem(dlg.getTopic(),
                                                 dlg.getFormat()));
    ui->listSubscribe->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onEditSubscription
//

void
CDlgConnSettingsMqtt::onEditSubscription(void)
{
  // bool ok;
  CDlgMqttSubscribe dlg(this);

  // Get selected row
  int row = ui->listSubscribe->currentRow();
  if (-1 == row)
    return;

  SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(row);
  if (nullptr == pitem)
    return;

  // QString topic = QInputDialog::getText(this,
  //                                       tr(APPNAME),
  //                                       tr("Subscription topic:"),
  //                                       QLineEdit::Normal,
  //                                       pitem->getTopic(),
  //                                       &ok);
  // if (ok && !topic.isEmpty()) {
  //     pitem->setTopic(topic);
  //     ui->listSubscribe->sortItems();
  // }

  dlg.setTopic(pitem->getTopic());
  dlg.setFormat(pitem->getFormat());

  if (QDialog::Accepted == dlg.exec()) {
    pitem->setFormat(dlg.getFormat());
    pitem->setTopic(dlg.getTopic());
    ui->listSubscribe->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onCloneSubscription
//

void
CDlgConnSettingsMqtt::onCloneSubscription(void)
{
  // Get selected row
  int row = ui->listSubscribe->currentRow();
  if (-1 == row)
    return;

  SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(row);
  if (nullptr == pitem)
    return;

  ui->listSubscribe->addItem(new SubscribeItem(pitem->getTopic()));

  ui->listSubscribe->sortItems();
}

///////////////////////////////////////////////////////////////////////////////
// onDeleteSubscription
//

void
CDlgConnSettingsMqtt::onDeleteSubscription(void)
{
  // Get selected row
  int row = ui->listSubscribe->currentRow();
  if (-1 == row)
    return;

  SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(row);
  if (nullptr == pitem)
    return;

  if (QMessageBox::Yes ==
      QMessageBox::question(
        this,
        tr(APPNAME),
        tr("Are you sure the subscription topic should be deleted?"),
        QMessageBox::Yes | QMessageBox::No)) {

    ui->listSubscribe->takeItem(row);
    // ui->listSubscribe->removeItemWidget();
    delete pitem;
  }
}

///////////////////////////////////////////////////////////////////////////////
// onAddPublish
//

void
CDlgConnSettingsMqtt::onAddPublish(void)
{
  CDlgMqttPublish dlg(this);

  if (QDialog::Accepted == dlg.exec()) {
    qDebug() << dlg.getQos();
    qDebug() << dlg.getRetain();
    ui->listPublish->addItem(new PublishItem(dlg.getTopic(),
                                             dlg.getFormat(),
                                             dlg.getQos(),
                                             dlg.getRetain()));
    ui->listPublish->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onEditSubscription
//

void
CDlgConnSettingsMqtt::onEditPublish(void)
{
  CDlgMqttPublish dlg(this);

  // Get selected row
  int row = ui->listPublish->currentRow();
  if (-1 == row)
    return;

  PublishItem* pitem = (PublishItem*)ui->listPublish->item(row);
  if (nullptr == pitem)
    return;

  dlg.setTopic(pitem->getTopic());
  dlg.setFormat(pitem->getFormat());
  dlg.setQos(pitem->getQos());
  dlg.setRetain(pitem->getRetain());

  if (QDialog::Accepted == dlg.exec()) {
    pitem->setQos(dlg.getQos());
    pitem->setRetain(dlg.getRetain());
    pitem->setFormat(dlg.getFormat());
    pitem->setTopic(dlg.getTopic());
    ui->listPublish->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onCloneSubscription
//

void
CDlgConnSettingsMqtt::onClonePublish(void)
{
  // Get selected row
  int row = ui->listPublish->currentRow();
  if (-1 == row) {
    return;
  }

  PublishItem* pitem = (PublishItem*)ui->listPublish->item(row);
  if (nullptr == pitem) {
    return;
  }

  ui->listPublish->addItem(new PublishItem(pitem->getTopic(),
                                           pitem->getFormat(),
                                           pitem->getQos(),
                                           pitem->getRetain()));

  ui->listPublish->sortItems();
}

///////////////////////////////////////////////////////////////////////////////
// onDeleteSubscription
//

void
CDlgConnSettingsMqtt::onDeletePublish(void)
{
  // Get selected row
  int row = ui->listPublish->currentRow();
  if (-1 == row)
    return;

  PublishItem* pitem = (PublishItem*)ui->listPublish->item(row);
  if (nullptr == pitem)
    return;

  if (QMessageBox::Yes ==
      QMessageBox::question(
        this,
        tr(APPNAME),
        tr("Are you sure the publish topic should be deleted?"),
        QMessageBox::Yes | QMessageBox::No)) {

    ui->listPublish->takeItem(row);
    delete pitem;
  }
}

///////////////////////////////////////////////////////////////////////////////
// onSubscribeItemClicked
//

void
CDlgConnSettingsMqtt::onSubscribeContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString("New subscription"),
                  this,
                  SLOT(onAddSubscription()));
  menu->addAction(QString("Edit subscription"),
                  this,
                  SLOT(onEditSubscription()));
  menu->addAction(QString("Clone subscription"),
                  this,
                  SLOT(onCloneSubscription()));
  menu->addAction(QString("delete subscription"),
                  this,
                  SLOT(onDeleteSubscription()));

  menu->popup(ui->listSubscribe->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// onPublishItemClicked
//

void
CDlgConnSettingsMqtt::onPublishContextMenu(const QPoint& pos)
{
  QMenu* menu = new QMenu(this);

  menu->addAction(QString("New publishing"), this, SLOT(onAddPublish()));
  menu->addAction(QString("Edit publishing"), this, SLOT(onEditPublish()));
  menu->addAction(QString("Clone publishing"), this, SLOT(onClonePublish()));
  menu->addAction(QString("delete publishing"),
                  this,
                  SLOT(onDeletePublish()));

  menu->popup(ui->listPublish->viewport()->mapToGlobal(pos));
}
