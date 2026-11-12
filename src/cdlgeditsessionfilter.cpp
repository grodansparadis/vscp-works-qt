// cdlgeditsessionfilter.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
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

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"
#include "sessionfilter.h"

#include "cdlgselectconstraint.h"
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
#include <QDesktopServices>
#include <QShortcut>
#include <QDebug>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

CDlgEditSessionFilter::CDlgEditSessionFilter(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CDlgEditSessionFilter)
{
  ui->setupUi(this);

  connect(ui->btnAddConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::addConstraint);
  connect(ui->btnEditConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::editConstraint);
  connect(ui->btnDeleteConstraint, &QPushButton::clicked, this, &CDlgEditSessionFilter::deleteConstraint);
  connect(ui->listConstraints, &QListWidget::itemDoubleClicked, this, &CDlgEditSessionFilter::editConstraint);
  ui->listConstraints->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->chkAllow->setChecked(true);

  // Open pop up menu on right click on VSCP type listbox
  connect(ui->listConstraints,
          &QListWidget::customContextMenuRequested,
          this,
          &CDlgEditSessionFilter::showContextMenu);

  // Help
  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
  shortcut->setAutoRepeat(false);

  QPushButton* helpButton = ui->buttonBox->button(QDialogButtonBox::Help);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

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

void
CDlgEditSessionFilter::setInitialFocus(void)
{
  ui->editName->setFocus();
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// setFilter
//

void
CDlgEditSessionFilter::setFilter(const CSessionFilter& filter)
{
  m_sessionFilter = filter;
  ui->editName->setText(QString::fromStdString(filter.getName()));
  ui->textDescription->setPlainText(QString::fromStdString(filter.getDescription()));
  ui->chkAllow->setChecked(filter.isAllow());
  refreshConstraintList();
}

///////////////////////////////////////////////////////////////////////////////
// getFilter
//

CSessionFilter
CDlgEditSessionFilter::getFilter(void)
{
  m_sessionFilter.setName(ui->editName->text().trimmed().toStdString());
  m_sessionFilter.setDescription(ui->textDescription->toPlainText().toStdString());
  m_sessionFilter.setAllow(ui->chkAllow->isChecked());
  return m_sessionFilter;
}

///////////////////////////////////////////////////////////////////////////////
// refreshConstraintList
//

void
CDlgEditSessionFilter::refreshConstraintList(void)
{
  int row = ui->listConstraints->currentRow();
  ui->listConstraints->clear();
  for (uint8_t type : m_sessionFilter.getConstraintTypes()) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setData(role_constraint_type, type);
    item->setText(QString("%1 - %2")
                    .arg(type, 2, 10, QChar('0'))
                    .arg(tr(CSessionFilter::getConstraintTypeName(type).c_str())));
    ui->listConstraints->addItem(item);
  }
  if (row >= ui->listConstraints->count()) {
    row = ui->listConstraints->count() - 1;
  }
  ui->listConstraints->setCurrentRow(row);
}

///////////////////////////////////////////////////////////////////////////////
// accept
//

void
CDlgEditSessionFilter::accept(void)
{
  if (ui->editName->text().trimmed().isEmpty()) {
    QMessageBox::information(this, tr(APPNAME), tr("The filter must have a name."), QMessageBox::Ok);
    ui->editName->setFocus();
    return;
  }
  QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////
// isConstraintDefined
//

bool
CDlgEditSessionFilter::isConstraintDefined(uint8_t chk)
{
  return m_sessionFilter.hasConstraint(chk);
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintReceive
//

void
CDlgEditSessionFilter::addConstraintReceive()
{
  // Received or transmitted, not both
  m_sessionFilter.removeConstraint(CSessionFilter::type_must_be_transmit);
  m_sessionFilter.addReceiveConstraint(true);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintReceive

//

void
CDlgEditSessionFilter::editConstraintReceive()
{
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintTransmit
//

void
CDlgEditSessionFilter::addConstraintTransmit()
{
  // Received or transmitted, not both
  m_sessionFilter.removeConstraint(CSessionFilter::type_must_be_receive);
  m_sessionFilter.addTransmitConstraint(true);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintTransmit
//

void
CDlgEditSessionFilter::editConstraintTransmit()
{
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintLevel1
//

void
CDlgEditSessionFilter::addConstraintLevel1()
{
  // Level I or Level II, not both
  m_sessionFilter.removeConstraint(CSessionFilter::type_must_be_level2);
  m_sessionFilter.addLevel1Constraint(true);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintLevel1
//

void
CDlgEditSessionFilter::editConstraintLevel1()
{
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintLevel2
//

void
CDlgEditSessionFilter::addConstraintLevel2()
{
  // Level I or Level II, not both
  m_sessionFilter.removeConstraint(CSessionFilter::type_must_be_level1);
  m_sessionFilter.addLevel2Constraint(true);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintLevel2
//

void
CDlgEditSessionFilter::editConstraintLevel2()
{
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintClass
//

void
CDlgEditSessionFilter::addConstraintClass(void)
{
  if (isConstraintDefined(CSessionFilter::type_class)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  CDlgSelectClass dlg;

  if (QDialog::Accepted == dlg.exec()) {

    std::deque<uint16_t> selected_classes = dlg.getSelectedClasses();
    if (!selected_classes.size()) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("No class defined. Att least one class need to be selected"),
                               QMessageBox::Ok);
      return;
    }

    // Add the selected classes to the filter
    for (int i = 0; i < selected_classes.size(); i++) {
      getSessionFilter()->addClassConstraint(selected_classes[i]);
    }

    std::deque<uint32_t> selected_types = dlg.getSelectedTypes();

    // Add the selected types to the filter
    for (int i = 0; i < selected_types.size(); i++) {
      getSessionFilter()->addTypeConstraint(selected_types[i]);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintClass
//

void
CDlgEditSessionFilter::editConstraintClass(void)
{
  CDlgSelectClass dlg;

  dlg.selectClasses(m_sessionFilter.getClasses());
  dlg.selectTypes(m_sessionFilter.getTypes());

  if (QDialog::Accepted == dlg.exec()) {

    std::deque<uint16_t> selected_classes = dlg.getSelectedClasses();
    if (!selected_classes.size()) {
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("No class defined. Att least one class need to be selected"),
                               QMessageBox::Ok);
      return;
    }

    getSessionFilter()->clearClasses();
    getSessionFilter()->clearTypes();

    // Add the selected classes to the filter
    for (int i = 0; i < selected_classes.size(); i++) {
      qDebug() << selected_classes[i];
      getSessionFilter()->addClassConstraint(selected_classes[i]);
    }

    std::deque<uint32_t> selected_types = dlg.getSelectedTypes();

    // Add the selected types to the filter
    for (int i = 0; i < selected_types.size(); i++) {
      getSessionFilter()->addTypeConstraint(selected_types[i]);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintGuid
//

void
CDlgEditSessionFilter::addConstraintGuid(void)
{
  if (isConstraintDefined(CSessionFilter::type_guid)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // GUID
  CDlgSelectGuid dlg;

  if (QDialog::Accepted == dlg.exec()) {

    for (int i = 0; i < 16; i++) {
      qDebug() << dlg.getGuidValue(i);
      getSessionFilter()->addGuidConstraint(i,
                                            dlg.getGuidValue(i),
                                            dlg.getGuidConstraint(i));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintGuid
//

void
CDlgEditSessionFilter::editConstraintGuid(void)
{
  CDlgSelectGuid dlg;

  std::deque<uint32_t> listGuid = getSessionFilter()->getGuids();
  for (auto const& item : listGuid) {
    qDebug() << (item & 0xff) << " " << ((item >> 16) & 0xff);
    dlg.setGuidValue((item >> 16) & 0xff, item & 0xff);
    dlg.setGuidConstraint((item >> 16) & 0xff,
                          static_cast<CSessionFilter::constraint>((item >> 8) & 0xff));
  }

  if (QDialog::Accepted == dlg.exec()) {

    for (int i = 0; i < 16; i++) {
      getSessionFilter()->addGuidConstraint(i,
                                            dlg.getGuidValue(i),
                                            dlg.getGuidConstraint(i));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintObid
//

void
CDlgEditSessionFilter::addConstraintObid(void)
{
  if (isConstraintDefined(CSessionFilter::type_obid)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  CDlgSelectObId dlg;

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addObidConstraint(dlg.getObidValue(),
                                      dlg.getObidConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintObid
//

void
CDlgEditSessionFilter::editConstraintObid(void)
{
  CDlgSelectObId dlg;

  dlg.setObidValue(m_sessionFilter.getObidValue());
  dlg.setObidConstraint(m_sessionFilter.getObidConstraint());

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addObidConstraint(dlg.getObidValue(),
                                      dlg.getObidConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintDate
//

void
CDlgEditSessionFilter::addConstraintDate(void)
{
  uint16_t value;
  CSessionFilter::constraint op;

  if (isConstraintDefined(CSessionFilter::type_date)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Date
  CDlgSelectDate dlg;
  if (QDialog::Accepted == dlg.exec()) {

    // Year
    value = dlg.getYearValue();
    if (value > 9999)
      value = 1956; // Well at least more realistic :)
    op = dlg.getYearConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_year,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_year);
    }

    // Month
    value = dlg.getMonthValue();
    if ((value > 12) || !value)
      value = 1;
    op = dlg.getMonthConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_month,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_month);
    }

    // Day
    value = dlg.getDayValue();
    if ((value > 31) || !value)
      value = 1;
    op = dlg.getDayConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_day,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_day);
    }

    // Hour
    value = dlg.getHourValue();
    if ((value > 23))
      value = 0;
    op = dlg.getHourConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_hour,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_hour);
    }

    // Minute
    value = dlg.getMinuteValue();
    if ((value > 59))
      value = 0;
    op = dlg.getMinuteConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_minute,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_minute);
    }

    // Second
    value = dlg.getSecondValue();
    if ((value > 59))
      value = 0;
    op = dlg.getSecondConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_second,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_second);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintDate
//

void
CDlgEditSessionFilter::editConstraintDate(void)
{
  uint16_t value;
  uint32_t cv;
  CSessionFilter::constraint op;

  // Date
  CDlgSelectDate dlg;

  // year
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_year);
  if (cv) {
    dlg.setYearValue(cv & 0xffff);
    dlg.setYearConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  // month
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_month);
  if (cv) {
    dlg.setMonthValue(cv & 0xff);
    dlg.setMonthConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  // day
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_day);
  if (cv) {
    dlg.setDayValue(cv & 0xffff);
    dlg.setDayConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  // hour
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_hour);
  if (cv) {
    dlg.setHourValue(cv & 0xff);
    dlg.setHourConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  // minute
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_minute);
  if (cv) {
    dlg.setMinuteValue(cv & 0xff);
    dlg.setMinuteConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  // second
  cv = m_sessionFilter.getDateConstraint(CSessionFilter::date_pos_second);
  if (cv) {
    dlg.setSecondValue(cv & 0xffff);
    dlg.setSecondConstraint(static_cast<CSessionFilter::constraint>((cv >> 16) & 0x0f));
  }

  if (QDialog::Accepted == dlg.exec()) {

    // Year
    value = dlg.getYearValue();
    if (value > 9999)
      value = 1956; // Well at least more realistic :)
    op = dlg.getYearConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_year,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_year);
    }

    // Month
    value = dlg.getMonthValue();
    if ((value > 12) || !value)
      value = 1;
    op = dlg.getMonthConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_month,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_month);
    }

    // Day
    value = dlg.getDayValue();
    if ((value > 31) || !value)
      value = 1;
    op = dlg.getDayConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_day,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_day);
    }

    // Hour
    value = dlg.getHourValue();
    if ((value > 23))
      value = 0;
    op = dlg.getHourConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_hour,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_hour);
    }

    // Minute
    value = dlg.getMinuteValue();
    if ((value > 59))
      value = 0;
    op = dlg.getMinuteConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_minute,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_minute);
    }

    // Second
    value = dlg.getSecondValue();
    if ((value > 59))
      value = 0;
    op = dlg.getSecondConstraint();
    if (CSessionFilter::constraint::ANY != op) {
      m_sessionFilter.addDateConstraint(CSessionFilter::date_pos_second,
                                        value,
                                        op);
    }
    else {
      m_sessionFilter.removeDateConstraint(CSessionFilter::date_pos_second);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintTimeStamp
//

void
CDlgEditSessionFilter::addConstraintTimeStamp(void)
{
  if (isConstraintDefined(CSessionFilter::type_timestamp)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Timestamp
  CDlgSelectTimeStamp dlg;
  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addTimeStampConstraint(dlg.getTimeStampValue(),
                                           dlg.getTimeStampConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintTimeStamp
//

void
CDlgEditSessionFilter::editConstraintTimeStamp(void)
{
  CDlgSelectTimeStamp dlg;

  dlg.setTimeStampValue(m_sessionFilter.getTimeStampValue());
  dlg.setTimeStampConstraint(m_sessionFilter.getTimeStampConstraint());

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addTimeStampConstraint(dlg.getTimeStampValue(),
                                           dlg.getTimeStampConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintData
//

void
CDlgEditSessionFilter::addConstraintData(void)
{
  if (isConstraintDefined(CSessionFilter::type_data)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Data content
  CDlgSelectData dlg;

  if (QDialog::Accepted == dlg.exec()) {

    std::deque<uint32_t> listData = dlg.getData();
    m_sessionFilter.addDataConstraints(listData);
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintData
//

void
CDlgEditSessionFilter::editConstraintData(void)
{
  // Data content
  CDlgSelectData dlg;

  std::deque<uint32_t> listData = m_sessionFilter.getDataConstraints();
  dlg.setData(listData);

  if (QDialog::Accepted == dlg.exec()) {

    std::deque<uint32_t> listData = dlg.getData();
    m_sessionFilter.addDataConstraints(listData);
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintDataSize
//

void
CDlgEditSessionFilter::addConstraintDataSize(void)
{
  if (isConstraintDefined(CSessionFilter::type_data_size)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Data size
  CDlgSelectDataSize dlg;

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addDataSizeConstraint(dlg.getDataSizeValue(), dlg.getDataSizeConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintDataSize
//

void
CDlgEditSessionFilter::editConstraintDataSize(void)
{
  // Data size
  CDlgSelectDataSize dlg;

  dlg.setDataSizeValue(m_sessionFilter.getDataSizeValue());
  dlg.setDataSizeConstraint(m_sessionFilter.getDataSizeConstraint());

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addDataSizeConstraint(dlg.getDataSizeValue(), dlg.getDataSizeConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintPriority
//

void
CDlgEditSessionFilter::addConstraintPriority(void)
{
  if (isConstraintDefined(CSessionFilter::type_priority)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Priority
  CDlgSelectPriority dlg;
  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addPriorityConstraint(dlg.getPriorityValue(), dlg.getPriorityConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintPriority
//

void
CDlgEditSessionFilter::editConstraintPriority(void)
{
  // Priority
  CDlgSelectPriority dlg;

  dlg.setPriorityValue(m_sessionFilter.getPriorityValue());
  dlg.setPriorityConstraint(m_sessionFilter.getPriorityConstraint());

  if (QDialog::Accepted == dlg.exec()) {

    m_sessionFilter.addPriorityConstraint(dlg.getPriorityValue(), dlg.getPriorityConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintMeasurement
//

void
CDlgEditSessionFilter::addConstraintMeasurement(void)
{
  m_sessionFilter.addMeasurementConstraint(true);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintMeasurement
//

void
CDlgEditSessionFilter::editConstraintMeasurement(void)
{
  // No edit to do
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintSensorIndex
//

void
CDlgEditSessionFilter::addConstraintSensorIndex(void)
{
  if (isConstraintDefined(CSessionFilter::type_sensor_index)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Measurement sensor index
  CDlgSelectSensorIndex dlg;
  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addSensorIndexConstraint(dlg.getSensorIndexValue(), dlg.getSensorIndexConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintSensorIndex
//

void
CDlgEditSessionFilter::editConstraintSensorIndex(void)
{
  // Sensor Index
  CDlgSelectSensorIndex dlg;

  dlg.setSensorIndexValue(m_sessionFilter.getSensorIndexValue());
  dlg.setSensorIndexConstraint(m_sessionFilter.getSensorIndexConstraint());

  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addSensorIndexConstraint(dlg.getSensorIndexValue(), dlg.getSensorIndexConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintValue
//

void
CDlgEditSessionFilter::addConstraintValue(void)
{
  if (isConstraintDefined(CSessionFilter::type_value)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Measurement value
  CDlgSelectMeasurementValue dlg;
  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementValueConstraint(dlg.getMeasurementValue(),
                                                  dlg.getMeasurementValueConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintValue
//

void
CDlgEditSessionFilter::editConstraintValue(void)
{
  // Sensor Index
  CDlgSelectMeasurementValue dlg;

  dlg.setMeasurementValue(m_sessionFilter.getMeasurementValue());
  dlg.setMeasurementValueConstraint(m_sessionFilter.getMeasurementValueConstraint());

  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementValueConstraint(dlg.getMeasurementValue(),
                                                  dlg.getMeasurementValueConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintUnit
//

void
CDlgEditSessionFilter::addConstraintUnit(void)
{
  if (isConstraintDefined(CSessionFilter::type_unit)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Measurement unit
  CDlgSelectMeasurementUnit dlg;
  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementUnitConstraint(dlg.getMeasurementUnitValue(),
                                                 dlg.getMeasurementUnitConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintUnit
//

void
CDlgEditSessionFilter::editConstraintUnit(void)
{
  // Sensor Index
  CDlgSelectMeasurementUnit dlg;

  dlg.setMeasurementUnitValue(m_sessionFilter.getMeasurementUnit());
  dlg.setMeasurementUnitConstraint(m_sessionFilter.getMeasurementUnitConstraint());

  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementUnitConstraint(dlg.getMeasurementUnitValue(),
                                                 dlg.getMeasurementUnitConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintDataCoding
//

void
CDlgEditSessionFilter::addConstraintDataCoding(void)
{
  if (isConstraintDefined(CSessionFilter::type_data_coding)) {
    // This constraint is already set
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("This constraint is already set. Use edit instead of add."),
                             QMessageBox::Ok);
    return;
  }

  // Measurement data coding
  CDlgSelectDataCoding dlg;
  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementDataCodingConstraint(dlg.getDataCodingValue(),
                                                       dlg.getDataCodingConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintDataCoding
//

void
CDlgEditSessionFilter::editConstraintDataCoding(void)
{
  // Sensor Index
  CDlgSelectDataCoding dlg;

  dlg.setDataCodingValue(m_sessionFilter.getMeasurementDataCoding());
  dlg.setDataCodingConstraint(m_sessionFilter.getMeasurementDataCodingConstraint());

  if (QDialog::Accepted == dlg.exec()) {
    m_sessionFilter.addMeasurementDataCodingConstraint(dlg.getDataCodingValue(),
                                                       dlg.getDataCodingConstraint());
  }
}

///////////////////////////////////////////////////////////////////////////////
// addConstraintScript
//

void
CDlgEditSessionFilter::addConstraintScript(void)
{
  QMessageBox::information(this,
                           tr(APPNAME),
                           tr("Script constraints are not supported yet."),
                           QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////
// editConstraintScript
//

void
CDlgEditSessionFilter::editConstraintScript(void)
{
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addConstraint
//

void
CDlgEditSessionFilter::addConstraint(void)
{
  CDlgSelectConstraint dlgSelect;
  if (QDialog::Accepted != dlgSelect.exec()) {
    return;
  }

  int constraint = dlgSelect.getSelectedConstraint();

  // Must be receive event
  if (CSessionFilter::type_must_be_receive == constraint) {
    addConstraintReceive();
  }

  // Must be transmit event
  else if (CSessionFilter::type_must_be_transmit == constraint) {
    addConstraintTransmit();
  }

  // Must be Level I
  else if (CSessionFilter::type_must_be_level1 == constraint) {
    addConstraintLevel1();
  }

  // Must be level II
  else if (CSessionFilter::type_must_be_level2 == constraint) {
    addConstraintLevel2();
  }

  // Add Class / Type
  else if ((CSessionFilter::type_class == constraint) ||
           (CSessionFilter::type_type == constraint)) {
    if (isConstraintDefined(CSessionFilter::type_class)) {
      editConstraintClass();
    }
    else {
      addConstraintClass();
    }
  }

  // * * * Add GUID
  else if (CSessionFilter::type_guid == constraint) {
    addConstraintGuid();
  }

  // * * * Add  OBID
  else if (CSessionFilter::type_obid == constraint) {
    addConstraintObid();
  }

  // * * * Add Date
  else if (CSessionFilter::type_date == constraint) {
    addConstraintDate();
  }

  // * * * Add
  else if (CSessionFilter::type_timestamp == constraint) {
    addConstraintTimeStamp();
  }

  // * * * Add
  else if (CSessionFilter::type_data == constraint) {
    addConstraintData();
  }

  // * * * Add
  else if (CSessionFilter::type_data_size == constraint) {
    addConstraintDataSize();
  }

  // * * * Add
  else if (CSessionFilter::type_priority == constraint) {
    addConstraintPriority();
  }

  // * * * Add
  else if (CSessionFilter::type_must_be_measurement == constraint) {
    addConstraintMeasurement();
  }

  // * * * Add
  else if (CSessionFilter::type_sensor_index == constraint) {
    addConstraintSensorIndex();
  }

  // * * * Add
  else if (CSessionFilter::type_value == constraint) {
    addConstraintValue();
  }

  // * * * Add
  else if (CSessionFilter::type_unit == constraint) {
    addConstraintUnit();
  }

  // * * * Add
  else if (CSessionFilter::type_data_coding == constraint) {
    addConstraintDataCoding();
  }

  // * * * Add
  else if (CSessionFilter::type_script == constraint) {
    addConstraintScript();
  }

  refreshConstraintList();
}

///////////////////////////////////////////////////////////////////////////////
// editConstraint
//

void
CDlgEditSessionFilter::editConstraint(void)
{
  if (-1 == ui->listConstraints->currentRow()) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("A constraint must be selected"),
                             QMessageBox::Ok);
    return;
  }

  QListWidgetItem* item = ui->listConstraints->currentItem();
  switch (item->data(role_constraint_type).toInt()) {

    // * * * Edit receive
    case CSessionFilter::type_must_be_receive:
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("There is nothing to edit for this constraint"),
                               QMessageBox::Ok);
      return;

    // * * * Edit transmit
    case CSessionFilter::type_must_be_transmit:
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("There is nothing to edit for this constraint"),
                               QMessageBox::Ok);
      return;

    // * * * Edit leve1
    case CSessionFilter::type_must_be_level1:
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("There is nothing to edit for this constraint"),
                               QMessageBox::Ok);
      return;

    // * * * Edit  level2
    case CSessionFilter::type_must_be_level2:
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("There is nothing to edit for this constraint"),
                               QMessageBox::Ok);
      return;

    // * * * Edit measurement
    case CSessionFilter::type_must_be_measurement:
      QMessageBox::information(this,
                               tr(APPNAME),
                               tr("There is nothing to edit for this constraint"),
                               QMessageBox::Ok);
      return;

    // * * * Edit Class
    case CSessionFilter::type_class:
      editConstraintClass();
      break;

    // * * * Edit Type
    case CSessionFilter::type_type:
      editConstraintClass();
      break;

    // * * * Edit Priority
    case CSessionFilter::type_priority:
      editConstraintPriority();
      break;

    // * * * Edit GUID
    case CSessionFilter::type_guid:
      editConstraintGuid();
      break;

    // * * * Edit OBID
    case CSessionFilter::type_obid:
      editConstraintObid();
      break;

    // * * * Edit
    case CSessionFilter::type_timestamp:
      editConstraintTimeStamp();
      break;

    // * * * Edit
    case CSessionFilter::type_date:
      editConstraintDate();
      break;

    // * * * Edit
    case CSessionFilter::type_data_size:
      editConstraintDataSize();
      break;

    // * * * Edit
    case CSessionFilter::type_data:
      editConstraintData();
      break;

    // * * * Edit
    case CSessionFilter::type_sensor_index:
      editConstraintSensorIndex();
      break;

    // * * * Edit
    case CSessionFilter::type_unit:
      editConstraintUnit();
      break;

    // * * * Edit
    case CSessionFilter::type_data_coding:
      editConstraintDataCoding();
      break;

    // * * * Edit
    case CSessionFilter::type_value:
      editConstraintValue();
      break;

    // * * * Edit
    case CSessionFilter::type_script:
      editConstraintScript();
      break;
  }

  refreshConstraintList();
}

///////////////////////////////////////////////////////////////////////////////
// deleteConstraint
//

void
CDlgEditSessionFilter::deleteConstraint(void)
{
  QListWidgetItem* item = ui->listConstraints->currentItem();
  if (nullptr == item) {
    QMessageBox::information(this,
                             tr(APPNAME),
                             tr("A constraint must be selected"),
                             QMessageBox::Ok);
    return;
  }

  m_sessionFilter.removeConstraint(item->data(role_constraint_type).toInt());
  refreshConstraintList();
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


///////////////////////////////////////////////////////////////////////////////
// showHelp
//

void
CDlgEditSessionFilter::showHelp(void)
{
  QString link = "https://grodansparadis.github.io/vscp-works-qt/#/session_window?id=filters";
  QDesktopServices::openUrl(QUrl(link));
}