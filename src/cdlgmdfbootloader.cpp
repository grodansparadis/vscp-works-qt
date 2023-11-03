// cdlgmdfbootloader.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, Grodans Paradis AB
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
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgmdfbootloader.h"
#include "ui_cdlgmdfbootloader.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfBootLoader::CDlgMdfBootLoader(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfBootLoader)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // Fill combo with predefined bootloader algorithms (vscp.h)
  ui->comboBoxAlgorithm->addItem("VSCP", VSCP_BOOTLOADER_VSCP); // VSCP boot loader algorithm
  ui->comboBoxAlgorithm->addItem("PIC1", VSCP_BOOTLOADER_PIC1); //  PIC algorithm 0
  ui->comboBoxAlgorithm->addItem("Reserved (0x02)", 2);
  ui->comboBoxAlgorithm->addItem("Reserved (0x03)", 3);
  ui->comboBoxAlgorithm->addItem("Reserved (0x04)", 4);
  ui->comboBoxAlgorithm->addItem("Reserved (0x05)", 5);
  ui->comboBoxAlgorithm->addItem("Reserved (0x06)", 6);
  ui->comboBoxAlgorithm->addItem("Reserved (0x07)", 7);
  ui->comboBoxAlgorithm->addItem("Reserved (0x08)", 8);
  ui->comboBoxAlgorithm->addItem("Reserved (0x09)", 9);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0A)", 10);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0B)", 11);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0C)", 12);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0D)", 13);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0E)", 14);
  ui->comboBoxAlgorithm->addItem("Reserved (0x0F)", 15);
  ui->comboBoxAlgorithm->addItem("AVR1", VSCP_BOOTLOADER_AVR1); //  AVR algorithm 0
  ui->comboBoxAlgorithm->addItem("Reserved (0x11)", 0x11);
  ui->comboBoxAlgorithm->addItem("Reserved (0x12)", 0x12);
  ui->comboBoxAlgorithm->addItem("Reserved (0x13)", 0x13);
  ui->comboBoxAlgorithm->addItem("Reserved (0x14)", 0x14);
  ui->comboBoxAlgorithm->addItem("Reserved (0x15)", 0x15);
  ui->comboBoxAlgorithm->addItem("Reserved (0x16)", 0x16);
  ui->comboBoxAlgorithm->addItem("Reserved (0x17)", 0x17);
  ui->comboBoxAlgorithm->addItem("Reserved (0x18)", 0x18);
  ui->comboBoxAlgorithm->addItem("Reserved (0x19)", 0x19);
  ui->comboBoxAlgorithm->addItem("Reserved (0x1A)", 0x1A);
  ui->comboBoxAlgorithm->addItem("Reserved (0x1B)", 0x1B);
  ui->comboBoxAlgorithm->addItem("Reserved (0x1C)", 0x1C);
  ui->comboBoxAlgorithm->addItem("Reserved (0x1D)", 0x1D);
  ui->comboBoxAlgorithm->addItem("Reserved (0x1E)", 0x1E);
  ui->comboBoxAlgorithm->addItem("NXP1", VSCP_BOOTLOADER_LPC1);            //  NXP/Philips/Freescale algorithm 0
  ui->comboBoxAlgorithm->addItem("Reserved (0x21)", 0x21);
  ui->comboBoxAlgorithm->addItem("Reserved (0x22)", 0x22);
  ui->comboBoxAlgorithm->addItem("Reserved (0x23)", 0x23);
  ui->comboBoxAlgorithm->addItem("Reserved (0x24)", 0x24);
  ui->comboBoxAlgorithm->addItem("Reserved (0x25)", 0x25);
  ui->comboBoxAlgorithm->addItem("Reserved (0x26)", 0x26);
  ui->comboBoxAlgorithm->addItem("Reserved (0x27)", 0x27);
  ui->comboBoxAlgorithm->addItem("Reserved (0x28)", 0x28);
  ui->comboBoxAlgorithm->addItem("Reserved (0x29)", 0x29);
  ui->comboBoxAlgorithm->addItem("Reserved (0x2A)", 0x2A);
  ui->comboBoxAlgorithm->addItem("Reserved (0x2B)", 0x2B);
  ui->comboBoxAlgorithm->addItem("Reserved (0x2C)", 0x2C);
  ui->comboBoxAlgorithm->addItem("Reserved (0x2D)", 0x2D);
  ui->comboBoxAlgorithm->addItem("Reserved (0x2E)", 0x2E);
  ui->comboBoxAlgorithm->addItem("ST", VSCP_BOOTLOADER_ST);                //  ST STR algorithm 0
  ui->comboBoxAlgorithm->addItem("Reserved (0x31)", 0x31);
  ui->comboBoxAlgorithm->addItem("Reserved (0x32)", 0x32);
  ui->comboBoxAlgorithm->addItem("Reserved (0x33)", 0x33);
  ui->comboBoxAlgorithm->addItem("Reserved (0x34)", 0x34);
  ui->comboBoxAlgorithm->addItem("Reserved (0x35)", 0x35);
  ui->comboBoxAlgorithm->addItem("Reserved (0x36)", 0x36);
  ui->comboBoxAlgorithm->addItem("Reserved (0x37)", 0x37);
  ui->comboBoxAlgorithm->addItem("Reserved (0x38)", 0x38);
  ui->comboBoxAlgorithm->addItem("Reserved (0x39)", 0x39);
  ui->comboBoxAlgorithm->addItem("Reserved (0x3A)", 0x3A);
  ui->comboBoxAlgorithm->addItem("Reserved (0x3B)", 0x3B);
  ui->comboBoxAlgorithm->addItem("Reserved (0x3C)", 0x3C);
  ui->comboBoxAlgorithm->addItem("Reserved (0x3D)", 0x3D);
  ui->comboBoxAlgorithm->addItem("Reserved (0x3E)", 0x3E);
  ui->comboBoxAlgorithm->addItem("FREESCALE", VSCP_BOOTLOADER_FREESCALE);  //  Freescale Kinetics algorithm 0
  ui->comboBoxAlgorithm->addItem("Reserved (0x41)", 0x41);
  ui->comboBoxAlgorithm->addItem("Reserved (0x42)", 0x42);
  ui->comboBoxAlgorithm->addItem("Reserved (0x43)", 0x43);
  ui->comboBoxAlgorithm->addItem("Reserved (0x44)", 0x44);
  ui->comboBoxAlgorithm->addItem("Reserved (0x45)", 0x45);
  ui->comboBoxAlgorithm->addItem("Reserved (0x46)", 0x46);
  ui->comboBoxAlgorithm->addItem("Reserved (0x47)", 0x47);
  ui->comboBoxAlgorithm->addItem("Reserved (0x48)", 0x48);
  ui->comboBoxAlgorithm->addItem("Reserved (0x49)", 0x49);
  ui->comboBoxAlgorithm->addItem("Reserved (0x4A)", 0x4A);
  ui->comboBoxAlgorithm->addItem("Reserved (0x4B)", 0x4B);
  ui->comboBoxAlgorithm->addItem("Reserved (0x4C)", 0x4C);
  ui->comboBoxAlgorithm->addItem("Reserved (0x4D)", 0x4D);
  ui->comboBoxAlgorithm->addItem("Reserved (0x4E)", 0x4E);
  ui->comboBoxAlgorithm->addItem("ESP", VSCP_BOOTLOADER_ESP);              //  Espressif algorithm 0
  for ( int i=0x51; i<VSCP_BOOTLOADER_NONE0; i++) {
    QString str = QString("Reserved 0x%d").arg(i, 2, 16);
    ui->comboBoxAlgorithm->addItem(str, i);
  }
  ui->comboBoxAlgorithm->addItem("User 0", VSCP_BOOTLOADER_NONE0);         //  Used defined bootloader 0
  ui->comboBoxAlgorithm->addItem("User 1", VSCP_BOOTLOADER_NONE1);         //  Used defined bootloader 1
  ui->comboBoxAlgorithm->addItem("User 2", VSCP_BOOTLOADER_NONE2);         //  Used defined bootloader 2
  ui->comboBoxAlgorithm->addItem("User 3", VSCP_BOOTLOADER_NONE3);         //  Used defined bootloader 3
  ui->comboBoxAlgorithm->addItem("User 4", VSCP_BOOTLOADER_NONE4);         //  Used defined bootloader 4
  ui->comboBoxAlgorithm->addItem("User 5", VSCP_BOOTLOADER_NONE5);         //  Used defined bootloader 5
  ui->comboBoxAlgorithm->addItem("User 6", VSCP_BOOTLOADER_NONE6);         //  Used defined bootloader 6
  ui->comboBoxAlgorithm->addItem("User 7", VSCP_BOOTLOADER_NONE7);         //  Used defined bootloader 7
  ui->comboBoxAlgorithm->addItem("User 8", VSCP_BOOTLOADER_NONE8);         //  Used defined bootloader 8
  ui->comboBoxAlgorithm->addItem("User 9", VSCP_BOOTLOADER_NONE9);         //  Used defined bootloader 9
  ui->comboBoxAlgorithm->addItem("User 10", VSCP_BOOTLOADER_NONE10);       //  Used defined bootloader 10
  ui->comboBoxAlgorithm->addItem("User 11", VSCP_BOOTLOADER_NONE11);       //  Used defined bootloader 11
  ui->comboBoxAlgorithm->addItem("User 12", VSCP_BOOTLOADER_NONE12);       // Used defined bootloader 12
  ui->comboBoxAlgorithm->addItem("User 13", VSCP_BOOTLOADER_NONE13);       //  Used defined bootloader 13
  ui->comboBoxAlgorithm->addItem("User 14", VSCP_BOOTLOADER_NONE14);       //  Used defined bootloader 14
  ui->comboBoxAlgorithm->addItem("No bootloader", VSCP_BOOTLOADER_NONE15); //  No bootloader available

  // QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+E", "Edit")), ui->editDate);
  // connect(shortcut, &QShortcut::activated, this, &CDlgMdfBootLoader::editDesc);

  // connect(ui->btnSetDummyGuid, &QPushButton::clicked, this, &cdlgmdfmodule::setDummyGuid);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfBootLoader::~CDlgMdfBootLoader()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfBootLoader::initDialogData(const CMDF_Object* pmdfobj, mdf_bootloader_index index)
{
  QString str;

  if (nullptr == pmdfobj) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pbootinfo = (CMDF_BootLoaderInfo*)pmdfobj;

  ui->editBlockSize->setText(QString::number(m_pbootinfo->getBlockSize()));
  ui->editBlockCount->setText(QString::number(m_pbootinfo->getBlockCount()));
  ui->comboBoxAlgorithm->setCurrentIndex(m_pbootinfo->getAlgorithm());

  switch (index) {
    case index_bootloader_blockcount:
      ui->editBlockCount->setFocus();
      break;

    case index_bootloader_algorithm:
      ui->comboBoxAlgorithm->setFocus();
      break;

    case index_bootloader_blocksize:
    default:
      ui->editBlockSize->setFocus();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfBootLoader::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getBlocksize
//

uint32_t
CDlgMdfBootLoader::getBlockSize(void)
{
  return (vscp_readStringValue(ui->editBlockSize->text().toStdString()));
}

///////////////////////////////////////////////////////////////////////////////
// setBlocksize
//

void
CDlgMdfBootLoader::setBlockSize(uint32_t blocksize)
{
  QString str = QString::number(blocksize);
  ui->editBlockSize->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfBootLoader::accept()
{
  std::string str;
  if (nullptr != m_pbootinfo) {

    str = ui->editBlockSize->text().toStdString();
    m_pbootinfo->setBlocksize(vscp_readStringValue(str));

    str = ui->editBlockCount->text().toStdString();
    m_pbootinfo->setBlockCount(vscp_readStringValue(str));

    // m_pbootinfo->setModuleLevel(ui->comboModuleLevel->currentIndex());

    int idx = ui->comboBoxAlgorithm->currentIndex(); //  ->text().toStdString();
    m_pbootinfo->setAlgorithm(idx);
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
