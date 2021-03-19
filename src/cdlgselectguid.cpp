// cdlgselectguid.cpp
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

#include "cdlgselectguid.h"
#include "ui_cdlgselectguid.h"

#include <QMessageBox>
#include <QFileDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectGuid::CDlgSelectGuid(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectGuid)
{
    ui->setupUi(this);

    // connect(ui->btnSetCaFile, &QPushButton::clicked, this, &CDlgSelectGuid::onSetCaFile );
    // connect(ui->btnSetCaPath, &QPushButton::clicked, this, &CDlgSelectGuid::onSetCaPath ); 
    // connect(ui->btnSetCertFile, &QPushButton::clicked, this, &CDlgSelectGuid::onSetCertFile ); 
    // connect(ui->btnSetKeyFile, &QPushButton::clicked, this, &CDlgSelectGuid::onSetKeyFile ); 
    // connect(ui->btnSetPasswordKeyFile, &QPushButton::clicked, this, &CDlgSelectGuid::onSetPwKeyFile );   
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectGuid::~CDlgSelectGuid()
{
    delete ui;
}




// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// onSetCaFile
//

// void
// CDlgSelectGuid::onSetCaFile(void)
// {
//     QFileDialog dlg;
//     QString str = dlg.getOpenFileName(this,
//                                       "Select CA file",
//                                       ".",
//                                       tr("Certificates (*.ca *.crt);;All (*)"));
//     if (str.length()) {
//         setCaFile(str);
//     }
// }

