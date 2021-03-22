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
    enum class constraint {ANY=0, NEQ, EQ, LT, LTEQ, GT, GTEQ};

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
    */
    bool checkValue(uint8_t val, 
                        uint8_t evval, 
                        constraint chk);
    /*!
        Check if event should be shown
        @param Event to check
        @return true if event should be displayed.
    */
    bool check(const vscpEvent *pev);

    void addReceiveConstraint(bool b=true) { m_bReceive = b; };
    bool isReceiveConstraint(void) { return m_bReceive; };

    void addTransmitConstraint(bool b=true) { m_bTransmit = b; };
    bool isTransmitConstraint(void) { return m_bTransmit; };

    void addLevel1Constraint(bool b=true) { m_bLevel1 = b; };
    bool isLevel1Constraint(void) { return m_bLevel1; };

    void addLevel2Constraint(bool b=true) { m_bLevel2 = b; };
    bool isLevel2Constraint(void) { return m_bLevel2; };

    void addMeasurementConstraint(bool b=true) { m_bMeasurement = b; };
    bool isMeasurementConstraint(void) { return m_bMeasurement; };

    bool addClassConstraint(uint16_t vscp_class);
    bool removeClassConstraint(uint16_t vscp_class);
    bool isClassAccepted(const vscpEvent *pev);

    bool addTypeConstraint(uint32_t type);
    bool removeTypeConstraint(uint32_t type);
    bool isTypeAccepted(const vscpEvent *pev);

    bool addDataConstraint(uint8_t pos, uint8_t val, constraint chk = constraint::ANY);
    bool removeDataConstraint(uint8_t pos);
    bool isDataAccepted(const vscpEvent *pev);

    constraint getDataSizeConstraint(void) { return m_constraint_data_size; };
    uint16_t getDataSizeValue(void) { return m_data_size; };
    bool setDataSizeConstraint(uint16_t size, uint16_t val, constraint chk = constraint::ANY);

    bool addGuidConstraint(uint8_t pos, uint8_t val, constraint chk = constraint::ANY);
    bool removeGuidConstraint(uint8_t pos);
    bool isGuidAccepted(const vscpEvent *pev);


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
    std::map<uint16_t, bool> m_mapclass;

    // Type
    //      Accepted/Denied types (no type is all)
    //      class : type  -> accept
    std::map<uint32_t, bool> m_maptype;

    /*!
        data compare (no data is all/none). Inverted for deny 
            pos (16) : op (8) : value (8)
            0 - accept any value
            1 - not equal
            2 - equal
            3 - less than
            4 - less than or equal
            5 - greater than
            6 - greate than or equal            
    */
    std::deque<uint32_t> m_listData;

    /*!
        data size constraint. Inverted for deny 
        size : value
    */
    constraint m_constraint_data_size;
    uint16_t m_data_size;


    /*!
        GUID(no data is all). Inverted for deny 
        op(8) : guid(8) : pos(8) 
 
            0 - accept any value
            1 - not equal
            2 - equal
            3 - less than
            4 - less than or equal
            5 - greater than
            6 - greate than or equal            
    */
    std::deque<uint32_t> m_listguid;

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
        // Check date
        // Byte 0
        //    Bit 0-3: Pos (0-7)
        //    Bit 4-7: OP (0-7)
        // Byte 1/2
        //    Value
            pos : op : value
            0 - accept any value
            1 - not equal
            2 - equal
            3 - less than
            4 - less than or equal
            5 - greater than
            6 - greate than or equal            
    */
    std::deque<uint32_t> m_date;

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