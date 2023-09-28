// cdlgconnsettingscanal.cpp
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

#include "cdlgconnsettingscanal.h"
#include "ui_cdlgconnsettingscanal.h"

#include "cdlglevel1filter.h"

#include "canal_xmlconfig.h"
#include "canalconfigwizard.h"
#include "vscphelper.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <string>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsCanal::CDlgConnSettingsCanal(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgConnSettingsCanal)
{
  ui->setupUi(this);

  connect(ui->btnTest, SIGNAL(clicked()), this, SLOT(testDriver()));
  connect(ui->btnSetPath, SIGNAL(clicked()), this, SLOT(setDriverPath()));
  connect(ui->btnWizard, SIGNAL(clicked()), this, SLOT(wizard()));
  connect(ui->btnFilterWizard, SIGNAL(clicked()), this, SLOT(filterwizard()));
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgConnSettingsCanal::~CDlgConnSettingsCanal()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void
CDlgConnSettingsCanal::setInitialFocus(void)
{
  ui->editDescription->setFocus();
}

// Getters / Setters

///////////////////////////////////////////////////////////////////////////////
// getName
//

QString
CDlgConnSettingsCanal::getName(void)
{
  return (ui->editDescription->text());
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void
CDlgConnSettingsCanal::setName(const QString& str)
{
  ui->editDescription->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPath
//

QString
CDlgConnSettingsCanal::getPath(void)
{
  return (ui->editPath->text());
}

///////////////////////////////////////////////////////////////////////////////
// setPath
//

void
CDlgConnSettingsCanal::setPath(const QString& str)
{
  ui->editPath->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getConfig
//

QString
CDlgConnSettingsCanal::getConfig(void)
{
  return (ui->editConfig->text());
}

///////////////////////////////////////////////////////////////////////////////
// setConfig
//

void
CDlgConnSettingsCanal::setConfig(const QString& str)
{
  ui->editConfig->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getFlags
//

uint32_t
CDlgConnSettingsCanal::getFlags(void)
{
  return vscp_readStringValue(ui->editFlags->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// getFlagsStr
//

std::string
CDlgConnSettingsCanal::getFlagsStr(void)
{
  return ui->editFlags->text().toStdString();
}

///////////////////////////////////////////////////////////////////////////////
// setFlags
//

void
CDlgConnSettingsCanal::setFlags(uint32_t flags)
{
  std::string str = vscp_str_format("0x%lX", (unsigned long)flags);
  ui->editFlags->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getDataRate
//

uint32_t
CDlgConnSettingsCanal::getDataRate(void)
{
  return vscp_readStringValue(ui->editDataRate->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setDataRate
//

void
CDlgConnSettingsCanal::setDataRate(uint32_t datarate)
{
  std::string str = vscp_str_format("%lu", (unsigned long)datarate);
  ui->editDataRate->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getJson
//

QJsonObject
CDlgConnSettingsCanal::getJson(void)
{
  m_jsonConfig["type"]     = static_cast<int>(CVscpClient::connType::CANAL);
  m_jsonConfig["name"]     = getName();
  m_jsonConfig["path"]     = getPath();
  m_jsonConfig["config"]   = getConfig();
  m_jsonConfig["flags"]    = (int)getFlags();
  m_jsonConfig["datarate"] = (int)getDataRate();
  return m_jsonConfig;
}

///////////////////////////////////////////////////////////////////////////////
// setJson
//

void
CDlgConnSettingsCanal::setJson(const QJsonObject* pobj)
{
  m_jsonConfig = *pobj;

  if (!m_jsonConfig["name"].isNull())
    setName(m_jsonConfig["name"].toString());
  if (!m_jsonConfig["path"].isNull())
    setPath(m_jsonConfig["path"].toString());
  if (!m_jsonConfig["config"].isNull())
    setConfig(m_jsonConfig["config"].toString());
  if (!m_jsonConfig["flags"].isNull())
    setFlags(uint32_t(m_jsonConfig["flags"].toInt()));
  if (!m_jsonConfig["datarate"].isNull())
    setDataRate(m_jsonConfig["datarate"].toInt());
}

///////////////////////////////////////////////////////////////////////////////
// testDriver
//

void
CDlgConnSettingsCanal::testDriver()
{
  std::string path = ui->editPath->text().toStdString();

  if (!((QFileInfo::exists(path.c_str()) && QFileInfo(path.c_str()).isFile()) || QFileInfo(path.c_str()).isSymLink())) {
    int ret = QMessageBox::warning(this, tr("vscpworks+"), tr("The driver does not exist."), QMessageBox::Ok);
  }

  // Save set path
  std::string save_path = m_vscpClient.m_canalif.getPath();

  // Set new path
  m_vscpClient.m_canalif.setPath(path);

  int rv;
  if (CANAL_ERROR_SUCCESS != (rv = m_vscpClient.m_canalif.init())) {
    std::string str = vscp_str_format("The driver did not load properly. rv=%d", rv);
    QMessageBox::warning(this,
                         tr("vscpworks+"),
                         str.c_str(),
                         QMessageBox::Ok);
  }
  else {
    uint32_t dllversion          = m_vscpClient.m_canalif.CanalGetDllVersion();
    const char* pVendor          = m_vscpClient.m_canalif.CanalGetVendorString();
    std::string strGenerationOne = m_vscpClient.m_canalif.isGenerationOne() ? "Generation 1 driver (wizard will not work)" : "Generation 2 driver";
    std::string str              = vscp_str_format("The driver is OK. \n\nVersion = %d.%d.%d.%d\n%s\n\n%s",
                                      (dllversion >> 24) & 0xff,
                                      (dllversion >> 16) & 0xff,
                                      (dllversion >> 8) & 0xff,
                                      dllversion & 0xff,
                                      pVendor,
                                      strGenerationOne.c_str());

    QMessageBox::information(this,
                             tr("vscpworks+"),
                             str.c_str(),
                             QMessageBox::Ok);
  }

  // Release the driver
  m_vscpClient.m_canalif.releaseDriver();

  // Restore old path
  m_vscpClient.m_canalif.setPath(save_path);
}

///////////////////////////////////////////////////////////////////////////////
// setDriverPath
//

void
CDlgConnSettingsCanal::setDriverPath()
{
#ifdef WIN32
  std::string filename =
    QFileDialog::getOpenFileName(this,
                                 tr("Set path to CANAL driver"),
                                 "/var/lib/vscp/drivers/level1",
                                 tr("Drivers (*.so)"))
      .toStdString();
#else
  std::string filename =
    QFileDialog::getOpenFileName(this,
                                 tr("Set path to CANAL driver"),
                                 "/var/lib/vscp/drivers/level1",
                                 tr("Drivers (*.so);;All (*)"))
      .toStdString();
#endif
  ui->editPath->setText(filename.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// wizard
//

void
CDlgConnSettingsCanal::wizard()
{
  ///////////////////////////////////////////////////////////////////////////
  //                          Get XML config data
  ///////////////////////////////////////////////////////////////////////////

  std::string path   = ui->editPath->text().toStdString();
  std::string config = getConfig().toStdString();
  uint32_t flags     = getFlags();

  // Get config items
  std::deque<std::string> m_configItems;
  vscp_split(m_configItems, getConfig().toStdString(), ";");

  // Save set path
  QString save_path = m_vscpClient.m_canalif.getPath().c_str();

  // Set new path
  m_vscpClient.m_canalif.setPath(path);

  int rv;
  if (CANAL_ERROR_SUCCESS != (rv = m_vscpClient.m_canalif.init())) {
    std::string str = vscp_str_format("The driver did not load properly. rv=%d", rv);
    QMessageBox::warning(this,
                         tr("vscpworks+"),
                         str.c_str(),
                         QMessageBox::Ok);
    // Release the driver
    m_vscpClient.m_canalif.releaseDriver();

    // Reset old path
    m_vscpClient.m_canalif.setPath(save_path.toStdString());

    return;
  }

  if (m_vscpClient.m_canalif.isGenerationOne()) {
    QMessageBox::warning(this,
                         tr("vscpworks+"),
                         tr("Driver is a generation one driver that does not have any configuration data embedded"),
                         QMessageBox::Ok);
    // Release the driver
    m_vscpClient.m_canalif.releaseDriver();

    // Reset old path
    m_vscpClient.m_canalif.setPath(save_path.toStdString());

    return;
  }

  const char* p = m_vscpClient.m_canalif.CanalGetDriverInfo();
  qDebug() << "Driver XML info \n"
           << p;

  std::string xml;
  canalXmlConfig xmlcfg;

  if (NULL != p) {
    xml.assign(p);

    if (!xmlcfg.parseXML(xml)) {
      QMessageBox::warning(this,
                           tr("vscpworks+"),
                           tr("Failed to parse XML file"),
                           QMessageBox::Ok);
    }
  }
  else {
    QMessageBox::warning(this,
                         tr("vscpworks+"),
                         tr("Sorry, no configuration information available"),
                         QMessageBox::Ok);
  }

  // Release the driver
  m_vscpClient.m_canalif.releaseDriver();

  // Reset old path
  m_vscpClient.m_canalif.setPath(save_path.toStdString());

  ///////////////////////////////////////////////////////////////////////////
  //                          Set up wizard steps
  ///////////////////////////////////////////////////////////////////////////

  CanalConfigWizard wizard(this);
  // wizard.setWizardStyle(QWizard::MacStyle);

  // ************************************************************************
  //                               Intro page
  // ************************************************************************
  QString lbl = tr("This wizard will help to generate and fill in the "
                   "configuration string \nand the flag bits for the. "
                   "the driver.\n\n");
  lbl += vscp_str_format("Driver is a level %d driver\n", (int)xmlcfg.getLevel() + 1).c_str();
  lbl += xmlcfg.getBlocking() ? "Driver is blocking.\n" : "Driver is not blocking.\n";

  wizard.addPage(new IntroPage(nullptr, xmlcfg.getDescription().c_str(), lbl, xmlcfg.getInfoUrl().c_str()));

  // For test use
  if (0) {
    wizard.addPage(new ConfigStringPage(this, wizardStepBase::wizardType::STRING, "cfg1", "String value", "Configuration pos 1", "This is a string test"));
    wizard.addPage(new ConfigStringPage(this, wizardStepBase::wizardType::STRING, "cfg2", "String value", "Configuration pos 2", "This is a string test"));
    wizard.addPage(new ConfigStringPage(this, wizardStepBase::wizardType::STRING, "cfg3", "String value", "Configuration pos 3", "This is a string test"));
    wizard.addPage(new ConfigBoolPage(this, "cfg4", "Boolean value", "Bool config", "This is a bool test"));
    std::list<std::string> list;
    list.push_back(std::string("Ettan"));
    list.push_back(std::string("Tvåan"));
    list.push_back(std::string("Trean"));
    list.push_back(std::string("Fyran"));
    list.push_back(std::string("Femman"));
    wizard.addPage(new ConfigChoicePage(this, "cfg5", list, "Choose one", "Choice config", "This is a choice test"));
    wizard.addPage(new ConfigChoicePage(this, "cfg6", list, "Choose one", "Choice config", "This is a choice test"));
  }

  int cfgidx = 0;
  std::list<wizardStepBase*>::iterator itcfg;
  for (itcfg = xmlcfg.m_stepsWizardItems.begin(); itcfg != xmlcfg.m_stepsWizardItems.end(); ++itcfg) {

    std::string idstr = vscp_str_format("config%d", cfgidx);          // id for wizard field
    std::string idtxt = vscp_str_format("Config pos %d", cfgidx + 1); // Text in head of wizard
    cfgidx++;

    std::string dlgcfgval;       // Set to current config value for this pos
    bool bUIseDlgCfgVal = false; // Set to tru of there is a value already set for this pos
    if (m_configItems.size()) {
      dlgcfgval = m_configItems.front();
      m_configItems.pop_front();
      bUIseDlgCfgVal = true;
    }

    switch ((*itcfg)->m_type) {

      case wizardStepBase::wizardType::NONE:
        break;

      case wizardStepBase::wizardType::STRING: {
        wizardStepString* item = (wizardStepString*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::STRING,
                                            idstr.c_str(),
                                            "String value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            bUIseDlgCfgVal ? dlgcfgval.c_str() : item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        // Set default value
        wizard.setField(QString::fromUtf8(idstr.c_str()).remove('*'),
                        bUIseDlgCfgVal ? dlgcfgval.c_str() : item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::BOOL: {
        wizardStepBool* item = (wizardStepBool*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigBoolPage(this,
                                          idstr.c_str(),
                                          "Bool value",
                                          idtxt.c_str(),
                                          item->getDescription().c_str(),
                                          item->getValue().c_str(),
                                          item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::INT32: {
        wizardStepInt32* item = (wizardStepInt32*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::INT32,
                                            idstr.c_str(),
                                            "Int32 valuee",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::UINT32: {
        wizardStepUInt32* item = (wizardStepUInt32*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::UINT32,
                                            idstr.c_str(),
                                            "Uint32 value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::INT64: {
        wizardStepInt64* item = (wizardStepInt64*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::INT64,
                                            idstr.c_str(),
                                            "Int64 value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::UINT64: {
        wizardStepUInt64* item = (wizardStepUInt64*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::UINT64,
                                            idstr.c_str(),
                                            "Uint64 value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::FLOAT: {
        wizardStepFloat* item = (wizardStepFloat*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::FLOAT,
                                            idstr.c_str(),
                                            "Floating point value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::CHOICE: {
        std::list<std::string> choicelist;
        wizardStepChoice* item = (wizardStepChoice*)(*itcfg);

        std::string description = item->getDescription();
        if (item->isOptional()) {
          description += " ";
          description += "(optional)";
        }
        else {
          idstr += "*";
        }

        std::vector<wizardChoiceItem*>::iterator it;
        for (it = item->m_choices.begin(); it != item->m_choices.end(); ++it) {
          wizardChoiceItem* itemChoice = (wizardChoiceItem*)(*it);
          choicelist.push_back(itemChoice->getDescription());
        }

        wizard.addPage(new ConfigChoicePage(this,
                                            idstr.c_str(),
                                            choicelist,
                                            "Choices",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            0,
                                            item->getInfoUrl().c_str()));
      } break;

      default:
        // we do nothing
        break;
    }
  }

  // ************************************************************************
  //                                Flags
  // ************************************************************************

  int flagidx = 0;
  std::list<wizardFlagBitBase*>::iterator itflg;
  for (itflg = xmlcfg.m_stepsWizardFlags.begin(); itflg != xmlcfg.m_stepsWizardFlags.end(); ++itflg) {

    std::string idstr = vscp_str_format("flag%d", flagidx); // id for wizard flag field
    flagidx++;

    bool bFlagValue      = false;
    std::string strFlags = getFlagsStr();
    uint32_t cfgflags    = vscp_readStringValue(strFlags);
    vscp_trim(strFlags);
    if (strFlags.length()) {
      bFlagValue = true;
    }

    switch ((*itflg)->m_type) {

      case wizardStepBase::wizardType::NONE:
        break;

      case wizardStepBase::wizardType::BOOL: {
        std::string idtxt;
        wizardFlagBitBool* item = (wizardFlagBitBool*)(*itflg);

        std::string description = item->getDescription();

        uint8_t width;
        if ((width = item->getWidth()) > 1) {
          idtxt = vscp_str_format("Flag bit %d-%d", item->getPos(), item->getPos() + width - 1);
        }
        else {
          idtxt = vscp_str_format("Flag bit %d", item->getPos());
        }

        wizard.addPage(new ConfigBoolPage(this,
                                          idstr.c_str(),
                                          item->getDescription().c_str(),
                                          idtxt.c_str(),
                                          item->getDescription().c_str(),
                                          item->getValue().c_str(),
                                          item->getInfoUrl().c_str()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::UINT32: {
        std::string idtxt;

        wizardFlagBitNumber* item = (wizardFlagBitNumber*)(*itflg);

        std::string description = item->getDescription();

        uint8_t width;
        if ((width = item->getWidth()) > 1) {
          idtxt = vscp_str_format("Flag bit %d-%d", item->getPos(), item->getPos() + width - 1);
        }
        else {
          idtxt = vscp_str_format("Flag bit %d", item->getPos());
        }
        // idstr += "*";

        wizard.addPage(new ConfigStringPage(this,
                                            wizardStepBase::wizardType::UINT32,
                                            idstr.c_str(),
                                            "Uint32 value",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            item->getValue().c_str(),
                                            item->getInfoUrl().c_str(),
                                            item->getWidth()));
        wizard.setField(idstr.c_str(), item->getValue().c_str());
      } break;

      case wizardStepBase::wizardType::CHOICE: {
        std::string idtxt;

        std::list<std::string> choicelist;
        wizardFlagBitChoice* item = (wizardFlagBitChoice*)(*itflg);

        std::string description = item->getDescription();

        uint8_t width;
        if ((width = item->getWidth()) > 1) {
          idtxt = vscp_str_format("Flag bit %d-%d", item->getPos(), item->getPos() + width - 1);
        }
        else {
          idtxt = vscp_str_format("Flag bit %d", item->getPos());
        }

        std::vector<wizardBitChoice*>::iterator it;
        for (it = item->m_choices.begin(); it != item->m_choices.end(); ++it) {
          wizardBitChoice* itemChoice = (wizardBitChoice*)(*it);
          choicelist.push_back(itemChoice->getDescription());
        }

        wizard.addPage(new ConfigChoicePage(this,
                                            idstr.c_str(),
                                            choicelist,
                                            "Choices",
                                            idtxt.c_str(),
                                            item->getDescription().c_str(),
                                            0,
                                            item->getInfoUrl().c_str()));
      } break;

      default:
        // we do nothing
        break;
    }
  }

  // ************************************************************************
  //                               Last page
  // ************************************************************************
  wizard.addPage(new ConclusionPage);

  // ************************************************************************
  //                               go, go, go...
  // ************************************************************************
  if (wizard.exec()) {

    uint32_t flags = 0;
    std::string strConfig;

    // Finish - OK

    int cfgidx = 0;
    std::list<std::string> configStrings;
    std::list<wizardStepBase*>::iterator itcfg;
    for (itcfg = xmlcfg.m_stepsWizardItems.begin(); itcfg != xmlcfg.m_stepsWizardItems.end(); ++itcfg) {

      std::string idstr = vscp_str_format("config%d", cfgidx); // id for wizard field
      cfgidx++;

      // Set the value
      std::string newValue = wizard.field(idstr.c_str()).toString().toStdString();
      (*itcfg)->setValue(newValue);

      qDebug() << "C: " << wizard.field(idstr.c_str()).toString();
      qDebug() << "R: " << (*itcfg)->getRealValue(wizard.field(idstr.c_str()).toString().toStdString()).c_str();

      // If optional and no value set we are done
      qDebug() << " O: " << (*itcfg)->isOptional();
      if (!newValue.length() && (*itcfg)->isOptional()) {
        break;
      }

      if (strConfig.length())
        strConfig += ";";
      strConfig += (*itcfg)->getRealValue(wizard.field(idstr.c_str()).toString().toStdString());
    }

    int flagidx = 0;
    std::list<wizardFlagBitBase*>::iterator itflg;
    for (itflg = xmlcfg.m_stepsWizardFlags.begin(); itflg != xmlcfg.m_stepsWizardFlags.end(); ++itflg) {

      std::string idstr = vscp_str_format("flag%d", flagidx); // id for wizard flag field
      flagidx++;

      qDebug() << "F: " << wizard.field(idstr.c_str()).toString();
      qDebug() << (*itflg)->getRealValue(wizard.field(idstr.c_str()).toString().toStdString());
      flags += (*itflg)->getRealValue(wizard.field(idstr.c_str()).toString().toStdString());
    }

    qDebug() << "Config str = " << strConfig.c_str();
    setConfig(strConfig.c_str());

    qDebug() << "flags = " << flags;
    setFlags(flags);
  }
}

///////////////////////////////////////////////////////////////////////////////
// filterwizard
//

void
CDlgConnSettingsCanal::filterwizard()
{
  CDlgLevel1Filter dlg(this);

  dlg.disableDescriptionField(); // Description filed is not needed here

  uint32_t canid = vscp_readStringValue(ui->editFilter->text().toStdString());
  uint32_t mask  = vscp_readStringValue(ui->editMask->text().toStdString());

  dlg.setVscpPriorityFilter(vscp_getHeadFromCANALid(canid) >> 5);
  dlg.setVscpPriorityMask(vscp_getHeadFromCANALid(mask) >> 5);

  if (vscp_getHeadFromCANALid(mask) & VSCP_HEADER_HARD_CODED) {
    dlg.setHardcoded(true);
  }
  else {
    dlg.setHardcoded(false);
  }

  dlg.setVscpClassFilter(vscp_getVscpClassFromCANALid(canid));
  dlg.setVscpClassMask(vscp_getVscpClassFromCANALid(mask));

  dlg.setVscpTypeFilter(vscp_getVscpTypeFromCANALid(canid));
  dlg.setVscpTypeMask(vscp_getVscpTypeFromCANALid(mask));

  dlg.setVscpNodeIdFilter(canid & 0xff);
  dlg.setVscpNodeIdMask(mask & 0xff);

  if (QDialog::Accepted == dlg.exec()) {

    vscpworks* pworks = (vscpworks*)QCoreApplication::instance();

    uint32_t canid = vscp_getCANALidFromData(dlg.getVscpPriorityFilter(),
                                             dlg.getVscpClassFilter(),
                                             dlg.getVscpTypeFilter());
    canid += dlg.getVscpNodeIdFilter();
    if (dlg.getHardcoded()) {
      canid |= (1 << 25);
    }

    uint32_t mask = vscp_getCANALidFromData(dlg.getVscpPriorityMask(),
                                            dlg.getVscpClassMask(),
                                            dlg.getVscpTypeMask());
    mask += dlg.getVscpNodeIdMask();
    if (dlg.getHardcoded()) {
      mask |= (1 << 25);
    }

    if (dlg.getInverted()) {
      mask = ~mask;
    }

    ui->editFilter->setText(pworks->decimalToStringInBase(canid));
    ui->editMask->setText(pworks->decimalToStringInBase(mask));
  }
}
