// ctxevent.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// "First they ignore you, then they laugh at you, then they fight you, then you win”
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

#ifndef CTXEVENT_H
#define CTXEVENT_H

#include <QString>
#include <QTimer>

/*!
    Class that represent a transmit event
*/

class CTxEvent : public QObject
{
    Q_OBJECT

 public:
    CTxEvent();
    ~CTxEvent();

    

    // Enable setters/getters
    void setEnable(bool bEnable) { m_bEnable = bEnable; };
    bool getEnable(void) { return m_bEnable; };
    bool isEnabled(void) { return m_bEnable; };

    // Name setters/getters
    void setName(QString name) { m_name = name; }
    QString getName(void) { QString str = m_name; return str; };

    // Count setters/getters
    void setCount(uint16_t count) { m_count = count; }
    uint16_t getCount(void) { return m_count; };

    // Period setters/getters
    void setPeriod(uint32_t period) { m_period = period; }
    uint32_t getPeriod(void) { return m_period; };

    // Events setters/getters
    bool newEvent(void);
    void setEvent(vscpEvent *pEvent) { m_pev = pEvent; }
    vscpEvent *getEvent(void) { return m_pev; };

 private:

    /// Event activation
    bool m_bEnable;

    /// Event name
    QString m_name;

    /// Count to send when tx is triggered
    uint16_t m_count;

    /// Period
    uint32_t m_period;

    /// Event to send
    vscpEvent *m_pev;    

    /// Timer used for periodic events
    QTimer m_timer;

};

// ----------------------------------------------------------------------------

#endif  //  CTXEVENT_H