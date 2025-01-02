// bootloaderwizard.cpp
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
// https://doc.qt.io/qt-6/qwizard.html

#ifndef CBOOTLOADWIZARD_H
#define CBOOTLOADWIZARD_H

#include <vscphelper.h>
#include <mdf.h>
#include <vscp-client-base.h>
#include <vscp-bootdevice.h>

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QWizard>

class CWizardPageNickname : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageNickname(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageNickname();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

  /*!
    Next page
  */
  int nextId(void) const;

  /// Set initial nickname
  void setNickname(uint16_t nickname) { m_nickname = nickname; }

  

private:
  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;

  // Default nickname id
  uint16_t m_nickname;
};

// ----------------------------------------------------------------------------

class CWizardPageInterface : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageInterface(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageInterface();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

public slots:

  /*!
    open known GUID dialog
  */
  void openKnownGuidDlg(void);

  /*!
    Set interface to all zeros
  */
  void setAllZeros(void);

private:
  /// The interface id
  QLineEdit* m_pInterface;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;
};

// ----------------------------------------------------------------------------

class CWizardPageGuid : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageGuid(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageGuid();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

public slots:

  /*!
    open known GUID dialog
  */
  void openKnownGuidDlg(void);

private:
  // The selected GUID
  QLineEdit* m_pGuid;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;
};

// ----------------------------------------------------------------------------

class CWizardPageLoadMdf : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageLoadMdf(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageLoadMdf();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

  /*!
    Call back used to display callback.
    @param progress Numerical representation of progress, normally 0-100
    @param str Description of progress step
  */
  void statusCallback(int progress, const char* str);

public slots:

private:

  /*!
    Checkbox that select if MDF should be
    loaded locally or from url fetched from device
  */
  QCheckBox* m_pchkLocalMdf;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;
};

// ----------------------------------------------------------------------------

class CWizardPageFirmware : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageFirmware(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageFirmware();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

public slots:

  /*!
    Get firmware file
  */
  void setFirmwareFile(void);

  /*!
    Local file checkbock clicked
  */
  void toggleLocalFile(bool checked);

private:

  CMDF m_mdf;

  /// Selected firmware file
  QLineEdit* m_editFirmwareFile;

  /// Button that open dialog to select firmware file
  QPushButton* m_btnSelectFirmware;

  // Checkbox that selects local file
  QCheckBox* m_chkLocalFile;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;
};

// ----------------------------------------------------------------------------

class CWizardPageFlash : public QWizardPage {
  Q_OBJECT

public:
  explicit CWizardPageFlash(QWidget* parent, CVscpClient* vscpClient);
  ~CWizardPageFlash();

  /*!
    Setup the page
  */
  void initializePage(void);

  /*!
    Clean up the page
  */
  void cleanupPage(void);

  /*!
    Called by the framework to check if to determine whether the Next or
    Finish button should be enabled or disabled.
    @return true if buttons should be displayed.
  */
  bool isComplete(void) const;

  /*!
    Called when next or finnish is pressed to perform
    last minute validation
    @return true if it ios  OK to move on
  */
  bool validatePage(void);

  /*!
    Add a status message
    @param str Status message to add. Not added if zero length
  */
  void addStatusMessage(const QString& str);

  /*!
    Callback for the flashing operation
    @param int progress for operation 0-100. If set to -1 progress is
      not updated.
    @param str Description of operation
  */
  void statusCallback(int progress, const char* str);

  /// Flash device using PIC1 algorithm
  void flashDevicePIC1(void);

  /// Flash device using VSCP algorithm
  void flashDeviceVSCP(void);

public slots:

  /// Flash the device
  void flashDevice(void);  

private:
  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;

  /// Pointer to the boot device
  CBootDevice* m_bootDev;

  /// Boot progress
  QProgressBar *m_progress;

  /// Last info message from boot process
  QTextEdit *m_infomsg;
  
};

// ----------------------------------------------------------------------------

class CBootLoadWizard : public QWizard {
  Q_OBJECT

public:
  explicit CBootLoadWizard(QWidget* parent, QJsonObject* pconn);
  ~CBootLoadWizard();

  enum { Page_Intro,
         Page_Nickname,
         Page_Interface,
         Page_Guid,
         Page_LoadMdf,
         Page_Firmware,
         Page_Flash,
         Page_Done };

  /*!
    Create the introduction page
  */
  QWizardPage* createIntroPage(void);

  /*!
    Start the bootloader wizard
  */
  int initBootLoaderWizard(void);

private:
  /// The VSCP client type
  CVscpClient::connType m_vscpConnType;

  /// Configuration data for the session
  QJsonObject m_connObject;

  /// A pointer to a VSCP Client
  CVscpClient* m_vscpClient;
};

#endif // CBOOTLOADWIZARD_H