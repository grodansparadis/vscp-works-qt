// CDlgSelectInterface.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <pch.h>
#include <iphlpapi.h> // GetAdaptersAddresses
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <vscp.h>

#include "vscp.h"
#include "vscphelper.h"
#include "vscpworks.h"
#include "cdlgknownguid.h"
#include "cdlgselectinterface.h"
#include "ui_cdlgselectinterface.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>
#include <QHelpEngineCore>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectInterface::CDlgSelectInterface(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgSelectInterface)
{
  ui->setupUi(this);

  connect(ui->listInterface, &QListWidget::itemDoubleClicked, this, &CDlgSelectInterface::onListItemDoubleClicked);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

#ifdef WIN32
  // https://learn.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses?redirectedfrom=MSDN
#else
  // https://stackoverflow.com/questions/2283494/get-ip-address-of-an-interface-on-linux
  struct ifaddrs* ifaddrs; // Interface addresses

  if (-1 == getifaddrs(&ifaddrs)) {
    perror("getifaddrs failed");
    exit(1);
  }

  for (struct ifaddrs* ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr != NULL) {
      // Check if it's an IPv4 address
      if (ifa->ifa_addr->sa_family == AF_INET) {
        // Get the address name
        char* interfaceName = ifa->ifa_name;
        // Get the IPv4 address
        struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
        char ip_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip_address, INET_ADDRSTRLEN);

        // Print the information
        printf("Interface: %s\n", interfaceName);
        printf("IP Address: %s\n", ip_address);

        QListWidgetItem* item = new QListWidgetItem(QString("%1 (%2)").arg(interfaceName).arg(ip_address));
        ui->listInterface->addItem(item);
      }
    }
  }

  freeifaddrs(ifaddrs);

#endif
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectInterface::~CDlgSelectInterface()
{
  delete ui;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// fillInterfaces
//

void
CDlgSelectInterface::fillInterfaces(void)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // uint8_t guid[16];
  // if (!vscp_getGuidFromStringToArray(guid, ui->lineEditGuid->text().toStdString())) {
  //   QMessageBox::information(this,
  //                            tr(APPNAME),
  //                            tr("Faild to convert GUID string"),
  //                            QMessageBox::Ok);
  //   return;
  // }
}

///////////////////////////////////////////////////////////////////////////////
// getInterface
//

QString
CDlgSelectInterface::getInterface(void)
{
  QString strInterface;
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // int idx = ui->listInterface->currentRow();
  QListWidgetItem* item = ui->listInterface->currentItem();
  if (nullptr != item) {
    strInterface = item->text();
  }

  return strInterface;
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//

void
CDlgSelectInterface::setInterface(QString& ifstr)
{
  // dlg.enableAccept(true);
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // if (QDialog::Accepted == dlg.exec()) {
  // }
}

// Slot implementation
void
CDlgSelectInterface::onListItemDoubleClicked(QListWidgetItem* item)
{
  // Handle selection
  qDebug() << "Selected item:" << item->text();
  accept(); // Close the dialog with Accepted status
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectInterface::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/";
  QDesktopServices::openUrl(QUrl(link));
}
