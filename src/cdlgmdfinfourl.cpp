// cdlgmdfinfourl.cpp
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

#include "cdlgmdfinfourl.h"
#include "ui_cdlgmdfinfourl.h"

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

CDlgMdfInfoUrl::CDlgMdfInfoUrl(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfInfoUrl)
{
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  // https://stackoverflow.com/questions/26307729/get-list-of-languages-in-qt5
  QList<QLocale> allLocales = QLocale::matchingLocales(
    QLocale::AnyLanguage,
    QLocale::AnyScript,
    QLocale::AnyCountry);

  QStringList iso639LanguageCodes;
  QStringList iso639Languages;

  for (const QLocale& locale : allLocales) {
    iso639Languages << QLocale::languageToString(locale.language());
    iso639LanguageCodes << locale.name().split('_').first().left(2);
  }

  iso639LanguageCodes.removeDuplicates();
  iso639LanguageCodes.sort();
  iso639LanguageCodes.removeOne("C");

  for (auto item = iso639LanguageCodes.begin(), end = iso639LanguageCodes.end(); item != end; ++item) {
    // qDebug() << *item;
    ui->comboBoxLang->addItem(*item);
  }

  connect(ui->btnSetLanguage, &QToolButton::clicked, this, &CDlgMdfInfoUrl::setLang);

  setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfInfoUrl::~CDlgMdfInfoUrl()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfInfoUrl::initDialogData(std::map<std::string, std::string>* pmap, QString* pselstr)
{
  QString str;

  if (nullptr == pmap) {
    spdlog::error("MDF module information - Invalid MDF object (initDialogData)");
    return;
  }

  // Save the map pbject
  m_pMapInfo = pmap;

  QString lang = (nullptr == pselstr) ? "en" : *pselstr;

  if (nullptr != pselstr) {
    std::string infourl = pmap->at(pselstr->toStdString());
    ui->editLanguage->setText(pselstr->toStdString().c_str());
    ui->editInfoUrl->setText(infourl.c_str());
  }
  else {
    ui->editLanguage->setText("en");
  }

  int idx = ui->comboBoxLang->findText(lang.toStdString().c_str());
  if (-1 != idx) {
    ui->comboBoxLang->setCurrentIndex(idx);
  }
}



///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfInfoUrl::setInitialFocus(void)
{
  ui->editInfoUrl->setFocus();
}

// ----------------------------------------------------------------------------
//                             Getters & Setters
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfInfoUrl::accept()
{
  std::string str;
  if (nullptr != m_pMapInfo) {

    str = ui->editLanguage->text().trimmed().left(2).toStdString();

    if (str.length() < 2) {
      QMessageBox::warning(this, tr("vscpworks+"), tr("Invalid description object. Language must be set to IOS639 value."), QMessageBox::Ok);
      return;
    }

    // If selstr has been change in edit mode we have to take
    // special care (we remove the old key).
    if (m_initial_selstr.length() && (m_initial_selstr != str.c_str())) {
      m_pMapInfo->erase(m_initial_selstr.toStdString());
    }

    (*m_pMapInfo)[str] = ui->editInfoUrl->toPlainText().toStdString();
  }
  else {
    spdlog::error("MDF module information - Invalid MDF Info URL object (accept)");
    QMessageBox::warning(this, tr("vscpworks+"), tr("Invalid Info URL object. Unable to save data."), QMessageBox::Ok);
  }

  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// setLang
//

void
CDlgMdfInfoUrl::setLang()
{
  ui->editLanguage->setText(ui->comboBoxLang->currentText());
}
