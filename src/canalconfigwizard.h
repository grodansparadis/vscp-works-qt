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


// ----------------------------------------------------------------------------


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


class ConclusionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConclusionPage(QWidget *parent = nullptr);

protected:
    void  initializePage() override;

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
    /*!
        CTor - Create config string class object
        @param parent Pointer to window that owns the wizard or nullptr
        @param fieldname Resulting name for the string field collted in action
        @param title Title to use in the wizard page
        @param subtitle Subtitle to use in the wizard page
    */
    ConfigStringPage(QWidget *parent, 
                        const std::string& fieldname,
                        const std::string& labeltext = "String value",
                        const std::string& title = "Set configuration: string",
                        const std::string& subtitle = "");

    void setLabelText(const std::string& labeltext);                       

protected:
    void  initializePage() override;
    void  cleanupPage() override;       // Prevent defaults on "back"

private:
    QLabel *m_label;
    QLineEdit *m_edit;

    std::string m_fieldName;
    std::string m_labelText;
};


// ----------------------------------------------------------------------------


class ConfigBoolPage : public QWizardPage
{
    Q_OBJECT

public:
    /*!
        CTor - Create config boolean class object
        @param parent Pointer to window that owns the wizard or nullptr
        @param fieldname Resulting name for the string field collted in action
        @param title Title to use in the wizard page
        @param subtitle Subtitle to use in the wizard page
    */
    ConfigBoolPage(QWidget *parent, 
                    const std::string& fieldname,
                    const std::string& labeltext = "String value",
                    const std::string& title = "Set configuration: string",
                    const std::string& subtitle = "");

protected:
    void  initializePage() override;
    void  cleanupPage() override;       // Prevent defaults on "back"

private:
    QCheckBox *m_checkbox;

    std::string m_fieldName;
    std::string m_labelText;
};


// ----------------------------------------------------------------------------


class ConfigChoicePage : public QWizardPage
{
    Q_OBJECT

public:
    /*!
        CTor - Create config choice class object
        @param parent Pointer to window that owns the wizard or nullptr
        @param fieldname Resulting name for the string field collted in action
        @param title Title to use in the wizard page
        @param subtitle Subtitle to use in the wizard page
    */
    ConfigChoicePage(QWidget *parent, 
                        const std::string& fieldname,
                        const std::list<std::string>& strlist,
                        const std::string& labeltext = "String value",
                        const std::string& title = "Set configuration: string",
                        const std::string& subtitle = "");

    
protected:
    void  initializePage() override;
    void  cleanupPage() override;       // Prevent defaults on "back"

private:
    QLabel *m_label;
    QListWidget *m_list;

    std::string m_fieldName;
    std::string m_labelText;
};

#endif