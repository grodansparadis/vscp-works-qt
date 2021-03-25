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

#ifndef SESSIONFILTER_H
#define SESSIONFILTER_H

#include <string>
#include <deque>
#include <map>

// Must be receive event     0/1
// Must be transmit event    0/1
// Classes & types           n
// Level I / Level II        0/1 
// Priority                  0/1
// GUID                      0/1
// OBID                      0/1
// timestamp                 0/1
// Date                      0/1
// Data size                 0/1
// Data                      0/1
// Must be measurement       0/1 
// Sensor index              0/1
// unit                      0/1
// data coding               0/1
// value                     0/1

class CSessionFilter
{

public:

    CSessionFilter();
    ~CSessionFilter();    

    /*
        0 - ANY:  accept any value
        1 - MEQ:  not equal
        2 - EQ:   equal
        3 - LT:   less than
        4 - LTEQ: less than or equal
        5 - GT:   greater than
        6 - GTEQ  greate than or equal 
    */
    enum class constraint {ANY=0, EQ, NEQ, LT, LTEQ, GT, GTEQ};

    static const uint8_t type_unknown = 0;
    static const uint8_t type_must_be_receive = 1;
    static const uint8_t type_must_be_transmit = 2;
    static const uint8_t type_must_be_level1 = 3;
    static const uint8_t type_must_be_level2 = 4;
    static const uint8_t type_must_be_measurement = 5;
    static const uint8_t type_class = 6;
    static const uint8_t type_type = 7;
    static const uint8_t type_priority = 8;
    static const uint8_t type_guid = 9;
    static const uint8_t type_obid = 10;
    static const uint8_t type_timestamp = 11;
    static const uint8_t type_date = 12;
    static const uint8_t type_data_size = 13;
    static const uint8_t type_data = 14;
    static const uint8_t type_sensor_index = 15;
    static const uint8_t type_unit = 16;
    static const uint8_t type_data_coding = 17;
    static const uint8_t type_value = 18;
    static const uint8_t type_script = 19;

    // Date constraint pos codes
    static const uint8_t date_pos_year = 0;
    static const uint8_t date_pos_month = 1;
    static const uint8_t date_pos_day = 2;
    static const uint8_t date_pos_hour = 3;
    static const uint8_t date_pos_minute = 4;
    static const uint8_t date_pos_second = 5;

    /*!
        Setters / Getters
    */

    std::string getName(void) { return m_name; };
    void setName(const std::string& name) { m_name = name; };

    std::string getDescription(void) { return m_description; };
    void setDescription(const std::string& description) { m_description = description; };

    std::string getScript(void) { return m_script; };
    void setScript(const std::string& script) { m_script = script; };

    /*!
        Check a value against another value with
        a specified set constraint 
        @param val Value
        @param evval Event value to check
        @param chk Constraint
    */
    bool checkValue(uint32_t val, 
                        uint32_t evval, 
                        constraint chk);
    /*!
        Check if event should be shown
        @param Event to check
        @return true if event should be displayed.
    */
    bool check(const vscpEvent *pev);

    // Direction RX  constraint handling
    void addReceiveConstraint(bool b=true) { m_bReceive = b; };
    bool isReceiveConstraint(void) { return m_bReceive; };

    // Direction TX constraint handling
    void addTransmitConstraint(bool b=true) { m_bTransmit = b; };
    bool isTransmitConstraint(void) { return m_bTransmit; };

    // Level I constraint handling
    void addLevel1Constraint(bool b=true) { m_bLevel1 = b; };
    bool isLevel1Constraint(void) { return m_bLevel1; };

    // Level II Constraint handling
    void addLevel2Constraint(bool b=true) { m_bLevel2 = b; };
    bool isLevel2Constraint(void) { return m_bLevel2; };

    // Measurement constraint handling
    void addMeasurementConstraint(bool b=true) { m_bMeasurement = b; };
    bool isMeasurementConstraint(void) { return m_bMeasurement; };

    // Class constraint handling
    bool addClassConstraint(uint16_t vscp_class);
    bool removeClassConstraint(uint16_t vscp_class);
    bool isClassAccepted(const vscpEvent *pev);
    std::deque<uint16_t> getClasses(void);
    void clearClasses(void) { m_mapClass.clear(); };

    // Type Constraint handling
    bool addTypeConstraint(uint32_t type);
    bool removeTypeConstraint(uint32_t type);
    bool isTypeAccepted(const vscpEvent *pev);
    std::deque<uint32_t> getTypes(void);
    void clearTypes(void) { m_mapType.clear(); };

    // Data constraint handling
    bool addDataConstraint(uint16_t pos, uint8_t val, constraint chk = constraint::ANY);
    void addDataConstraints(const std::deque<uint32_t>& listData);
    bool removeDataConstraint(uint16_t pos);
    std::deque<uint32_t> getDataConstraints(void);
    bool isDataAccepted(const vscpEvent *pev);

    // Data size constraint handling
    constraint getDataSizeConstraint(void) { return m_constraint_data_size; };
    uint16_t getDataSizeValue(void) { return m_data_size; };
    bool setDataSizeConstraint(uint16_t size, uint16_t val, constraint chk = constraint::ANY);

    // GUID constraint handling
    bool addGuidConstraint(uint8_t pos, uint8_t val, constraint chk = constraint::ANY);
    bool removeGuidConstraint(uint8_t pos);
    bool isGuidAccepted(const vscpEvent *pev);
    std::deque<uint32_t> getGuids(void);

    // Date constraint handling
    bool addDateConstraint(uint8_t pos, uint16_t val, constraint chk = constraint::ANY);
    uint32_t getDateConstraint(uint8_t pos);
    bool removeDateConstraint(uint8_t pos);
    bool isDateAccepted(const vscpEvent *pev);

    // OBID constraint
    bool addObidConstraint(uint32_t val, constraint chk = constraint::ANY);
    bool removeObidConstraint(void);
    bool isObidAccepted(const vscpEvent *pev);
    uint32_t getObidValue(void) { return m_obid; }
    constraint getObidConstraint(void) { return m_constraint_obid; }

    // TimeStamp constraint
    bool addTimeStampConstraint(uint32_t val, constraint chk = constraint::ANY);
    bool removeTimeStampConstraint(void);
    bool isTimeStampAccepted(const vscpEvent *pev);
    uint32_t getTimeStampValue(void) { return m_timestamp; }
    constraint getTimeStampConstraint(void) { return m_constraint_timestamp; }    

 private:

    /*!
        Name of filter
    */
    std::string m_name;

    /*!
        Filter description
    */
    std::string m_description;

    /*!
        Javascript code to execute
    */
    std::string m_script;

    // Must be receive event
    bool m_bReceive;

    // Must be transmit event
    bool m_bTransmit;

    // Class
    //      Accepted/Denied classes (no type is all)
    //      class -> accept
    std::map<uint16_t, bool> m_mapClass;

    // Type
    //      Accepted/Denied types (no type is all)
    //      class : type  -> accept
    std::map<uint32_t, bool> m_mapType;

    /*!
        data compare (no data is all/none). Inverted for deny 
            pos (16) -> op (8) : value (8)           
    */
    std::map<uint16_t,uint32_t> m_mapData;

    /*!
        data size constraint. Inverted for deny 
        size : value
    */
    constraint m_constraint_data_size;
    uint16_t m_data_size;


    /*!
        GUID(no data is all). Inverted for deny 
        pos(8) -> chk(8) : val(8) 
        If not defined zero is defined wish is 
        don't care           
    */
    std::map<uint8_t, uint16_t> m_mapGuid;

    /*!
        obid constraint
    */
    constraint m_constraint_obid;
    uint32_t m_obid;

    /*!
        timestamp constraint
    */
    constraint m_constraint_timestamp;
    uint32_t m_timestamp;

    /*!
        Date (no data is all) 
        pos -> chk(8) : val(16)         
    */
    std::map<uint8_t,uint32_t> m_mapDateTime;

    /*! 
        Must be Level I event if set, don't care if not
        True if constraint is active
    */
    bool m_bLevel1;

    /*!
         Must be level II event if set, don't care if not
         True if constraint is active
    */
    bool m_bLevel2;

    /*!
        Must me measurement if set, don't care if not
        True if constraint is active
    */
    bool m_bMeasurement;

    /*!
        sensor index constraint
    */
    constraint m_constraint_sensor_index;
    uint32_t m_sensor_index;

    /*!
        data coding constraint
    */
    constraint m_constraint_data_coding;
    uint32_t m_data_coding;

    /*!
        unit constraint
    */
    constraint m_constraint_unit;
    uint32_t m_unit;

    /*!
        Measurement value constraint
    */
    constraint m_constraint_value;
    double m_value;
};

#endif  // SESSIONFILTER_H