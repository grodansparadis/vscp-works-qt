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

#include <mdf.h>
#include <register.h>
#include <vscp.h>
#include <vscp-client-base.h>

#include "cdlgselectbootalgorithm.h"
#include "filedownloader.h"
#include "vscpworks.h"

#include "cdlgbootfirmware.h"
#include "cdlgconnsettingscanal.h"
#include "cdlgconnsettingsmqtt.h"
#include "cdlgconnsettingsmulticast.h"
#include "cdlgconnsettingsrawcan.h"
#include "cdlgconnsettingsrawmqtt.h"
#include "cdlgconnsettingssocketcan.h"
#include "cdlgconnsettingstcpip.h"
#include "cdlgconnsettingsudp.h"
#include "cdlgconnsettingsws1.h"
#include "cdlgconnsettingsws2.h"
#include "cdlgknownguid.h"
#include "version.h"
#include "vscp-bootdevice-pic1.h"

#include <curl/curl.h>

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

CWizardPageNickname::CWizardPageNickname(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageNickname::~CWizardPageNickname(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageNickname::initializePage(void)
{
  setTitle("Set node id (nickname) for device");

  QLabel* label = new QLabel(tr("The device you want to update is "
                                "identified by a node id (nickname). A "
                                "node id is a 8-bit number between 1-254 (0xfe).\n\n"
                                "Id=254 is used for a device that is already set into "
                                "bootloader mode, typically this is used by a device that never "
                                "have been loaded with firmware previously.\n\n"
                                "For 16-bit node id's use full GUID with node id set in the two"
                                "LSB's\n\n"
                                "You can use decimal, hexadecimal(0x), octal(0o or binary(0b) numbers"
                                "to set node id\n\n"));
  label->setWordWrap(true);

  QLineEdit* pNodeId        = new QLineEdit("0x01");
  QCheckBox* pSetInBootMode = new QCheckBox("Remote device is already in bootmode");
  pNodeId->setMaximumWidth(100);
  // pNodeIdSpinBox->setMinimum(1);
  // pNodeIdSpinBox->setMaximum(255); // 0xfe = 254 is uninitialized device
  // pNodeIdSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // pNodeIdSpinBox->setMaximumWidth(100);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  QFormLayout* layoutForm = new QFormLayout;
  layoutForm->addRow("", pSetInBootMode);
  layoutForm->addRow(tr("node id: "), pNodeId);
  layout->addLayout(layoutForm);
  layout->addWidget(pSetInBootMode);
  setLayout(layout);

  registerField("boot.nickname", pNodeId);
  registerField("boot.activebootmode", pSetInBootMode);

  registerField("boot.firmware.devicename", new QLineEdit("unknown"));

  // Current GUID
  registerField("boot.firmware.guid", new QLineEdit("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));

  // Path to downloaded MDF
  registerField("boot.firmware.mdf", new QLineEdit(""));

  // Version of current firmware
  registerField("boot.firmware.version.major", new QSpinBox(0));
  registerField("boot.firmware.version.minor", new QSpinBox(0));
  registerField("boot.firmware.version.sub", new QSpinBox(0));

  // Boot loader algorith device expect
  registerField("boot.firmware.bootloader.algorithm", new QSpinBox(0));

  // Firmware device code of current firmware
  registerField("boot.firmware.device.code", new QSpinBox(0));

  // Path to local firmware file
  registerField("boot.firmware.path", new QLineEdit(""));
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageNickname::cleanupPage(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageNickname::isComplete(void) const
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageNickname::validatePage(void)
{
  QString path;

  if (field("boot.activebootmode").toBool()) {

    // Select bootload algorithm
    CDlgSelectBootAlgorithm dlg;
    if (QDialog::Accepted == dlg.exec()) {
      setField("boot.firmware.bootloader.algorithm", dlg.getBootAlgorithm());
    }

    path = QFileDialog::getOpenFileName(this,
                                        tr("Set local firmware file"),
                                        "",
                                        tr("Firmware (*.hex *.mcs *.int *.ihex *ihe *.ihx);;Text (*.txt);;All (*.*)"));
    setField("boot.firmware.path", path);

    // If local file - Verify that the firmware file exists
    if (!vscp_fileExists(path.toStdString())) {
      spdlog::debug("Firmware file = {0}", path.toStdString());
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("Downloaded firmware file is missing on disk"),
                           QMessageBox::Ok);
      spdlog::error("Firmware download file not available on disk: {}", path.toStdString());
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// nextId
//

int
CWizardPageNickname::nextId(void) const
{
  /*
    If already in boot mode we skip MDF download
    and go directly to flash loading
  */
  if (field("boot.activebootmode").toBool()) {
    return CBootLoadWizard::Page_Flash;
  }

  // This should not happen... :)
  if (m_vscpClient->isFullLevel2()) {
    return CBootLoadWizard::Page_Interface;
  }

  return CBootLoadWizard::Page_LoadMdf;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CWizardPageInterface::CWizardPageInterface(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageInterface::~CWizardPageInterface(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageInterface::initializePage(void)
{
  setTitle("Set interface");

  QLabel* label = new QLabel(tr("The communication method you have selected may be able to connect to several "
                                "interfaces. Interfaces are defined by GUID's which "
                                "have zeros in the two lowest bytes. This is just a method "
                                "used to talk to level I devicess over a level II connection\n\n"));
  label->setWordWrap(true);

  m_pInterface = new QLineEdit("");
  m_pInterface->setSizePolicy(QSizePolicy::Policy(QSizePolicy::GrowFlag | QSizePolicy::ShrinkFlag | QSizePolicy::ExpandFlag), QSizePolicy::Fixed);
  m_pInterface->setToolTip(tr("Set the interface to all zeros to not use any interface. "));

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
  layoutH->addWidget(m_pInterface);
  layoutH->addWidget(pSelect);
  layoutH->addWidget(pAllZero);
  layout->addLayout(layoutH);

  setLayout(layout);

  registerField("boot.interface*", m_pInterface);

  connect(pAllZero, &QPushButton::clicked, this, &CWizardPageInterface::setAllZeros);
  connect(pSelect, &QPushButton::clicked, this, &CWizardPageInterface::openKnownGuidDlg);
}

///////////////////////////////////////////////////////////////////////////////
// setAllZeros
//

void
CWizardPageInterface::setAllZeros(void)
{
  m_pInterface->setText("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
}

///////////////////////////////////////////////////////////////////////////////
// openKnownGuidDlg
//

void
CWizardPageInterface::openKnownGuidDlg(void)
{
  CDlgKnownGuid dlg;

  dlg.enableAccept(true); // Doubleclick accepts GUID
  dlg.setInterfaceShow(true);

  if (QDialog::Accepted == dlg.exec()) {
    cguid guid;
    if (dlg.getSelectedGuid(guid)) {
      m_pInterface->setText(guid.toString().c_str());
    }
    else {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to get interface"),
                               QMessageBox::Ok);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageInterface::cleanupPage(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageInterface::isComplete(void) const
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageInterface::validatePage(void)
{
  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CWizardPageGuid::CWizardPageGuid(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageGuid::~CWizardPageGuid(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageGuid::initializePage(void)
{
  setTitle("Set GUID");

  QLabel* label = new QLabel(tr("The device you want to update is indentified by a full GUID. "
                                "Please set or select this GUID here. \n\n"
                                "For 16-bit nickname id's use full GUID with nickname set in the two"
                                "LSB's\n\n"));
  label->setWordWrap(true);

  m_pGuid = new QLineEdit("");
  m_pGuid->setSizePolicy(QSizePolicy::Policy(QSizePolicy::GrowFlag | QSizePolicy::ShrinkFlag | QSizePolicy::ExpandFlag), QSizePolicy::Fixed);
  m_pGuid->setToolTip(tr("GUID for the device you want to update firmware for."));

  QPushButton* pSelect = new QPushButton("...");
  pSelect->setToolTip(tr("Set interface from saved GUID"));
  pSelect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pSelect->setMaximumWidth(100);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);

  QHBoxLayout* layoutH = new QHBoxLayout;
  layoutH->addWidget(m_pGuid);
  layoutH->addWidget(pSelect);
  layout->addLayout(layoutH);

  setLayout(layout);

  registerField("boot.guid*", m_pGuid);

  connect(pSelect, &QPushButton::clicked, this, &CWizardPageGuid::openKnownGuidDlg);
}

void
CWizardPageGuid::openKnownGuidDlg(void)
{
  CDlgKnownGuid dlg;

  dlg.enableAccept(true); // Doubleclick accepts GUID
  dlg.setInterfaceShow(false);

  if (QDialog::Accepted == dlg.exec()) {
    cguid guid;
    if (dlg.getSelectedGuid(guid)) {
      m_pGuid->setText(guid.toString().c_str());
    }
    else {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Failed to get interface"),
                               QMessageBox::Ok);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageGuid::cleanupPage(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageGuid::isComplete(void) const
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageGuid::validatePage(void)
{
  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CWizardPageLoadMdf::CWizardPageLoadMdf(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageLoadMdf::~CWizardPageLoadMdf(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageLoadMdf::initializePage(void)
{
  setTitle("Ready to fetch boot loader information");

  QLabel* label = new QLabel(tr("When you press <next> the selected device "
                                "will be connected to and boot loader information will be "
                                "fetched from the device and the MDF will be downloaded and parsed\n\n"

                                "Alternativly if you select the 'use local MDF' you will be asked to"
                                "provide a path to a local MDF file\n\n"));
  label->setWordWrap(true);

  m_pchkLocalMdf = new QCheckBox("Use local MDF");

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  layout->addWidget(m_pchkLocalMdf);
  setLayout(layout);

  // connect(pbtnGetData, &QPushButton::clicked, this, &CWizardPagePreFlash::getDeviceInfo);
}

///////////////////////////////////////////////////////////////////////////////
// statusCallback
//

void
CWizardPageLoadMdf::statusCallback(int progress, const char* str)
{
  spdlog::info("Bootload wizard {0} {1}\n", progress, str);
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageLoadMdf::cleanupPage(void)
{
  m_pchkLocalMdf->setChecked(false);
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageLoadMdf::isComplete(void) const
{

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageLoadMdf::validatePage(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  switch (m_vscpClient->getType()) {

    case CVscpClient::connType::TCPIP:

      break;

    case CVscpClient::connType::CANAL: {
      int rv;
      CStandardRegisters stdregs;

      cguid node_guid;
      node_guid.clear();

      cguid interface_guid;
      interface_guid.clear();

      // QApplication::setOverrideCursor(Qt::WaitCursor);
      rv = stdregs.init(*m_vscpClient, node_guid, interface_guid);
      // QApplication::restoreOverrideCursor();
    } break;

#ifndef WIN32
    case CVscpClient::connType::SOCKETCAN: {
      CStandardRegisters stdregs;

      cguid node_guid;
      node_guid.clear();
      node_guid[15] = vscp_readStringValue(field("boot.nickname").toString().toStdString());
      spdlog::debug("nickname of GUID set to {}", node_guid[15]);

      cguid interface_guid;
      interface_guid.clear();

      QApplication::setOverrideCursor(Qt::WaitCursor);
      if (VSCP_ERROR_SUCCESS != (rv = stdregs.init(*m_vscpClient, node_guid, interface_guid))) {
        spdlog::error("Failed to read standard registers {}", rv);
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Failed to read standard registers "),
                             QMessageBox::Ok);
        return false;
      }
      QApplication::restoreOverrideCursor();

      setField("boot.firmware.guid", stdregs.getGUIDStr().c_str());

      // Version of current firmware
      setField("boot.firmware.version.major", stdregs.getFirmwareMajorVersion());
      setField("boot.firmware.version.minor", stdregs.getFirmwareMinorVersion());
      setField("boot.firmware.version.sub", stdregs.getFirmwareSubMinorVersion());

      // Boot loader algorith device expect
      setField("boot.firmware.bootloader.algorithm", stdregs.getBootloaderAlgorithm());

      // Firmware device code of current firmware
      setField("boot.firmware.device.code", stdregs.getFirmwareDeviceCode());

      // * * * Download MDF * * *

      CMDF mdf;

      if (m_pchkLocalMdf->isChecked()) {
        QString path = QFileDialog::getOpenFileName(this,
                                                    tr("Set local MDF file"),
                                                    "/home/jana",
                                                    tr("MDF Files (*.xml *.json *.mdf);;All (*.*)"));
        int rv       = mdf.parseMDF(path.toStdString());
        if (VSCP_ERROR_SUCCESS != rv) {
          // if (nullptr != statusCallback) {
          //   statusCallback(80, "Faild to parse MDF");
          // }
          spdlog::error("Failed to parse MDF {0} rv={1}", path.toStdString(), rv);
          return false;
        }
      }
      else {
        spdlog::trace("Download MDF");
        QApplication::setOverrideCursor(Qt::WaitCursor);

        using namespace std::placeholders;
        auto callback = std::bind(&CWizardPageLoadMdf::statusCallback, this, _1, _2);
        // lambda version for reference
        // auto cb = [this](auto a, auto b) { this->statusCallback(a, b); };
        QString path;
        if (VSCP_ERROR_SUCCESS != pworks->downloadMDF(stdregs, mdf, path, callback)) {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this,
                               tr(APPNAME),
                               tr("Failed to download and parse MDF"),
                               QMessageBox::Ok);
          return false;
        }
        setField("boot.firmware.mdf", path);
        QString str = mdf.getModuleName().c_str();
        str += " Model: ";
        str += mdf.getModuleModel().c_str();
        setField("boot.firmware.devicename", str);
      }
    } break;
#endif

    case CVscpClient::connType::MQTT:
      break;

    defult:
      spdlog::error("No bootloader wizard (yet!?) for this type of "
                    "communication interface - {}",
                    static_cast<int>(m_vscpClient->getType()));
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("Sorry, no bootloader wizard (yet!?) for this type of "
                                  "communication interface."),
                               QMessageBox::Ok);
      return false;
  }
  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CWizardPageFirmware::CWizardPageFirmware(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageFirmware::~CWizardPageFirmware(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageFirmware::initializePage(void)
{
  QString str;
  setTitle("Select firmare to upload to device");

  QLabel* label = new QLabel("Select the firmware you want to upload to the remote device. "
                             "You can use a local file or you can use one of the files listed "
                             "in the MDF of the device.\n\n"
                             "Check the device code of the firmware you select so it is equal to "
                             "the firmware you intend to upload.\n");
  label->setWordWrap(true);

  QLabel* labelDeviceName         = new QLabel("Device name: " + field("boot.firmware.devicename").toString());
  QLabel* labelGUID               = new QLabel("Current firmware GUID: " + field("boot.firmware.guid").toString());
  QLabel* labelVer                = new QLabel("Current firmware version: " +
                                field("boot.firmware.version.major").toString() + "." +
                                field("boot.firmware.version.minor").toString() + "." +
                                field("boot.firmware.version.sub").toString());
  QLabel* labelBootloadAlgorithm  = new QLabel("Current firmware bootload algorithm: " +
                                              field("boot.firmware.bootloader.algorithm").toString() + " - " +
                                              vscp_getBootLoaderDescription(field("boot.firmware.bootloader.algorithm").toInt()));
  QLabel* labelBootloadDeviceCode = new QLabel("Current firmware device code: " +
                                               field("boot.firmware.device.code").toString());
  m_chkLocalFile                  = new QCheckBox("Use local firmware file");
  m_btnSelectFirmware             = new QPushButton("Select firmware file");
  m_editFirmwareFile              = new QLineEdit("No firmware file selected");

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  layout->addWidget(labelDeviceName);
  layout->addWidget(labelGUID);
  layout->addWidget(labelVer);
  layout->addWidget(labelBootloadAlgorithm);
  layout->addWidget(labelBootloadDeviceCode);
  layout->addWidget(m_chkLocalFile);
  layout->addWidget(m_btnSelectFirmware);
  layout->addWidget(m_editFirmwareFile);
  setLayout(layout);

  registerField("boot.firmware.url", m_editFirmwareFile);
  // setField("boot.firmware.path", new QLineEdit(""));
  registerField("boot.firmware.targetcode", new QSpinBox(0));
  registerField("boot.firmware.format", new QLineEdit("INTELHEX"));
  registerField("boot.firmware.localfile", m_chkLocalFile);

  connect(m_btnSelectFirmware, &QPushButton::clicked, this, &CWizardPageFirmware::setFirmwareFile);
  connect(m_chkLocalFile, &QCheckBox::clicked, this, &CWizardPageFirmware::toggleLocalFile);

  int rv = m_mdf.parseMDF(field("boot.firmware.mdf").toString().toStdString());
  if (VSCP_ERROR_SUCCESS != rv) {
    spdlog::error("Failed to parse MDF {0} rv={1}", field("boot.firmware.mdf").toString().toStdString(), rv);
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setFirmwareFile
//

void
CWizardPageFirmware::setFirmwareFile(void)
{
  CDlgBootFirmware dlg;
  dlg.initDialogData(&m_mdf);
  if (QDialog::Accepted == dlg.exec()) {
    if (dlg.isLocalFile()) {
      setField("boot.firmware.path", dlg.getLocalFilePath());
      setField("boot.firmware.url", dlg.getLocalFilePath());
      setField("boot.firmware.targetcode", "0");
      setField("boot.firmware.format", dlg.getFileType());
    }
    else {
      CMDF_Firmware* pfirmware = dlg.getSelectedItem();
      setField("boot.firmware.url", pfirmware->getUrl().c_str());
      setField("boot.firmware.targetcode", pfirmware->getTargetCode());
      printf("%s\n", pfirmware->getFormat().c_str());
      setField("boot.firmware.format", QString(pfirmware->getFormat().c_str()));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// toggleLocalFile
//

void
CWizardPageFirmware::toggleLocalFile(bool checked)
{
  printf("%s\n", (checked ? "on" : "off"));
  if (checked) {
    m_btnSelectFirmware->setEnabled(false);
    m_editFirmwareFile->setEnabled(false);
  }
  else {
    m_btnSelectFirmware->setEnabled(true);
    m_editFirmwareFile->setEnabled(true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageFirmware::cleanupPage(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageFirmware::isComplete(void) const
{
  return true;
}

static size_t
write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  spdlog::trace("curl write_data");
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageFirmware::validatePage(void)
{
  if ("No firmware file selected" == m_editFirmwareFile->text()) {
    m_editFirmwareFile->clear();
    setField("boot.firmware.path", "");
  }

  /*!
    Open file selction dialog if local file is checked
    and filname is nill
  */
  if (m_chkLocalFile->isChecked() && !m_editFirmwareFile->text().trimmed().size()) {
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Set local firmware file"),
                                                "",
                                                tr("Firmware (*.hex *.mcs *.int *.ihex *ihe *.ihx);;Text (*.txt);;All (*.*)"));
    m_editFirmwareFile->setText(path);
    setField("boot.firmware.path", path);
  }

  // If local file - Verify that the firmware file exists
  QString path = field("boot.firmware.path").toString();
  if (path.size()) {
    if (!vscp_fileExists(path.toStdString())) {
      spdlog::debug("Firmware file = {0}", path.toStdString());
      QMessageBox::warning(this,
                           tr(APPNAME),
                           tr("Downloaded firmware file is missing on disk"),
                           QMessageBox::Ok);
      spdlog::error("Firmware download file not available on disk: {}", path.toStdString());
      return false;
    }
    return true;
  }

  // Should be URL

  QString url = field("boot.firmware.url").toString();
  if (url.contains("http://") ||
      url.contains("https://") ||
      url.contains("ftp://") ||
      url.contains("ftps://")) {

    // std::string url = path.toStdString();
    spdlog::debug("URL for Firmware file = {0}", url.toStdString());

    // create a temporary file name for remote MDF
    std::string tempFileName;
    for (int i = 0; i < url.length(); i++) {
      if ((url[i] == '/') || (url[i] == '\\')) {
        tempFileName += "_";
      }
      else {
        tempFileName += url.at(i).toLatin1();
      }
    }

    { // mkstemp()}
      std::string tempPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
          .toStdString();
      tempPath += "/";
      tempPath += tempFileName;
      // tempPath += std::tmpnam(nullptr);
      path = tempPath.c_str();
    }

    spdlog::debug("Temporary path for firmware download: {}", path.toStdString());

    // CURLcode curl_rv;
    // curl_rv = CMDF::downLoadMDF(url.toStdString(), path.toStdString());
    // if (CURLE_OK != curl_rv) {
    //   spdlog::error("Failed to download MDF {0} curl rv={1}", url.toStdString(), (int)curl_rv);
    //   return VSCP_ERROR_COMMUNICATION;
    // }

    CURL* curl;
    FILE* fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
      // path = std::tmpnam(nullptr);
      fp = fopen(path.toStdString().c_str(), "wb");
      printf("path: %s\n", path.toStdString().c_str());
      printf("URL: %s\n", url.toStdString().c_str());
      curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      res = curl_easy_perform(curl);
      // always cleanup
      curl_easy_cleanup(curl);
      fclose(fp);
      if (CURLE_OK != res) {
        QMessageBox::warning(this,
                             tr(APPNAME),
                             tr("Downloaded firmware file is missing on disk"),
                             QMessageBox::Ok);
        spdlog::error("Failed to download firmware {0} to {1}",
                      url.toStdString(),
                      path.toStdString());
        return false;
      }
    } // curl
    else {
      spdlog::error("Unable to initialize cURL (curl_easy_init) for download of firmeware file.");
      QMessageBox::critical(this,
                            tr(APPNAME),
                            tr("Failed to download firmware file (cURL: curl_easy_init)"),
                            QMessageBox::Ok);
      return false;
    }
  }
  else {
    QMessageBox::critical(this,
                          tr(APPNAME),
                          tr("URL is not valid"),
                          QMessageBox::Ok);
    return false;
  }

  if (!vscp_fileExists(path.toStdString())) {
    QMessageBox::warning(this,
                         tr(APPNAME),
                         tr("Downloaded firmware file is missing on disk"),
                         QMessageBox::Ok);
    spdlog::error("Firmware download file not available on disk: {}", path.toStdString());
    return false;
  }

  setField("boot.firmware.path", path);
  spdlog::info("Downloaded firmware file {0}", path.toStdString());

  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CWizardPageFlash::CWizardPageFlash(QWidget* parent, CVscpClient* vscpClient)
  : QWizardPage(parent)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  m_vscpClient      = vscpClient;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CWizardPageFlash::~CWizardPageFlash(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void
CWizardPageFlash::initializePage(void)
{
  setTitle("Ready to update firmware");

  QLabel* label = new QLabel("We are ready to update firmware.\n\n");
  label->setWordWrap(true);

  m_progress = new QProgressBar();
  m_progress->setStyleSheet("background-color: rgb(145, 65, 172);");

  m_infomsg = new QTextEdit();
  m_infomsg->setReadOnly(true);
  m_infomsg->setStyleSheet("background-color:lightgrey;");

  QPushButton* btnFlash = new QPushButton(tr("Flash device"));
  btnFlash->setMaximumWidth(200);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(label);
  layout->addWidget(m_progress);
  layout->addWidget(m_infomsg);
  layout->addWidget(btnFlash);

  setLayout(layout);

  connect(btnFlash, &QPushButton::clicked, this, &CWizardPageFlash::flashDevice);
  // m_bootDev = new CBootDevice(m_vscpClient,vscp_readStringValue(field("boot.nickname").toString().toStdString()));
}

///////////////////////////////////////////////////////////////////////////////
// statusCallback
//

void
CWizardPageFlash::statusCallback(int progress, const char* str)
{
  spdlog::trace("Flashing {0} {1}\n", progress, str);
  if (-1 != progress) {
    m_progress->setValue(progress);
  }

  if (strlen(str)) {
    addStatusMessage(str);
  }
}

///////////////////////////////////////////////////////////////////////////////
// statusCallback
//

void
CWizardPageFlash::addStatusMessage(const QString& str)
{
  QString strmsg;

  strmsg = str;
  strmsg += "\n";
  strmsg += m_infomsg->toPlainText();
  m_infomsg->setText(strmsg);
}

///////////////////////////////////////////////////////////////////////////////
// flashDevice
//

void
CWizardPageFlash::flashDevice(void)
{
  int rv;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Lets rock'n roll
  spdlog::info("Starting flash device process.");
  addStatusMessage("Starting flash device process.");

  m_infomsg->clear();

  // Set remote device in boot mode
  spdlog::info("Bootloading algorithm: {}", field("boot.firmware.bootloader.algorithm").toInt());
  switch (field("boot.firmware.bootloader.algorithm").toInt()) {

    case VSCP_BOOTLOADER_VSCP: {
      addStatusMessage("VSCP Bootloader.");
    } break;

    case VSCP_BOOTLOADER_PIC1: {
      cguid guid;
      addStatusMessage("Microchip PIC1 Bootloader.");
      guid.setNicknameID(vscp_readStringValue(field("boot.nickname").toString().toStdString()));
      using namespace std::placeholders;
      // auto callback = std::bind(&CWizardPageLoadMdf::statusCallback, this, _1, _2);
      //  lambda version for reference
      auto callback = [this](auto a, auto b) { this->statusCallback(a, b); };
      CBootDevice_PIC1 boot(m_vscpClient, vscp_readStringValue(field("boot.nickname").toString().toStdString()), callback);
      addStatusMessage("Hex file path: " + field("boot.firmware.path").toString());
      boot.loadIntelHexFile(field("boot.firmware.path").toString().toStdString());

      uint32_t min, max;
      rv = boot.getMinMaxForRange(CBootDevice_PIC1::MEM_CODE_START, CBootDevice_PIC1::MEM_CODE_END, &min, &max);
      if (VSCP_ERROR_SUCCESS == rv) {
        spdlog::info("Flash code range: {0:X}. {1:X}", min, max);
        addStatusMessage(QString("Flash code range: 0x{%0}. 0x{%1}").arg(min, 8, 16, QChar('0')).arg(max, 8, 16, QChar('0')));
      }
      else {
        spdlog::error("getMinMaxForRange: failed rv={0}", rv);
      }

      rv = boot.getMinMaxForRange(CBootDevice_PIC1::MEM_USERID_START, CBootDevice_PIC1::MEM_USERID_END, &min, &max);
      if (VSCP_ERROR_SUCCESS == rv) {
        spdlog::info("User id: {0:X}. {1:X}", min, max);
        addStatusMessage(QString("User id range: 0x{%0}. 0x{%1}").arg(min, 8, 16, QChar('0')).arg(max, 8, 16, QChar('0')));
      }
      else {
        spdlog::error("getMinMaxForRange: failed rv={0}", rv);
      }

      rv = boot.getMinMaxForRange(CBootDevice_PIC1::MEM_CONFIG_START, CBootDevice_PIC1::MEM_CONFIG_END, &min, &max);
      if (VSCP_ERROR_SUCCESS == rv) {
        spdlog::info("Config: {0:X}. {1:X}", min, max);
        addStatusMessage(QString("Config range: 0x{%0}. 0x{%1}").arg(min, 8, 16, QChar('0')).arg(max, 8, 16, QChar('0')));
      }
      else {
        spdlog::error("getMinMaxForRange: failed rv={0}", rv);
      }

      rv = boot.getMinMaxForRange(CBootDevice_PIC1::MEM_EEPROM_START, CBootDevice_PIC1::MEM_EEPROM_END, &min, &max);
      if (VSCP_ERROR_SUCCESS == rv) {
        spdlog::info("EEPROM: {0:X}. {1:X}", min, max);
        addStatusMessage(QString("EEPROM range: 0x{%0}. 0x{%1}").arg(min, 8, 16, QChar('0')).arg(max, 8, 16, QChar('0')));
      }
      else {
        spdlog::error("getMinMaxForRange: failed rv={0}", rv);
      }

      spdlog::info("Init remote device");
      addStatusMessage("Init remote device.");
      QApplication::setOverrideCursor(Qt::WaitCursor);
      cguid our_guid;
      if (VSCP_ERROR_SUCCESS != (rv = boot.deviceInit(our_guid, 0, pworks->m_firmware_devicecode_required))) {
        QApplication::restoreOverrideCursor();
        spdlog::error("Failed to set device into boot mode rv={}", rv);
        addStatusMessage(QString("Failed to init remote device: rv = {%0}.").arg(rv));
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Failed to set the device in boot mode"),
                              QMessageBox::Ok);
        return;
      }
      addStatusMessage("Remote device initiated.");

      spdlog::info("Load firmware to remote device");
      addStatusMessage("Load firmware to remote device.");
      if (VSCP_ERROR_SUCCESS != (rv = boot.deviceLoad())) {
        QApplication::restoreOverrideCursor();
        spdlog::error("Failed to load firmare to device rv={}", rv);
        addStatusMessage(QString("Failed to load firmare to device: rv = {%0}.").arg(rv));
        QMessageBox::critical(this,
                              tr(APPNAME),
                              tr("Failed to load firmware to device"),
                              QMessageBox::Ok);
        return;
      }
      addStatusMessage("Firmware loaded to remote device.");

      spdlog::info("Restart remote device");
      addStatusMessage("Restart remote device.");
      if (VSCP_ERROR_SUCCESS != (rv = boot.deviceRestart())) {
        spdlog::warn("Failed to confirm restart of device. (Usually not indicating an error) rv={}", rv);
        addStatusMessage(QString("Failed to confirm restart of device. (Usually not indicating an error): rv = {%0}.").arg(rv));
      }
      addStatusMessage("Remote device restarted.");

      spdlog::info("Reboot remote device");
      addStatusMessage("Reboot remote device.");
      if (VSCP_ERROR_SUCCESS != (rv = boot.deviceReboot())) {
        QApplication::restoreOverrideCursor();
        spdlog::error("Failed to confirm reboot of device rv={}", rv);
        addStatusMessage(QString("Failed to confirm reboot of device: rv = {%0}.").arg(rv));
        QMessageBox::information(this,
                                 tr(APPNAME),
                                 tr("Failed to reboot remote device"),
                                 QMessageBox::Ok);
        return;
      }
      addStatusMessage("Remote device rebooted.");

      m_progress->setValue(100);
      addStatusMessage(QString("Success."));
      QApplication::restoreOverrideCursor();

    } break;

    case VSCP_BOOTLOADER_NONE0:
    case VSCP_BOOTLOADER_NONE1:
    case VSCP_BOOTLOADER_NONE2:
    case VSCP_BOOTLOADER_NONE3:
    case VSCP_BOOTLOADER_NONE4:
    case VSCP_BOOTLOADER_NONE5:
    case VSCP_BOOTLOADER_NONE6:
    case VSCP_BOOTLOADER_NONE7:
    case VSCP_BOOTLOADER_NONE8:
    case VSCP_BOOTLOADER_NONE9:
    case VSCP_BOOTLOADER_NONE10:
    case VSCP_BOOTLOADER_NONE11:
    case VSCP_BOOTLOADER_NONE12:
    case VSCP_BOOTLOADER_NONE13:
    case VSCP_BOOTLOADER_NONE14:
      // User specified bootloader for this device
      break;

    case VSCP_BOOTLOADER_NONE15:
      // No bootloader available for this device
      break;

    default:
      // No handler
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// cleanupPage
//

void
CWizardPageFlash::cleanupPage(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// isComplete
//

bool
CWizardPageFlash::isComplete(void) const
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// validatePage
//

bool
CWizardPageFlash::validatePage(void)
{
  return true;
}

// ----------------------------------------------------------------------------

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

  // We need a helpbutton
  setOption(QWizard::HaveHelpButton);

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
      if (!m_vscpClient->isFullLevel2()) {
        setPage(Page_Nickname, new CWizardPageNickname(this, m_vscpClient));
        setPage(Page_Interface, new CWizardPageInterface(this, m_vscpClient));
      }
      setPage(Page_Guid, new CWizardPageGuid(this, m_vscpClient));
      setPage(Page_LoadMdf, new CWizardPageLoadMdf(this, m_vscpClient));
      setPage(Page_Firmware, new CWizardPageFirmware(this, m_vscpClient));
      setPage(Page_Flash, new CWizardPageFlash(this, m_vscpClient));
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
      setPage(Page_Nickname, new CWizardPageNickname(this, m_vscpClient));
      setPage(Page_LoadMdf, new CWizardPageLoadMdf(this, m_vscpClient));
      setPage(Page_Firmware, new CWizardPageFirmware(this, m_vscpClient));
      setPage(Page_Flash, new CWizardPageFlash(this, m_vscpClient));
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
      setPage(Page_Nickname, new CWizardPageNickname(this, m_vscpClient));
      setPage(Page_LoadMdf, new CWizardPageLoadMdf(this, m_vscpClient));
      setPage(Page_Firmware, new CWizardPageFirmware(this, m_vscpClient));
      setPage(Page_Flash, new CWizardPageFlash(this, m_vscpClient));
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
      if (m_vscpClient->isFullLevel2()) {
        setPage(Page_Guid, new CWizardPageGuid(this, m_vscpClient));
      }
      else {
        setPage(Page_Nickname, new CWizardPageNickname(this, m_vscpClient));
      }
      setPage(Page_LoadMdf, new CWizardPageLoadMdf(this, m_vscpClient));
      setPage(Page_Firmware, new CWizardPageFirmware(this, m_vscpClient));
      setPage(Page_Flash, new CWizardPageFlash(this, m_vscpClient));
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
