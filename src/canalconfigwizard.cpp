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


///////////////////////////////////////////////////////////////////////////////
// CTor - CanalConfigWizard
//

CanalConfigWizard::CanalConfigWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new IntroPage);
    //addPage(new ClassInfoPage);
    addPage(new ConfigStringPage(this,"String config", "This is a string test")); 
    addPage(new ConfigBoolPage(this,"Bool config", "This is a bool test"));
    addPage(new ConfigChoicePage(this,"Choice config", "This is a choice test"));
    //addPage(new CodeStylePage);
    //addPage(new OutputFilesPage);
    addPage(new ConclusionPage);

    setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
    setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

    setWindowTitle(tr("Class Wizard"));
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
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark1.png"));

    label = new QLabel(tr("This wizard will help to generate and fill in the "
                          "configuration string and the flag bits. "));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}


// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// CTor - ClassInfoPage
//

ClassInfoPage::ClassInfoPage(QWidget *parent)
    : QWizardPage(parent)
{

    setTitle(tr("Class Information"));
    setSubTitle(tr("Specify basic information about the class for which you "
                   "want to generate skeleton source code files."));
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo1.png"));


    classNameLabel = new QLabel(tr("&Class name:"));
    classNameLineEdit = new QLineEdit;
    classNameLabel->setBuddy(classNameLineEdit);

    baseClassLabel = new QLabel(tr("B&ase class:"));
    baseClassLineEdit = new QLineEdit;
    baseClassLabel->setBuddy(baseClassLineEdit);

    qobjectMacroCheckBox = new QCheckBox(tr("Generate Q_OBJECT &macro"));


    groupBox = new QGroupBox(tr("C&onstructor"));


    qobjectCtorRadioButton = new QRadioButton(tr("&QObject-style constructor"));
    qwidgetCtorRadioButton = new QRadioButton(tr("Q&Widget-style constructor"));
    defaultCtorRadioButton = new QRadioButton(tr("&Default constructor"));
    copyCtorCheckBox = new QCheckBox(tr("&Generate copy constructor and "
                                        "operator="));

    defaultCtorRadioButton->setChecked(true);

    connect(defaultCtorRadioButton, &QAbstractButton::toggled,
            copyCtorCheckBox, &QWidget::setEnabled);


    registerField("className*", classNameLineEdit);
    registerField("baseClass", baseClassLineEdit);
    registerField("qobjectMacro", qobjectMacroCheckBox);

    registerField("qobjectCtor", qobjectCtorRadioButton);
    registerField("qwidgetCtor", qwidgetCtorRadioButton);
    registerField("defaultCtor", defaultCtorRadioButton);
    registerField("copyCtor", copyCtorCheckBox);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout;

    groupBoxLayout->addWidget(qobjectCtorRadioButton);
    groupBoxLayout->addWidget(qwidgetCtorRadioButton);
    groupBoxLayout->addWidget(defaultCtorRadioButton);
    groupBoxLayout->addWidget(copyCtorCheckBox);
    groupBox->setLayout(groupBoxLayout);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(classNameLabel, 0, 0);
    layout->addWidget(classNameLineEdit, 0, 1);
    layout->addWidget(baseClassLabel, 1, 0);
    layout->addWidget(baseClassLineEdit, 1, 1);
    layout->addWidget(qobjectMacroCheckBox, 2, 0, 1, 2);
    layout->addWidget(groupBox, 3, 0, 1, 2);
    setLayout(layout);

}


// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// CTor - CodeStylePage
//

CodeStylePage::CodeStylePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Code Style Options"));
    setSubTitle(tr("Choose the formatting of the generated code."));
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo2.png"));

    commentCheckBox = new QCheckBox(tr("&Start generated files with a "

                                       "comment"));
    commentCheckBox->setChecked(true);

    protectCheckBox = new QCheckBox(tr("&Protect header file against multiple "
                                       "inclusions"));
    protectCheckBox->setChecked(true);

    macroNameLabel = new QLabel(tr("&Macro name:"));
    macroNameLineEdit = new QLineEdit;
    macroNameLabel->setBuddy(macroNameLineEdit);

    includeBaseCheckBox = new QCheckBox(tr("&Include base class definition"));
    baseIncludeLabel = new QLabel(tr("Base class include:"));
    baseIncludeLineEdit = new QLineEdit;
    baseIncludeLabel->setBuddy(baseIncludeLineEdit);

    connect(protectCheckBox, &QAbstractButton::toggled,
            macroNameLabel, &QWidget::setEnabled);
    connect(protectCheckBox, &QAbstractButton::toggled,
            macroNameLineEdit, &QWidget::setEnabled);
    connect(includeBaseCheckBox, &QAbstractButton::toggled,
            baseIncludeLabel, &QWidget::setEnabled);
    connect(includeBaseCheckBox, &QAbstractButton::toggled,
            baseIncludeLineEdit, &QWidget::setEnabled);

    registerField("comment", commentCheckBox);
    registerField("protect", protectCheckBox);
    registerField("macroName", macroNameLineEdit);
    registerField("includeBase", includeBaseCheckBox);
    registerField("baseInclude", baseIncludeLineEdit);

    QGridLayout *layout = new QGridLayout;
    layout->setColumnMinimumWidth(0, 20);
    layout->addWidget(commentCheckBox, 0, 0, 1, 3);
    layout->addWidget(protectCheckBox, 1, 0, 1, 3);
    layout->addWidget(macroNameLabel, 2, 1);
    layout->addWidget(macroNameLineEdit, 2, 2);
    layout->addWidget(includeBaseCheckBox, 3, 0, 1, 3);
    layout->addWidget(baseIncludeLabel, 4, 1);
    layout->addWidget(baseIncludeLineEdit, 4, 2);

    setLayout(layout);
}


///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void CodeStylePage::initializePage()
{
    QString className = field("className").toString();
    macroNameLineEdit->setText(className.toUpper() + "_H");

    QString baseClass = field("baseClass").toString();

    includeBaseCheckBox->setChecked(!baseClass.isEmpty());
    includeBaseCheckBox->setEnabled(!baseClass.isEmpty());
    baseIncludeLabel->setEnabled(!baseClass.isEmpty());
    baseIncludeLineEdit->setEnabled(!baseClass.isEmpty());

    QRegularExpression rx("Q[A-Z].*");
    if (baseClass.isEmpty()) {
        baseIncludeLineEdit->clear();
    } else if (rx.match(baseClass).hasMatch()) {
        baseIncludeLineEdit->setText('<' + baseClass + '>');
    } else {
        baseIncludeLineEdit->setText('"' + baseClass.toLower() + ".h\"");
    }
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - OutputFilesPage
//

OutputFilesPage::OutputFilesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Output Files"));
    setSubTitle(tr("Specify where you want the wizard to put the generated "
                   "skeleton code."));
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo3.png"));

    outputDirLabel = new QLabel(tr("&Output directory:"));
    outputDirLineEdit = new QLineEdit;
    outputDirLabel->setBuddy(outputDirLineEdit);

    headerLabel = new QLabel(tr("&Header file name:"));
    headerLineEdit = new QLineEdit;
    headerLabel->setBuddy(headerLineEdit);

    implementationLabel = new QLabel(tr("&Implementation file name:"));
    implementationLineEdit = new QLineEdit;
    implementationLabel->setBuddy(implementationLineEdit);

    registerField("outputDir*", outputDirLineEdit);
    registerField("header*", headerLineEdit);
    registerField("implementation*", implementationLineEdit);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(outputDirLabel, 0, 0);
    layout->addWidget(outputDirLineEdit, 0, 1);
    layout->addWidget(headerLabel, 1, 0);
    layout->addWidget(headerLineEdit, 1, 1);
    layout->addWidget(implementationLabel, 2, 0);
    layout->addWidget(implementationLineEdit, 2, 1);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void OutputFilesPage::initializePage()
{
    QString className = field("className").toString();
    headerLineEdit->setText(className.toLower() + ".h");
    implementationLineEdit->setText(className.toLower() + ".cpp");
    outputDirLineEdit->setText(QDir::toNativeSeparators(QDir::tempPath()));
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConclusionPage
//

ConclusionPage::ConclusionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Conclusion"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark2.png"));

    label = new QLabel;
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void ConclusionPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');
    label->setText(tr("String")
                   .arg(finishText));
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigStringPage
//

ConfigStringPage::ConfigStringPage(QWidget *parent, 
                                    const std::string& title,
                                    const std::string& subtitle)
    : QWizardPage(parent)
{
    setTitle(QString::fromUtf8(title.c_str()));
    setSubTitle(QString::fromUtf8(subtitle.c_str()));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark2.png"));

    m_label = new QLabel;
    m_label->setWordWrap(true);
    m_edit = new QLineEdit;
    m_label->setBuddy(m_edit);

    registerField("edit1", m_edit);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_label, 0, 0);
    layout->addWidget(m_edit, 0, 1);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void ConfigStringPage::initializePage()
{
    QString stringText = wizard()->buttonText(QWizard::FinishButton);
    //finishText.remove('&');
    m_label->setText(tr("Value")
                   .arg(stringText));
    QString cfgstr = field("cfgstr").toString();
    m_edit->setText(cfgstr);
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigBoolPage
//

ConfigBoolPage::ConfigBoolPage(QWidget *parent, 
                                const std::string& title,
                                const std::string& subtitle)
    : QWizardPage(parent)
{
    setTitle(QString::fromUtf8(title.c_str()));
    setSubTitle(QString::fromUtf8(subtitle.c_str()));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark2.png"));

    m_checkbox = new QCheckBox(tr("&bool"));
    m_checkbox->setChecked(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_checkbox, 0, 0);
    //layout->addWidget(m_edit, 0, 1);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void ConfigBoolPage::initializePage()
{
    // QString stringText = wizard()->buttonText(QWizard::FinishButton);
    // //finishText.remove('&');
    // m_label->setText(tr("Value")
    //                .arg(stringText));
    // QString cfgstr = field("cfgstr").toString();
    // m_edit->setText(cfgstr);
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// CTor - ConfigChoicePage
//

ConfigChoicePage::ConfigChoicePage(QWidget *parent, 
                                    const std::string& title,
                                    const std::string& subtitle)
    : QWizardPage(parent)
{
    setTitle(QString::fromUtf8(title.c_str()));
    setSubTitle(QString::fromUtf8(subtitle.c_str()));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark2.png"));

    m_label = new QLabel;
    m_label->setWordWrap(true);
    m_list = new QListWidget;
    m_label->setBuddy(m_list);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_list);
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////
// initializePage
//

void ConfigChoicePage::initializePage()
{
    // QString stringText = wizard()->buttonText(QWizard::FinishButton);
    // //finishText.remove('&');
    m_label->setText(tr("Select on of the available choices"));
    // QString cfgstr = field("cfgstr").toString();
    // m_edit->setText(cfgstr);
}
