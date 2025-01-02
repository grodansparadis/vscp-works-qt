// cdlgactionaparam.cpp
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

#include "cdlgactionparam.h"
#include "ui_cdlgactionparam.h"

#include <QMessageBox>
#include <QSpinBox>

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
  m_pBit = nullptr;
  //m_pos = 0;
  //m_width = 0;
  m_mask = 0;
  m_value = 0;
}

///////////////////////////////////////////////////////////////////////////////
// QTableWidgetItemBits
//

QTableWidgetItemBits::~QTableWidgetItemBits(void)
{
  ;
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
          SIGNAL(cellClicked(int, int)),
          this,
          SLOT(cellClicked(int,int)));

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

    // -- Bits
    pitem = new QTableWidgetItem(str.c_str());
    pitem->setTextAlignment(Qt::AlignCenter);
    pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    ui->tableWidgetBits->setItem(i, 0, pitem);

    // -- Value

    // Construct mask
    uint8_t mask = 0;
    for (int k=pbitlist->at(i)->getPos(); k<pbitlist->at(i)->getPos() + pbitlist->at(i)->getWidth(); k++) {
      mask |= (1 << k);
    }

    uint8_t value = (m_actionParam & mask) >> pbitlist->at(i)->getPos();

    QTableWidgetItemBits *pitemValue = new QTableWidgetItemBits(QString::number(value));
    pitemValue->m_pBit = pbitlist->at(i);
    pitemValue->m_value = value;
    //pitemValue->m_pos = pbitlist->at(i)->getPos();
    //pitemValue->m_width = pbitlist->at(i)->getWidth();
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

    // -- Name
    pitem = new QTableWidgetItem(pbitlist->at(i)->getName().c_str());
    pitem->setToolTip(QString::fromStdString(pbitlist->at(i)->getDescription()));
    pitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    ui->tableWidgetBits->setItem(i, 2, pitem);
  }

  // ui->tableWidgetBits->insertRow(8);
  // pitem = new QTableWidgetItem();  
  // ui->tableWidgetBits->setItem(8, 2, pitem);

  // ui->tableWidgetBits->insertRow(9);
  // pitem = new QTableWidgetItem();  
  // ui->tableWidgetBits->setItem(9, 2, pitem);

  // QComboBox *pComboBox = new QComboBox();
  // pComboBox->addItem("OFF");
  // pComboBox->addItem("ON");
  // ui->tableWidgetBits->setCellWidget(8, 2, pComboBox);

  // QSpinBox *pSpinBox = new QSpinBox();
  // pSpinBox->setRange(0, 1);
  // ui->tableWidgetBits->setCellWidget(9, 2, pSpinBox);
}

///////////////////////////////////////////////////////////////////////////////
// cellClicked
//

void 
CDlgActionParam::cellClicked(int row, int column)
{
  QTableWidgetItemBits *pitem = (QTableWidgetItemBits *)ui->tableWidgetBits->item(row, BIT_COLUMN_VALUE);
  if (NULL == pitem) {
    ui->infoArea->setText("INTERNAL ERROR: No bit defintion.");
    return;
  }

  //ui->infoArea->setText(tr("Cell clicked: %1, %2").arg(row).arg(column));
  std::string str;
  str = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head>";
  str += "<meta name=\"qrichtext\" content=\"1\" />";
  str += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  str += "</head><body>";
  //str += "<body style=\"font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">";
  //str += "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
  //str += "<br /></p><p>";
  str += "<h4>";
  str += pitem->m_pBit->getName();
  str += "</h4>";
  str += "<p>";
  str += pitem->m_pBit->getDescription();
  str += "</p>";
  //str += tr("<b>Cell clicked:</b> %1, %2<br>").arg(row).arg(column).toStdString();
  str += tr("<b>Pos:</b> %1<br>").arg(pitem->m_pBit->getPos()).toStdString();
  str += tr("<b>Width:</b> %1<br>").arg(pitem->m_pBit->getWidth()).toStdString();
  str += tr("<b>Min:</b> %1<br>").arg(pitem->m_pBit->getMin()).toStdString();
  str += tr("<b>Max:</b> %1<br>").arg(pitem->m_pBit->getMax()).toStdString();
  str += tr("<b>Mask:</b> 0x%1<br>").arg(pitem->m_mask, 2, 16, QLatin1Char('0')).toStdString();
  str += tr("<b>Value in pos:</b> %1<br>").arg(pitem->m_value).toStdString();
  str += "</p>";
  str += "<p><a href=\"";
  str += pitem->m_pBit->getInfoURL();
  str += "\">";
  str += pitem->m_pBit->getInfoURL();
  str += "</a></p>";
  str += "<p>";
  std::deque<CMDF_Value *> *plistValues = pitem->m_pBit->getListValues();
  if (nullptr != plistValues) {
    for (int i=0; i<pitem->m_pBit->getListValues()->size(); i++) {
      str += "<b>";
      str += plistValues->at(i)->getValue();
      str += "</b> - ";
      str += plistValues->at(i)->getName();
      str += "<br>";
      str += plistValues->at(i)->getDescription();
      str += "<br>";
      str += plistValues->at(i)->getInfoURL();
      str += "\">";
      str += plistValues->at(i)->getInfoURL();
      str += "</a></p>";
    }
  }
  str += "</p>";
  str += "</body></html>";
  ui->infoArea->setHtml(str.c_str());
}


///////////////////////////////////////////////////////////////////////////////
// valueChanged
//

void 
CDlgActionParam::valueChanged(int row, int column)
{
  if (BIT_COLUMN_VALUE != column) {
    return;
  }

  QTableWidgetItemBits *pitem = (QTableWidgetItemBits *)ui->tableWidgetBits->item(row, BIT_COLUMN_VALUE);
  if (NULL != pitem) {
    uint8_t mask = pitem->m_mask >> pitem->m_pBit->getPos();
    //std::cout << "Row: " << row << "Value: " << pitem->text().toStdString() << std::endl;
    uint8_t value = vscp_readStringValue(pitem->text().toStdString());
    value &= mask;
    pitem->setText(QString::number(value));
    pitem->m_value = (value << pitem->m_pBit->getPos());
  }
}

///////////////////////////////////////////////////////////////////////////////
// getActionParameter
//

uint8_t 
CDlgActionParam::getActionParameter(void)
{
  //Calculate new action parameter
  m_actionParam = 0;
  for (int i=0; i<ui->tableWidgetBits->rowCount(); i++) {
    QTableWidgetItemBits *pitemtbl = (QTableWidgetItemBits *)ui->tableWidgetBits->item(i, BIT_COLUMN_VALUE);
    m_actionParam += pitemtbl->m_value;
  }
  return m_actionParam;
}