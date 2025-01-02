// cldgselectconstraint.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#ifdef WIN32
#include <pch.h>
#endif

#include "cdlgselectconstraint.h"
#include "ui_cdlgselectconstraint.h"

#include <QMessageBox>
#include <QFileDialog>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgSelectConstraint::CDlgSelectConstraint(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgSelectConstraint)
{
    ui->setupUi(this);

    connect(ui->listConstraints, &QListWidget::itemClicked, this, &CDlgSelectConstraint::constraintSelected ); 
    connect(ui->listConstraints, &QListWidget::itemDoubleClicked, this, &CDlgSelectConstraint::constraintSelected );
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgSelectConstraint::~CDlgSelectConstraint()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
// getSelectedConstraint
//

int
CDlgSelectConstraint::getSelectedConstraint(void)
{
    return ui->listConstraints->currentRow();
}

///////////////////////////////////////////////////////////////////////////////
// constraintSelected
//

void
CDlgSelectConstraint::constraintSelected(QListWidgetItem *item)
{
    accept();
}



// ----------------------------------------------------------------------------



