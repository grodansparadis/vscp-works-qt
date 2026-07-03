// cfrmmeasurementview.cpp
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

#include "cfrmmeasurementview.h"

#include <QComboBox>
#include <QDateTime>
#include <QDial>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QPainter>
#include <QStackedWidget>
#include <QTextStream>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <algorithm>
#include <cmath>
#include <cstring>

bool
CMeasurementSourceSpec::matches(const vscp_event_t* pev) const
{
  if (nullptr == pev) {
    return false;
  }

  if (!vscp_isMeasurement(pev)) {
    return false;
  }

  if ((pev->vscp_class != vscpClass) || (pev->vscp_type != vscpType)) {
    return false;
  }

  if (vscp_getMeasurementSensorIndex(pev) != sensorIndex) {
    return false;
  }

  if (vscp_getMeasurementUnit(pev) != unit) {
    return false;
  }

  return !memcmp(pev->GUID, guid.data(), guid.size());
}

CFrmMeasurementView::CFrmMeasurementView(const CMeasurementSourceSpec& source,
                                         const QString& sourceDescription,
                                         QWidget* parent)
  : QWidget(parent, Qt::Window)
  , m_source(source)
  , m_sourceDescription(sourceDescription)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setupUi();
}

CFrmMeasurementView::~CFrmMeasurementView()
{
}

bool
CFrmMeasurementView::matchesEvent(const vscp_event_t* pev) const
{
  return m_source.matches(pev);
}

void
CFrmMeasurementView::appendMeasurement(const vscp_event_t* pev)
{
  if (!matchesEvent(pev)) {
    return;
  }

  double value = 0;
  if (vscp_getMeasurementAsDouble(&value, const_cast<vscp_event_t*>(pev))) {
    return;
  }

  if (m_samples.empty()) {
    m_minSeen = value;
    m_maxSeen = value;
  }
  else {
    m_minSeen = std::min(m_minSeen, value);
    m_maxSeen = std::max(m_maxSeen, value);
  }

  measurementSample sample;
  sample.timestamp = QDateTime::currentDateTimeUtc();
  sample.value     = value;
  m_samples.push_back(sample);

  if (m_samples.size() > MAX_SAMPLES) {
    m_samples.erase(m_samples.begin(), m_samples.begin() + (m_samples.size() - MAX_SAMPLES));
  }

  refreshDisplay(value);
}

void
CFrmMeasurementView::setupUi()
{
  setWindowTitle(tr("Realtime Measurement"));
  resize(860, 560);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QHBoxLayout* topLayout = new QHBoxLayout();
  topLayout->addWidget(new QLabel(tr("Display mode:"), this));

  m_modeCombo = new QComboBox(this);
  m_modeCombo->addItem(tr("Diagram"));
  m_modeCombo->addItem(tr("Value"));
  m_modeCombo->addItem(tr("Speed meter"));
  m_modeCombo->addItem(tr("Percent"));
  topLayout->addWidget(m_modeCombo, 0);

  topLayout->addStretch(1);

  m_saveButton = new QPushButton(tr("Save..."), this);
  topLayout->addWidget(m_saveButton);

  m_clearButton = new QPushButton(tr("Clear"), this);
  topLayout->addWidget(m_clearButton);

  mainLayout->addLayout(topLayout);

  m_sourceLabel = new QLabel(m_sourceDescription, this);
  m_sourceLabel->setWordWrap(true);
  mainLayout->addWidget(m_sourceLabel);

  m_displayStack = new QStackedWidget(this);
  mainLayout->addWidget(m_displayStack, 1);

  // Diagram page
  QWidget* pageDiagram    = new QWidget(this);
  QVBoxLayout* diagLayout = new QVBoxLayout(pageDiagram);
  m_chart                 = new QtCharts::QChart();
  m_chart->legend()->hide();
  m_chart->setTitle(tr("Measurement history"));
  m_lineSeries = new QtCharts::QLineSeries();
  m_chart->addSeries(m_lineSeries);
  m_axisX = new QtCharts::QValueAxis();
  m_axisX->setTitleText(tr("Sample"));
  m_axisX->setRange(0, MAX_SAMPLES);
  m_chart->addAxis(m_axisX, Qt::AlignBottom);
  m_lineSeries->attachAxis(m_axisX);
  m_axisY = new QtCharts::QValueAxis();
  m_axisY->setTitleText(tr("Value"));
  m_axisY->setRange(0, 100);
  m_chart->addAxis(m_axisY, Qt::AlignLeft);
  m_lineSeries->attachAxis(m_axisY);
  QtCharts::QChartView* chartView = new QtCharts::QChartView(m_chart, pageDiagram);
  chartView->setRenderHint(QPainter::Antialiasing);
  diagLayout->addWidget(chartView);
  m_displayStack->addWidget(pageDiagram);

  // Value page
  QWidget* pageValue      = new QWidget(this);
  QVBoxLayout* valueLayout = new QVBoxLayout(pageValue);
  m_valueLabel             = new QLabel("--", pageValue);
  m_valueLabel->setAlignment(Qt::AlignCenter);
  QFont valueFont = m_valueLabel->font();
  valueFont.setPointSize(34);
  valueFont.setBold(true);
  m_valueLabel->setFont(valueFont);
  valueLayout->addStretch(1);
  valueLayout->addWidget(m_valueLabel);
  valueLayout->addStretch(1);
  m_displayStack->addWidget(pageValue);

  // Speed meter page
  QWidget* pageSpeed      = new QWidget(this);
  QVBoxLayout* speedLayout = new QVBoxLayout(pageSpeed);
  m_speedMeter             = new QDial(pageSpeed);
  m_speedMeter->setNotchesVisible(true);
  m_speedMeter->setWrapping(false);
  m_speedMeter->setEnabled(false);
  m_speedMeter->setRange(0, 100);
  m_speedValueLabel = new QLabel("--", pageSpeed);
  m_speedValueLabel->setAlignment(Qt::AlignCenter);
  speedLayout->addStretch(1);
  speedLayout->addWidget(m_speedMeter, 0, Qt::AlignCenter);
  speedLayout->addWidget(m_speedValueLabel);
  speedLayout->addStretch(1);
  m_displayStack->addWidget(pageSpeed);

  // Percent page
  QWidget* pagePercent      = new QWidget(this);
  QVBoxLayout* percentLayout = new QVBoxLayout(pagePercent);
  m_percentBar               = new QProgressBar(pagePercent);
  m_percentBar->setRange(0, 100);
  m_percentBar->setTextVisible(false);
  m_percentValueLabel = new QLabel("--", pagePercent);
  m_percentValueLabel->setAlignment(Qt::AlignCenter);
  percentLayout->addStretch(1);
  percentLayout->addWidget(m_percentBar);
  percentLayout->addWidget(m_percentValueLabel);
  percentLayout->addStretch(1);
  m_displayStack->addWidget(pagePercent);

  connect(m_modeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &CFrmMeasurementView::setDisplayMode);
  connect(m_saveButton, &QPushButton::clicked, this, &CFrmMeasurementView::saveSamples);
  connect(m_clearButton, &QPushButton::clicked, this, &CFrmMeasurementView::clearSamples);
}

void
CFrmMeasurementView::setDisplayMode(int index)
{
  m_mode = static_cast<displayMode>(index);
  m_displayStack->setCurrentIndex(index);
  if (!m_samples.empty()) {
    refreshDisplay(m_samples.back().value);
  }
}

void
CFrmMeasurementView::refreshDisplay(double latestValue)
{
  refreshDiagram();
  refreshValueDisplay(latestValue);
  refreshSpeedMeterDisplay(latestValue);
  refreshPercentDisplay(latestValue);
}

void
CFrmMeasurementView::refreshDiagram()
{
  if (nullptr == m_lineSeries) {
    return;
  }

  QVector<QPointF> points;
  points.reserve(static_cast<int>(m_samples.size()));

  for (size_t i = 0; i < m_samples.size(); ++i) {
    points.push_back(QPointF(static_cast<qreal>(i), m_samples[i].value));
  }

  m_lineSeries->replace(points);

  if (m_samples.empty()) {
    m_axisX->setRange(0, MAX_SAMPLES);
    m_axisY->setRange(0, 100);
    return;
  }

  m_axisX->setRange(0, std::max(1.0, static_cast<double>(m_samples.size() - 1)));

  double minY = m_minSeen;
  double maxY = m_maxSeen;

  if (std::fabs(maxY - minY) < 0.000001) {
    minY -= 1.0;
    maxY += 1.0;
  }
  else {
    const double margin = (maxY - minY) * 0.1;
    minY -= margin;
    maxY += margin;
  }

  m_axisY->setRange(minY, maxY);
}

void
CFrmMeasurementView::refreshValueDisplay(double value)
{
  if (nullptr == m_valueLabel) {
    return;
  }

  m_valueLabel->setText(QString::number(value, 'f', 3));
}

void
CFrmMeasurementView::refreshSpeedMeterDisplay(double value)
{
  if ((nullptr == m_speedMeter) || (nullptr == m_speedValueLabel)) {
    return;
  }

  double minValue = m_minSeen;
  double maxValue = m_maxSeen;
  if (std::fabs(maxValue - minValue) < 0.000001) {
    minValue -= 1.0;
    maxValue += 1.0;
  }

  m_speedMeter->setRange(static_cast<int>(std::floor(minValue)),
                         static_cast<int>(std::ceil(maxValue)));
  m_speedMeter->setValue(static_cast<int>(std::round(value)));
  m_speedValueLabel->setText(
    tr("%1  (range %2 .. %3)")
      .arg(QString::number(value, 'f', 3))
      .arg(QString::number(minValue, 'f', 2))
      .arg(QString::number(maxValue, 'f', 2)));
}

void
CFrmMeasurementView::refreshPercentDisplay(double value)
{
  if ((nullptr == m_percentBar) || (nullptr == m_percentValueLabel)) {
    return;
  }

  const int percent = static_cast<int>(std::round(std::max(0.0, std::min(100.0, value))));
  m_percentBar->setValue(percent);
  m_percentValueLabel->setText(
    tr("%1%  (raw %2)").arg(QString::number(percent)).arg(QString::number(value, 'f', 3)));
}

void
CFrmMeasurementView::saveSamples()
{
  QString path = QFileDialog::getSaveFileName(this,
                                              tr("Save measurement samples"),
                                              "",
                                              tr("CSV (*.csv);;All Files (*.*)"));
  if (path.isEmpty()) {
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this,
                         tr("Realtime Measurement"),
                         tr("Unable to open '%1' for writing.").arg(path));
    return;
  }

  QTextStream stream(&file);
  stream << "timestamp_utc,value\n";
  for (const auto& sample : m_samples) {
    stream << sample.timestamp.toString(Qt::ISODateWithMs) << "," << QString::number(sample.value, 'f', 8)
           << "\n";
  }
}

void
CFrmMeasurementView::clearSamples()
{
  m_samples.clear();
  m_minSeen = 0;
  m_maxSeen = 0;
  refreshDisplay(0);
}
