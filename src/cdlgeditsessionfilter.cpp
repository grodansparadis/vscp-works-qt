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
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgEditSessionFilter::CDlgEditSessionFilter(QWidget *parent) :
        QDialog(parent),
    ui(new Ui::CDlgEditSessionFilter)
{
    ui->setupUi(this);

    connect(ui->btnAddConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::addConstraint );
    connect(ui->btnEditConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::editConstraint ); 
    connect(ui->btnDeleteConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::deleteConstraint ); 

    // Open pop up menu on right click on VSCP type listbox
    connect(ui->listConstraints,
            &QListWidget::customContextMenuRequested,
            this,
            &CDlgEditSessionFilter::showContextMenu); 
    
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
    for (int row=0; row < ui->listConstraints->count(); row++) {
        if (ui->listConstraints->item(row)->data(role_constraint_type) == chk) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// addConstraint
//

void
CDlgEditSessionFilter::addConstraint(void)
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

        // Must be receive event
        if (item.contains(tr("[1]"))) {
            // Must be received event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_receive) && 
                  isConstraintDefined(CSessionFilter::type_must_be_receive))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_receive);
                item->setText(tr("00 - Must be received event"));
                ui->listConstraints->addItem(item);
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
        // Must be transmit event
        else if (item.contains(tr("[2]"))) {
            // Must be transitted event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_receive) && 
                  isConstraintDefined(CSessionFilter::type_must_be_receive))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("01 - Must be transmitt event"));
                ui->listConstraints->addItem(item);
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
        // Must be Level I
        else if (item.contains(tr("[12]"))) {
            // Must be Level I event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("12 - Must be level I event"));
                ui->listConstraints->addItem(item);
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
        // Must be level II
        else if (item.contains(tr("[13]"))) {
            // Must be Level II event (no meaning to have both)
            if (!(isConstraintDefined(CSessionFilter::type_must_be_level1) && 
                  isConstraintDefined(CSessionFilter::type_must_be_level2))) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_must_be_transmit);
                item->setText(tr("13 - Must be level 2 event"));
                ui->listConstraints->addItem(item);
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

            if (isConstraintDefined(CSessionFilter::type_class)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            CDlgSelectClass dlg;

            if (QDialog::Accepted == dlg.exec()) {
                
                std::deque<uint16_t> selected_classes = dlg.getSelectedClasses();
                if (!selected_classes.size()) {
                    QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("No class defined. Att least one class need to be selected"),
                              QMessageBox::Ok );
                    return;          
                }

                // Add the selected classes to the filter
                for (int i=0; i<selected_classes.size(); i++) {
                    getSessionFilter()->addClassConstraint(selected_classes[i]);
                }

                std::deque<uint32_t> selected_types = dlg.getSelectedTypes();

                // Add the selected types to the filter
                for (int i=0; i<selected_types.size(); i++) {
                    getSessionFilter()->addTypeConstraint(selected_types[i]);
                }

                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_class);
                item->setText(tr("03 - Must be specific VSCP Class/Type"));
                ui->listConstraints->addItem(item);
            }            
        }
        // * * * GUID
        else if (item.contains(tr("[4]"))) { 

            if (isConstraintDefined(CSessionFilter::type_guid)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // GUID
            CDlgSelectGuid dlg;
            if (QDialog::Accepted == dlg.exec()) {

                for (int i=0; i<16; i++) {
                    
                }

                QListWidgetItem *item = new QListWidgetItem();
                item->setData(role_constraint_type, CSessionFilter::type_guid);
                item->setText(tr("04 - Must be specific GUID"));
                ui->listConstraints->addItem(item);
            }

        }
        else if (item.contains(tr("[5]"))) {

            if (isConstraintDefined(CSessionFilter::type_obid)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // OBID
            CDlgSelectObId dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[6]"))) {

            if (isConstraintDefined(CSessionFilter::type_date)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Date
            CDlgSelectDate dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[7]"))) {

            if (isConstraintDefined(CSessionFilter::type_timestamp)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Timestamp
            CDlgSelectTimeStamp dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[8]"))) {

            if (isConstraintDefined(CSessionFilter::type_data)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Data content
            CDlgSelectData dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[9]"))) {

            if (isConstraintDefined(CSessionFilter::type_data_size)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Data size
            CDlgSelectDataSize dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[10]"))) {

            if (isConstraintDefined(CSessionFilter::type_priority)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

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
                ui->listConstraints->addItem(item);
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
                ui->listConstraints->addItem(item);
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
                ui->listConstraints->addItem(item);
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

            if (isConstraintDefined(CSessionFilter::type_sensor_index)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Measurement sensor index
            CDlgSelectSensorIndex dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[15]"))) {

            if (isConstraintDefined(CSessionFilter::type_value)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Measurement value
            CDlgSelectMeasurementValue dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[16]"))) {

            if (isConstraintDefined(CSessionFilter::type_unit)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Measurement unit
            CDlgSelectMeasurementUnit dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[17]"))) {

            if (isConstraintDefined(CSessionFilter::type_data_coding)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }

            // Measurement data coding
            CDlgSelectDataCoding dlg;
            if (QDialog::Accepted == dlg.exec()) {

            }
        }
        else if (item.contains(tr("[18]"))) {

            if (isConstraintDefined(CSessionFilter::type_script)) {
                // This constraint is already set
                QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("This constraint is already set. Use edit instead of add."),
                              QMessageBox::Ok );
                return;                               
            }
            // Script
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// editConstraint
//

void
CDlgEditSessionFilter::editConstraint(void)
{
    if (-1 == ui->listConstraints->currentRow()){
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("A constraint must be selected"),
                              QMessageBox::Ok );
        return;                              
    }

    QListWidgetItem *item = ui->listConstraints->currentItem();
    switch( item->data(role_constraint_type).toInt()) {

        case CSessionFilter::type_must_be_receive:
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("There is nothing to edit for this constraint"),
                              QMessageBox::Ok );
            return;

        case CSessionFilter::type_must_be_transmit:
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("There is nothing to edit for this constraint"),
                              QMessageBox::Ok );
            return;

        case CSessionFilter::type_must_be_level1:
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("There is nothing to edit for this constraint"),
                              QMessageBox::Ok );
            return;

        case CSessionFilter::type_must_be_level2:
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("There is nothing to edit for this constraint"),
                              QMessageBox::Ok );
            return;

        case CSessionFilter::type_must_be_measurement:
            QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("There is nothing to edit for this constraint"),
                              QMessageBox::Ok );
            return;

        case CSessionFilter::type_class:
            {
                CDlgSelectClass dlg;

                dlg.selectClasses(m_sessionFilter.getClasses());
                dlg.selectTypes(m_sessionFilter.getTypes());

                if (QDialog::Accepted == dlg.exec()) {

                    std::deque<uint16_t> selected_classes = dlg.getSelectedClasses();
                    if (!selected_classes.size()) {
                        QMessageBox::information(this, 
                                tr("vscpworks+"),
                                tr("No class defined. Att least one class need to be selected"),
                                QMessageBox::Ok );
                        return;          
                    }

                    getSessionFilter()->clearClasses();
                    getSessionFilter()->clearTypes();

                    // Add the selected classes to the filter
                    for (int i=0; i<selected_classes.size(); i++) {
                        qDebug() << selected_classes[i];
                        getSessionFilter()->addClassConstraint(selected_classes[i]);
                    }

                    std::deque<uint32_t> selected_types = dlg.getSelectedTypes();

                    // Add the selected types to the filter
                    for (int i=0; i<selected_types.size(); i++) {
                        getSessionFilter()->addTypeConstraint(selected_types[i]);
                    }
                    
                }

            }
            break;

        case CSessionFilter::type_type:
            break;

        case CSessionFilter::type_priority:
            break;

        case CSessionFilter::type_guid:
            break;

        case CSessionFilter::type_obid:
            break;

        case CSessionFilter::type_timestamp:
            break;

        case CSessionFilter::type_date:
            break;

        case CSessionFilter::type_data_size:
            break;

        case CSessionFilter::type_data:
            break;

        case CSessionFilter::type_sensor_index:
            break;

        case CSessionFilter::type_unit:
            break;

        case CSessionFilter::type_data_coding:
            break;

        case CSessionFilter::type_value:
            break;

        case CSessionFilter::type_script:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// deleteConstraint
//

void
CDlgEditSessionFilter::deleteConstraint(void)
{
    if (-1 == ui->listConstraints->currentRow()){
        QMessageBox::information(this, 
                              tr("vscpworks+"),
                              tr("A constraint must be selected"),
                              QMessageBox::Ok );
        return;                              
    }
}

///////////////////////////////////////////////////////////////////////////////
// showContextMenu
//

void
CDlgEditSessionFilter::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    menu->addAction(QString(tr("Add constraint...")),
                    this,
                    SLOT(addConstraint()));

    menu->addAction(QString(tr("Edit constraint...")),
                    this,
                    SLOT(editConstraint()));

    menu->addAction(QString(tr("Delete constraint")),
                    this,
                    SLOT(deleteConstraint()));

    menu->popup(ui->listConstraints->viewport()->mapToGlobal(pos));
}

