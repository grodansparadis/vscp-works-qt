// cdlgeditdm.h
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

#include "cdlgeditdm.h"
#include "ui_cdlgeditdm.h"
#include "cdlglevel1filterwizard.h"
#include "cdlgactionparam.h"

#include <QMessageBox>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgEditDm::CDlgEditDm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgEditDm)
{
    ui->setupUi(this);

    //m_pDM = nullptr;
    m_pMDF = nullptr;

    // Node id change
    connect(ui->chkEnableRow,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(enableRow_stateChanged(int)));

    // Node id change
    connect(ui->chkEnableRow,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(enableRow_stateChanged(int)));

    // Origin address check
    connect(ui->chkOriginAddressMatch,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(originAddressMatch_stateChanged(int)));

    // Origin address is hardcoded
    connect(ui->chkOriginHardcoded,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(originHardcoded_stateChanged(int)));

    // Match zone
    connect(ui->chkMatchZone,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(matchZone_stateChanged(int)));

    // Match subzone
    connect(ui->chkMatchSubzone,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(matchSubzone_stateChanged(int)));

    // Class mask bit 8 changed
    connect(ui->chkClassMaskBit8,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(classMaskBit8_stateChanged(int)));

    // Class filter bit 8 changed
    connect(ui->chkClassFilterBit8,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(classFilterBit8_stateChanged(int)));       
  
    connect(ui->btnFilterWizard,
            SIGNAL(clicked(bool)),
            this,
            SLOT(filterWizard(void)));

    connect(ui->btnActionParameterWizard,
            SIGNAL(clicked()),
            this,
            SLOT(actionParameterWizard()));

    connect(ui->comboAction,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(currentIndexChangedActions(int)));            
    
    setInitialFocus();
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgEditDm::~CDlgEditDm()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void 
CDlgEditDm::setInitialFocus(void)
{
    ui->editAddressOrigin->setFocus();
}

///////////////////////////////////////////////////////////////////////////////
// flagsToValue
//

void 
CDlgEditDm::flagsToValue(void)
{
  uint8_t value = 0;

  if (ui->chkEnableRow->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_ENABLED;
  }

  if (ui->chkOriginAddressMatch->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_ORIGIN_MATCH;
  }

  if (ui->chkOriginHardcoded->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_ORIGIN_HARDCODED;
  }

  if (ui->chkMatchZone->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_MATCH_ZONE;
  }

  if (ui->chkMatchSubzone->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_MATCH_SUBZONE;
  }

  if (ui->chkClassMaskBit8->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_CLASS_MASK_BIT8;
  }

  if (ui->chkClassFilterBit8->isChecked()) {
    value |= VSCP_LEVEL1_DM_FLAG_CLASS_FILTER_BIT8;
  }

  QString str = "0x" + QString::number(value, 16).toUpper();
  str += " (" + QString::number(value, 10) + ")";
  ui->labelFlags->setText(str);  
}

///////////////////////////////////////////////////////////////////////////////
// valueToFlags
//

void 
CDlgEditDm::valueToFlags(void)
{
  uint8_t value = vscp_readStringValue(ui->labelFlags->text().toStdString());

  // Enable
  if (value & VSCP_LEVEL1_DM_FLAG_ENABLED) {
    ui->chkEnableRow->setChecked(true);
  }
  else {
    ui->chkEnableRow->setChecked(false);
  }

  // Origin address match
  if (value & VSCP_LEVEL1_DM_FLAG_ORIGIN_MATCH) {
    ui->chkOriginAddressMatch->setChecked(true);
  }
  else {
    ui->chkOriginAddressMatch->setChecked(false);
  }

  // Origin hard coded
  if (value & VSCP_LEVEL1_DM_FLAG_ORIGIN_HARDCODED) {
    ui->chkOriginHardcoded->setChecked(true);
  }
  else {
    ui->chkOriginHardcoded->setChecked(false);
  }

  // Match zone
  if (value & VSCP_LEVEL1_DM_FLAG_MATCH_ZONE) {
    ui->chkMatchZone->setChecked(true);
  }
  else {
    ui->chkMatchZone->setChecked(false);
  }

  // Match sub zone
  if (value & VSCP_LEVEL1_DM_FLAG_MATCH_SUBZONE) {
    ui->chkMatchSubzone->setChecked(true);
  }
  else {
    ui->chkMatchSubzone->setChecked(false);
  }

  // CLASS Mask bit 8
  if (value & VSCP_LEVEL1_DM_FLAG_CLASS_MASK_BIT8) {
    ui->chkClassMaskBit8->setChecked(true);
  }
  else {
    ui->chkClassMaskBit8->setChecked(false);
  }

  // CLASS Filter bit 8
  if (value & VSCP_LEVEL1_DM_FLAG_CLASS_FILTER_BIT8) {
    ui->chkClassFilterBit8->setChecked(true);
  }
  else {
    ui->chkClassFilterBit8->setChecked(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setDm
//

void 
//CDlgEditDm::setDm(CMDF_DecisionMatrix *pDM)
CDlgEditDm::setMDF(CMDF *pMDF)
{
  if (nullptr == pMDF) {
    return;
  }

  m_pMDF = pMDF;

  // Fill in action combo box
  if (nullptr != pMDF->getDM()) {
    ui->comboAction->clear();
    for (auto const& item : *pMDF->getDM()->getActionList()) {
      ui->comboAction->addItem(item->getName().c_str(), QVariant(item->getCode()));
    }  
    fillHtmlInfo();
  }
  else {
    ui->comboAction->clear();
  }
}

///////////////////////////////////////////////////////////////////////////////
// enableRow_stateChanged
//

void 
CDlgEditDm::enableRow_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// originAddressMatch_stateChanged
//

void 
CDlgEditDm::originAddressMatch_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// originHardcoded_stateChanged
//

void 
CDlgEditDm::originHardcoded_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// matchZone_stateChanged
//

void 
CDlgEditDm::matchZone_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// matchSubzone_stateChanged
//

void 
CDlgEditDm::matchSubzone_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// classMaskBit8_stateChanged
//

void 
CDlgEditDm::classMaskBit8_stateChanged(int state)
{
  flagsToValue();  
}

///////////////////////////////////////////////////////////////////////////////
// classFilterBit8_stateChanged
//

void 
CDlgEditDm::classFilterBit8_stateChanged(int state)
{
  flagsToValue();  
}



///////////////////////////////////////////////////////////////////////////////
// getDmAddressOrigin
//

uint8_t CDlgEditDm::getDmAddressOrigin(void)
{
  return vscp_readStringValue(ui->editAddressOrigin->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmAddressOrigin
//

void CDlgEditDm::setDmAddressOrigin(const std::string& str)
{
  ui->editAddressOrigin->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDmFlags
//

uint8_t CDlgEditDm::getDmFlags(void)
{   
  flagsToValue();
  return vscp_readStringValue(ui->labelFlags->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setDmFlags
//

void CDlgEditDm::setDmFlags(const std::string& str)
{
  ui->labelFlags->setText(str.c_str());
  valueToFlags();
}

///////////////////////////////////////////////////////////////////////////////
// getDmClassMask
//

uint8_t CDlgEditDm::getDmClassMask(void)
{
  return vscp_readStringValue(ui->editClassMask->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmClassMask
//

void CDlgEditDm::setDmClassMask(const std::string& str)
{
  ui->editClassMask->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDmClassFilter
//

uint8_t CDlgEditDm::getDmClassFilter(void)
{
  return vscp_readStringValue(ui->editClassFilter->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmClassFilter
//

void CDlgEditDm::setDmClassFilter(const std::string& str)
{
  ui->editClassFilter->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDmTypeMask
//

uint8_t CDlgEditDm::getDmTypeMask(void)
{
  return vscp_readStringValue(ui->editTypeMask->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmTypeMask
//

void CDlgEditDm::setDmTypeMask(const std::string& str)
{
  ui->editTypeMask->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDmTypFilter
//

uint8_t CDlgEditDm::getDmTypeFilter(void)
{
  return vscp_readStringValue(ui->editTypeFilter->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmTypeFilter
//

void CDlgEditDm::setDmTypeFilter(const std::string& str)
{
  ui->editTypeFilter->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDmAction
//

uint8_t CDlgEditDm::getDmAction(void)
{
  return ui->comboAction->currentData().toInt();
}

///////////////////////////////////////////////////////////////////////////////
// setDmAction
//

void CDlgEditDm::setDmAction(int idx)
{
  ui->comboAction->setCurrentIndex(idx);
}

///////////////////////////////////////////////////////////////////////////////
// setDmActionFromCode
//

void CDlgEditDm::setDmActionFromCode(uint8_t code)
{
  for (int i=0; i<ui->comboAction->count(); i++) {
    if (ui->comboAction->itemData(i).toInt() == code) {
      ui->comboAction->setCurrentIndex(i);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// getDmActionParameter
//

uint8_t CDlgEditDm::getDmActionParameter(void)
{
  return vscp_readStringValue(ui->editActionParameter->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setDmActionParameter
//

void CDlgEditDm::setDmActionParameter(const std::string& str)
{
  ui->editActionParameter->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// currentIndexChangedActions
//

void 
CDlgEditDm::currentIndexChangedActions(int index)
{
  if (-1 == index) {
    return;
  }
  
  std::deque<CMDF_Action *> *actionList = m_pMDF->getDM()->getActionList();
  CMDF_Action *pAction = actionList->at(ui->comboAction->currentIndex());
  std::deque<CMDF_ActionParameter *> *pActionParams = pAction->getListActionParameter();

}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void 
CDlgEditDm::accept(void)
{
  //saveData();
  done(QDialog::Accepted);
}

/*
///////////////////////////////////////////////////////////////////////////////
// reject
//

void CDlgEditDm::reject(void)
{
  reject();  
} */

///////////////////////////////////////////////////////////////////////////////
// filterWizard
//

void 
CDlgEditDm::filterWizard(void)
{
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();
  CDlgLevel1FilterWizard dlg(this);

  dlg.setVscpPriorityFilter(0);
  dlg.setVscpPriorityMask(0);
  dlg.disablePriorityFields();

  dlg.setVscpClassFilter(vscp_readStringValue(ui->editClassMask->text().toStdString()));
  dlg.setVscpClassMask(vscp_readStringValue(ui->editClassFilter->text().toStdString()));
    
  dlg.setVscpTypeFilter(vscp_readStringValue(ui->editTypeFilter->text().toStdString()));
  dlg.setVscpTypeMask(vscp_readStringValue(ui->editTypeMask->text().toStdString()));
    
  dlg.setVscpNodeIdFilter(0);
  dlg.setVscpNodeIdMask(0);
  dlg.disableNodeIdFields();

  //dlg.setNumBaseComboIndex(parent()->ui->m_baseComboBox->currentIndex());
    
  if (QDialog::Accepted == dlg.exec()) {

    //setVscpPriorityFilter( dlg.getVscpPriorityFilter() );
    //setVscpPriorityMask( dlg.getVscpPriorityMask() );

    ui->editClassFilter->setText(pworks->decimalToStringInBase(dlg.getVscpClassFilter()));
    ui->editClassMask->setText(pworks->decimalToStringInBase(dlg.getVscpClassMask()));

    ui->editTypeFilter->setText(pworks->decimalToStringInBase(dlg.getVscpTypeFilter()));
    ui->editTypeMask->setText(pworks->decimalToStringInBase(dlg.getVscpTypeMask()));

    //setVscpNodeIdFilter( dlg.getVscpNodeIdFilter() );
    //setVscpNodeIdMask( dlg.getVscpNodeIdMask() );

    //ui->m_baseComboBox->setCurrentIndex(dlg.getNumComboIndex());
  
  }
}

///////////////////////////////////////////////////////////////////////////////
// fillHtmlInfo
//

void 
CDlgEditDm::fillHtmlInfo(void)
{  
  QString html;
  std::string str;
  vscpworks *pworks = (vscpworks *)QCoreApplication::instance();

  html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">";
  html += "<html><head>";
  html += "<meta name=\"qrichtext\" content=\"1\" />";
  html += "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>";
  html += "</head>";
  html += "<body style=\"font-family:'Ubuntu'; font-size:11pt; "
          "font-weight:400; font-style:normal;\">";
  html += "<h2>";
  html += tr("Decision Matrix");
  html += "</h2>";

  if (nullptr == m_pMDF->getDM()) {
    html += "<p>There is no decsion matrix data.</p>";
  }
  else {
    // rows
    html += "<p><b>";
    html += tr("Number of rows : </b><font color=\"#009900\">");
    html += QString::number((int)m_pMDF->getDM()->getRowCount());
    html += "</font><br>";

    // row size
    html += "<b>";
    html += tr("Row size : </b><font color=\"#009900\">");
    html += QString::number((int)m_pMDF->getDM()->getRowSize());
    html += "</font></p>";

    // Actions
    html += "<h2><font color=\"#1a53ff\">Actions</font></h2>";
    std::deque<CMDF_Action *> *actionList = m_pMDF->getDM()->getActionList();
    if (nullptr == actionList) {
      html += "<p>No actions defined.</p>";
    }
    else {
      html += "<p>";
      for (auto const& item : *actionList) {      
        html += "<p><b><font color=\"#009900\">";               
        html += QString::number((int)item->getCode());
        html += " -- ";
        html += item->getName().c_str();
        html += "</font></b> ";
        html += " <a href=\"";
        html += item->getInfoURL().c_str();
        html += "\">";
        html += item->getInfoURL().c_str();
        html += "</a>";
        html += "<br>";
        str = item->getDescription();
        str = m_pMDF->format(str);
        html += str.c_str();
        if (item->getListActionParameter()->size()) {
          html += "<br><b>Parameters</b><br>";
          for (auto const& itemParam : *item->getListActionParameter()) { 
            html += "<b><font color=\"#cc8800\">";
            html += itemParam->getName().c_str();
            html += "</font></b><br>";
            if (itemParam->getListValues()->size()) {
              for (auto const& itemValue : *itemParam->getListValues()) { 
                html += "<b><font color=\"#8585ad\"> ";
                html += itemValue->getValue().c_str();
                html += " -- ";
                html += itemValue->getName().c_str();
                html += "</font></b> ";
                html += " <a href=\"";
                html += itemValue->getInfoURL().c_str();
                html += "\">";
                html += itemValue->getInfoURL().c_str();
                html += "</a><br>";
              }
            }
            if (itemParam->getListBits()->size()) {
              for (auto const& itemBit : *itemParam->getListBits()) { 
                html += " -- bit ";
                html += QString::number(itemBit->getPos());
                html += " - <b><font color=\"#8585ad\">";
                html += itemBit->getName().c_str();
                html += "</font></b> ";
                html += " <a href=\"";
                html += itemBit->getInfoURL().c_str();
                html += "\">";
                html += itemBit->getInfoURL().c_str();
                html += "</a><br>";
              }
            }
          }          
        }
        html += "</p>";
      }
      html += "</p>";
    }
  }

  html += "</body></html>";

  
  // Write the HTML to the text browser
  ui->infoArea->setHtml(html);
}

///////////////////////////////////////////////////////////////////////////////
// actionParameterWizard
//  

void 
CDlgEditDm::actionParameterWizard(void)
{
  CDlgActionParam dlg(this);

  std::deque<CMDF_Action *> *actionList = m_pMDF->getDM()->getActionList();
  CMDF_Action *pAction = actionList->at(ui->comboAction->currentIndex());
  std::deque<CMDF_ActionParameter *> *pActionParams = pAction->getListActionParameter();

  dlg.setActionParameter(vscp_readStringValue(ui->editActionParameter->text().toStdString()));
  
  if (pActionParams->size()) {

    // Level I has only one parameter
    CMDF_ActionParameter *pActionParam = pActionParams->at(0);
  
    // -- values
    if (pActionParam->getListValues()->size()) {
      dlg.showValues(true);
      dlg.showBits(false);
      for (auto const& itemValue : *pActionParam->getListValues()) {
        std::string strvalue = itemValue->getValue().c_str();
        std::string name = itemValue->getName().c_str();
        uint32_t value = vscp_readStringValue(strvalue);
        dlg.addValue(value, name);
      }
    }
    // -- bits
    else if (pActionParam->getListBits()->size()) {
      dlg.showBits(true);
      dlg.showValues(false);      
      dlg.addBitValue(pActionParam->getListBits());
    }
    else  {
      QMessageBox::information(
          this,
          tr(APPNAME),
          tr("This action does not have any paramerters defined."),
          QMessageBox::Ok);
      return;    
    }
  }
  else {
    // No parameters
    QMessageBox::information(
          this,
          tr(APPNAME),
          tr("This action does not have any paramerters defined."),
          QMessageBox::Ok);
      return; 
  }

  if (QDialog::Accepted == dlg.exec()) {

    // Level I has only one parameter
    CMDF_ActionParameter *pActionParam = pActionParams->at(0);

    if (pActionParam->getListValues()->size()) {
      
    }
    else if (pActionParam->getListBits()->size()) {
      ui->editActionParameter->setText(QString::number(dlg.getActionParameter()));  
    }

  }

}