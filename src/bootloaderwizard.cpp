// bootloaderwizard.cpp
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include "filedownloader.h"
#include "vscpworks.h"

#include "cdlgconnsettingscanal.h"
#include "cdlgconnsettingslocal.h"
#include "cdlgconnsettingsmqtt.h"
#include "cdlgconnsettingsmulticast.h"
#include "cdlgconnsettingsrawcan.h"
#include "cdlgconnsettingsrawmqtt.h"
#include "cdlgconnsettingsrest.h"
#include "cdlgconnsettingssocketcan.h"
#include "cdlgconnsettingstcpip.h"
#include "cdlgconnsettingsudp.h"
#include "cdlgconnsettingsws1.h"
#include "cdlgconnsettingsws2.h"
#include "version.h"
#include "vscp_client_base.h"

#include "bootloaderwizard.h"

#include <list>
#include <string>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CBootLoadWizard::CBootLoadWizard(QWidget* parent, QJsonObject* pconn)
  : QWizard(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // No connection set yet
  m_vscpConnType = CVscpClient::connType::NONE;
  m_vscpClient   = nullptr;

  if (nullptr == pconn) {
    spdlog::error(std::string(tr("Bootloader wizard - pconn is null").toStdString()));
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Communication configuration data is missing"),
                             QMessageBox::Ok);
    return;
  }

  // Save session configuration
  m_connObject = *pconn;

  // Must have a type
  if (m_connObject["type"].isNull()) {
    spdlog::error(std::string(tr("Type is not defined in JSON data").toStdString()));
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("Can't open node configuration  window - The "
                                "connection type is unknown"),
                             QMessageBox::Ok);
    return;
  }

  m_vscpConnType = static_cast<CVscpClient::connType>(m_connObject["type"].toInt());

  QString str = tr("VSCP Works Bootloader Wizard - ");
  str += pworks->getConnectionName(m_vscpConnType);
  str += tr(" - ");
  if (!m_connObject["name"].isNull()) {
    str += m_connObject["name"].toString();
  }
  else {
    str += tr("Unknown");
  }

  setWindowTitle(str);

  setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
  setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));
  setPixmap(QWizard::LogoPixmap, QPixmap(":vscp-logo-48x48"));
  setPixmap(QWizard::WatermarkPixmap, QPixmap(":canal_watermark"));

  setWindowTitle(tr("VSCP Works Bootloader Wizard"));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CBootLoadWizard::~CBootLoadWizard(void)
{
  int rv;

  // Clean up client
  if (nullptr != m_vscpClient) {
    if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->disconnect())) {
      spdlog::error("Failed to load communication interface. rv={}", rv);
    }
    delete m_vscpClient;
  }
}

///////////////////////////////////////////////////////////////////////////////
// initBootLoaderWizard
//

int
CBootLoadWizard::initBootLoaderWizard(void)
{
  int rv;

  QJsonDocument doc(m_connObject);
  QString strJson(doc.toJson(QJsonDocument::Compact));

  switch (m_vscpConnType) {

    case CVscpClient::connType::TCPIP:

      m_vscpClient = new vscpClientTcp();
      if (nullptr == m_vscpClient) {
        spdlog::error("Can't start bootloader wizard as there is no vscpClient specified.");
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Can't start bootloader wizard as there is no vscpClient specified."),
                              QMessageBox::Ok);
        return VSCP_ERROR_PARAMETER;
      }

      // Initialize communication parameters
      m_vscpClient->initFromJson(strJson.toStdString());
      // m_vscpClient->setCallback(eventReceived, this);

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(tr("Bootloader wizard: Unable to connect to remote host.").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
        QApplication::restoreOverrideCursor();
        delete m_vscpClient;
        return VSCP_ERROR_CONNECTION;
      }
      else {
        spdlog::info(std::string(tr("Bootloader wizard: Successful connect to remote client.").toStdString()));
      }

      QApplication::restoreOverrideCursor();

      setPage(Page_Intro, createIntroPage());
      setPage(Page_Nickname, createNicknamePage());
      setPage(Page_Interface, createInterfacePage());
      setPage(Page_Guid, createGuidPage());
      setPage(Page_PreDevice, createPreDevicePage());
      setPage(Page_Device, createDevicePage());
      break;

    case CVscpClient::connType::CANAL:

      m_vscpClient = new vscpClientCanal();
      if (nullptr == m_vscpClient) {
        spdlog::error("Can't start bootloader wizard as there is no vscpClient specified.");
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Can't start bootloader wizard as there is no vscpClient specified."),
                              QMessageBox::Ok);
        return VSCP_ERROR_PARAMETER;
      }

      // Initialize communication parameters
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(
          this,
          tr(APPNAME),
          tr("Failed to initialize CANAL driver. See log for more details."));
        return VSCP_ERROR_PARAMETER;
      }

      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();

      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str = tr("Bootloader wizard: Unable to connect to the CANAL driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the CANAL "
                                    "driver (see log for more info)."),
                                 QMessageBox::Ok);
        delete m_vscpClient;
        QApplication::restoreOverrideCursor();
        return VSCP_ERROR_CONNECTION;
      }
      else {
        spdlog::info(std::string(tr("Bootloader wizard: Successful connected to the CANAL driver.").toStdString()));
      }
      QApplication::restoreOverrideCursor();

      setPage(Page_Intro, createIntroPage());
      setPage(Page_Nickname, createNicknamePage());
      setPage(Page_PreDevice, createPreDevicePage());
      setPage(Page_Device, createDevicePage());
      break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN:
      m_vscpClient = new vscpClientSocketCan();
      if (nullptr == m_vscpClient) {
        spdlog::error("Can't start bootloader wizard as there is no vscpClient specified.");
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Can't start bootloader wizard as there is no vscpClient specified."),
                              QMessageBox::Ok);
        return VSCP_ERROR_PARAMETER;
      }

      // Initialize communication parameters
      if (!m_vscpClient->initFromJson(strJson.toStdString())) {
        // Failed to initialize
        QMessageBox::warning(this,
                             tr(""),
                             tr("Failed to initialize SOCKETCAN driver. See "
                                "log for more details."));
        return VSCP_ERROR_PARAMETER;
      }
      QApplication::setOverrideCursor(Qt::WaitCursor);
      QApplication::processEvents();
      if (VSCP_ERROR_SUCCESS != (rv = m_vscpClient->connect())) {
        QString str = tr("Bootloader wizard: Unable to connect to the SOCKETCAN driver. rv=");
        str += rv;
        spdlog::error(str.toStdString());
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to SOCKETCAN "
                                    "(see log for more info)."),
                                 QMessageBox::Ok);
        delete m_vscpClient;
        QApplication::restoreOverrideCursor();
        return VSCP_ERROR_CONNECTION;
      }
      else {
        spdlog::info(std::string(tr("Bootloader wizard: Successful connected to SOCKETCAN.").toStdString()));
      }
      QApplication::restoreOverrideCursor();

      setPage(Page_Intro, createIntroPage());
      setPage(Page_Nickname, createNicknamePage());
      setPage(Page_PreDevice, createPreDevicePage());
      setPage(Page_Device, createDevicePage());
      break;
#endif

    case CVscpClient::connType::MQTT:
      m_vscpClient = new vscpClientMqtt();
      if (nullptr == m_vscpClient) {
        spdlog::error("Can't start bootloader wizard as there is no vscpClient specified.");
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Can't start bootloader wizard as there is no vscpClient specified."),
                              QMessageBox::Ok);
        return VSCP_ERROR_PARAMETER;
      }

      // Initialize communication parameters
      m_vscpClient->initFromJson(strJson.toStdString());

      if (VSCP_ERROR_SUCCESS != m_vscpClient->connect()) {
        spdlog::error(std::string(tr("Bootloader wizard: Unable to connect to remote host").toStdString()));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to open a connection to the remote "
                                    "host (see log for more info)."),
                                 QMessageBox::Ok);
        delete m_vscpClient;
        QApplication::restoreOverrideCursor();
        return VSCP_ERROR_CONNECTION;
      }
      else {
        spdlog::info(std::string(tr("Bootloader wizard: Successful connect to remote host").toStdString()));
      }

      setPage(Page_Intro, createIntroPage());
      setPage(Page_Guid, createGuidPage());
      setPage(Page_PreDevice, createPreDevicePage());
      setPage(Page_Device, createDevicePage());
      break;

    defult:
      spdlog::error("No bootloader wizard (yet!?) for this type of "
                    "communication interface - {}",
                    static_cast<int>(m_vscpConnType));
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Sorry, no bootloader wizard (yet!?) for this type of "
                                  "communication interface."),
                               QMessageBox::Ok);
      return VSCP_ERROR_PARAMETER;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// createIntroPage
//

QWizardPage*
CBootLoadWizard::createIntroPage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Introduction");
  // page->setSubTitle("test");

  QLabel* label = new QLabel(tr("The VSCP Works bootloader wizard will now walk you "
                                "through the steps needed to update the firmware of "
                                "the (remote) device.\n\n"
                                "The device you need to update can be located on "
                                "an interface off your local machine or be "
                                "attached to an interface of a remote machine.\n\n"
                                "Please do not turn off the power of the remote device "
                                "until the firmware update process is complete.\n\n"));
  label->setWordWrap(true);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  page->setLayout(layout);

  return page;
}

///////////////////////////////////////////////////////////////////////////////
// createNicknamePage
//

QWizardPage*
CBootLoadWizard::createNicknamePage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Set node id (nickname) for device");

  QLabel* label = new QLabel(tr("The device you want to update is "
                                "identified by a node id (nickname). A "
                                "node id is a 8-bit number between 1-254 (0xfe).\n\n"
                                "Id=254 is used for a device that is already set into "
                                "bootloader mode, typically this is used by a device that never "
                                "have been loaded with firmware previously.\n\n"
                                "For 16-bit node id's use full GUID with node id set in the two"
                                "LSB's\n\n"));
  label->setWordWrap(true);

  QSpinBox* pNodeIdSpinBox = new QSpinBox();
  pNodeIdSpinBox->setMinimum(1);
  pNodeIdSpinBox->setMaximum(253);
  pNodeIdSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pNodeIdSpinBox->setMaximumWidth(100);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  QFormLayout* layoutForm = new QFormLayout;
  layoutForm->addRow(tr("node id: "), pNodeIdSpinBox);
  layout->addLayout(layoutForm);
  page->setLayout(layout);

  return page;
}

///////////////////////////////////////////////////////////////////////////////
// createInterfacePage
//

QWizardPage*
CBootLoadWizard::createInterfacePage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Set interface");

  QLabel* label = new QLabel(tr("The communication method you have selected may be able to connect to several "
                                "interfaces. Interfaces are defined by GUID's which "
                                "have zeros in the two lowest bytes. This is just a method "
                                "used to talk to level I devicess over a level II connection\n\n"));
  label->setWordWrap(true);

  QLineEdit* pInterfaces = new QLineEdit("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  pInterfaces->setSizePolicy(QSizePolicy::Policy(QSizePolicy::GrowFlag | QSizePolicy::ShrinkFlag | QSizePolicy::ExpandFlag), QSizePolicy::Fixed);
  pInterfaces->setToolTip(tr("Set interface to use or set interface to all zeros "
                             "to not use any interface."));

  QPushButton* pSelect = new QPushButton("...");
  pSelect->setToolTip(tr("Set interface from saved GUID"));
  pSelect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pSelect->setMaximumWidth(100);

  QPushButton* pAllZero = new QPushButton("00:00:00...");
  pSelect->setToolTip(tr("Set interface to all zeros"));
  pAllZero->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pAllZero->setMaximumWidth(100);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);

  QHBoxLayout* layoutH = new QHBoxLayout;
  layoutH->addWidget(pInterfaces);
  layoutH->addWidget(pSelect);
  layoutH->addWidget(pAllZero);
  layout->addLayout(layoutH);

  page->setLayout(layout);

  return page;
}

///////////////////////////////////////////////////////////////////////////////
// createGuidPage
//

QWizardPage*
CBootLoadWizard::createGuidPage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Set GUID");

  QLabel* label = new QLabel(tr("The device you want to update is indentified by a full GUID. "
                                "Please set or select this GUID here. \n\n"
                                "For 16-bit nickname id's use full GUID with nickname set in the two"
                                "LSB's\n\n"));
  label->setWordWrap(true);

  QLineEdit* pInterfaces = new QLineEdit("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  pInterfaces->setSizePolicy(QSizePolicy::Policy(QSizePolicy::GrowFlag | QSizePolicy::ShrinkFlag | QSizePolicy::ExpandFlag), QSizePolicy::Fixed);
  pInterfaces->setToolTip(tr("GUID for the device you want to update firmware for."));

  QPushButton* pSelect = new QPushButton("...");
  pSelect->setToolTip(tr("Set interface from saved GUID"));
  pSelect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pSelect->setMaximumWidth(100);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);

  QHBoxLayout* layoutH = new QHBoxLayout;
  layoutH->addWidget(pInterfaces);
  layoutH->addWidget(pSelect);
  layout->addLayout(layoutH);

  page->setLayout(layout);

  return page;
}

///////////////////////////////////////////////////////////////////////////////
// createPreDevicePage
//

QWizardPage*
CBootLoadWizard::createPreDevicePage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Ready to fetch boot loader information");

  QLabel* label = new QLabel(tr("When you press <next> the selected device "
                                "will be connected to and boot loader information will be "
                                "fetched from the device.\n\n"));
  label->setWordWrap(true);

  QProgressBar* pbar = new QProgressBar();
  pbar->setMinimum(0);
  pbar->setMaximum(100);
  pbar->setValue(0);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  layout->addWidget(pbar);
  page->setLayout(layout);

  return page;
}

///////////////////////////////////////////////////////////////////////////////
// createDevicePage
//

QWizardPage*
CBootLoadWizard::createDevicePage(void)
{
  QWizardPage* page = new QWizardPage;
  page->setTitle("Ready to update firmware");

  QLabel* label = new QLabel("We are ready to update firmware.");
  label->setWordWrap(true);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  page->setLayout(layout);

  return page;
}
