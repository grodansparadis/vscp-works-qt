// cdlgtls.cpp
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

#include "cdlgtls.h"
#include "../ui/ui_cdlgtls.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgTLS::CDlgTLS(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgTLS)
{
    ui->setupUi(this);

    connect(ui->btnSetCaFile, &QPushButton::clicked, this, &CDlgTLS::onSetCaFile );
    connect(ui->btnSetCaPath, &QPushButton::clicked, this, &CDlgTLS::onSetCaPath ); 
    connect(ui->btnSetCertFile, &QPushButton::clicked, this, &CDlgTLS::onSetCertFile ); 
    connect(ui->btnSetKeyFile, &QPushButton::clicked, this, &CDlgTLS::onSetKeyFile ); 
    connect(ui->btnSetPasswordKeyFile, &QPushButton::clicked, this, &CDlgTLS::onSetPwKeyFile );   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgTLS::~CDlgTLS()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// isTLSEnabled
//

bool 
CDlgTLS::isTLSEnabled(void)
{
    return ui->chkTlsEnable->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// enableTLS
//

void 
CDlgTLS::enableTLS(bool btls)
{
    return ui->chkTlsEnable->setChecked(btls);
}

///////////////////////////////////////////////////////////////////////////////
// isVerifyPeerEnabled
//

bool 
CDlgTLS::isVerifyPeerEnabled(void)
{
    return ui->chkVerifyPeer->isChecked(); 
}

///////////////////////////////////////////////////////////////////////////////
// enableVerifyPeer
//

void 
CDlgTLS::enableVerifyPeer(bool bverifypeer)
{
    return ui->chkVerifyPeer->setChecked(bverifypeer);
}

///////////////////////////////////////////////////////////////////////////////
// getCaFile
//

QString 
CDlgTLS::getCaFile(void)
{
    return ui->editCaFile->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setCaFile
//

void 
CDlgTLS::setCaFile(const QString& str)
{
    ui->editCaFile->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getCaPath
//

QString 
CDlgTLS::getCaPath(void)
{
    return ui->editCaPath->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setCaPath
//

void 
CDlgTLS::setCaPath(const QString& str)
{
    ui->editCaPath->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getCertFile
//

QString 
CDlgTLS::getCertFile(void)
{
    return ui->editCertFile->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setCertFile
//

void 
CDlgTLS::setCertFile(const QString& str)
{
    ui->editCertFile->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getKeyFile
//

QString 
CDlgTLS::getKeyFile(void)
{
    return ui->editKeyFile->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setKeyFile
//

void 
CDlgTLS::setKeyFile(const QString& str)
{
    ui->editKeyFile->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// getPwKeyFile
//

QString 
CDlgTLS::getPwKeyFile(void)
{
    return ui->editPwKeyFile->text(); 
}

///////////////////////////////////////////////////////////////////////////////
// setPwKeyFile
//

void 
CDlgTLS::setPwKeyFile(const QString& str)
{
    ui->editPwKeyFile->setText(str);
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// onSetCaFile
//

void
CDlgTLS::onSetCaFile(void)
{
    QFileDialog dlg;
    QString str = dlg.getOpenFileName(this,
                                      "Select CA file",
                                      ".",
                                      tr("Certificates (*.ca *.crt);;All (*)"));
    if (str.length()) {
        setCaFile(str);
    }
}

///////////////////////////////////////////////////////////////////////////////
// onSetCaPath
// 

void 
CDlgTLS::onSetCaPath(void)
{
    QFileDialog dlg;  
    QString str = dlg.getExistingDirectory(this,"Select CA path", ".");
    if (str.length()) {
        setCaPath(str);
    }
}

///////////////////////////////////////////////////////////////////////////////
// onSetCertFile
//

void
CDlgTLS::onSetCertFile(void)
{
    QFileDialog dlg;
    QString str = dlg.getOpenFileName(this,
                                      "Select cert file",
                                      ".",
                                      tr("Certificates (*.ca *.crt);;All (*)"));
    if (str.length()) {
        setCertFile(str);
    }
}

///////////////////////////////////////////////////////////////////////////////
// onSetKeyFile
//

void
CDlgTLS::onSetKeyFile(void)
{
    QFileDialog dlg;
    QString str = dlg.getOpenFileName(this,
                                      "Select key file",
                                      ".",
                                      tr("Key-files (*.key);;All (*)"));
    if (str.length()) {
        setKeyFile(str);
    }
}

///////////////////////////////////////////////////////////////////////////////
// onSetPwKeyFile
//

void
CDlgTLS::onSetPwKeyFile(void)
{
    QFileDialog dlg;
    QString str = dlg.getOpenFileName(this,
                                      "Select key file",
                                      ".",
                                      tr("Key-files (*.key);;All (*)"));
    if (str.length()) {
        setPwKeyFile(str);
    }
}