// cdlgeditsessionfilter.cpp
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

#include <vscp.h>
#include <vscphelper.h>

#include "sessionfilter.h"

#include "cdlgselectclass.h"
#include "cdlgselectguid.h"
#include "cdlgselectobid.h"
#include "cdlgselectdate.h"
#include "cdlgselecttimestamp.h"
#include "cdlgselectdata.h"
#include "cdlgselectdatasize.h"
#include "cdlgselectpriority.h"
#include "cdlgselectsensorindex.h"
#include "cdlgselectmeasurementvalue.h"
#include "cdlgselectunit.h"
#include "cdlgselectdatacoding.h"

#include "cdlgeditsessionfilter.h"
#include "ui_cdlgeditsessionfilter.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QInputDialog>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgEditSessionFilter::CDlgEditSessionFilter(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgEditSessionFilter)
{
    ui->setupUi(this);

    connect(ui->btnAddAllowConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::addAllowConstraint );
    connect(ui->btnEditAllowConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::editAllowConstraint ); 
    connect(ui->btnEditAllowConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::editAllowConstraint ); 

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listAllow,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgEditSessionFilter::showAllowContextMenu); 

    // connect(ui->btnAddDenyConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::addDenyConstraint );
    // connect(ui->btnEditDenyConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::editDenyConstraint ); 
    // connect(ui->btnDeleteDenyConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::deleteDenyConstraint ); 

    // // Open pop up menu on right click on VSCP type listbox
    // connect(ui->listDeny,
    //         &QListWidget::customContextMenuRequested,
    //         this,
    //         &CDlgEditSessionFilter::showDenyContextMenu);   
    
    setInitialFocus(); 
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

CDlgEditSessionFilter::~CDlgEditSessionFilter()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// setInitialFocus
//

void CDlgEditSessionFilter::setInitialFocus(void)
{
    ui->editName->setFocus();
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// isConstraintDefined
//

bool
CDlgEditSessionFilter::isConstraintDefined(uint8_t chk)
{
    for (int row=0; row<ui->listAllow->count(); row++) {
        if (ui->listAllow->item(row)->data(role_constraint_type) == chk) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::addAllowConstraint(void)
{
    QStringList items;
    items   << tr("Select constraint to add")
            << tr("[1] - Must be received event")
            << tr("[2] - Must be transmitted event")
            << tr("[3] - Class/type") 
            << tr("[4] - GUID") 
            << tr("[5] - OBID") 
            << tr("[6] - Date")
            << tr("[7] - Timestamp")
            << tr("[8] - Data value(s)")
            << tr("[9] - Date size")
            << tr("[10] - Priority")
            << tr("[11] - Must be measurement")
            << tr("[12] - Must be Level I")
            << tr("[13] - Must be Level II")
            << tr("[14] - Sensor Index")
            << tr("[15] - Measurement Value")
            << tr("[16] - Measurement Unit")
            << tr("[17] - Measurement data coding")
            << tr("[18] - Script");

    bool ok;
    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                         tr("Constraint:"), items, 0, false, &ok);
    if (ok && !item.isEmpty()) {

        if (item.contains(tr("[1]"))) {
            // Must be received event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_receive) && 
                  isConstraintDefined(CSessionFilter::type_must_be_receive))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_receive);
                item->setText(tr("00 - Must be received event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addReceiveConstraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        else if (item.contains(tr("[2]"))) {
            // Must be transitted event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_receive) && 
                  isConstraintDefined(CSessionFilter::type_must_be_receive))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("01 - Must be transmitt event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addTransmitConstraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        else if (item.contains(tr("[12]"))) {
            // Must be Level I event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("01 - Must be transmitt event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addLevel1Constraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        else if (item.contains(tr("[13]"))) {
            // Must be Level II event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("01 - Must be transmitt event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addLevel2Constraint(true);
            }
            else {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        // Class
        else if (item.contains(tr("[3]"))) {
            CDlgSelectClass dlg;

            if (isConstraintDefined(CSessionFilter::type_class)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
            }

            if (QDialog::Accepted == dlg.exec()) {
                
                QList<QListWidgetItem *> selected_classes = dlg.getSelectedClasses();
                if (!selected_classes.size()) {
                    QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No class defined. Att least one must be selected"),
                              QMessageBox::Ok );
                    return;          
                }

                for (int i=0; i<selected_classes.size(); i++) {
                    getSessionFilter()->addClassConstraint(selected_classes[i]->data(Qt::UserRole).toUInt());
                }

                QList<QListWidgetItem *> selected_types = dlg.getSelectedTypes();

                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_class);
                item->setText(tr("03 - Must be VSCP Class / Type"));
                ui->listAllow->addItem(item);
            }            
        }
        else if (item.contains(tr("[4]"))) { 
            // GUID
            CDlgSelectGuid dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[5]"))) { 
            // OBID
            CDlgSelectObId dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[6]"))) {
            // Date
            CDlgSelectDate dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[7]"))) {
            // Timestamp
            CDlgSelectTimeStamp dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[8]"))) {
            // Data content
            CDlgSelectData dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[9]"))) {
            // Data size
            CDlgSelectDataSize dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[10]"))) {
            // Priority
            CDlgSelectPriority dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[11]"))) {
            // Must be mesurement
            // Must be Level II event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_measurement))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_measurement);
                item->setText(tr("11 - Must be measurement event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addLevel2Constraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        else if (item.contains(tr("[12]"))) {
            // Must be Level I
            // Must be Level II event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_level1);
                item->setText(tr("12 - Must be Level I event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addLevel2Constraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }
        else if (item.contains(tr("[13]"))) {
            // Must be level II
            // Must be Level II event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_level2);
                item->setText(tr("13 - Must be Level II event"));
                ui->listAllow->addItem(item);
                m_sessionFilter.addLevel2Constraint(true);
            }
            else {
                // This constraint is already set
                 QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set"),
                              QMessageBox::Ok );
            }
        }        
        else if (item.contains(tr("[14]"))) {
            // Measurement sensor index
            CDlgSelectSensorIndex dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[15]"))) {
            // Measurement value
            CDlgSelectMeasurementValue dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[16]"))) {
            // Measurement unit
            CDlgSelectMeasurementUnit dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[17]"))) {
            // Measurement data coding
            CDlgSelectDataCoding dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[18]"))) {
            // Script
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::editAllowConstraint(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::deleteAllowConstraint(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::showAllowContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    menu->addAction(QString(tr("Add allow filter constraint...")),
                    this,
                    SLOT(deleteAllowConstraint()));

    menu->addAction(QString(tr("Edit allow filter constraint...")),
                    this,
                    SLOT(deleteAllowConstraint()));

    menu->addAction(QString(tr("Delete allow filter constraint")),
                    this,
                    SLOT(deleteAllowConstraint()));

    menu->popup(ui->listAllow->viewport()->mapToGlobal(pos));
}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::addDenyConstraint(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::editDenyConstraint(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::deleteDenyConstraint(void)
{

}

///////////////////////////////////////////////////////////////////////////////
// addAllowConstraint
//

void
CDlgEditSessionFilter::showDenyContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    menu->addAction(QString(tr("Add deny filter constraint...")),
                    this,
                    SLOT(deleteDenyConstraint()));

    menu->addAction(QString(tr("Edit deny filter constraint...")),
                    this,
                    SLOT(deleteDenyConstraint()));

    menu->addAction(QString(tr("Delete deny filter constraint")),
                    this,
                    SLOT(deleteDenyConstraint()));

    //menu->popup(ui->listDeny->viewport()->mapToGlobal(pos));
}