// sessionfilter.h
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

#include "vscpworks.h"

#include "sessionfilter.h"


///////////////////////////////////////////////////////////////////////////////
// CTOR
//

CSessionFilter::CSessionFilter()
{
    m_bReceive = false;
    m_bTransmit = false;
    m_bLevel1 = false;
    m_bLevel2 = false;
    m_bMeasurement = false;

    m_constraint_data_size = constraint::ANY;
    m_data_size = 0;

    m_constraint_obid = constraint::ANY;
    m_obid = 0;

    m_constraint_timestamp = constraint::ANY;
    m_timestamp = 0;

    m_constraint_sensor_index = constraint::ANY;
    m_sensor_index = 0;

    m_constraint_data_coding = constraint::ANY;
    m_data_coding = 0;

    m_constraint_unit = constraint::ANY;
    m_unit = 0;

    m_constraint_value = constraint::ANY;
    m_value = 0;
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

CSessionFilter::~CSessionFilter()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// addClassConstraint
//

bool CSessionFilter::addClassConstraint(uint16_t vscp_class)
{
    m_mapclass[vscp_class] = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClassConstraint
//

bool CSessionFilter::removeClassConstraint(uint16_t vscp_class)
{
    m_mapclass.erase(vscp_class);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isClassAccepted
//

bool CSessionFilter::isClassAccepted(const vscpEvent *pev)
{
    if (nullptr == pev) return false;
    return m_mapclass[pev->vscp_class];
}

///////////////////////////////////////////////////////////////////////////////
// addTypeConstraint
//

bool CSessionFilter::addTypeConstraint(uint32_t type)
{
    m_mapclass[type] = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTypeConstraint
//

bool CSessionFilter::removeTypeConstraint(uint32_t type)
{
    m_mapclass.erase(type);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isTypeAccepted
//

bool CSessionFilter::isTypeAccepted(const vscpEvent *pev)
{
    if (nullptr == pev) return false;
    return m_mapclass[((uint32_t)(pev->vscp_class))<<16 + pev->vscp_type];
}

///////////////////////////////////////////////////////////////////////////////
// removeDataConstraint
//

bool CSessionFilter::removeDataConstraint(uint8_t pos)
{
    for (auto it = m_listData.begin(); it != m_listData.end(); ) {
        if ((*it >> 16) & 0x1ff) {
            it = m_listData.erase(it);    
        }
        else {
            it++;
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isDataAccepted
//

bool CSessionFilter::isDataAccepted(const vscpEvent *pev)
{
    for (int i=0; i<m_listData.size(); i++) {
        uint32_t item = m_listData[i];
        uint8_t pos = (item >> 16) & 0x1ff;
        uint8_t val = item & 0xff;
        constraint op = static_cast<constraint>(item >> 8);
        switch(op) {
            case constraint::ANY:
                break;
            case constraint::NEQ:
                break;
            case constraint::EQ:
                break;
            case constraint::LT:
                break;
            case constraint::LTEQ:
                break;
            case constraint::GT:
                break;    
            case constraint::GTEQ:
                break;                    
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkValue
//

bool CSessionFilter::checkValue(uint8_t val, 
                                    uint8_t evval, 
                                    constraint chk)
{
    switch (chk) { 
        case constraint::NEQ:
            if (!(val != evval)) return false;
            break;

        case constraint::EQ:
            if (!(val == evval)) return false; 
            break;

        case constraint::LT: 
            if (!(val < evval)) return false;
            break;

        case constraint::LTEQ: 
            if (!(val <= evval)) return false;
            break;

        case constraint::GT: 
            if (!(val > evval)) return false;
            break;

        case constraint::GTEQ:
            if (!(val >= evval)) return false;
            break;

        case constraint::ANY:
        default:
            break;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// check
//

bool CSessionFilter::check(const vscpEvent *pev)
{
    if ( nullptr == pev) return false;

    // if class checks should be done, class must be present
    if (m_mapclass.size() && !m_mapclass[pev->vscp_class]) {
        return false;
    }
    // if type checks should be done, type must be present
    else if (m_maptype.size() && !m_maptype[pev->vscp_type]) {
        return false;
    }
    // if any guid constraint is defined checks should be performed
    else if (m_listguid.size()) {

        for (int i=0; i<m_listguid.size(); i++) {

            uint32_t item = m_listguid[i];          // op(8) : guid(8) : pos(8) 
            uint8_t pos = m_listguid[i] & 0xff;
            uint8_t val = (m_listguid[i] >> 8) & 0xff;
            
            if ( !checkValue(val, pev->GUID[pos], static_cast<CSessionFilter::constraint>(item >> 16))) {
                return false;
            }

        }    
    }

    // if any data constraint is defined checks should be performed
    else if (m_listData.size()) {
        for (int i=0; i<m_listData.size(); i++) {

            if (!pev->sizeData) return false;           // Must be data to check
            if (nullptr == pev->pdata) return false;    // Must be data to check
            
            uint32_t item = m_listData[i];              // op(8) : data(8) : pos(8) 
            uint8_t pos = m_listData[i] & 0xff;
            if (pos >= pev->sizeData) return false;     // Data does not fullfill
            uint8_t val = (m_listData[i] >> 8) & 0xff;

            if ( !checkValue(val, pev->pdata[pos], static_cast<CSessionFilter::constraint>(item >> 16))) {
                return false;
            }
                      
        }
    }

    // Check data size
    else if (constraint::ANY != m_constraint_data_size) {
    
        if ( !checkValue(m_data_size, pev->sizeData, m_constraint_data_size)) {
            return false;
        }
    
    }

    // Check obid
    else if (constraint::ANY != m_constraint_obid) {
    
        if ( !checkValue(m_obid, pev->obid, m_constraint_obid)) {
            return false;
        }
    
    }

    // Check obid
    else if (constraint::ANY != m_constraint_timestamp) {
    
        if ( !checkValue(m_timestamp, pev->timestamp, m_constraint_timestamp)) {
            return false;
        }
    
    }

    // Check obid
    else if (constraint::ANY != m_constraint_timestamp) {
    
        if ( !checkValue(m_timestamp, pev->timestamp, m_constraint_timestamp)) {
            return false;
        }
    
    }

    // Check date
    // Byte 0
    //    Bit 0-3: Pos (0-7)
    //    Bit 4-7: OP (0-7)
    // Byte 1/2
    //    Value

    if (m_date.size()) {
        for (int i=0; i<m_date.size(); i++) {
            
            uint32_t item = m_date[i];                  // value(16) : op : value (16)  
            uint8_t pos = m_date[i] & 0x0f;
            constraint chk = static_cast<constraint>(((m_date[i] >> 4) & 0x0f));
            uint16_t val = (m_date[i] >> 16) & 0xffff;

            switch (pos) {

                case 0: // Year
                    if (!checkValue(val, pev->hour, chk)) {
                        return false;
                    }
                    break;

                case 1: // Month
                    if (!checkValue(val, pev->month, chk)) {
                        return false;
                    }
                    break;              

                case 2: // Day
                    if (!checkValue(val, pev->day, chk)) {
                        return false;
                    }
                    break;    

                case 3: // Hour
                    if (!checkValue(val, pev->hour, chk)) {
                        return false;
                    }
                    break;    

                case 4: // Minute
                    if (!checkValue(val, pev->minute, chk)) {
                        return false;
                    }
                    break;    

                case 5: // Second
                    if (!checkValue(val, pev->second, chk)) {
                        return false;
                    }
                    break;    

            }                 
        }
    }

    // Level I
    else if (m_bLevel1 && (pev->vscp_class >= 1024)) {
        return false;
    }

    // Level II
    else if (m_bLevel2 && (pev->vscp_class < 1024)) {
        return false;
    }

    // Measurement (set automatically if any measuement
    // related constraint set
    else if (m_bMeasurement && !vscp_isMeasurement(pev)) {
        return false;
    }

    // Sensor index
    else if (constraint::ANY != m_constraint_sensor_index) {

        if ( !checkValue(m_sensor_index, vscp_getMeasurementSensorIndex(pev), m_constraint_sensor_index)) {
            return false;
        }

    }

    // Data coding
    else if (constraint::ANY != m_constraint_data_coding) {

        if ( !checkValue(m_data_coding, vscp_getMeasurementDataCoding(pev), m_constraint_data_coding)) {
            return false;
        }

    }   

    // unit
    else if (constraint::ANY != m_constraint_unit) {

        if ( !checkValue(m_unit, vscp_getMeasurementDataCoding(pev), m_constraint_unit)) {
            return false;
        }

    } 

    // Measurement value
    if (constraint::ANY != m_constraint_value) {

        double mvalue;
        if (vscp_getMeasurementAsDouble( &mvalue, pev)) {
            return false;
        }

        switch (m_constraint_value) { 
            case constraint::NEQ:
                if (!(m_value != mvalue)) return false;
                break;

            case constraint::EQ:
                if (!(m_value == mvalue)) return false; 
                break;

            case constraint::LT: 
                if (!(m_value < mvalue)) return false;
                break;

            case constraint::LTEQ: 
                if (!(m_value <= mvalue)) return false;
                break;

            case constraint::GT: 
                if (!(m_value > mvalue)) return false;
                break;

            case constraint::GTEQ:
                if (!(m_value >= mvalue)) return false;
                break;

        }

        return true;

    }

    return true;
}