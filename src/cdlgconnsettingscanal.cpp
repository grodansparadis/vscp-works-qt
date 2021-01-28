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
#include "vscphelper.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>

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

    std::string xml;
    if ( NULL != p ) {
        xml.assign(p);

        canalXmlConfig cfg;
        if (!cfg.parseXML(xml)) {
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



}