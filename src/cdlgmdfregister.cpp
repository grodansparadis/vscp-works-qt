// cdlgmdfregister.cpp
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

#include "cdlgmdfregister.h"
#include "ui_cdlgmdfregister.h"

#include <QColorDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

const char CDlgMdfRegister::pre_str_register[] = "Register: ";

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfRegister::CDlgMdfRegister(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRegister)
{
  ui->setupUi(this);

  // m_type = mdf_type_unknown;
  m_preg = nullptr;

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRegister::~CDlgMdfRegister()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRegister::initDialogData(CMDF* pmdf, CMDF_Register* preg, int index)
{
  QString str;

  if (nullptr == pmdf) {
    spdlog::error("MDF register information - Invalid MDF object (initDialogData)");
    return;
  }

  m_pmdf = pmdf;

  if (nullptr == preg) {
    spdlog::error("MDF register information - Invalid MDF register object (initDialogData)");
    return;
  }

  m_preg = preg;

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

  setName(preg->getName().c_str());
  setPage(preg->getPage());
  setOffset(preg->getOffset());
  setType(preg->getType());
  setSpan(preg->getSpan());
  setWidth(preg->getWidth());
  setMin(preg->getMin());
  setMax(preg->getMax());
  setAccess(preg->getAccess());
  uint8_t val;
  bool bDefault = preg->getDefault(val);
  str           = preg->getDefault(val) ? QString("%1").arg(val) : "UNDEF";
  setDefault(str);
  setForegroundColor(preg->getForegroundColor());
  setBackgroundColor(preg->getBackgroundColor());

  std::set<uint16_t> pages;
  uint32_t cnt = pmdf->getPages(pages);
  ui->comboPage->clear();
  int pos = 0;
  for (std::set<uint16_t>::iterator it=pages.begin(); it!=pages.end(); ++it) {
    ui->comboPage->addItem(QString("Page %1").arg(*it), *it);
    if (preg->getPage() == *it) {
      ui->comboPage->setCurrentIndex(pos);
    }
    pos++;
  }


  switch (index) {
    case index_name:
      ui->editName->setFocus();
      break;

    case index_page:
      ui->comboPage->setFocus();
      break;

    case index_offset:
      ui->editOffset->setFocus();
      break;

    case index_type:
      ui->comboType->setFocus();
      break;

    case index_span:
      ui->spinSpan->setFocus();
      break;

    case index_width:
      ui->spinWidth->setFocus();
      break;

    case index_min:
      ui->spinMin->setFocus();
      break;

    case index_max:
      ui->spinMax->setFocus();
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
CDlgMdfRegister::setReadOnly(void)
{
  ui->comboPage->setEnabled(false);
  ui->editOffset->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRegister::setInitialFocus(void)
{
  // ui->editName->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// setUndef
//

void
CDlgMdfRegister::setUndef(void)
{
  ui->editDefault->setText("UNDEF");
}

///////////////////////////////////////////////////////////////////////////////
// showFgColorDlg
//

void
CDlgMdfRegister::showFgColorDlg(void)
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
CDlgMdfRegister::showBgColorDlg(void)
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
CDlgMdfRegister::getName(void)
{
  return ui->editName->text();
};

void
CDlgMdfRegister::setName(const QString& name)
{
  ui->editName->setText(name);
};

// -----------------------------------------------------------------------

uint16_t
CDlgMdfRegister::getPage(void)
{
  int index = ui->comboPage->currentIndex();
  if (-1 == index) {
    return 0;
  }

  return ui->comboPage->currentData().toInt();
}

void
CDlgMdfRegister::setPage(uint16_t page)
{
  for (int i = 0; i < ui->comboPage->count(); i++) {
    if (page == ui->comboPage->itemData(i)) {
      ui->comboPage->setCurrentIndex(i);
    }
  }
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRegister::getOffset(void)
{
  return vscp_readStringValue(ui->editOffset->text().toStdString());
}

void
CDlgMdfRegister::setOffset(uint32_t offset)
{
  ui->editOffset->setText(QString("%1").arg(offset));
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegister::getType(void)
{
  return ui->comboPage->currentIndex();
}

void
CDlgMdfRegister::setType(uint8_t type)
{
  if (type <= MDF_REG_TYPE_BLOCK) {
    ui->comboPage->setCurrentIndex(type);
  }
}

// -----------------------------------------------------------------------

uint16_t
CDlgMdfRegister::getSpan(void)
{
  return ui->spinSpan->value();
}

void
CDlgMdfRegister::setSpan(uint16_t span)
{
  ui->spinSpan->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegister::getWidth(void)
{
  return ui->spinWidth->value();
}

void
CDlgMdfRegister::setWidth(uint8_t span)
{
  ui->spinWidth->setValue(span);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegister::getMin(void)
{
  return ui->spinMin->value();
}

void
CDlgMdfRegister::setMin(uint8_t min)
{
  ui->spinMin->setValue(min);
}

// -----------------------------------------------------------------------

uint8_t
CDlgMdfRegister::getMax(void)
{
  return ui->spinMax->value();
}

void
CDlgMdfRegister::setMax(uint8_t span)
{
  ui->spinMax->setValue(span);
}

// -----------------------------------------------------------------------

mdf_access_mode
CDlgMdfRegister::getAccess(void)
{
  return static_cast<mdf_access_mode>(ui->comboAccess->currentIndex());
}

void
CDlgMdfRegister::setAccess(uint8_t access)
{
  if (access <= MDF_REG_ACCESS_READ_WRITE) {
    ui->comboAccess->setCurrentIndex(access);
  }
}

// -----------------------------------------------------------------------

QString
CDlgMdfRegister::getDefault(void)
{
  return ui->editDefault->text();
}

void
CDlgMdfRegister::setDefault(QString& str)
{
  ui->editDefault->setText(str);
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRegister::getForegroundColor(void)
{
  return vscp_readStringValue(ui->editFgColor->text().toStdString());
}

void
CDlgMdfRegister::setForegroundColor(uint32_t color)
{
  ui->editFgColor->setText(QString("0x%1").arg(color, 8, 16, QChar('0')));
}

// -----------------------------------------------------------------------

uint32_t
CDlgMdfRegister::getBackgroundColor(void)
{
  return vscp_readStringValue(ui->editBgColor->text().toStdString());
}
void
CDlgMdfRegister::setBackgroundColor(uint32_t color)
{
  ui->editBgColor->setText(QString("0x%1").arg(color, 8, 16, QChar('0')));
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRegister::accept()
{
  std::string str;
  if (nullptr != m_preg) {
    m_preg->setName(getName().toStdString());
    m_preg->setPage(getPage());
    m_preg->setOffset(getOffset());
    m_preg->setType(static_cast<mdf_register_type>(getType()));
    m_preg->setSpan(getSpan());
    m_preg->setWidth(getWidth());
    m_preg->setMin(getMin());
    m_preg->setMax(getMax());
    m_preg->setAccess(getAccess());
    m_preg->setDefault(getDefault().toStdString());
    m_preg->setForegroundColor(getForegroundColor());
    m_preg->setBackgroundColor(getBackgroundColor());
  }
  else {
    spdlog::error("MDF module information - Invalid MDF object (accept)");
  }

  QDialog::accept();
}
