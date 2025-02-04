// cdlgmdfcontact.cpp
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

#include "cdlgmdfcontact.h"
#include "ui_cdlgmdfcontact.h"

#include "cdlgmdfdescription.h"
#include "cdlgmdfinfourl.h"

#include <QDebug>
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

CDlgMdfContact::CDlgMdfContact(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfContact)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddDesc, &QToolButton::clicked, this, &CDlgMdfContact::addDesc);
  connect(ui->btnEditDesc, &QToolButton::clicked, this, &CDlgMdfContact::editDesc);
  connect(ui->btnDupDesc, &QToolButton::clicked, this, &CDlgMdfContact::dupDesc);
  connect(ui->btnDelDesc, &QToolButton::clicked, this, &CDlgMdfContact::deleteDesc);

  connect(ui->btnAddInfo, &QToolButton::clicked, this, &CDlgMdfContact::addInfo);
  connect(ui->btnEditInfo, &QToolButton::clicked, this, &CDlgMdfContact::editInfo);
  connect(ui->btnDupInfo, &QToolButton::clicked, this, &CDlgMdfContact::dupInfo);
  connect(ui->btnDelInfo, &QToolButton::clicked, this, &CDlgMdfContact::deleteInfo);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfContact::~CDlgMdfContact()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfContact::initDialogData(const CMDF_Object* pitemobj, mdf_dlg_contact_type type, QString title)
{
  QString str;

  if (nullptr == pitemobj) {
    spdlog::error("MDF contact information - Invalid MDF object (initDialogData)");
    return;
  }

  setWindowTitle(title);

  m_pitemobj = (CMDF_Item*)pitemobj;

  int idx = static_cast<int>(type);
  ui->comboType->setCurrentIndex(idx);
  ui->comboType->setEnabled(false);

  ui->editValue->setText(m_pitemobj->getName().c_str());

  // Fill in descriptions
  fillDescription();

  // Fill in help URL's
  fillInfoUrl();
}

///////////////////////////////////////////////////////////////////////////////
// fillDescription
//

void
CDlgMdfContact::fillDescription(void)
{
  std::map<std::string, std::string>* pmapDescription = m_pitemobj->getMapDescription();
  std::map<std::string, std::string>::iterator itDesc = pmapDescription->begin();
  while (itDesc != pmapDescription->end()) {
    std::string lang        = itDesc->first; // key
    std::string description = itDesc->second;
    QString str             = lang.c_str() + tr(" - ") + description.c_str();
    ui->listDescription->addItem(str);
    itDesc++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillInfoUrl
//

void
CDlgMdfContact::fillInfoUrl(void)
{
  std::map<std::string, std::string>* pmapHelpUrl     = m_pitemobj->getMapInfoUrl();
  std::map<std::string, std::string>::iterator itInfo = pmapHelpUrl->begin();
  while (itInfo != pmapHelpUrl->end()) {
    std::string lang = itInfo->first; // key
    std::string info = itInfo->second;
    QString str      = lang.c_str() + tr(" - ") + info.c_str();
    ui->listInfo->addItem(str);
    itInfo++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfContact::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgMdfContact::getValue(void)
{
  return (ui->editValue->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgMdfContact::setValue(const QString& str)
{
  ui->editValue->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// addDesc
//

void
CDlgMdfContact::addDesc(void)
{
  QString selstr = "en"; // Default language

  CDlgMdfDescription dlg(this);
  dlg.initDialogData(m_pitemobj->getMapDescription() /*, &selstr*/);
  if (QDialog::Accepted == dlg.exec()) {
    ui->listDescription->clear();
    fillDescription();
  }
}

///////////////////////////////////////////////////////////////////////////////
// editDesc
//

void
CDlgMdfContact::editDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {

    // Save the row
    int idx = ui->listDescription->currentRow();

    QListWidgetItem* pitem = ui->listDescription->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pitemobj->getMapDescription(), &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listDescription->clear();
      fillDescription();
      ui->listDescription->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupDesc
//

void
CDlgMdfContact::dupDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {
    CDlgMdfDescription dlg(this);
    dlg.initDialogData(m_pitemobj->getMapDescription());
    if (QDialog::Accepted == dlg.exec()) {
      ui->listDescription->clear();
      fillDescription();
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteDesc
//

void
CDlgMdfContact::deleteDesc(void)
{
  if (-1 != ui->listDescription->currentRow()) {

    // Save the row
    int idx = ui->listDescription->currentRow();

    QListWidgetItem* pitem = ui->listDescription->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    m_pitemobj->getMapDescription()->erase(selstr.toStdString());
    ui->listDescription->clear();
    fillDescription();
  }
}

///////////////////////////////////////////////////////////////////////////////
// addInfo
//

void
CDlgMdfContact::addInfo(void)
{
  QString selstr = "en"; // Default language

  CDlgMdfInfoUrl dlg(this);
  dlg.initDialogData(m_pitemobj->getMapInfoUrl() /*, &selstr*/);
  if (QDialog::Accepted == dlg.exec()) {
    ui->listInfo->clear();
    fillInfoUrl();
  }
}

///////////////////////////////////////////////////////////////////////////////
// editInfo
//

void
CDlgMdfContact::editInfo(void)
{
  if (-1 != ui->listInfo->currentRow()) {

    // Save the row
    int idx = ui->listInfo->currentRow();

    QListWidgetItem* pitem = ui->listInfo->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    CDlgMdfInfoUrl dlg(this);
    dlg.initDialogData(m_pitemobj->getMapInfoUrl(), &selstr);
    if (QDialog::Accepted == dlg.exec()) {
      ui->listInfo->clear();
      fillInfoUrl();
      ui->listInfo->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupInfo
//

void
CDlgMdfContact::dupInfo(void)
{
  if (-1 != ui->listInfo->currentRow()) {
    CDlgMdfInfoUrl dlg(this);
    dlg.initDialogData(m_pitemobj->getMapInfoUrl());
    if (QDialog::Accepted == dlg.exec()) {
      ui->listInfo->clear();
      fillInfoUrl();
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteInfo
//

void
CDlgMdfContact::deleteInfo(void)
{
  if (-1 != ui->listInfo->currentRow()) {

    // Save the row
    int idx = ui->listInfo->currentRow();

    QListWidgetItem* pitem = ui->listInfo->currentItem();
    QString selstr         = pitem->text().split('_').first().left(2);

    m_pitemobj->getMapInfoUrl()->erase(selstr.toStdString());
    ui->listInfo->clear();
    fillInfoUrl();
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfContact::accept()
{
  std::string str;
  if (nullptr != m_pitemobj) {

    // str = ui->editName->text().toStdString();
    // m_pmdf->setModuleName(str);

    // str = ui->editModel->text().toStdString();
    // m_pmdf->setModuleModel(str);

    // m_pmdf->setModuleLevel(ui->comboModuleLevel->currentIndex());

    // str = ui->editVersion->text().toStdString();
    // m_pmdf->setModuleVersion(str);

    // str = ui->editDate->text().toStdString();
    // m_pmdf->setModuleChangeDate(str);

    // m_pmdf->setModuleBufferSize(ui->editBufferSize->value());

    // str = ui->editCopyright->text().toStdString();
    // m_pmdf->setModuleCopyright(str);
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
CDlgMdfContact::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/mdf";
  QDesktopServices::openUrl(QUrl(link));
}