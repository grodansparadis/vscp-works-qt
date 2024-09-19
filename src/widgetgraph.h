// widgetgraph.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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

#ifndef WIDGET_GRAPH_H
#define WIDGET_GRAPH_H

#include <vscp.h>

#include <vscp-client-base.h>

#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>

//QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
//QT_CHARTS_END_NAMESPACE

//QT_CHARTS_USE_NAMESPACE

class XYSeriesIODevice;

//QT_BEGIN_NAMESPACE

//QT_END_NAMESPACE

class WidgetGraph : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetGraph(QWidget *parent = nullptr);
    ~WidgetGraph();

private:
    XYSeriesIODevice *m_device = nullptr;
    QChart *m_chart;
    QLineSeries *m_series ;

    /// The VSCP client type
    CVscpClient::connType m_vscpConnType;

    /// Configuration data for the session
    QJsonObject m_connObject;

    /// A pointer to a VSCP Client 
    CVscpClient *m_vscpClient;
};

#endif // WIDGET_GRAPH_H