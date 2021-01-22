/********************************************************************************
** Form generated from reading UI file 'cdlgconnsettingstcpip.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CDLGCONNSETTINGSTCPIP_H
#define CDLGCONNSETTINGSTCPIP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CDlgConnSettingsTcpip
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *m_description;
    QLabel *label_2;
    QLineEdit *m_host;
    QLabel *label_3;
    QLineEdit *m_user;
    QLabel *label_4;
    QLineEdit *m_password;
    QLabel *label_5;
    QCheckBox *checkBox_fullLevel2;
    QLineEdit *m_port;
    QLabel *portLabel;
    QComboBox *m_interface;
    QPushButton *pushButtonSetFilter;
    QPushButton *pushButtonTestConnection;
    QPushButton *pushButtonGetInterfaces;
    QPushButton *pushButtonSSL;

    void setupUi(QDialog *CDlgConnSettingsTcpip)
    {
        if (CDlgConnSettingsTcpip->objectName().isEmpty())
            CDlgConnSettingsTcpip->setObjectName(QString::fromUtf8("CDlgConnSettingsTcpip"));
        CDlgConnSettingsTcpip->resize(547, 382);
        CDlgConnSettingsTcpip->setBaseSize(QSize(0, 0));
        buttonBox = new QDialogButtonBox(CDlgConnSettingsTcpip);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(0, 340, 511, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(true);
        formLayoutWidget = new QWidget(CDlgConnSettingsTcpip);
        formLayoutWidget->setObjectName(QString::fromUtf8("formLayoutWidget"));
        formLayoutWidget->setGeometry(QRect(20, 10, 501, 221));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(formLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        m_description = new QLineEdit(formLayoutWidget);
        m_description->setObjectName(QString::fromUtf8("m_description"));

        formLayout->setWidget(0, QFormLayout::FieldRole, m_description);

        label_2 = new QLabel(formLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        m_host = new QLineEdit(formLayoutWidget);
        m_host->setObjectName(QString::fromUtf8("m_host"));

        formLayout->setWidget(1, QFormLayout::FieldRole, m_host);

        label_3 = new QLabel(formLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(4, QFormLayout::LabelRole, label_3);

        m_user = new QLineEdit(formLayoutWidget);
        m_user->setObjectName(QString::fromUtf8("m_user"));

        formLayout->setWidget(4, QFormLayout::FieldRole, m_user);

        label_4 = new QLabel(formLayoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(5, QFormLayout::LabelRole, label_4);

        m_password = new QLineEdit(formLayoutWidget);
        m_password->setObjectName(QString::fromUtf8("m_password"));

        formLayout->setWidget(5, QFormLayout::FieldRole, m_password);

        label_5 = new QLabel(formLayoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(6, QFormLayout::LabelRole, label_5);

        checkBox_fullLevel2 = new QCheckBox(formLayoutWidget);
        checkBox_fullLevel2->setObjectName(QString::fromUtf8("checkBox_fullLevel2"));

        formLayout->setWidget(7, QFormLayout::FieldRole, checkBox_fullLevel2);

        m_port = new QLineEdit(formLayoutWidget);
        m_port->setObjectName(QString::fromUtf8("m_port"));

        formLayout->setWidget(2, QFormLayout::FieldRole, m_port);

        portLabel = new QLabel(formLayoutWidget);
        portLabel->setObjectName(QString::fromUtf8("portLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, portLabel);

        m_interface = new QComboBox(formLayoutWidget);
        m_interface->setObjectName(QString::fromUtf8("m_interface"));

        formLayout->setWidget(6, QFormLayout::FieldRole, m_interface);

        pushButtonSetFilter = new QPushButton(CDlgConnSettingsTcpip);
        pushButtonSetFilter->setObjectName(QString::fromUtf8("pushButtonSetFilter"));
        pushButtonSetFilter->setGeometry(QRect(390, 250, 131, 25));
        pushButtonSetFilter->setMinimumSize(QSize(0, 0));
        pushButtonSetFilter->setBaseSize(QSize(0, 0));
        pushButtonTestConnection = new QPushButton(CDlgConnSettingsTcpip);
        pushButtonTestConnection->setObjectName(QString::fromUtf8("pushButtonTestConnection"));
        pushButtonTestConnection->setGeometry(QRect(390, 280, 131, 25));
        pushButtonGetInterfaces = new QPushButton(CDlgConnSettingsTcpip);
        pushButtonGetInterfaces->setObjectName(QString::fromUtf8("pushButtonGetInterfaces"));
        pushButtonGetInterfaces->setGeometry(QRect(390, 310, 131, 25));
        pushButtonSSL = new QPushButton(CDlgConnSettingsTcpip);
        pushButtonSSL->setObjectName(QString::fromUtf8("pushButtonSSL"));
        pushButtonSSL->setGeometry(QRect(30, 250, 61, 25));
        pushButtonSSL->setMinimumSize(QSize(0, 0));
        pushButtonSSL->setBaseSize(QSize(0, 0));

        retranslateUi(CDlgConnSettingsTcpip);
        QObject::connect(buttonBox, SIGNAL(accepted()), CDlgConnSettingsTcpip, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), CDlgConnSettingsTcpip, SLOT(reject()));

        QMetaObject::connectSlotsByName(CDlgConnSettingsTcpip);
    } // setupUi

    void retranslateUi(QDialog *CDlgConnSettingsTcpip)
    {
        CDlgConnSettingsTcpip->setWindowTitle(QCoreApplication::translate("CDlgConnSettingsTcpip", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Description", nullptr));
        label_2->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Host URL", nullptr));
        m_host->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "tcp://localhost", nullptr));
        label_3->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Username", nullptr));
        m_user->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "admin", nullptr));
        label_4->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Password", nullptr));
        m_password->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "secret", nullptr));
        label_5->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Interface", nullptr));
        checkBox_fullLevel2->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Full Level II", nullptr));
        m_port->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "9598", nullptr));
        portLabel->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Port", nullptr));
        pushButtonSetFilter->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Set filter", nullptr));
        pushButtonTestConnection->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Test connection", nullptr));
        pushButtonGetInterfaces->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "Get interfaces", nullptr));
        pushButtonSSL->setText(QCoreApplication::translate("CDlgConnSettingsTcpip", "SSL", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CDlgConnSettingsTcpip: public Ui_CDlgConnSettingsTcpip {};
} // namespace Ui

QT_END_NAMESPACE

#endif // CDLGCONNSETTINGSTCPIP_H
