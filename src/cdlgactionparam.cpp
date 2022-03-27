// cdlgactionaparam.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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

#include "cdlgactionparam.h"
#include "ui_cdlgactionparam.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// QTableWidgetItemBits
//

QTableWidgetItemBits::QTableWidgetItemBits(const QString &text)
                                            : QTableWidgetItem(text)
{
    m_pos = 0;
    m_width = 0;
    m_mask = 0;
    m_value = 0;
}

///////////////////////////////////////////////////////////////////////////////
// QTableWidgetItemBits
//

QTableWidgetItemBits::~QTableWidgetItemBits(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgActionParam::CDlgActionParam(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgActionParam)
{
    ui->setupUi(this);

    ui->comboValues->hide();
    ui->labelValueList->hide();
    // Node id change
    // connect(ui->chkEnableRow,
    //       SIGNAL(stateChanged(int)),
    //       this,
    //       SLOT(enableRow_stateChanged(int)));

    connect(ui->tableWidgetBits,
            SIGNAL(cellChanged(int, int)),
            this,
            SLOT(valueChanged(int,int)));
    
    setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgActionParam::~CDlgActionParam()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void 
CDlgActionParam::setInitialFocus(void)
{
  //ui->editAddressOrigin->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// addValue
//

void 
CDlgActionParam::addValue(int value, std::string& name)
{
    ui->comboValues->addItem(QString::fromStdString(name), value);
}

///////////////////////////////////////////////////////////////////////////////
// addBitValue
//

void 
CDlgActionParam::addBitValue(std::deque<CMDF_Bit *> *pbitlist)
{
  QTableWidgetItem *pitem;
  std::string str;
  for (int i=0; i<pbitlist->size(); i++) {

    ui->tableWidgetBits->insertRow(i);
    str = QString::number(pbitlist->at(i)->getPos()).toStdString();
    if (pbitlist->at(i)->getWidth() > 1) {
      str += "-";
      str += QString::number(pbitlist->at(i)->getPos() + pbitlist->at(i)->getWidth() - 1 ).toStdString();
    }
    // Bits
    pitem = new QTableWidgetItem(str.c_str());
    pitem->setTextAlignment(Qt::AlignCenter);
    pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    ui->tableWidgetBits->setItem(i, 0, pitem);

    // Value
    uint8_t mask = 0;
    for (int k=pbitlist->at(i)->getPos(); k<pbitlist->at(i)->getPos() + pbitlist->at(i)->getWidth(); k++) {
      mask |= (1 << k);
    }

    uint8_t value = (m_actionParam & mask) >> pbitlist->at(i)->getPos();

    QTableWidgetItemBits *pitemValue = new QTableWidgetItemBits(QString::number(value));
    pitemValue->m_value = value;
    pitemValue->m_pos = pbitlist->at(i)->getPos();
    pitemValue->m_width = pbitlist->at(i)->getWidth();
    pitemValue->m_mask = mask;
    pitemValue->setTextAlignment(Qt::AlignCenter);
    pitemValue->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    std::string str = tr("Value can be 0 ").toStdString();
    if (1 == pbitlist->at(i)->getWidth()) {
      str += "or 1";
    }
    else {
      str += " - ";
      str += QString::number((1 << pbitlist->at(i)->getWidth())-1).toStdString();
    }    
    pitemValue->setToolTip(str.c_str());
    ui->tableWidgetBits->setItem(i, 1, pitemValue);

    // Name
    pitem = new QTableWidgetItem(pbitlist->at(i)->getName().c_str());
    pitem->setToolTip(QString::fromStdString(pbitlist->at(i)->getDescription()));
    pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    ui->tableWidgetBits->setItem(i, 2, pitem);
  }
}

///////////////////////////////////////////////////////////////////////////////
// showValues
//

void 
CDlgActionParam::showValues(bool bShow) 
{  
  ui->labelValueList->setVisible(bShow); 
  ui->comboValues->setVisible(bShow); 
}

///////////////////////////////////////////////////////////////////////////////
// showBits
//

void 
CDlgActionParam::showBits(bool bShow) 
{  
  ui->labelBitList->setVisible(bShow); 
  ui->tableWidgetBits->setVisible(bShow); 
}

///////////////////////////////////////////////////////////////////////////////
// valueChanged
//

void 
CDlgActionParam::valueChanged(int row, int column)
{
  QTableWidgetItemBits *pitem = (QTableWidgetItemBits *)ui->tableWidgetBits->item(row, column);
  if (NULL != pitem) {
    //uint8_t value = pitem->m_value;
    uint8_t mask = pitem->m_mask >> pitem->m_pos;
    uint8_t pos = pitem->m_pos;
    uint8_t width = pitem->m_width;
    uint8_t value = pitem->text().toUInt();
    value &= mask;
    pitem->setText(QString::number(value));
  }
}

