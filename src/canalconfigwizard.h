// canalconfigwizard.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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

#include <canal_xmlconfig.h>
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
    IntroPage(QWidget *parent, const QString& title, const QString& lbltext, const QString& infolink="");

protected:
    void  initializePage() override;
    
private:
    QLabel *m_label;
    QLabel *m_labelInfo;

    QString m_labelText;
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
        @param value Initial value on string form
        @param infolink Link to information about setting
        @param width Number of bits for numerical value - determines maximum value
    */
    ConfigStringPage(QWidget *parent, 
                        wizardStepBase::wizardType type,
                        const QString& fieldname,
                        const QString& labeltext = "String value",
                        const QString& title = "Set configuration: string",
                        const QString& subtitle = "",
                        const QString& value = "",
                        const QString& infolink="",
                        uint8_t width = 0);

    /*! 
        Set text for label
        @param labeltext Labeltext to set
    */
    void setLabelText(const QString& labeltext);                       

public slots:
    /// Show info link URL content in browser
    void showInfo(void);

protected:

    /// Initialize the wizard page
    void  initializePage() override;

    /// Prevent load of defaults on "back"
    void  cleanupPage() override;

private:
    QLabel *m_label;
    QLineEdit *m_edit;
    QLabel *m_labelInfo;

    wizardStepBase::wizardType m_type;
    QString m_fieldName;
    QString m_labelText;
    QString m_value;
    QString m_infolink;
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
                    const QString& fieldname,
                    const QString& labeltext = "String value",
                    const QString& title = "Set configuration: string",
                    const QString& subtitle = "",
                    bool bvalue = false,
                    const QString& infolink="");

    /*! 
        Set text for label
        @param labeltext Labeltext to set
    */
    void setLabelText(const QString& labeltext);                       

public slots:
    /// Show info link URL content in browser
    void showInfo(void);                    

protected:
    /// Initialize the wizard page
    void  initializePage() override;

    /// Prevent load of defaults on "back"
    void  cleanupPage() override;       

private:
    QCheckBox *m_checkbox;
    QLabel *m_labelInfo;

    QString m_fieldName;
    QString m_labelText;
    QString m_infolink;

    bool m_bvalue;
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
                        const QString& fieldname,
                        const std::list<std::string>& strlist,
                        const QString& labeltext = "String value",
                        const QString& title = "Set configuration: string",
                        const QString& subtitle = "",
                        uint8_t nSelect = 0,
                        const QString& infolink="");

    /*! 
        Set text for label
        @param labeltext Labeltext to set
    */
    void setLabelText(const QString& labeltext);                       

// public slots:
//     /// Show info link URL content in browser
//     void showInfo(void);
    
protected:
    /// Initialize the wizard page
    void  initializePage() override;

    /// Prevent load of defaults on "back"
    void  cleanupPage() override;       

private:
    QLabel *m_label;
    QListWidget *m_list;
    QLabel *m_labelInfo;

    QString m_fieldName;
    QString m_labelText;
};

#endif