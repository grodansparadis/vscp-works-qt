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
    

    setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
    setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

    setWindowTitle(tr("VSCP level I (CANAL) driver configuration Wizard"));
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

IntroPage::IntroPage(QWidget *parent, const QString& title, const QString& lbltext)
    : QWizardPage(parent)
{
    setTitle(title);
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));
    setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));

    m_label = new QLabel(lbltext);
    m_label->setOpenExternalLinks(true);
    m_label->setWordWrap(true);

    m_button = new QPushButton("Open manual page...");
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_button);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void IntroPage:: initializePage()
{
    // const QSize BUTTON_SIZE = QSize(200, 30);
    // m_button->setMaximumSize(BUTTON_SIZE);
    m_label->setOpenExternalLinks(true);
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
                                    const QString& fieldname,
                                    const QString& labeltext,
                                    const QString& title,
                                    const QString& subtitle,
                                    const QString& value,
                                    const QString& infolink)
    : QWizardPage(parent)
{
    m_value = value;
    m_labelText = labeltext;
    m_fieldName = fieldname;
    m_infolink = infolink;
    
    setTitle(title);
    if (subtitle.length()) {
        setSubTitle(subtitle);
    }
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    m_label = new QLabel;
    m_label->setWordWrap(true);
    m_edit = new QLineEdit;
    m_label->setBuddy(m_edit);

    registerField(fieldname, m_edit);
    setField(fieldname, value);

    m_button = new QPushButton(tr("Info..."));
    connect(m_button, &QPushButton::clicked, this, &ConfigStringPage::showInfo );

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_label, 0, 0);
    layout->addWidget(m_edit, 0, 1);
    layout->addWidget(m_button, 3, 1);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  showInfo
//

void ConfigStringPage::showInfo(void)
{
    if (m_infolink.length()) {
        QDesktopServices::openUrl(QUrl(m_infolink));
    }
    else {
        QDesktopServices::openUrl(QUrl("https://docs.vscp.org"));
    }
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConfigStringPage::initializePage()
{
    //m_label->setText(m_labelText);
    QString strvalue = field(m_fieldName).toString();
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

void ConfigStringPage::setLabelText(const QString& labeltext)
{
    m_labelText = labeltext;
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigBoolPage
//

ConfigBoolPage::ConfigBoolPage(QWidget *parent, 
                                const QString& fieldname,
                                const QString& labeltext,
                                const QString& title,
                                const QString& subtitle,
                                bool bvalue,
                                const QString& infolink)
    : QWizardPage(parent)
{
    m_bvalue = bvalue;
    m_labelText = labeltext;
    m_fieldName = fieldname;
    m_infolink = infolink;

    setTitle(title);
    if (subtitle.length()) {
        setSubTitle(subtitle);
    }
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark-canal.png"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":attachment.png"));

    m_checkbox = new QCheckBox(labeltext);    

    registerField(fieldname, m_checkbox);
    setField(fieldname, bvalue);

    m_button = new QPushButton(tr("Info..."));
    connect(m_button, &QPushButton::clicked, this, &ConfigBoolPage::showInfo );

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_checkbox, 0, 0);
    layout->addWidget(m_button, 1, 0);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
//  initializePage
//

void ConfigBoolPage:: initializePage()
{
    m_checkbox->setChecked(field(m_fieldName).toBool());
}

///////////////////////////////////////////////////////////////////////////////
//  cleanupPage
//

void ConfigBoolPage::cleanupPage()
{

}

///////////////////////////////////////////////////////////////////////////////
//  setLabelText
//

void ConfigBoolPage::setLabelText(const QString& labeltext)
{
    m_labelText = labeltext;
}

///////////////////////////////////////////////////////////////////////////////
//  showInfo
//

void ConfigBoolPage::showInfo(void)
{
    if (m_infolink.length()) {
        QDesktopServices::openUrl(QUrl(m_infolink));
    }
    else {
        QDesktopServices::openUrl(QUrl("https://docs.vscp.org"));
    }
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigChoicePage
//

ConfigChoicePage::ConfigChoicePage(QWidget *parent, 
                                    const QString& fieldname,
                                    const std::list<std::string>& strlist,
                                    const QString& labeltext,
                                    const QString& title,
                                    const QString& subtitle,
                                    uint8_t nSelect,
                                    const QString& infolink)
    : QWizardPage(parent)
{
    m_labelText = labeltext;
    m_fieldName = fieldname;

    setTitle(title);
    if (subtitle.length()) {
        setSubTitle(subtitle);
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