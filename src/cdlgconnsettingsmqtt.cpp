// cdlgconnsettingsmqtt.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#include <vscphelper.h>

#include "vscpworks.h"

#include "cdlgconnsettingsmqtt.h"
#include "ui_cdlgconnsettingsmqtt.h"
#include "cdlgmqttpublish.h"

#include "cdlgtls.h"

#include <QDebug>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QMenu>

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

SubscribeItem::SubscribeItem(const QString &topic) :
    QListWidgetItem(topic)
{
    m_topic = topic;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

SubscribeItem::~SubscribeItem()
{

}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor
//

PublishItem::PublishItem(const QString &topic, int qos, bool bretain) :
    QListWidgetItem(vscp_str_format("%s - qos=%d retain=%s",
                                        topic.toStdString().c_str(),
                                        qos, 
                                        bretain ? "true" : "false" ).c_str())
{
    m_topic = topic;
    m_qos = qos;
    m_bRetain = bretain;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

PublishItem::~PublishItem()
{

}

///////////////////////////////////////////////////////////////////////////////
// setTopic
//

void PublishItem::setTopic(const QString& topic) 
{ 
    m_topic = topic; 
    setText( vscp_str_format("%s - qos=%d retain=%s",
                                topic.toStdString().c_str(),
                                getQos(), 
                                getRetain() ? "true" : "false" ).c_str() ); 
};


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsMqtt::CDlgConnSettingsMqtt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsMqtt)
{
    ui->setupUi(this);

    // Set defaults
    m_bTLS = false;
    //setConnectionTimeout(TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS);
    //setResponseTimeout(TCPIP_DEFAULT_RESPONSE_TIMEOUT);
    setBroker("localhost");
    setPort(1883);
    setUser("admin");
    setPassword("secret");

    connect(ui->btnTLS, &QPushButton::clicked, this, &CDlgConnSettingsMqtt::onTLSSettings );
    connect(ui->btnAddSubscription, &QPushButton::clicked, this, &CDlgConnSettingsMqtt::onAddSubscription );
    connect(ui->btnAddPublish, &QPushButton::clicked, this, &CDlgConnSettingsMqtt::onAddPublish );
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &CDlgConnSettingsMqtt::onTestConnection );

    // Help button
    connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &CDlgConnSettingsMqtt::onGetHelp );

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listSubscribe,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgConnSettingsMqtt::onSubscribeContextMenu);

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listPublish,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgConnSettingsMqtt::onPublishContextMenu);
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

void CDlgConnSettingsMqtt::setInitialFocus(void)
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

QString CDlgConnSettingsMqtt::getName(void)
{
    return (ui->editDescription->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsMqtt::setName(const QString& str)
{
    ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getBroker
//

QString CDlgConnSettingsMqtt::getBroker(void)
{
    return (ui->editHost->text()); 
}

///////////////////////////////////////////////////////////////////////////////
// setBroker
//

void CDlgConnSettingsMqtt::setBroker(const QString& str)
{
    ui->editHost->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPort
//

short 
CDlgConnSettingsMqtt::getPort(void)
{
    short port = vscp_readStringValue(ui->editPort->text().toStdString());
    return port; 
}

///////////////////////////////////////////////////////////////////////////////
// setPort
//

void 
CDlgConnSettingsMqtt::setPort(short port)
{
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString str = pworks->decimalToStringInBase(port);
    ui->editPort->setText(str);
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
// getConnectionTimeout
//

uint32_t CDlgConnSettingsMqtt::getConnectionTimeout(void)
{
    return m_client.getConnectionTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void CDlgConnSettingsMqtt::setConnectionTimeout(uint32_t timeout)
{
    m_client.setConnectionTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t CDlgConnSettingsMqtt::getResponseTimeout(void)
{
    return m_client.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void CDlgConnSettingsMqtt::setResponseTimeout(uint32_t timeout)
{   
    vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
    QString str = pworks->decimalToStringInBase(timeout);
    ui->editKeepAlive->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getKeepAlive
//

uint32_t CDlgConnSettingsMqtt::getKeepAlive(void)
{
    return vscp_readStringValue(ui->editKeepAlive->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setKeepAlive
//

void CDlgConnSettingsMqtt::setKeepAlive(uint32_t timeout)
{
    m_client.setResponseTimeout(timeout);
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

QJsonObject CDlgConnSettingsMqtt::getJson(void)
{
    m_jsonConfig["type"] = static_cast<int>(CVscpClient::connType::MQTT);
    m_jsonConfig["name"] = getName();
    m_jsonConfig["host"] = getBroker();
    m_jsonConfig["port"] = getPort();
    m_jsonConfig["user"] = getUser();
    m_jsonConfig["password"] = getPassword();
    m_jsonConfig["connection-timeout"] = (int)getConnectionTimeout();
    m_jsonConfig["response-timeout"] = (int)getResponseTimeout();
    m_jsonConfig["keepalive"] = (int)getKeepAlive();
    m_jsonConfig["extended-security"] = isExtendedSecurityEnabled();

    m_jsonConfig["btls"] = isTLSEnabled();
    m_jsonConfig["bverifypeer"] = isVerifyPeerEnabled();
    m_jsonConfig["cafile"] = getCaFile();
    m_jsonConfig["capath"] = getCaPath();
    m_jsonConfig["certfile"] = getCertFile();
    m_jsonConfig["keyfile"] = getKeyFile();
    m_jsonConfig["pwkeyfile"] = getPwKeyFile();

    // Save all subscriptions
    QJsonArray subscriptionArray;
    for ( int i=0; i<ui->listSubscribe->count(); i++) {        
        QJsonObject obj;
        SubscribeItem *pitem = (SubscribeItem * )ui->listSubscribe->item(i);
        obj["topic"] = pitem->getTopic();
        subscriptionArray.append(obj);
    }
    m_jsonConfig["subscriptions"] = subscriptionArray;


    // Save all publishing
    QJsonArray publishingArray;
    for ( int i=0; i<ui->listPublish->count(); i++) {        
        QJsonObject obj;
        PublishItem *pitem = (PublishItem * )ui->listPublish->item(i);
        obj["topic"] = pitem->getTopic();
        obj["qos"] = pitem->getQos();
        obj["bretain"] = pitem->getRetain();
        publishingArray.append(obj);
    }
    m_jsonConfig["publishing"] = publishingArray;

    return m_jsonConfig; 
}


///////////////////////////////////////////////////////////////////////////////
// setJson
//

void CDlgConnSettingsMqtt::setJson(const QJsonObject* pobj)
{
    m_jsonConfig = *pobj;    

    if (!m_jsonConfig["name"].isNull()) setName(m_jsonConfig["name"].toString());
    if (!m_jsonConfig["host"].isNull()) setBroker(m_jsonConfig["host"].toString());
    if (!m_jsonConfig["port"].isNull()) setPort((short)m_jsonConfig["port"].toInt());
    if (!m_jsonConfig["user"].isNull()) setUser(m_jsonConfig["user"].toString());
    if (!m_jsonConfig["password"].isNull()) setPassword(m_jsonConfig["password"].toString());   
    if (!m_jsonConfig["connection-timeout"].isNull()) setConnectionTimeout((uint32_t)m_jsonConfig["connection-timeout"].toInt()); 
    if (!m_jsonConfig["response-timeout"].isNull()) setResponseTimeout((uint32_t)m_jsonConfig["response-timeout"].toInt());  
    if (!m_jsonConfig["keepalive"].isNull()) setKeepAlive((uint32_t)m_jsonConfig["keepalive"].toInt());
    if (!m_jsonConfig["extended-security"].isNull()) enableExtendedSecurity(m_jsonConfig["extended-security"].toBool());

    if (!m_jsonConfig["btls"].isNull()) enableTLS((short)m_jsonConfig["btls"].toBool());
    if (!m_jsonConfig["bverifypeer"].isNull()) enableVerifyPeer(m_jsonConfig["bverifypeer"].toBool());
    if (!m_jsonConfig["cafile"].isNull()) setCaFile(m_jsonConfig["cafile"].toString());
    if (!m_jsonConfig["capath"].isNull()) setCaPath(m_jsonConfig["capath"].toString());
    if (!m_jsonConfig["certfile"].isNull()) setCertFile(m_jsonConfig["certfile"].toString());
    if (!m_jsonConfig["keyfile"].isNull()) setKeyFile(m_jsonConfig["keyfile"].toString());
    if (!m_jsonConfig["pwkeyfile"].isNull()) setPwKeyFile(m_jsonConfig["pwkeyfile"].toString());

    // Subscriptions
    if (m_jsonConfig["subscriptions"].isArray()) {
        QJsonArray interfacesArray = m_jsonConfig["subscriptions"].toArray();  

        for (auto v : interfacesArray) {

            QJsonObject item = v.toObject();
            if (!item["topic"].isNull()) {
                SubscribeItem *pitem = new SubscribeItem(item["topic"].toString());
                ui->listSubscribe->addItem(pitem);                
            }

        }

        ui->listSubscribe->sortItems(); 
    }

    // Publish
    if (m_jsonConfig["publishing"].isArray()) {
        QJsonArray interfacesArray = m_jsonConfig["publishing"].toArray();  

        for (auto v : interfacesArray) {
            
            QJsonObject item = v.toObject();
            if (!item["topic"].isNull() && !item["qos"].isNull() && !item["bretain"].isNull()) {
                PublishItem *pitem = new PublishItem(item["topic"].toString(),
                                                        item["qos"].toInt(),
                                                        item["bretain"].toBool() );
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

    // Initialize host connection
    if ( VSCP_ERROR_SUCCESS != m_client.init(getBroker().toStdString().c_str(),
                                                getPort(),
                                                "#",
                                                "test",
                                                "",
                                                getUser().toStdString().c_str(),
                                                getPassword().toStdString().c_str())) {
        QApplication::restoreOverrideCursor();                                                        
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to initialize MQTT client"));        
        return;                                                
    }

    // Connect to remote host
    if ( VSCP_ERROR_SUCCESS != m_client.connect() ) {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to connect to remote MQTT broker"));
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
    if ( VSCP_ERROR_SUCCESS != m_client.disconnect() ) {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, tr("vscpworks+"), tr("Failed to disconnect from remote MQTT broker"));        
        return;
    }    

    QApplication::restoreOverrideCursor();

    QString msg = tr("Connection test was successful");
    msg += "\n";
    //msg += strVersion;
    QMessageBox::information(this, tr("vscpworks+"), msg );
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

    if (QDialog::Accepted == dlg.exec() ) {
        m_bTLS = dlg.isTLSEnabled();
        m_bVerifyPeer = dlg.isVerifyPeerEnabled();
        m_cafile = dlg.getCaFile();
        m_capath = dlg.getCaPath();
        m_certfile = dlg.getCertFile();
        m_keyfile = dlg.getKeyFile();
        m_pwkeyfile = dlg.getPwKeyFile();
    }
}

///////////////////////////////////////////////////////////////////////////////
// onAddSubscription
//

void 
CDlgConnSettingsMqtt::onAddSubscription(void)
{
    bool ok;
    QString topic = QInputDialog::getText(this, 
                                            tr("vscpworks+"),
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
CDlgConnSettingsMqtt::onEditSubscription(void)
{
    bool ok;

    // Get selected row
    int row = ui->listSubscribe->currentRow();
    if (-1 == row) return;

    SubscribeItem *pitem = (SubscribeItem * )ui->listSubscribe->item(row);
    if (nullptr == pitem) return;

    QString topic = QInputDialog::getText(this, 
                                            tr("vscpworks+"),
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
CDlgConnSettingsMqtt::onCloneSubscription(void)
{
    // Get selected row
    int row = ui->listSubscribe->currentRow();
    if (-1 == row) return;

    SubscribeItem *pitem = (SubscribeItem * )ui->listSubscribe->item(row);
    if (nullptr == pitem) return;

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
    if (-1 == row) return;

    SubscribeItem *pitem = (SubscribeItem * )ui->listSubscribe->item(row);
    if (nullptr == pitem) return;

    if ( QMessageBox::Yes == QMessageBox::question(this, tr("vscpworks+"), 
                                                    tr("Are you sure the subscription topic should be deleted?"),
                                                    QMessageBox::Yes|QMessageBox::No) ) {                
        
        ui->listSubscribe->takeItem(row);
        //ui->listSubscribe->removeItemWidget();
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

    if ( QDialog::Accepted == dlg.exec() ) {
        qDebug() << dlg.getQos();
        qDebug() << dlg.getRetain();
        ui->listPublish->addItem(new PublishItem(dlg.getTopic(),
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
    if (-1 == row) return;

    PublishItem *pitem = (PublishItem * )ui->listPublish->item(row);
    if (nullptr == pitem) return;

    dlg.setTopic(pitem->getTopic());
    dlg.setQos(pitem->getQos());
    dlg.setRetain(pitem->getRetain());

    if ( QDialog::Accepted == dlg.exec() ) {
        pitem->setQos(dlg.getQos());
        pitem->setRetain(dlg.getRetain());
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
    if (-1 == row) return;

    PublishItem *pitem = (PublishItem * )ui->listPublish->item(row);
    if (nullptr == pitem) return;

    ui->listPublish->addItem(
      new PublishItem(pitem->getTopic(), pitem->getQos(), pitem->getRetain()));

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
    if (-1 == row) return;

    PublishItem *pitem = (PublishItem * )ui->listPublish->item(row);
    if (nullptr == pitem) return;

    if ( QMessageBox::Yes == QMessageBox::question(this, tr("vscpworks+"), 
                                                    tr("Are you sure the publish topic should be deleted?"),
                                                    QMessageBox::Yes|QMessageBox::No) ) {                
        
        ui->listPublish->takeItem(row);
        delete pitem;
    }
}

///////////////////////////////////////////////////////////////////////////////
// onSubscribeItemClicked
//

void CDlgConnSettingsMqtt::onSubscribeContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("New subscription"), this, SLOT(onAddSubscription()));
    menu->addAction(QString("Edit subscription"), this, SLOT(onEditSubscription()));
    menu->addAction(QString("Clone subscription"), this, SLOT(onCloneSubscription()));
    menu->addAction(QString("delete subscription"), this, SLOT(onDeleteSubscription()));

    menu->popup(ui->listSubscribe->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// onPublishItemClicked
//

void CDlgConnSettingsMqtt::onPublishContextMenu(const QPoint& pos)
{
    QMenu *menu = new QMenu(this);

    menu->addAction(QString("New publishing"), this, SLOT(onAddPublish()));
    menu->addAction(QString("Edit publishing"), this, SLOT(onEditPublish()));
    menu->addAction(QString("Clone publishing"), this, SLOT(onClonePublish()));
    menu->addAction(QString("delete publishing"), this, SLOT(onDeletePublish()));

    menu->popup(ui->listPublish->viewport()->mapToGlobal(pos));
}