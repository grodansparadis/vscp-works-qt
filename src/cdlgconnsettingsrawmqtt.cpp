// cdlgconnsettingsrawmqtt.cpp
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
#include "cdlgconnsettingsrawmqtt.h"
#include "ui_cdlgconnsettingsrawmqtt.h"

#include "cdlgmqttpublish.h"
#include "cdlgtls.h"

#include <QDebug>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsRawMqtt::CDlgConnSettingsRawMqtt(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsRawMqtt)
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
  setPort(1883);
  setUser("admin");
  setPassword("secret");

  // Clear filter
  memset(&m_filter, 0, sizeof(vscpEventFilter));

  connect(ui->btnTLS,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsRawMqtt::onTLSSettings);
  connect(ui->btnAddSubscription,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsRawMqtt::onAddSubscription);
  connect(ui->btnAddPublish,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsRawMqtt::onAddPublish);
  connect(ui->btnTestConnection,
          &QPushButton::clicked,
          this,
          &CDlgConnSettingsRawMqtt::onTestConnection);

  // Help button
  connect(ui->buttonBox,
          &QDialogButtonBox::helpRequested,
          this,
          &CDlgConnSettingsRawMqtt::onGetHelp);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listSubscribe,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgConnSettingsRawMqtt::onSubscribeContextMenu);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listPublish,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgConnSettingsRawMqtt::onPublishContextMenu);

          QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(ui->helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsRawMqtt::~CDlgConnSettingsRawMqtt()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsRawMqtt::setInitialFocus(void)
{
  ui->editDescription->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// onGetHelp
//

void
CDlgConnSettingsRawMqtt::onGetHelp()
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
CDlgConnSettingsRawMqtt::getName(void)
{
  return (ui->editDescription->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgConnSettingsRawMqtt::setName(const QString& str)
{
  ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getBroker
//

QString
CDlgConnSettingsRawMqtt::getBroker(void)
{
  return (ui->editHost->text());
}

///////////////////////////////////////////////////////////////////////////////
// setBroker
//

void
CDlgConnSettingsRawMqtt::setBroker(const QString& str)
{
  ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPort
//

short
CDlgConnSettingsRawMqtt::getPort(void)
{
  short port = vscp_readStringValue(ui->editPort->text().toStdString());
  return port;
}

///////////////////////////////////////////////////////////////////////////////
// setPort
//

void
CDlgConnSettingsRawMqtt::setPort(short port)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->decimalToStringInBase(port);
  ui->editPort->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

QString
CDlgConnSettingsRawMqtt::getUser(void)
{
  return (ui->editUser->text());
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

void
CDlgConnSettingsRawMqtt::setUser(const QString& str)
{
  ui->editUser->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPassword
//

QString
CDlgConnSettingsRawMqtt::getPassword(void)
{
  return (ui->editPassword->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPassword
//

void
CDlgConnSettingsRawMqtt::setPassword(const QString& str)
{
  ui->editPassword->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
CDlgConnSettingsRawMqtt::getConnectionTimeout(void)
{
  return m_client.getConnectionTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
CDlgConnSettingsRawMqtt::setConnectionTimeout(uint32_t timeout)
{
  m_client.setConnectionTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
CDlgConnSettingsRawMqtt::getResponseTimeout(void)
{
  return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
CDlgConnSettingsRawMqtt::setResponseTimeout(uint32_t timeout)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  QString str       = pworks->decimalToStringInBase(timeout);
  ui->editKeepAlive->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getKeepAlive
//

uint32_t
CDlgConnSettingsRawMqtt::getKeepAlive(void)
{
  return vscp_readStringValue(ui->editKeepAlive->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setKeepAlive
//

void
CDlgConnSettingsRawMqtt::setKeepAlive(uint32_t timeout)
{
  m_client.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// isExtendedSecurityEnabled
//

bool
CDlgConnSettingsRawMqtt::isExtendedSecurityEnabled(void)
{
  return ui->chkExtendedSecurity->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
// enableExtendedSecurity
//

void
CDlgConnSettingsRawMqtt::enableExtendedSecurity(bool extended)
{
  ui->chkExtendedSecurity->setChecked(extended);
}

///////////////////////////////////////////////////////////////////////////////
// isTLSEnabled
//

bool
CDlgConnSettingsRawMqtt::isTLSEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableTLS
//

void
CDlgConnSettingsRawMqtt::enableTLS(bool btls)
{
  m_bTLS = btls;
}

///////////////////////////////////////////////////////////////////////////////
// isVerifyPeerEnabled
//

bool
CDlgConnSettingsRawMqtt::isVerifyPeerEnabled(void)
{
  return m_bTLS;
}

///////////////////////////////////////////////////////////////////////////////
// enableVerifyPeer
//

void
CDlgConnSettingsRawMqtt::enableVerifyPeer(bool bverifypeer)
{
  m_bVerifyPeer = bverifypeer;
}

///////////////////////////////////////////////////////////////////////////////
// getCaFile
//

QString
CDlgConnSettingsRawMqtt::getCaFile(void)
{
  return m_cafile;
}

///////////////////////////////////////////////////////////////////////////////
// setCaFile
//

void
CDlgConnSettingsRawMqtt::setCaFile(const QString& str)
{
  m_cafile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCaPath
//

QString
CDlgConnSettingsRawMqtt::getCaPath(void)
{
  return m_capath;
}

///////////////////////////////////////////////////////////////////////////////
// setCaPath
//

void
CDlgConnSettingsRawMqtt::setCaPath(const QString& str)
{
  m_capath = str;
}

///////////////////////////////////////////////////////////////////////////////
// getCertFile
//

QString
CDlgConnSettingsRawMqtt::getCertFile(void)
{
  return m_certfile;
}

///////////////////////////////////////////////////////////////////////////////
// setCertFile
//

void
CDlgConnSettingsRawMqtt::setCertFile(const QString& str)
{
  m_certfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getKeyFile
//

QString
CDlgConnSettingsRawMqtt::getKeyFile(void)
{
  return m_keyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setKeyFile
//

void
CDlgConnSettingsRawMqtt::setKeyFile(const QString& str)
{
  m_keyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getPwKeyFile
//

QString
CDlgConnSettingsRawMqtt::getPwKeyFile(void)
{
  return m_pwkeyfile;
}

///////////////////////////////////////////////////////////////////////////////
// setPwKeyFile
//

void
CDlgConnSettingsRawMqtt::setPwKeyFile(const QString& str)
{
  m_pwkeyfile = str;
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

json
CDlgConnSettingsRawMqtt::getJson(void)
{
  m_jsonConfig["type"]               = static_cast<int>(CVscpClient::connType::RAWMQTT);
  m_jsonConfig["name"]               = getName();
  m_jsonConfig["host"]               = getBroker();
  m_jsonConfig["port"]               = getPort();
  m_jsonConfig["user"]               = getUser();
  m_jsonConfig["password"]           = getPassword();
  m_jsonConfig["connection-timeout"] = (int)getConnectionTimeout();
  m_jsonConfig["response-timeout"]   = (int)getResponseTimeout();
  m_jsonConfig["keepalive"]          = (int)getKeepAlive();
  m_jsonConfig["extended-security"]  = isExtendedSecurityEnabled();

  m_jsonConfig["btls"]        = isTLSEnabled();
  m_jsonConfig["bverifypeer"] = isVerifyPeerEnabled();
  m_jsonConfig["cafile"]      = getCaFile();
  m_jsonConfig["capath"]      = getCaPath();
  m_jsonConfig["certfile"]    = getCertFile();
  m_jsonConfig["keyfile"]     = getKeyFile();
  m_jsonConfig["pwkeyfile"]   = getPwKeyFile();

  // Save all subscriptions
  json subscriptionArray = json::array();
 
  for (int i = 0; i < ui->listSubscribe->count(); i++) {
    json obj;
    SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(i);
    obj["topic"]         = pitem->getTopic();
    obj["format"]        = pitem->getFormatInt();
    obj["qos"]           = pitem->getQos();
    subscriptionArray.append(obj);
  }
  m_jsonConfig["subscriptions"] = subscriptionArray;

  // Save all publishing
  json publishingArray = json::array();
  
  for (int i = 0; i < ui->listPublish->count(); i++) {
    json obj;
    PublishItem* pitem = (PublishItem*)ui->listPublish->item(i);
    obj["topic"]       = pitem->getTopic();
    obj["format"]      = pitem->getFormatInt();
    obj["qos"]         = pitem->getQos();
    obj["bretain"]     = pitem->getRetain();
    publishingArray.append(obj);
  }
  m_jsonConfig["publishing"] = publishingArray;

  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsRawMqtt::setJson(const json* pobj)
{
  m_jsonConfig = *pobj;

  if (!m_jsonConfig["name"].isNull())
    setName(m_jsonConfig["name"].toString());
  if (!m_jsonConfig["host"].isNull())
    setBroker(m_jsonConfig["host"].toString());
  if (!m_jsonConfig["port"].isNull())
    setPort((short)m_jsonConfig["port"].toInt());
  if (!m_jsonConfig["user"].isNull())
    setUser(m_jsonConfig["user"].toString());
  if (!m_jsonConfig["password"].isNull())
    setPassword(m_jsonConfig["password"].toString());
  if (!m_jsonConfig["connection-timeout"].isNull())
    setConnectionTimeout(
      (uint32_t)m_jsonConfig["connection-timeout"].toInt());
  if (!m_jsonConfig["response-timeout"].isNull())
    setResponseTimeout((uint32_t)m_jsonConfig["response-timeout"].toInt());
  if (!m_jsonConfig["keepalive"].isNull())
    setKeepAlive((uint32_t)m_jsonConfig["keepalive"].toInt());
  if (!m_jsonConfig["extended-security"].isNull())
    enableExtendedSecurity(m_jsonConfig["extended-security"].toBool());

  if (!m_jsonConfig["btls"].isNull())
    enableTLS((short)m_jsonConfig["btls"].toBool());
  if (!m_jsonConfig["bverifypeer"].isNull())
    enableVerifyPeer(m_jsonConfig["bverifypeer"].toBool());
  if (!m_jsonConfig["cafile"].isNull())
    setCaFile(m_jsonConfig["cafile"].toString());
  if (!m_jsonConfig["capath"].isNull())
    setCaPath(m_jsonConfig["capath"].toString());
  if (!m_jsonConfig["certfile"].isNull())
    setCertFile(m_jsonConfig["certfile"].toString());
  if (!m_jsonConfig["keyfile"].isNull())
    setKeyFile(m_jsonConfig["keyfile"].toString());
  if (!m_jsonConfig["pwkeyfile"].isNull())
    setPwKeyFile(m_jsonConfig["pwkeyfile"].toString());

  // Subscriptions
  if (m_jsonConfig["subscriptions"].isArray()) {
    json interfacesArray = m_jsonConfig["subscriptions"];

    for (auto v : interfacesArray) {

      json item = v.toObject();
      if (!item["topic"].isNull()) {
        SubscribeItem* pitem =
          new SubscribeItem(item["topic"].toString());
        ui->listSubscribe->addItem(pitem);
      }
    }

    ui->listSubscribe->sortItems();
  }

  // Publish
  if (m_jsonConfig["publishing"].isArray()) {
    json interfacesArray = m_jsonConfig["publishing"];

    for (auto v : interfacesArray) {

      json item = v.toObject();
      if (!item["topic"].isNull() && !item["format"].isNull() &&
          !item["qos"].isNull() && !item["bretain"].isNull()) {
        PublishItem* pitem = new PublishItem(
          item["topic"].toString(),
          static_cast<enumMqttMsgFormat>(item["format"].toInt()),
          item["qos"].toInt(),
          item["bretain"].toBool());
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
CDlgConnSettingsRawMqtt::onTestConnection(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::processEvents();

  // Initialize host connection
  // if (VSCP_ERROR_SUCCESS !=
  //     m_client.init(getBroker().toStdString().c_str(),
  //                   // getPort(),
  //                   // "#",
  //                   // "test",
  //                   "",
  //                   getUser().toStdString().c_str(),
  //                   getPassword().toStdString().c_str())) {
  //     QApplication::restoreOverrideCursor();
  //     QMessageBox::information(this,
  //                              tr(APPNAME),
  //                              tr("Failed to initialize MQTT client"));
  //     return;
  // }

  // Connect to remote host
  if (VSCP_ERROR_SUCCESS != m_client.connect()) {
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Failed to connect to remote MQTT broker"));
    m_client.disconnect();
    return;
  }

  // // Get server version
  // uint8_t major_ver;
  // uint8_t minor_ver;
  // uint8_t release_ver;
  // uint8_t build_ver;
  // QString strVersion;
  // if ( VSCP_ERROR_SUCCESS == m_client.getversion( &major_ver,
  //                                                 &minor_ver,
  //                                                 &release_ver,
  //                                                 &build_ver ) ) {

  //     strVersion = vscp_str_format("Remote server version: %d.%d.%d.%d",
  //                                     (int)major_ver,
  //                                     (int)minor_ver,
  //                                     (int)release_ver,
  //                                     (int)build_ver ).c_str();
  // }
  // else {
  //     strVersion = tr("Failed to get version from server");
  // }

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
CDlgConnSettingsRawMqtt::onTLSSettings(void)
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
CDlgConnSettingsRawMqtt::onAddSubscription(void)
{
  bool ok;
  QString topic = QInputDialog::getText(this,
                                        tr(APPNAME),
                                        tr("Subscription topic:"),
                                        QLineEdit::Normal,
                                        "",
                                        &ok);
  if (ok && !topic.isEmpty()) {
    ui->listSubscribe->addItem(new SubscribeItem(topic));
    ui->listSubscribe->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onEditSubscription
//

void
CDlgConnSettingsRawMqtt::onEditSubscription(void)
{
  bool ok;

  // Get selected row
  int row = ui->listSubscribe->currentRow();
  if (-1 == row)
    return;

  SubscribeItem* pitem = (SubscribeItem*)ui->listSubscribe->item(row);
  if (nullptr == pitem)
    return;

  QString topic = QInputDialog::getText(this,
                                        tr(APPNAME),
                                        tr("Subscription topic:"),
                                        QLineEdit::Normal,
                                        pitem->getTopic(),
                                        &ok);
  if (ok && !topic.isEmpty()) {
    pitem->setTopic(topic);
    ui->listSubscribe->sortItems();
  }
}

///////////////////////////////////////////////////////////////////////////////
// onCloneSubscription
//

void
CDlgConnSettingsRawMqtt::onCloneSubscription(void)
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
CDlgConnSettingsRawMqtt::onDeleteSubscription(void)
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
CDlgConnSettingsRawMqtt::onAddPublish(void)
{
  CDlgMqttPublish dlg(this);

  if (QDialog::Accepted == dlg.exec()) {
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
CDlgConnSettingsRawMqtt::onEditPublish(void)
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
CDlgConnSettingsRawMqtt::onClonePublish(void)
{
  // Get selected row
  int row = ui->listPublish->currentRow();
  if (-1 == row)
    return;

  PublishItem* pitem = (PublishItem*)ui->listPublish->item(row);
  if (nullptr == pitem)
    return;

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
CDlgConnSettingsRawMqtt::onDeletePublish(void)
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
CDlgConnSettingsRawMqtt::onSubscribeContextMenu(const QPoint& pos)
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
CDlgConnSettingsRawMqtt::onPublishContextMenu(const QPoint& pos)
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


///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgConnSettingsRawMqtt::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/connections?id=canal";
  QDesktopServices::openUrl(QUrl(link));
}