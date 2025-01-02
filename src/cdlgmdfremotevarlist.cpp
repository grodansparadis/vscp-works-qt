// cdlgmdfremotevarlist.cpp
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
#include "cdlgmdfremotevar.h"
#include "cdlgmdfremotevarlist.h"
#include "ui_cdlgmdfremotevarlist.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgMdfRemoteVarList::CDlgMdfRemoteVarList(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgMdfRemoteVarList)
{
  m_pmdf = nullptr;
  ui->setupUi(this);

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

  connect(ui->btnAddRemoteVar, &QToolButton::clicked, this, &CDlgMdfRemoteVarList::addRemoteVar);
  connect(ui->btnEditRemoteVar, &QToolButton::clicked, this, &CDlgMdfRemoteVarList::editRemoteVar);
  connect(ui->btnDupRemoteVar, &QToolButton::clicked, this, &CDlgMdfRemoteVarList::dupRemoteVar);
  connect(ui->btnDelRemoteVar, &QToolButton::clicked, this, &CDlgMdfRemoteVarList::deleteRemoteVar);

  connect(ui->listRemoteVar, &QListWidget::doubleClicked, this, &CDlgMdfRemoteVarList::editRemoteVar);

  setInitialFocus();
  this->setFixedSize(this->size());
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgMdfRemoteVarList::~CDlgMdfRemoteVarList()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// initDialogData
//

void
CDlgMdfRemoteVarList::initDialogData(CMDF* pmdf, uint16_t page)
{
  QString str;

  if (nullptr == pmdf) {
    QMessageBox::critical(this, tr("MDF contact information"), tr("Invalid MDF manufacturing object"));
    spdlog::error("MDF contact information - Invalid MDF manufacturing object");
    return;
  }

  // Save MDF  and page
  m_pmdf = pmdf;

  // m_pmdf->getRegisterMap(m_page, pages);

  // Fill in defined register items
  renderRemoteVarItems();

  // Fill the page combo with page information
  // renderComboPage();

  // connect(ui->comboPage, SIGNAL(currentIndexChanged(int)), this, SLOT(onPageComboChange(int)));
}

///////////////////////////////////////////////////////////////////////////////
// renderComboPage
//

// void
// CDlgMdfRemoteVarList::renderComboPage(void)
// {
//   setWindowTitle(tr("Registers for %1").arg(m_page));

//   // Fill available pages in combo
//   std::set<uint16_t> pages;
//   uint32_t cnt = m_pmdf->getPages(pages);
//   ui->comboPage->clear();
//   int pos = 0;
//   for (std::set<uint16_t>::iterator it = pages.begin(); it != pages.end(); ++it) {
//     ui->comboPage->addItem(QString("Page %1").arg(*it), *it);
//     if (m_page == *it) {
//       ui->comboPage->setCurrentIndex(pos);
//     }
//     pos++;
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// renderRemoteVarItems
//

void
CDlgMdfRemoteVarList::renderRemoteVarItems(void)
{
  std::map<uint64_t, CMDF_RemoteVariable*> rvmap;
  std::set<uint64_t> sortedSet;

  if (nullptr == m_pmdf) {
    return;
  }

  std::deque<CMDF_RemoteVariable*>* rvset = m_pmdf->getRemoteVariableList();

  ui->listRemoteVar->clear();

  // Make sorted set of registers on this page
  for (auto it = rvset->cbegin(); it != rvset->cend(); ++it) {
    uint64_t token = ((uint64_t)((*it)->getPage()) << 32) + (*it)->getOffset();
    //qDebug() << Qt::hex << token << (*it)->getPage() << (*it)->getOffset();
    sortedSet.insert(token + (*it)->getOffset());
    rvmap[token + (*it)->getOffset()] = *it;
  }

  //qDebug() << rvset->size();

  for (auto it1 = sortedSet.cbegin(); it1 != sortedSet.cend(); ++it1) {
    CMDF_RemoteVariable* prvar = rvmap[*it1];
    if (nullptr != prvar) {
      QString str = QString("%1 [%2:%3]").arg(prvar->getName().c_str()).arg(prvar->getPage()).arg(prvar->getOffset());
      ui->listRemoteVar->addItem(str);
      // Set data to register index
      ui->listRemoteVar->item(ui->listRemoteVar->count() - 1)->setData(Qt::UserRole, prvar->getOffset());
      ui->listRemoteVar->item(ui->listRemoteVar->count() - 1)->setData(Qt::UserRole + 1, prvar->getPage());
    }
  }

  // for (auto it = rvset->cbegin(); it != rvset->cend(); ++it) {
  //   // m_registersSet.insert((*it)->getOffset());
  //   CMDF_RemoteVariable* prvar = *it;
  //   if (nullptr != prvar) {
  //     QString str = QString("%1 [%2:%3]").arg(prvar->getName().c_str()).arg(prvar->getPage()).arg(prvar->getOffset());
  //     ui->listRemoteVar->addItem(str);
  //     // Set data to register index
  //     ui->listRemoteVar->item(ui->listRemoteVar->count() - 1)->setData(Qt::UserRole, prvar->getOffset());
  //     ui->listRemoteVar->item(ui->listRemoteVar->count() - 1)->setData(Qt::UserRole+1, prvar->getPage());
  //   }
  // }

  // for (auto it = regs->cbegin(); it != regs->cend(); ++it) {
  //   m_registersSet.insert((*it)->getOffset());
  //   if ((*it)->getPage() == m_page) {
  //     QString str = QString("Register  %1 -- %2").arg((*it)->getOffset()).arg((*it)->getName().c_str());
  //     ui->listRemoteVar->addItem(str);
  //     // Set data to register index
  //     ui->listRemoteVar->item(ui->listRemoteVar->count() - 1)->setData(Qt::UserRole, (*it)->getOffset());
  //   }
  // }
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgMdfRemoteVarList::setInitialFocus(void)
{
  // ui->editGuid->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// addRemoteVar
//

void
CDlgMdfRemoteVarList::addRemoteVar(void)
{
  bool ok;
  CMDF_RemoteVariable* pvarnew = new CMDF_RemoteVariable();

  // Save the selected row
  int idx = ui->listRemoteVar->currentRow();

  CDlgMdfRemoteVar dlg(this);
  dlg.initDialogData(m_pmdf, pvarnew);
addregdlg:
  if (QDialog::Accepted == dlg.exec()) {
    // Check if register is already defined
    // CMDF_RemoteVariable* preg = m_pmdf->getRemoteVariable(pvarnew->getOffset(), pvarnew->getPage());
    // if (nullptr != preg) {
    //   QMessageBox::warning(this, tr("MDF add new register"), tr("Register page=%1 offset=%2 is already define. Must be unique.").arg(pvarnew->getPage()).arg(pvarnew->getOffset()));
    //   goto addregdlg;
    // }
    // qDebug() << "Page=" << pvarnew->getPage() << " Offset=" << pvarnew->getOffset();
    m_pmdf->getRemoteVariableList()->push_back(pvarnew);
    ui->listRemoteVar->clear();
    renderRemoteVarItems();
    if (-1 != idx) {
      ui->listRemoteVar->setCurrentRow(idx);
    }

  }
  else {
    delete pvarnew;
  }
}

///////////////////////////////////////////////////////////////////////////////
// editRemoteVar
//

void
CDlgMdfRemoteVarList::editRemoteVar(void)
{
  bool ok;

  if (-1 != ui->listRemoteVar->currentRow()) {

    // Save the selected row
    int idx = ui->listRemoteVar->currentRow();

    QListWidgetItem* pitem    = ui->listRemoteVar->currentItem();
    CMDF_RemoteVariable* pvar = m_pmdf->getRemoteVariable(pitem->data(Qt::UserRole).toUInt(), 
                                                            pitem->data(Qt::UserRole + 1).toUInt());

    CDlgMdfRemoteVar dlg(this);
    dlg.initDialogData(m_pmdf, pvar);
    // Don't allow editing of page and offset
    dlg.setReadOnly();
    if (QDialog::Accepted == dlg.exec()) {
      ui->listRemoteVar->clear();
      renderRemoteVarItems();
      ui->listRemoteVar->setCurrentRow(idx);
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }
}

///////////////////////////////////////////////////////////////////////////////
// dupRemoteVar
//

void
CDlgMdfRemoteVarList::dupRemoteVar(void)
{
  if (-1 != ui->listRemoteVar->currentRow()) {

    // Save the selected row
    int idx = ui->listRemoteVar->currentRow();

    QListWidgetItem* pitem    = ui->listRemoteVar->currentItem();
    CMDF_RemoteVariable* prvar = m_pmdf->getRemoteVariable(pitem->data(Qt::UserRole).toUInt(), pitem->data(Qt::UserRole + 1).toUInt());
    CMDF_RemoteVariable* prvarnew = new CMDF_RemoteVariable();

    // Make copy
    *prvarnew = *prvar;

    CDlgMdfRemoteVar dlg(this);
    dlg.initDialogData(m_pmdf, prvarnew);
  dupregdlg:
    if (QDialog::Accepted == dlg.exec()) {
      // Check if register is already defined
      CMDF_RemoteVariable* pvarold = m_pmdf->getRemoteVariable(prvarnew->getOffset(), prvarnew->getPage());
      if (nullptr != pvarold) {
        QMessageBox::warning(this, tr("MDF duplicate register"), tr("Register page=%1 offset=%2 is already define. Must be unique.").arg(prvarnew->getPage()).arg(prvarnew->getOffset()));
        goto dupregdlg;
      }
      qDebug() << "Page=" << prvarnew->getPage() << " Offset=" << prvarnew->getOffset();
      m_pmdf->getRemoteVariableList()->push_back(prvarnew);

      ui->listRemoteVar->clear();
      renderRemoteVarItems();
      if (-1 != idx) {
        ui->listRemoteVar->setCurrentRow(idx);
      }
    }
    else {
      delete prvarnew;
    }
  }
  else {
    QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  }

  // if (-1 != ui->listContact->currentRow()) {

  //   // Save the selected row
  //   int idx = ui->listContact->currentRow();

  //   QListWidgetItem* pitem = ui->listContact->currentItem();
  //   QString selstr         = pitem->text().split('_').first().left(2);

  //   std::string str = QInputDialog::getText(this,
  //                                           tr("Edit contact item"),
  //                                           tr("Contact value:"),
  //                                           QLineEdit::Normal,
  //                                           pitem->text(),
  //                                           &ok)
  //                       .toStdString();

  //   if (ok && str.length()) {

  //     CMDF_Item* pitem = new CMDF_Item();
  //     if (nullptr != pitem) {
  //       pitem->setValue(str);
  //       m_pContactList->push_back(pitem);
  //       ui->listContact->clear();
  //       fillContactItems();
  //     }
  //     else {
  //       QMessageBox::warning(this, tr(APPNAME), tr("Memory problem could not add item"), QMessageBox::Ok);
  //     }
  //   }
  // }
  // else {
  //   QMessageBox::warning(this, tr(APPNAME), tr("An item must be selected"), QMessageBox::Ok);
  // }
}

///////////////////////////////////////////////////////////////////////////////
// deleteRemoteVar
//

void
CDlgMdfRemoteVarList::deleteRemoteVar(void)
{
  if (-1 != ui->listRemoteVar->currentRow()) {

    // Save the row
    int idx = ui->listRemoteVar->currentRow();

    QListWidgetItem* pitem    = ui->listRemoteVar->currentItem();
    CMDF_RemoteVariable* pvar = m_pmdf->getRemoteVariable(pitem->data(Qt::UserRole).toUInt(), pitem->data(Qt::UserRole + 1).toUInt());
    m_pmdf->deleteRemoteVariable(pvar);
    //delete pvar;
    ui->listRemoteVar->removeItemWidget(pitem);
    renderRemoteVarItems();
    ui->listRemoteVar->setCurrentRow(idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgMdfRemoteVarList::accept()
{
  std::string str;
  if (nullptr != m_pmdf) {

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
