// sessionfilter.h
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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include "vscpworks.h"

#include "sessionfilter.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// addClassConstraint
//

bool CSessionFilter::addClassConstraint(uint16_t vscp_class)
{
    m_mapClass[vscp_class] = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClassConstraint
//

bool CSessionFilter::removeClassConstraint(uint16_t vscp_class)
{
    m_mapClass.erase(vscp_class);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isClassAccepted
//

bool CSessionFilter::isClassAccepted(const vscpEvent *pev)
{
    if (nullptr == pev) return false;
    return m_mapClass[pev->vscp_class];
}

///////////////////////////////////////////////////////////////////////////////
// getClasses
//

std::deque<uint16_t> CSessionFilter::getClasses(void) 
{
    std::deque<uint16_t> listClasses;
    for (auto const& item : m_mapClass) {
        listClasses.push_back(item.first);
    }
    return listClasses;
}



// ----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// addTypeConstraint
//

bool CSessionFilter::addTypeConstraint(uint32_t type)
{
    m_mapType[type] = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTypeConstraint
//

bool CSessionFilter::removeTypeConstraint(uint32_t type)
{
    m_mapType.erase(type);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isTypeAccepted
//

bool CSessionFilter::isTypeAccepted(const vscpEvent *pev)
{
    if (nullptr == pev) return false;
    return m_mapType[(((uint32_t)(pev->vscp_class))<<16) + pev->vscp_type];
}

///////////////////////////////////////////////////////////////////////////////
// getTypes
//

std::deque<uint32_t> CSessionFilter::getTypes(void) 
{
    std::deque<uint32_t> listTypes;
    for (auto const& item : m_mapType) {
        listTypes.push_back(item.first);
    }
    return listTypes;
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// addGuidConstraint
//

bool CSessionFilter::addGuidConstraint(uint8_t pos, uint8_t val, constraint chk)
{
    // pos(8) -> chk(8) : guid(8)
    m_mapGuid[pos] = (((uint16_t)chk) << 8) + val;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeGuidConstraint
//

bool CSessionFilter::removeGuidConstraint(uint8_t pos)
{
    m_mapGuid.erase(pos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isGuidAccepted
//

bool CSessionFilter::isGuidAccepted(const vscpEvent *pev)
{
    for (auto const& item : m_mapGuid) {

        uint8_t pos = item.first;
        uint8_t chk = (item.second >> 8) & 0xff;
        uint8_t val = item.second & 0xff;
        
        switch (static_cast<constraint>(chk)) {

            case constraint::NEQ:
                if (!(val != pev->GUID[pos])) return false;
                break;

            case constraint::EQ:
                if (!(val == pev->GUID[pos])) return false;
                break;

            case constraint::LT:
                if (!(val < pev->GUID[pos])) return false;
                break;

            case constraint::LTEQ:
                if (!(val <= pev->GUID[pos])) return false;
                break; 

            case constraint::GT:
                if (!(val > pev->GUID[pos])) return false;
                break;

            case constraint::GTEQ:
                if (!(val >= pev->GUID[pos])) return false;
                break;

        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getGuids
//

std::deque<uint32_t> 
CSessionFilter::getGuids(void)
{
    std::deque<uint32_t> listGuid;

    for (auto const& item : m_mapGuid){
        uint32_t val = (((uint32_t)item.first) << 16) + item.second;
        listGuid.push_back(val);    
    }

    return listGuid;   
}




// ----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// addDateConstraint
//

bool CSessionFilter::addDateConstraint(uint8_t pos, uint16_t val, constraint chk)
{
    if (pos > 7) return false;
    m_mapDateTime[pos] = (static_cast<uint32_t>(chk) << 16) + val;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getDateConstraint
//

uint32_t CSessionFilter::getDateConstraint(uint8_t pos)
{
    if (pos > 7) return 0;
    return m_mapDateTime[pos];
}

///////////////////////////////////////////////////////////////////////////////
// removeDateConstraint
//

bool CSessionFilter::removeDateConstraint(uint8_t pos)
{
    m_mapDateTime.erase(pos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isDateAccepted
//

bool CSessionFilter::isDateAccepted(const vscpEvent *pev)
{
    if (m_mapDateTime.size()) {

        for (auto const& item : m_mapDateTime) {

            uint8_t pos = item.first;
            constraint chk = static_cast<constraint>((item.second >> 16) & 0xff);
            uint16_t val = item.second & 0xffff;
            
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

    return true;
}



// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// addObidConstraint
//

bool CSessionFilter::addObidConstraint(uint32_t val, constraint chk)
{
    m_obid = val;
    m_constraint_obid = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeObidConstraint
//

bool CSessionFilter::removeObidConstraint(void)
{
    m_constraint_obid = constraint::ANY;
    m_obid = 0;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeObidConstraint
//

bool CSessionFilter::isObidAccepted(const vscpEvent *pev)
{
    if (!checkValue(m_obid, pev->obid, m_constraint_obid)) {
        return false;
    }

    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// addTimeStampConstraint
//

bool CSessionFilter::addTimeStampConstraint(uint32_t val, constraint chk)
{
    m_timestamp = val;
    m_constraint_timestamp = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTimeStampConstraint
//

bool CSessionFilter::removeTimeStampConstraint(void)
{
    m_constraint_timestamp = constraint::ANY;
    m_timestamp = 0;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isTimeStampAccepted
//

bool CSessionFilter::isTimeStampAccepted(const vscpEvent *pev)
{
    if (!checkValue(m_obid, pev->timestamp, m_constraint_timestamp)) {
        return false;
    }

    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// addDataConstraints
//

void CSessionFilter::addDataConstraints(const std::deque<uint32_t>& listData)
{
    m_mapData.clear();

    // listData: pos (16) : op (8) : value (8)     
    for (int i=0; i< listData.size(); i++) {
        uint16_t pos = (listData[i] >> 16) & 0x1ff;
        // map: pos (16) -> op (8) : value (8) 
        m_mapData[pos] = listData[i] & 0x1fffffff;
    }
}

///////////////////////////////////////////////////////////////////////////////
// getDataConstraints
//

std::deque<uint32_t> CSessionFilter::getDataConstraints(void)
{
    std::deque<uint32_t> listData;
    for (auto const& item : m_mapData) {
        // listData: pos (16) : op (8) : value (8
        listData.push_back(item.second);
    }
    return listData;
}

///////////////////////////////////////////////////////////////////////////////
// addDataConstraint
//

bool CSessionFilter::addDataConstraint(uint16_t pos, uint8_t val, constraint chk)
{
    // pos(16) -> chk(8) : val (8) 
    m_mapData[pos] = (((uint16_t)chk) << 16) + val;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeDataConstraint
//

bool CSessionFilter::removeDataConstraint(uint16_t pos)
{
    m_mapData.erase(pos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isDataAccepted
//

bool CSessionFilter::isDataAccepted(const vscpEvent *pev)
{
    for (auto const& item : m_mapData) {
        
        // If data pos > size - no match
        if (pev->sizeData >= item.first) {
            return false;
        }

        if (!checkValue( item.second & 0xff,
                            pev->pdata[item.first],
                            static_cast<constraint>((item.second >> 8) & 0xff))) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setDataSizeConstraint
//

bool CSessionFilter::addDataSizeConstraint(uint16_t size, constraint chk)
{
    if (size > 511) size = 511;
    m_data_size = size;
    m_constraint_data_size = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addPriorityConstraint
//

bool CSessionFilter::addPriorityConstraint(uint8_t priority, constraint chk)
{
    if (priority > 7) m_priority = 7;
    m_priority = priority;
    m_constraint_priority = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addPriorityConstraint
//

bool CSessionFilter::addSensorIndexConstraint(uint8_t sensor_index, constraint chk)
{
    m_sensor_index = sensor_index;
    m_constraint_sensor_index = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementValueConstraint
//

bool CSessionFilter::addMeasurementValueConstraint(double value, constraint chk)
{
    m_value = value;
    m_constraint_value = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementUnitConstraint
//

bool CSessionFilter::addMeasurementUnitConstraint(uint8_t unit, constraint chk)
{
    m_unit = unit;
    m_constraint_unit = chk;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementDataCodingConstraint
//

bool CSessionFilter::addMeasurementDataCodingConstraint(uint8_t coding, constraint chk)
{
    m_data_coding = coding;
    m_constraint_data_coding = chk;
    return true;
}

// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// checkValue
//

bool CSessionFilter::checkValue(uint32_t val,
                                    uint32_t evval,
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
    if (m_mapClass.size() && !m_mapClass[pev->vscp_class]) {
        return false;
    }
    // if type checks should be done, type must be present
    else if (m_mapType.size() && !m_mapType[pev->vscp_type]) {
        return false;
    }
    // if any guid constraint is defined checks should be performed
    else if (m_mapGuid.size()) {
        if (!isGuidAccepted(pev)) return false;    
    }

    // if any data constraint is defined checks should be performed
    else if (m_mapData.size()) {
        if (!isDataAccepted(pev)) return false;
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

    // Check timestamp
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

    if (m_mapDateTime.size()) {
        if (!isDateAccepted(pev)) return false;        
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
