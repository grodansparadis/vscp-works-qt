// cdlgselectbootalgorithm.cpp
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

#include "vscp.h"
#include "vscphelper.h"

#include "vscpworks.h"

#include "sessionfilter.h"

#include "cdlgselectbootalgorithm.h"
#include "ui_cdlgselectbootalgorithm.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectBootAlgorithm::CDlgSelectBootAlgorithm(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgSelectBootAlgorithm)
{
  ui->setupUi(this);

  // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  ui->comboBootAlgorithm->clear();
  ui->comboBootAlgorithm->addItem("VSCP Algorithm", VSCP_BOOTLOADER_VSCP);
  ui->comboBootAlgorithm->addItem("PIC algorithm 0", VSCP_BOOTLOADER_PIC1);
  ui->comboBootAlgorithm->addItem("AVR algorithm 0", VSCP_BOOTLOADER_AVR1);
  ui->comboBootAlgorithm->addItem("NXP/Philips/Freescale algorithm 0", VSCP_BOOTLOADER_LPC1);
  ui->comboBootAlgorithm->addItem("NXP/Philips/Freescale algorithm 0", VSCP_BOOTLOADER_NXP1);
  ui->comboBootAlgorithm->addItem("ST STR algorithm 0", VSCP_BOOTLOADER_ST);
  ui->comboBootAlgorithm->addItem("Freescale Kinetics algorithm 0", VSCP_BOOTLOADER_FREESCALE);
  ui->comboBootAlgorithm->addItem("Espressif algorithm 0", VSCP_BOOTLOADER_ESP);
  ui->comboBootAlgorithm->addItem("User boot loader 0", VSCP_BOOTLOADER_NONE0);
  ui->comboBootAlgorithm->addItem("User boot loader 1", VSCP_BOOTLOADER_NONE1);
  ui->comboBootAlgorithm->addItem("User boot loader 2", VSCP_BOOTLOADER_NONE2);
  ui->comboBootAlgorithm->addItem("User boot loader 3", VSCP_BOOTLOADER_NONE3);
  ui->comboBootAlgorithm->addItem("User boot loader 4", VSCP_BOOTLOADER_NONE4);
  ui->comboBootAlgorithm->addItem("User boot loader 5", VSCP_BOOTLOADER_NONE5);
  ui->comboBootAlgorithm->addItem("User boot loader 6", VSCP_BOOTLOADER_NONE6);
  ui->comboBootAlgorithm->addItem("User boot loader 7", VSCP_BOOTLOADER_NONE7);
  ui->comboBootAlgorithm->addItem("User boot loader 8", VSCP_BOOTLOADER_NONE8);
  ui->comboBootAlgorithm->addItem("User boot loader 9", VSCP_BOOTLOADER_NONE9);
  ui->comboBootAlgorithm->addItem("User boot loader 10", VSCP_BOOTLOADER_NONE10);
  ui->comboBootAlgorithm->addItem("User boot loader 11", VSCP_BOOTLOADER_NONE11);
  ui->comboBootAlgorithm->addItem("User boot loader 12", VSCP_BOOTLOADER_NONE12);
  ui->comboBootAlgorithm->addItem("User boot loader 13", VSCP_BOOTLOADER_NONE13);
  ui->comboBootAlgorithm->addItem("User boot loader 14", VSCP_BOOTLOADER_NONE14);
  ui->comboBootAlgorithm->addItem("No boot loader", VSCP_BOOTLOADER_NONE15);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectBootAlgorithm::~CDlgSelectBootAlgorithm()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getBootAlgorithm
//

uint8_t
CDlgSelectBootAlgorithm::getBootAlgorithm(void)
{
  int idx = ui->comboBootAlgorithm->currentIndex();
  return ui->comboBootAlgorithm->currentData().toInt();
}

///////////////////////////////////////////////////////////////////////////////
// setBootAlgorithm
//

void
CDlgSelectBootAlgorithm::setBootAlgorithm(uint8_t value)
{
  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  ui->comboBootAlgorithm->setCurrentIndex(value);
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgSelectBootAlgorithm::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/bootload_window";
  QDesktopServices::openUrl(QUrl(link));
}