// cdlgconnsettingscanal.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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


#include "cdlgconnsettingscanal.h"
#include "ui_cdlgconnsettingscanal.h"
#include "canal_xmlconfig.h"
#include "canalconfigwizard.h"
#include "vscphelper.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>

#include <string>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgConnSettingsCanal::CDlgConnSettingsCanal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgConnSettingsCanal)
{
    ui->setupUi(this);

    connect(ui->btnTest, SIGNAL(clicked()), this, SLOT(testDriver()));
    connect(ui->btnSetPath, SIGNAL(clicked()), this, SLOT(setDriverPath()));
    connect(ui->btnWizard, SIGNAL(clicked()), this, SLOT(wizard()));
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

void CDlgConnSettingsCanal::setInitialFocus(void)
{
    ui->m_description->setFocus();
}


///////////////////////////////////////////////////////////////////////////////
// getName
//

std::string CDlgConnSettingsCanal::getName(void)
{
    return (ui->m_description->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CDlgConnSettingsCanal::setName(const std::string& str)
{
    ui->m_description->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getPath
//

std::string CDlgConnSettingsCanal::getPath(void)
{
    return (ui->m_path->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setPath
//

void CDlgConnSettingsCanal::setPath(const std::string& str)
{
    ui->m_path->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getConfig
//

std::string CDlgConnSettingsCanal::getConfig(void)
{
    return (ui->m_config->text().toStdString());
}

///////////////////////////////////////////////////////////////////////////////
// setConfig
//

void CDlgConnSettingsCanal::setConfig(const std::string& str)
{
    ui->m_config->setText(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// getFlags
//

uint32_t CDlgConnSettingsCanal::getFlags(void)
{
    return vscp_readStringValue(ui->m_flags->text().toStdString()); 
}

///////////////////////////////////////////////////////////////////////////////
// setFlags
//

void CDlgConnSettingsCanal::setFlags(uint32_t flags)
{
    std::string str = vscp_str_format("%lu", (unsigned long)flags);
    ui->m_flags->insert(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// testDriver
//

void CDlgConnSettingsCanal::testDriver()
{
    std::string path = ui->m_path->text().toStdString();

    if (!((QFileInfo::exists(path.c_str()) && QFileInfo(path.c_str()).isFile()) || QFileInfo(path.c_str()).isSymLink())) {
        int ret = QMessageBox::warning(this, tr("vscpworks+"),
                               tr("The driver does not exist."),
                               QMessageBox::Ok );    
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
                                QMessageBox::Ok );
    }
    else {
        uint32_t dllversion = m_vscpClient.m_canalif.CanalGetDllVersion();
        const char *pVendor = m_vscpClient.m_canalif.CanalGetVendorString();
        std::string strGenerationOne = m_vscpClient.m_canalif.isGenerationOne() ?
                                            "Generation one driver (wizard will not work)" : "Generation two driver";
        std::string str = vscp_str_format("The driver is OK. \n\nVersion = %d.%d.%d.%d\n%s\n\n%s", 
                                            (dllversion >> 24) & 0xff,
                                            (dllversion >> 16) & 0xff,
                                            (dllversion >> 8) & 0xff,
                                             dllversion & 0xff,
                                             pVendor,
                                             strGenerationOne.c_str());
                                               
        QMessageBox::information(this, 
                                    tr("vscpworks+"),
                                    str.c_str(),
                                    QMessageBox::Ok );                                      
    }

    // Release the driver
    m_vscpClient.m_canalif.releaseDriver(); 

    // Restore old path
    m_vscpClient.m_canalif.setPath(save_path);
}

///////////////////////////////////////////////////////////////////////////////
// setDriverPath
//

void CDlgConnSettingsCanal::setDriverPath()
{
#ifdef WIN32
    std::string filename = 
            QFileDialog::getOpenFileName(this, 
                                            tr("Set path to CANAL driver"), 
                                            "/var/lib/vscp/drivers/level1", 
                                            tr("Drivers (*.so)")).toStdString();
#else
    std::string filename = 
            QFileDialog::getOpenFileName(this, 
                                            tr("Set path to CANAL driver"), 
                                            "/var/lib/vscp/drivers/level1", 
                                            tr("Drivers (*.so);;All (*)")).toStdString();
#endif     
    ui->m_path->setText(filename.c_str());                                       
}

///////////////////////////////////////////////////////////////////////////////
// wizard
//

void CDlgConnSettingsCanal::wizard()
{
    ///////////////////////////////////////////////////////////////////////////
    //                          Get XML config data
    ///////////////////////////////////////////////////////////////////////////

    std::string path = ui->m_path->text().toStdString();

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
                                QMessageBox::Ok );
        // Release the driver
        m_vscpClient.m_canalif.releaseDriver(); 

        // Reset old path
        m_vscpClient.m_canalif.setPath(save_path);

        return;
    }

    if (m_vscpClient.m_canalif.isGenerationOne()) {
        QMessageBox::warning(this, 
                                tr("vscpworks+"),
                                tr("Driver is a generation one driver that does not have any configuration data embedded"),
                                QMessageBox::Ok );
        // Release the driver
        m_vscpClient.m_canalif.releaseDriver();

        // Reset old path
        m_vscpClient.m_canalif.setPath(save_path);        

        return;
    }

    const char *p = m_vscpClient.m_canalif.CanalGetDriverInfo();

    qDebug() << "Driver XML info \n" << p;

    std::string xml;
    canalXmlConfig xmlcfg;

    if ( NULL != p ) {
        xml.assign(p);
        
        if (!xmlcfg.parseXML(xml)) {
            QMessageBox::warning(this, 
                                tr("vscpworks+"),
                                tr("Failed to parse XML file"),
                                QMessageBox::Ok );
        }
    }
    else {
        QMessageBox::warning(this, 
                                tr("vscpworks+"),
                                tr("Sorry, no configuration information available"),
                                QMessageBox::Ok );
    }

    // Release the driver
    m_vscpClient.m_canalif.releaseDriver();

    // Reset old path
    m_vscpClient.m_canalif.setPath(save_path);

    ///////////////////////////////////////////////////////////////////////////
    //                          Set up wizard steps
    ///////////////////////////////////////////////////////////////////////////

    CanalConfigWizard wizard(this);
    //wizard.setWizardStyle(QWizard::MacStyle);


    QString lbl = tr("This wizard will help to generate and fill in the "
                        "configuration string \nand the flag bits for the. "
                        "the driver.\n\n");
    lbl += vscp_str_format("Driver is a level %d driver\n",(int)xmlcfg.getLevel()+1).c_str();
    lbl += xmlcfg.getBlocking() ? "Driver is blocking.\n" : "Driver is not blocking.\n";   
    lbl += "\n\n";
    lbl += "<a href=\"https://docs.vscxp.org\">VSCP Documentaion page</a>";                    

    wizard.addPage(new IntroPage(nullptr, xmlcfg.getDescription().c_str(), lbl));

    // For test use
    if (0) {  
        wizard.addPage(new ConfigStringPage(this, "cfg1", "String value", "Configuration pos 1", "This is a string test"));
        wizard.addPage(new ConfigStringPage(this, "cfg2", "String value", "Configuration pos 2", "This is a string test")); 
        wizard.addPage(new ConfigStringPage(this, "cfg3", "String value", "Configuration pos 3", "This is a string test")); 
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

    int idx = 0;
    std::list<wizardStepBase *>::iterator it;
    for (it = xmlcfg.m_stepsWizardItems.begin(); it != xmlcfg.m_stepsWizardItems.end(); ++it){
        
        fprintf(stderr,"\tType = %d\n", static_cast<int>((*it)->m_type));
        std::string idstr = vscp_str_format("config%d", idx);           // id for wizard field
        std::string idtxt = vscp_str_format("Config pos %d", idx + 1);  // Text in head of wizard
        idx++;
        
        switch ((*it)->m_type) {

            case wizardStepBase::wizardType::NONE:
                fprintf(stderr, "\twizardType::NONE\n"); 
                break;
            
            case wizardStepBase::wizardType::STRING:
                {
                    wizardStepString *item = (wizardStepString *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }
                    
                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "String value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::BOOL:
                { 
                    wizardStepBool *item = (wizardStepBool *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }

                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Bool value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::INT32:
                {
                    wizardStepInt32 *item = (wizardStepInt32 *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }

                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Int32 value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::UINT32:
                { 
                    wizardStepUInt32 *item = (wizardStepUInt32 *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }

                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Uint32 value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::INT64:
                {
                    wizardStepInt64 *item = (wizardStepInt64 *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }

                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Int64 value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::UINT64:
                {
                    wizardStepUInt64 *item = (wizardStepUInt64 *)(*it); 

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }

                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Uint64 value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str());
                }
                break;
            
            case wizardStepBase::wizardType::FLOAT:
                {
                    wizardStepFloat *item = (wizardStepFloat *)(*it);

                    std::string description = item->getDescription();
                    if (item->isOptional()) {
                        description += " ";
                        description += "(optional)";
                    }
                    else {
                        idstr += "*";
                    }
                    
                    wizard.addPage(new ConfigStringPage(this,
                                                            idstr.c_str(),
                                                            "Floating point value",
                                                            idtxt.c_str(),
                                                            item->getDescription().c_str(),
                                                            item->getValue().c_str(),
                                                            item->getInfoUrl().c_str()));
                    wizard.setField(idstr.c_str(), item->getValue().c_str()); 
                }
                break;

            case wizardStepBase::wizardType::CHOICE:
                {
                    wizardStepChoice *item = (wizardStepChoice *)(*it);

                    std::list<wizardChoiceItem *>::iterator it;
                    for (it = item->m_choices.begin(); it != item->m_choices.end(); ++it){
                        wizardChoiceItem *itemChoice = (wizardChoiceItem *)(*it);
                        fprintf(stderr, 
                                    "\t\tChoice Value=%s Description=%s\n",
                                    itemChoice->getValue().c_str(), 
                                    itemChoice->getDescription().c_str());
                    }
                }
                break;    

            default:
                // we do nothing
                break;                               
        }
    }

    wizard.addPage(new ConclusionPage);
    wizard.exec();

}