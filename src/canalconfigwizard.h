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
// Built on this example https://doc.qt.io/qt-5/qtwidgets-dialogs-classwizard-example.html
// https://code.qt.io/cgit/qt/qtbase.git/tree/examples/widgets/dialogs/classwizard/classwizard.cpp?h=5.15
//

#ifndef CANALCONFIGWIZARD_H
#define CANALCONFIGWIZARD_H

#include "vscp_client_canal.h"

#include <QWizard>


QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QListWidget;
QT_END_NAMESPACE

class CanalConfigWizard : public QWizard
{
    Q_OBJECT

public:
    CanalConfigWizard(QWidget *parent = nullptr);

    void accept() override;
};


// ----------------------------------------------------------------------------


class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(QWidget *parent = nullptr);

private:
    QLabel *label;
};


// ----------------------------------------------------------------------------


class ClassInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ClassInfoPage(QWidget *parent = nullptr);

private:
    QLabel *classNameLabel;
    QLabel *baseClassLabel;
    QLineEdit *classNameLineEdit;
    QLineEdit *baseClassLineEdit;
    QCheckBox *qobjectMacroCheckBox;
    QGroupBox *groupBox;
    QRadioButton *qobjectCtorRadioButton;
    QRadioButton *qwidgetCtorRadioButton;
    QRadioButton *defaultCtorRadioButton;
    QCheckBox *copyCtorCheckBox;
};


// ----------------------------------------------------------------------------

class CodeStylePage : public QWizardPage
{
    Q_OBJECT

public:
    CodeStylePage(QWidget *parent = nullptr);

protected:
    void initializePage() override;

private:
    QCheckBox *commentCheckBox;
    QCheckBox *protectCheckBox;
    QCheckBox *includeBaseCheckBox;
    QLabel *macroNameLabel;
    QLabel *baseIncludeLabel;
    QLineEdit *macroNameLineEdit;
    QLineEdit *baseIncludeLineEdit;
};


// ----------------------------------------------------------------------------

class OutputFilesPage : public QWizardPage
{
    Q_OBJECT

public:
    OutputFilesPage(QWidget *parent = nullptr);

protected:
    void initializePage() override;

private:
    QLabel *outputDirLabel;
    QLabel *headerLabel;
    QLabel *implementationLabel;
    QLineEdit *outputDirLineEdit;
    QLineEdit *headerLineEdit;
    QLineEdit *implementationLineEdit;
};


// ----------------------------------------------------------------------------

class ConclusionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConclusionPage(QWidget *parent = nullptr);

protected:
    void initializePage() override;

private:
    QLabel *label;    
};


// ----------------------------------------------------------------------------
//                                    Pages
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------


class ConfigStringPage : public QWizardPage
{
    Q_OBJECT

public:
    ConfigStringPage(QWidget *parent = nullptr, 
                        const std::string& title = "Set configuration: string",
                        const std::string& subtitle = "");

protected:
    void initializePage() override;

private:
    QLabel *m_label;
    QLineEdit *m_edit;
};


// ----------------------------------------------------------------------------


class ConfigBoolPage : public QWizardPage
{
    Q_OBJECT

public:
    ConfigBoolPage(QWidget *parent = nullptr, 
                    const std::string& title = "Set configuration: bool",
                    const std::string& subtitle="");

protected:
    void initializePage() override;

private:
    QCheckBox *m_checkbox;
};


// ----------------------------------------------------------------------------


class ConfigChoicePage : public QWizardPage
{
    Q_OBJECT

public:
    ConfigChoicePage(QWidget *parent = nullptr, 
                        const std::string& title = "Set configuration: choice",
                        const std::string& subtitle="");

protected:
    void initializePage() override;

private:
    QLabel *m_label;
    QListWidget *m_list;
};

#endif