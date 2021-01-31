// canalconfigwizard.cpp
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

#include <QtWidgets>
#include <QDebug>

#include "canalconfigwizard.h"

#include <string>
#include <list>


///////////////////////////////////////////////////////////////////////////////
// CTor - CanalConfigWizard
//

CanalConfigWizard::CanalConfigWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new IntroPage);
    addPage(new ConfigStringPage(this, "cfg1", "String value", "Configuration pos 1", "This is a string test"));
    addPage(new ConfigStringPage(this, "cfg2", "String value", "Configuration pos 2", "This is a string test")); 
    addPage(new ConfigStringPage(this, "cfg3", "String value", "Configuration pos 3", "This is a string test")); 
    addPage(new ConfigBoolPage(this, "cfg4", "Boolean value", "Bool config", "This is a bool test"));
    std::list<std::string> list;
    list.push_back(std::string("Ettan"));
    list.push_back(std::string("Tvåan"));
    list.push_back(std::string("Trean"));
    list.push_back(std::string("Fyran"));
    list.push_back(std::string("Femman"));
    addPage(new ConfigChoicePage(this, "cfg5", list, "Choose one", "Choice config", "This is a choice test"));

    addPage(new ConfigChoicePage(this, "cfg5", list, "Choose one", "Choice config", "This is a choice test"));
    addPage(new ConclusionPage);

    setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
    setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

    setWindowTitle(tr("CANAL driver configuration Wizard"));
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void CanalConfigWizard::accept()
{
    QByteArray className = field("className").toByteArray();

    QString outputDir = field("edit1").toString();
    qDebug() << outputDir;
    QString header = field("header").toString();
    QString implementation = field("implementation").toString();

    if (field("comment").toBool()) {

    }

    QDialog::accept();
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - IntroPage
//

IntroPage::IntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("CANAL configuration"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    label = new QLabel(tr("This wizard will help to generate and fill in the "
                          "configuration string and the flag bits for the. "
                          "Level I (CANAL) driver."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConclusionPage
//

ConclusionPage::ConclusionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Wizard is ready"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    label = new QLabel;
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConclusionPage:: initializePage()
{
    label->setText(tr("Pressing finish will write configuration and flags "
                       "to the configuration dialog."));
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigStringPage
//

ConfigStringPage::ConfigStringPage(QWidget *parent, 
                                    const std::string& fieldname,
                                    const std::string& labeltext,
                                    const std::string& title,
                                    const std::string& subtitle)
    : QWizardPage(parent)
{
    m_labelText = labeltext;
    m_fieldName = fieldname;

    setTitle(QString::fromUtf8(title.c_str()));
    if (subtitle.length()) {
        setSubTitle(QString::fromUtf8(subtitle.c_str()));
    }
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    m_label = new QLabel;
    m_label->setWordWrap(true);
    m_edit = new QLineEdit;
    m_label->setBuddy(m_edit);

    qDebug() <<  "Fieldname " << QString::fromUtf8(m_fieldName.c_str());
    registerField(QString::fromUtf8(m_fieldName.c_str()), m_edit);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_label, 0, 0);
    layout->addWidget(m_edit, 0, 1);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConfigStringPage::initializePage()
{
    m_label->setText(QString::fromUtf8(m_labelText.c_str()));
    QString strvalue = field(m_fieldName.c_str()).toString();
    m_edit->setText(strvalue);
}

///////////////////////////////////////////////////////////////////////////////
//  cleanupPage
//

void ConfigStringPage::cleanupPage()
{

}

///////////////////////////////////////////////////////////////////////////////
//  setLabelText
//

void ConfigStringPage::setLabelText(const std::string& labeltext)
{
    m_labelText = labeltext;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigBoolPage
//

ConfigBoolPage::ConfigBoolPage(QWidget *parent, 
                                const std::string& fieldname,
                                const std::string& labeltext,
                                const std::string& title,
                                const std::string& subtitle)
    : QWizardPage(parent)
{
    m_labelText = labeltext;
    m_fieldName = fieldname;

    setTitle(QString::fromUtf8(title.c_str()));
    if (subtitle.length()) {
        setSubTitle(QString::fromUtf8(subtitle.c_str()));
    }
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    m_checkbox = new QCheckBox(QString::fromUtf8(labeltext.c_str()));
    

    registerField(QString::fromUtf8(m_fieldName.c_str()), m_checkbox);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_checkbox, 0, 0);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConfigBoolPage:: initializePage()
{
    m_checkbox->setChecked(field(QString::fromUtf8(m_fieldName.c_str())).toBool());
}

///////////////////////////////////////////////////////////////////////////////
//  cleanupPage
//

void ConfigBoolPage::cleanupPage()
{

}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigChoicePage
//

ConfigChoicePage::ConfigChoicePage(QWidget *parent, 
                                    const std::string& fieldname,
                                    const std::list<std::string>& strlist,
                                    const std::string& labeltext,
                                    const std::string& title,
                                    const std::string& subtitle)
    : QWizardPage(parent)
{
    m_labelText = labeltext;
    m_fieldName = fieldname;

    setTitle(QString::fromUtf8(title.c_str()));
    if (subtitle.length()) {
        setSubTitle(QString::fromUtf8(subtitle.c_str()));
    }
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    m_label = new QLabel;
    m_label->setWordWrap(true);
    m_list = new QListWidget;
    m_label->setBuddy(m_list);

    for (std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); ++it){
        qDebug() << it->c_str();
        m_list->addItem(QString::fromUtf8(it->c_str()));
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_list);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConfigChoicePage:: initializePage()
{
    m_label->setText(tr("Select on of the available choices"));
}

///////////////////////////////////////////////////////////////////////////////
//  cleanupPage
//

void ConfigChoicePage::cleanupPage()
{

}