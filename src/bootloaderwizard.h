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
// https://doc.qt.io/qt-6/qwizard.html

#ifndef CBOOTLOADWIZARD_H
#define CBOOTLOADWIZARD_H

#include "vscphelper.h"
#include <vscp_client_base.h>

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QWizard>
#include <QMessageBox>
#include <QJsonObject>


class CBootLoadWizard : public QWizard {
  Q_OBJECT

public:
  explicit CBootLoadWizard(QWidget *parent, QJsonObject* pconn);
  ~CBootLoadWizard();

  enum { Page_Intro,
         Page_Nickname,
         Page_Interface,
         Page_Guid,
         Page_PreDevice,
         Page_Device,
         Page_Firmware,
         Page_Done };


  /*!
    Create the introduction page
  */
  QWizardPage *createIntroPage(void); 

  /*!
    Create the nickname page
  */
  QWizardPage *createNicknamePage(void); 

  /*!
    Create the interface page
  */
  QWizardPage *createInterfacePage(void);

  /*!
    Create the introduction page
  */
  QWizardPage *createGuidPage(void); 

  /*!
    Create the page to show before data collection
  */
  QWizardPage *createPreDevicePage(void); 

  /*!
    Create the introduction page
  */
  QWizardPage *createDevicePage(void); 

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
    CVscpClient *m_vscpClient;
};

#endif // CBOOTLOADWIZARD_H