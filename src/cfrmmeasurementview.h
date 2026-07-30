// cfrmmeasurementview.h
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

#ifndef CFRMMEASUREMENTVIEW_H
#define CFRMMEASUREMENTVIEW_H

#include <vscp.h>

#include <QDateTime>
#include <QWidget>

#include <array>
#include <vector>

class QComboBox;
class QLabel;
class QDial;
class QProgressBar;
class QStackedWidget;
class QPushButton;
class QLineSeries;
class QChart;
class QValueAxis;

struct CMeasurementSourceSpec {
  uint16_t vscpClass{ 0 };
  uint16_t vscpType{ 0 };
  uint8_t sensorIndex{ 0 };
  uint8_t unit{ 0 };
  std::array<uint8_t, 16> guid{};

  bool matches(const vscp_event_t* pev) const;
};

class CFrmMeasurementView : public QWidget {
  Q_OBJECT

public:
  enum class displayMode { diagram = 0,
                           value,
                           speed_meter,
                           percent };

  struct measurementSample {
    QDateTime timestamp;
    double value{ 0 };
  };

  explicit CFrmMeasurementView(const CMeasurementSourceSpec& source,
                               const QString& sourceDescription,
                               QWidget* parent = nullptr);
  ~CFrmMeasurementView();

  bool matchesEvent(const vscp_event_t* pev) const;
  void appendMeasurement(const vscp_event_t* pev);
  const CMeasurementSourceSpec& getSource() const { return m_source; }

private:
  void setupUi();
  void setDisplayMode(int index);
  void refreshDisplay(double latestValue);
  void refreshDiagram();
  void refreshValueDisplay(double value);
  void refreshSpeedMeterDisplay(double value);
  void refreshPercentDisplay(double value);
  void saveSamples();
  void clearSamples();

  static constexpr int MAX_SAMPLES = 500;

  CMeasurementSourceSpec m_source;
  QString m_sourceDescription;
  std::vector<measurementSample> m_samples;
  displayMode m_mode{ displayMode::diagram };
  double m_minSeen{ 0 };
  double m_maxSeen{ 0 };

  QComboBox* m_modeCombo{ nullptr };
  QLabel* m_sourceLabel{ nullptr };
  QStackedWidget* m_displayStack{ nullptr };
  QPushButton* m_saveButton{ nullptr };
  QPushButton* m_clearButton{ nullptr };

  QChart* m_chart{ nullptr };
  QLineSeries* m_lineSeries{ nullptr };
  QValueAxis* m_axisX{ nullptr };
  QValueAxis* m_axisY{ nullptr };

  QLabel* m_valueLabel{ nullptr };
  QDial* m_speedMeter{ nullptr };
  QLabel* m_speedValueLabel{ nullptr };
  QProgressBar* m_percentBar{ nullptr };
  QLabel* m_percentValueLabel{ nullptr };
};

#endif // CFRMMEASUREMENTVIEW_H
