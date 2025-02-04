// cdlgmdfremotevar.cpp
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

#include <vscp.h>
#include <vscphelper.h>

#include <vscpworks.h>

#include "cdlgmdfremotevar.h"
#include "ui_cdlgmdfremotevar.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfRemoteVar::pre_str_remote_variable[] = "Remote variable: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfRemoteVar::CDlgMdfRemoteVar(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRemoteVar)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_pvar = nullptr;

  // Help
QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(showHelp()));
shortcut->setAutoRepeat(false);

QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRemoteVar::~CDlgMdfRemoteVar()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRemoteVar::initDialogData(CMDF* pmdf, CMDF_RemoteVariable* pvar, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF remote variable information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == pvar) {
    spdlog::error("MDF remote variable information - Invalid MDF remote variable object (initDialogData)");
    return;
  }

  m_pvar = pvar;

  // Connect set foreground color button
  connect(ui->btnSetFgColor,
          SIGNAL(clicked()),
          this,
          SLOT(showFgColorDlg()));

  // Connect set background color button
  connect(ui->btnSetBgColor,
          SIGNAL(clicked()),
          this,
          SLOT(showBgColorDlg()));

  // Connect set background color button
  connect(ui->btnSetUndef,
          SIGNAL(clicked()),
          this,
          SLOT(setUndef()));

  setName(pvar->getName().c_str());
  setPage(pvar->getPage());
  setOffset(pvar->getOffset());
  setBitOffset(pvar->getBitPos());
  setType(pvar->getType());
  setSpan(pvar->getTypeByteCount());
  setAccess(pvar->getAccess());
  str = pvar->getDefault().c_str();
  setDefault(str);
  setForegroundColor(pvar->getForegroundColor());
  setBackgroundColor(pvar->getBackgroundColor());

  // Fill page combo box with used pages
  std::set<uint16_t> pages;
  uint32_t cnt = pmdf->getPages(pages);
  ui->comboPage->clear();
  int pos = 0;
  for (std::set<uint16_t>::iterator it = pages.begin(); it != pages.end(); ++it) {
    ui->comboPage->addItem(QString("Page %1").arg(*it), *it);
    if (pvar->getPage() == *it) {
      ui->comboPage->setCurrentIndex(pos);
    }
    pos++;
  }

  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_type:
      ui->comboType->setFocus();
      break;

    case index_span:
      ui->spinSpan->setFocus();
      break;

    case index_page:
      ui->comboPage->setFocus();
      break;

    case index_offset:
      ui->spinOffset->setFocus();
      break;

    case index_bit_offset:
      ui->spinBitOffset->setFocus();
      break;

    case index_access:
      ui->comboAccess->setFocus();
      break;

    case index_default:
      ui->editDefault->setFocus();
      break;

    case index_fgcolor:
      ui->editFgColor->setFocus();
      break;

    case index_bgcolor:
      ui->editBgColor->setFocus();
      break;

    default:
      ui->editName->setFocus();
      break;
  }

  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// setReadOnly
//

void
CDlgMdfRemoteVar::setReadOnly(void)
{
  ui->comboPage->setEnabled(false);
  ui->spinOffset->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRemoteVar::setInitialFocus(void)
{
  // ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setUndef
//

void
CDlgMdfRemoteVar::setUndef(void)
{
  ui->editDefault->setText("UNDEF");
}

///////////////////////////////////////////////////////////////////////////////
// showFgColorDlg
//

void
CDlgMdfRemoteVar::showFgColorDlg(void)
{
  QColorDialog dlg(this);
  dlg.setCurrentColor(QColor(vscp_readStringValue(ui->editFgColor->text().toStdString())));

  if (QDialog::Accepted == dlg.exec()) {
    setForegroundColor(dlg.selectedColor().rgb());
  }
}

///////////////////////////////////////////////////////////////////////////////
// showBgColorDlg
//

void
CDlgMdfRemoteVar::showBgColorDlg(void)
{
  QColorDialog dlg(this);
  dlg.setCurrentColor(QColor(vscp_readStringValue(ui->editBgColor->text().toStdString())));

  if (QDialog::Accepted == dlg.exec()) {
    setBackgroundColor(dlg.selectedColor().rgb());
  }
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

QString
CDlgMdfRemoteVar::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfRemoteVar::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

uint16_t
CDlgMdfRemoteVar::getPage(void)
{
  int index = ui->comboPage->currentIndex();
  if (-1 == index) {
    return 0;
  }

  return ui->comboPage->currentData().toInt();
}

void
CDlgMdfRemoteVar::setPage(uint16_t page)
{
  for (int i = 0; i < ui->comboPage->count(); i++) {
    if (page == ui->comboPage->itemData(i)) {
      ui->comboPage->setCurrentIndex(i);
    }
  }
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRemoteVar::getOffset(void)
{
  return ui->spinOffset->value();
}

void
CDlgMdfRemoteVar::setOffset(uint32_t offset)
{
  ui->spinOffset->setValue(offset);
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRemoteVar::getBitOffset(void)
{
  return ui->spinBitOffset->value();
}

void
CDlgMdfRemoteVar::setBitOffset(uint32_t offset)
{
  ui->spinBitOffset->setValue(offset);
}

// -----------------------------------------------------------------------

vscp_remote_variable_type
CDlgMdfRemoteVar::getType(void)
{
  return static_cast<vscp_remote_variable_type>(ui->comboType->currentIndex());
}

void
CDlgMdfRemoteVar::setType(vscp_remote_variable_type type)
{
  ui->comboType->setCurrentIndex(static_cast<int>(type));
}

// -----------------------------------------------------------------------

uint16_t
CDlgMdfRemoteVar::getSpan(void)
{
  return ui->spinSpan->value();
}

void
CDlgMdfRemoteVar::setSpan(uint16_t span)
{
  ui->spinSpan->setValue(span);
}

// -----------------------------------------------------------------------

// uint8_t
// CDlgMdfRemoteVar::getWidth(void)
// {
//   return ui->spinWidth->value();
// }

// void
// CDlgMdfRemoteVar::setWidth(uint8_t span)
// {
//   ui->spinWidth->setValue(span);
// }

// -----------------------------------------------------------------------

mdf_access_mode
CDlgMdfRemoteVar::getAccess(void)
{
  return static_cast<mdf_access_mode>(ui->comboAccess->currentIndex());
}

void
CDlgMdfRemoteVar::setAccess(mdf_access_mode access)
{
  ui->comboAccess->setCurrentIndex(static_cast<int>(access));
}

// -----------------------------------------------------------------------

QString
CDlgMdfRemoteVar::getDefault(void)
{
  return ui->editDefault->text();
}

void
CDlgMdfRemoteVar::setDefault(QString& str)
{
  ui->editDefault->setText(str);
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRemoteVar::getForegroundColor(void)
{
  return vscp_readStringValue(ui->editFgColor->text().toStdString());
}

void
CDlgMdfRemoteVar::setForegroundColor(uint32_t color)
{
  ui->editFgColor->setText(QString("0x%1").arg(color, 8, 16, QChar('0')));
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRemoteVar::getBackgroundColor(void)
{
  return vscp_readStringValue(ui->editBgColor->text().toStdString());
}
void
CDlgMdfRemoteVar::setBackgroundColor(uint32_t color)
{
  ui->editBgColor->setText(QString("0x%1").arg(color, 8, 16, QChar('0')));
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRemoteVar::accept()
{
  std::string str;
  if (nullptr != m_pvar) {
    m_pvar->setName(getName().toStdString());
    m_pvar->setPage(getPage());
    m_pvar->setOffset(getOffset());
    m_pvar->setType(static_cast<vscp_remote_variable_type>(getType()));
    // m_pvar->setSpan(getSpan());
    // m_pvar->setMin(getMin());
    // m_pvar->setMax(getMax());
    m_pvar->setAccess(getAccess());
    m_pvar->setDefault(getDefault().toStdString());
    m_pvar->setForegroundColor(getForegroundColor());
    m_pvar->setBackgroundColor(getBackgroundColor());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgMdfRemoteVar::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}
