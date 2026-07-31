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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef WIN32
#include <pch.h>
#endif

#include "cfrmmeasurementview.h"

#include <vscphelper.h>
#include <vscpworks.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QToolButton>
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

class CarGaugeWidget : public QWidget
{
public:
  explicit CarGaugeWidget(QWidget* parent = nullptr)
    : QWidget(parent)
  {
    setMinimumSize(260, 260);
    setMaximumSize(520, 520);
  }

  void setRange(int minValue, int maxValue)
  {
    m_minValue = minValue;
    m_maxValue = maxValue;
    update();
  }

  void setValue(int value)
  {
    m_value = value;
    update();
  }

  void setStyle(int style)
  {
    m_style = style;
    update();
  }

protected:
  void paintEvent(QPaintEvent*) override
  {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int side = std::min(width(), height()) - 24;
    const QRectF gaugeRect((width() - side) / 2.0, (height() - side) / 2.0, side, side);
    const QPointF center = gaugeRect.center();
    const qreal radius = gaugeRect.width() / 2.0 - 24.0;
    const qreal startAngle = 135.0;
    const qreal spanAngle = 270.0;

    const double ratio = std::clamp((m_value - m_minValue) / std::max(1.0, double(m_maxValue - m_minValue)),
                                    0.0,
                                    1.0);
    const double angle = startAngle + spanAngle * ratio;

    QRadialGradient background(center, radius + 16.0);
    background.setColorAt(0.0, QColor(250, 250, 250));
    background.setColorAt(1.0, QColor(228, 232, 240));
    painter.setBrush(background);
    painter.setPen(QPen(QColor(190, 198, 210), 2));
    painter.drawEllipse(gaugeRect);

    painter.setPen(QPen(QColor(70, 70, 70), 10, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(gaugeRect.adjusted(10, 10, -10, -10), static_cast<int>(startAngle * 16.0), static_cast<int>(spanAngle * 16.0));

    QColor valueColor(220, 50, 50);
    QColor needleColor(QColor(40, 40, 40));
    QColor accentColor(QColor(255, 160, 0));
    if (m_style == 1) {
      valueColor = QColor(30, 120, 220);
      needleColor = QColor(30, 120, 220);
    }
    else if (m_style == 2) {
      valueColor = QColor(255, 160, 0);
      needleColor = QColor(220, 50, 50);
      accentColor = QColor(255, 160, 0);
    }
    painter.setPen(QPen(valueColor, 14, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(gaugeRect.adjusted(10, 10, -10, -10), static_cast<int>(startAngle * 16.0), static_cast<int>((spanAngle * ratio) * 16.0));

    if (m_style == 2) {
      painter.setPen(QPen(accentColor, 6, Qt::SolidLine, Qt::RoundCap));
      painter.drawArc(gaugeRect.adjusted(4, 4, -4, -4), static_cast<int>(startAngle * 16.0), static_cast<int>((spanAngle * ratio) * 16.0));
    }

    painter.setPen(QPen(QColor(80, 80, 80), 1.5));
    for (int tick = 0; tick <= 10; ++tick) {
      const double tickAngle = startAngle + (spanAngle * tick / 10.0);
      const qreal rad = tickAngle * M_PI / 180.0;
      const QPointF tickStart(center.x() + (radius - 16.0) * std::cos(rad),
                              center.y() + (radius - 16.0) * std::sin(rad));
      const QPointF tickEnd(center.x() + (radius - 6.0) * std::cos(rad),
                            center.y() + (radius - 6.0) * std::sin(rad));
      painter.drawLine(QLineF(tickStart, tickEnd));
    }

    painter.setPen(QPen(needleColor, 3, Qt::SolidLine, Qt::RoundCap));
    const qreal needleLength = radius - 34.0;
    const qreal needleAngle = angle * M_PI / 180.0;
    const QPointF needleEnd(center.x() + needleLength * std::cos(needleAngle),
                            center.y() + needleLength * std::sin(needleAngle));
    painter.drawLine(QLineF(center, needleEnd));

    painter.setBrush(QColor(20, 20, 20));
    if (m_style == 1) {
      painter.setBrush(QColor(30, 120, 220));
    }
    else if (m_style == 2) {
      painter.setBrush(QColor(255, 160, 0));
    }
    painter.drawEllipse(QRectF(center.x() - 10.0, center.y() - 10.0, 20.0, 20.0));

    painter.setPen(QColor(40, 40, 40));
    painter.setFont(QFont("Sans Serif", 18, QFont::Bold));
    QRectF valueRect = gaugeRect.adjusted(20, 20, -20, -20);
    valueRect.setTop(valueRect.top() + 26.0);
    valueRect.setBottom(valueRect.bottom() + 26.0);
    painter.drawText(valueRect, Qt::AlignCenter, QString::number(m_value));

    painter.setFont(QFont("Sans Serif", 9));
    painter.drawText(gaugeRect.adjusted(0, 0, 0, 0), Qt::AlignBottom | Qt::AlignHCenter, QString("%1 .. %2").arg(m_minValue).arg(m_maxValue));
  }

private:
  int m_minValue{ 0 };
  int m_maxValue{ 100 };
  int m_value{ 0 };
  int m_style{ 0 };
};

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
  if (!vscp_getMeasurementAsDouble(&value, const_cast<vscp_event_t*>(pev))) {
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

  evaluateAlarm(value);

  if (!m_paused) {
    refreshDisplay(value);
  }
}

void
CFrmMeasurementView::evaluateAlarm(double value)
{
  if (!m_alarmEnabled) {
    m_alarmActive = false;
    return;
  }

  const bool shouldAlarm = (m_alarmMode == 0) ? (value > m_alarmThreshold) : (value < m_alarmThreshold);
  if (shouldAlarm) {
    if (!m_alarmActive) {
      QApplication::beep();
      m_alarmActive = true;
    }
  }
  else {
    m_alarmActive = false;
  }
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
  m_modeCombo->addItem(tr("Gauge"));
  m_modeCombo->addItem(tr("Percent"));
  topLayout->addWidget(m_modeCombo, 0);

  m_gaugeStyleLabel = new QLabel(tr("Gauge style:"), this);
  m_gaugeStyleCombo = new QComboBox(this);
  m_gaugeStyleCombo->addItem(tr("Classic"));
  m_gaugeStyleCombo->addItem(tr("Blue needle"));
  m_gaugeStyleCombo->addItem(tr("Accent ring"));
  topLayout->addWidget(m_gaugeStyleLabel, 0);
  topLayout->addWidget(m_gaugeStyleCombo, 0);
  m_gaugeStyleLabel->setVisible(false);
  m_gaugeStyleCombo->setVisible(false);

  m_diagramAxisLabel = new QLabel(tr("Diagram axis:"), this);
  m_diagramAxisModeCombo = new QComboBox(this);
  m_diagramAxisModeCombo->addItem(tr("Realtime"));
  m_diagramAxisModeCombo->addItem(tr("Timestamp"));
  m_diagramAxisModeCombo->addItem(tr("HH:MM:SS"));
  topLayout->addWidget(m_diagramAxisLabel, 0);
  topLayout->addWidget(m_diagramAxisModeCombo, 0);
  m_diagramAxisLabel->setVisible(false);
  m_diagramAxisModeCombo->setVisible(false);

  topLayout->addWidget(new QLabel(tr("Alarm:"), this));
  m_alarmEnabledCheckBox = new QCheckBox(tr("Enable"), this);
  topLayout->addWidget(m_alarmEnabledCheckBox, 0);

  m_alarmValueSpinBox = new QDoubleSpinBox(this);
  m_alarmValueSpinBox->setRange(-1000000.0, 1000000.0);
  m_alarmValueSpinBox->setDecimals(3);
  m_alarmValueSpinBox->setSingleStep(1.0);
  m_alarmValueSpinBox->setValue(0.0);
  topLayout->addWidget(m_alarmValueSpinBox, 0);

  m_alarmModeCombo = new QComboBox(this);
  m_alarmModeCombo->addItem(tr("Above"));
  m_alarmModeCombo->addItem(tr("Below"));
  topLayout->addWidget(m_alarmModeCombo, 0);

  topLayout->addStretch(1);

  m_saveButton = new QPushButton(tr("Save..."), this);
  topLayout->addWidget(m_saveButton);

  m_clearButton = new QPushButton(tr("Clear"), this);
  topLayout->addWidget(m_clearButton);

  m_pauseButton = new QPushButton(tr("Pause"), this);
  m_pauseButton->setCheckable(true);
  topLayout->addWidget(m_pauseButton);

  mainLayout->addLayout(topLayout);

  m_sourceLabel = new QLabel(m_sourceDescription, this);
  m_sourceLabel->setWordWrap(true);
  mainLayout->addWidget(m_sourceLabel);

  m_unitLabel = new QLabel(tr("Unit: --"), this);
  m_unitLabel->setWordWrap(true);
  m_unitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  mainLayout->addWidget(m_unitLabel);

  m_displayStack = new QStackedWidget(this);
  mainLayout->addWidget(m_displayStack, 1);

  // Diagram page
  QWidget* pageDiagram    = new QWidget(this);
  pageDiagram->setStyleSheet("background-color: #fff9c4;");
  pageDiagram->setAutoFillBackground(true);
  QVBoxLayout* diagLayout = new QVBoxLayout(pageDiagram);
  m_chart                 = new QChart();
  m_chart->legend()->hide();
  m_chart->setTitle(tr("Measurement history"));
  m_lineSeries = new QLineSeries();
  m_chart->addSeries(m_lineSeries);
  m_chart->setBackgroundBrush(QBrush(QColor("#fff9c4")));
  m_chart->setPlotAreaBackgroundBrush(QBrush(QColor("#fff9c4")));
  m_axisX = new QValueAxis();
  m_axisX->setTitleText(tr("Time"));
  m_axisX->setRange(0, 100);
  m_chart->addAxis(m_axisX, Qt::AlignBottom);
  m_lineSeries->attachAxis(m_axisX);
  m_axisY = new QValueAxis();
  m_axisY->setTitleText(tr("Value"));
  m_axisY->setRange(0, 100);
  m_chart->addAxis(m_axisY, Qt::AlignLeft);
  m_lineSeries->attachAxis(m_axisY);
  QChartView* chartView = new QChartView(m_chart, pageDiagram);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setMouseTracking(true);
  chartView->setRubberBand(QChartView::RectangleRubberBand);

  QHBoxLayout* chartControls = new QHBoxLayout();
  QToolButton* zoomInButton = new QToolButton(pageDiagram);
  zoomInButton->setText("+");
  zoomInButton->setToolTip(tr("Zoom in"));
  connect(zoomInButton, &QToolButton::clicked, this, [this, chartView]() {
    if (nullptr != m_axisX) {
      const double range = m_axisX->max() - m_axisX->min();
      const double center = (m_axisX->max() + m_axisX->min()) / 2.0;
      const double newRange = std::max(1.0, range * 0.8);
      m_axisX->setRange(std::max(0.0, center - newRange / 2.0), center + newRange / 2.0);
    }
    if (nullptr != m_axisY) {
      const double range = m_axisY->max() - m_axisY->min();
      const double center = (m_axisY->max() + m_axisY->min()) / 2.0;
      const double newRange = std::max(1.0, range * 0.8);
      m_axisY->setRange(center - newRange / 2.0, center + newRange / 2.0);
    }
    chartView->update();
  });

  QToolButton* zoomOutButton = new QToolButton(pageDiagram);
  zoomOutButton->setText("−");
  zoomOutButton->setToolTip(tr("Zoom out"));
  connect(zoomOutButton, &QToolButton::clicked, this, [this, chartView]() {
    if (nullptr != m_axisX) {
      const double range = m_axisX->max() - m_axisX->min();
      const double center = (m_axisX->max() + m_axisX->min()) / 2.0;
      const double newRange = std::max(1.0, range / 0.8);
      m_axisX->setRange(std::max(0.0, center - newRange / 2.0), center + newRange / 2.0);
    }
    if (nullptr != m_axisY) {
      const double range = m_axisY->max() - m_axisY->min();
      const double center = (m_axisY->max() + m_axisY->min()) / 2.0;
      const double newRange = std::max(1.0, range / 0.8);
      m_axisY->setRange(center - newRange / 2.0, center + newRange / 2.0);
    }
    chartView->update();
  });

  QToolButton* resetZoomButton = new QToolButton(pageDiagram);
  resetZoomButton->setText(tr("Reset"));
  resetZoomButton->setToolTip(tr("Reset zoom"));
  connect(resetZoomButton, &QToolButton::clicked, this, [this]() {
    if (m_samples.empty()) {
      if (nullptr != m_axisX) m_axisX->setRange(0, MAX_SAMPLES);
      if (nullptr != m_axisY) m_axisY->setRange(0, 100);
    }
    else {
      refreshDiagram();
    }
  });

  chartControls->addStretch(1);
  chartControls->addWidget(zoomInButton);
  chartControls->addWidget(zoomOutButton);
  chartControls->addWidget(resetZoomButton);

  diagLayout->addLayout(chartControls);
  diagLayout->addWidget(chartView);
  m_statsLabel = new QLabel("", pageDiagram);
  m_statsLabel->setAlignment(Qt::AlignCenter);
  m_statsLabel->setWordWrap(true);
  diagLayout->addWidget(m_statsLabel);
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
  m_valueStatsLabel = new QLabel("", pageValue);
  m_valueStatsLabel->setAlignment(Qt::AlignCenter);
  m_valueStatsLabel->setWordWrap(true);
  valueLayout->addStretch(1);
  valueLayout->addWidget(m_valueLabel);
  valueLayout->addWidget(m_valueStatsLabel);
  valueLayout->addStretch(1);
  m_displayStack->addWidget(pageValue);

  // Speed meter page
  QWidget* pageSpeed      = new QWidget(this);
  QVBoxLayout* speedLayout = new QVBoxLayout(pageSpeed);
  m_speedMeter             = new CarGaugeWidget(pageSpeed);
  m_speedMeter->setRange(0, 100);
  m_speedValueLabel = new QLabel("--", pageSpeed);
  m_speedValueLabel->setAlignment(Qt::AlignCenter);
  m_speedValueLabel->setWordWrap(true);
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

  refreshUnitDisplay();

  connect(m_modeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &CFrmMeasurementView::setDisplayMode);
  connect(m_gaugeStyleCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          [this](int index) {
            if (nullptr != m_speedMeter) {
              m_speedMeter->setStyle(index);
              m_speedMeter->update();
              m_speedMeter->repaint();
              if (!m_samples.empty()) {
                refreshDisplay(m_samples.back().value);
              }
              else {
                refreshSpeedMeterDisplay(0.0);
              }
            }
          });
  connect(m_diagramAxisModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
    m_diagramAxisMode = index;
    if (!m_samples.empty()) {
      refreshDiagram();
    }
  });
  connect(m_alarmEnabledCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
    m_alarmEnabled = checked;
    if (!checked) {
      m_alarmActive = false;
    }
  });
  connect(m_alarmValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
    m_alarmThreshold = value;
  });
  connect(m_alarmModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
    m_alarmMode = index;
  });
  connect(m_saveButton, &QPushButton::clicked, this, &CFrmMeasurementView::saveSamples);
  connect(m_clearButton, &QPushButton::clicked, this, &CFrmMeasurementView::clearSamples);
  connect(m_pauseButton, &QPushButton::toggled, this, [this](bool checked) {
    m_paused = checked;
    m_pauseButton->setText(checked ? tr("Resume") : tr("Pause"));
    if (!checked && !m_samples.empty()) {
      refreshDisplay(m_samples.back().value);
    }
  });

  setDisplayMode(m_modeCombo->currentIndex());
}

void
CFrmMeasurementView::setDisplayMode(int index)
{
  m_mode = static_cast<displayMode>(index);
  m_displayStack->setCurrentIndex(index);

  if (m_gaugeStyleCombo != nullptr) {
    const bool showGaugeControls = (index == static_cast<int>(displayMode::speed_meter));
    m_gaugeStyleCombo->setVisible(showGaugeControls);
    if (m_gaugeStyleLabel != nullptr) {
      m_gaugeStyleLabel->setVisible(showGaugeControls);
    }
  }

  if (m_diagramAxisModeCombo != nullptr) {
    const bool showDiagramControls = (index == static_cast<int>(displayMode::diagram));
    m_diagramAxisModeCombo->setVisible(showDiagramControls);
    if (m_diagramAxisLabel != nullptr) {
      m_diagramAxisLabel->setVisible(showDiagramControls);
    }
  }

  if (!m_samples.empty()) {
    refreshDisplay(m_samples.back().value);
  }
}

void
CFrmMeasurementView::refreshDisplay(double latestValue)
{
  if (m_paused) {
    return;
  }

  refreshDiagram();
  refreshStatsDisplay(latestValue);
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

  if (!m_samples.empty()) {
    if (m_diagramAxisMode == 1) {
      const qint64 first = m_samples.front().timestamp.toMSecsSinceEpoch();
      for (size_t i = 0; i < m_samples.size(); ++i) {
        const qint64 timestamp = m_samples[i].timestamp.toMSecsSinceEpoch();
        points.push_back(QPointF(static_cast<qreal>(timestamp), m_samples[i].value));
      }
      Q_UNUSED(first);
    }
    else if (m_diagramAxisMode == 2) {
      for (size_t i = 0; i < m_samples.size(); ++i) {
        const QTime time = m_samples[i].timestamp.time();
        const int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
        points.push_back(QPointF(static_cast<qreal>(seconds), m_samples[i].value));
      }
    }
    else {
      const qint64 first = m_samples.front().timestamp.toMSecsSinceEpoch();
      for (size_t i = 0; i < m_samples.size(); ++i) {
        const qint64 timestamp = m_samples[i].timestamp.toMSecsSinceEpoch();
        points.push_back(QPointF(static_cast<qreal>((timestamp - first) / 1000.0), m_samples[i].value));
      }
    }
  }

  m_lineSeries->replace(points);

  if (m_samples.empty()) {
    m_axisX->setRange(0, 100);
    m_axisY->setRange(0, 100);
    return;
  }

  if (m_diagramAxisMode == 1) {
    const qint64 first = m_samples.front().timestamp.toMSecsSinceEpoch();
    const qint64 last = m_samples.back().timestamp.toMSecsSinceEpoch();
    m_axisX->setRange(static_cast<double>(first), static_cast<double>(last));
  }
  else if (m_diagramAxisMode == 2) {
    const QTime first = m_samples.front().timestamp.time();
    const QTime last = m_samples.back().timestamp.time();
    const int firstSeconds = first.hour() * 3600 + first.minute() * 60 + first.second();
    const int lastSeconds = last.hour() * 3600 + last.minute() * 60 + last.second();
    const int span = std::max(1, lastSeconds - firstSeconds);
    m_axisX->setRange(firstSeconds, firstSeconds + span);
  }
  else {
    const qint64 first = m_samples.front().timestamp.toMSecsSinceEpoch();
    const qint64 last = m_samples.back().timestamp.toMSecsSinceEpoch();
    const double start = 0.0;
    const double end = static_cast<double>(last - first) / 1000.0;
    const double span = std::max(1.0, end - start);
    m_axisX->setRange(start, start + span);
  }

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
CFrmMeasurementView::refreshUnitDisplay()
{
  if (nullptr == m_unitLabel) {
    return;
  }

  vscpworks* pworks = (vscpworks*)QCoreApplication::instance();
  if (nullptr == pworks) {
    m_unitLabel->setText(tr("Unit: --"));
    return;
  }

  CVscpUnit unitInfo = pworks->getUnitInfo(m_source.vscpClass,
                                           m_source.vscpType,
                                           m_source.unit);

  QString unitText = tr("Unit: %1").arg(unitInfo.m_name.c_str());
  QString axisUnitText = unitInfo.m_name.c_str();
  if (!unitInfo.m_symbol_ascii.empty()) {
    unitText += QString(" (%1)").arg(unitInfo.m_symbol_ascii.c_str());
    axisUnitText = unitInfo.m_symbol_ascii.c_str();
  }
  else if (!unitInfo.m_symbol_utf8.empty()) {
    unitText += QString(" (%1)").arg(unitInfo.m_symbol_utf8.c_str());
    axisUnitText = unitInfo.m_symbol_utf8.c_str();
  }

  if (unitInfo.m_name.empty()) {
    unitText = tr("Unit: %1").arg(m_source.unit);
    axisUnitText = QString::number(m_source.unit);
  }

  m_unitLabel->setText(unitText);
  if (nullptr != m_axisY) {
    m_axisY->setTitleText(tr("Value (%1)").arg(axisUnitText));
  }
}

void
CFrmMeasurementView::refreshStatsDisplay(double value)
{
  if (nullptr == m_statsLabel) {
    return;
  }

  if (m_samples.empty()) {
    m_statsLabel->setText(tr("Min: --  Max: --  Avg: --"));
    return;
  }

  const double average = std::accumulate(m_samples.begin(),
                                         m_samples.end(),
                                         0.0,
                                         [](double sum, const measurementSample& sample) {
                                           return sum + sample.value;
                                         }) /
                         static_cast<double>(m_samples.size());

  m_statsLabel->setText(
    tr("Min: %1  Max: %2  Avg: %3")
      .arg(QString::number(m_minSeen, 'f', 3))
      .arg(QString::number(m_maxSeen, 'f', 3))
      .arg(QString::number(average, 'f', 3)));
}

void
CFrmMeasurementView::refreshValueDisplay(double value)
{
  if (nullptr == m_valueLabel) {
    return;
  }

  m_valueLabel->setText(QString::number(value, 'f', 3));

  if (nullptr != m_valueStatsLabel) {
    if (m_samples.empty()) {
      m_valueStatsLabel->setText(tr("Min: --  Max: --  Avg: --"));
    }
    else {
      const double average = std::accumulate(m_samples.begin(),
                                             m_samples.end(),
                                             0.0,
                                             [](double sum, const measurementSample& sample) {
                                               return sum + sample.value;
                                             }) /
                             static_cast<double>(m_samples.size());

      m_valueStatsLabel->setText(
        tr("Min: %1  Max: %2  Avg: %3")
          .arg(QString::number(m_minSeen, 'f', 3))
          .arg(QString::number(m_maxSeen, 'f', 3))
          .arg(QString::number(average, 'f', 3)));
    }
  }
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

  if (m_samples.empty()) {
    m_speedValueLabel->setText(tr("Value: %1\nMin: --  Max: --  Avg: --")
                                .arg(QString::number(value, 'f', 3)));
  }
  else {
    const double average = std::accumulate(m_samples.begin(),
                                           m_samples.end(),
                                           0.0,
                                           [](double sum, const measurementSample& sample) {
                                             return sum + sample.value;
                                           }) /
                           static_cast<double>(m_samples.size());

    m_speedValueLabel->setText(
      tr("Value: %1\nMin: %2  Max: %3  Avg: %4")
        .arg(QString::number(value, 'f', 3))
        .arg(QString::number(m_minSeen, 'f', 3))
        .arg(QString::number(m_maxSeen, 'f', 3))
        .arg(QString::number(average, 'f', 3)));
  }
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
