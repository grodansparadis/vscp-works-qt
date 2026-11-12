// sessionfilter.h
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

#ifndef SESSIONFILTER_H
#define SESSIONFILTER_H

#include <vscp.h>

#include <nlohmann/json.hpp>

#include <deque>
#include <map>
#include <string>
#include <vector>

/*!
  A display filter for the session window.

  A filter is a set of constraints. An event is shown when it passes all
  constraints that are set. Constraints that compare a value read
  "event value <constraint> filter value", for example LT accepts events
  with a value less than the value of the filter.

  Constraints
    Must be receive event       Must be transmit event
    Must be Level I event       Must be Level II event
    Must be measurement
    Classes & types             Priority
    GUID (per byte)             OBID
    Timestamp                   Date (year .. second)
    Data size                   Data (per position)
    Sensor index                Unit
    Data coding                 Measurement value
*/

class CSessionFilter {

public:
  CSessionFilter();
  ~CSessionFilter();

  /*!
    Order is the order of the constraint combo boxes in the select dialogs
    and is stored in filter files, so do not change it.
  */
  enum class constraint { ANY = 0, EQ, NEQ, LT, LTEQ, GT, GTEQ };

  static const uint8_t type_unknown             = 0;
  static const uint8_t type_must_be_receive     = 1;
  static const uint8_t type_must_be_transmit    = 2;
  static const uint8_t type_must_be_level1      = 3;
  static const uint8_t type_must_be_level2      = 4;
  static const uint8_t type_must_be_measurement = 5;
  static const uint8_t type_class               = 6;
  static const uint8_t type_type                = 7;
  static const uint8_t type_priority            = 8;
  static const uint8_t type_guid                = 9;
  static const uint8_t type_obid                = 10;
  static const uint8_t type_timestamp           = 11;
  static const uint8_t type_date                = 12;
  static const uint8_t type_data_size           = 13;
  static const uint8_t type_data                = 14;
  static const uint8_t type_sensor_index        = 15;
  static const uint8_t type_unit                = 16;
  static const uint8_t type_data_coding         = 17;
  static const uint8_t type_value               = 18;
  static const uint8_t type_script              = 19;

  // Date constraint pos codes
  static const uint8_t date_pos_year   = 0;
  static const uint8_t date_pos_month  = 1;
  static const uint8_t date_pos_day    = 2;
  static const uint8_t date_pos_hour   = 3;
  static const uint8_t date_pos_minute = 4;
  static const uint8_t date_pos_second = 5;

  /*!
      Setters / Getters
  */

  std::string getName(void) const { return m_name; };
  void setName(const std::string& name) { m_name = name; };

  std::string getDescription(void) const { return m_description; };
  void setDescription(const std::string& description) { m_description = description; };

  std::string getScript(void) const { return m_script; };
  void setScript(const std::string& script) { m_script = script; };

  /*!
      Check a value of an event against a filter value
      @param val Filter value
      @param evval Value of the event
      @param chk Constraint, "evval chk val"
      @return true if the constraint is met
  */
  static bool checkValue(uint32_t val, uint32_t evval, constraint chk);

  /*!
      Check if event should be shown
      @param pev Event to check
      @param bReceived True for a received event, false for a transmitted one
      @return true if event should be displayed. An allow filter shows the
        events that meet all constraints, a deny filter hides them.
  */
  bool check(const vscp_event_t* pev, bool bReceived = true) const;

  /*!
      Check if an event meets all constraints of the filter
      @param pev Event to check
      @param bReceived True for a received event, false for a transmitted one
      @return true if all constraints are met
  */
  bool matches(const vscp_event_t* pev, bool bReceived = true) const;

  /// Allow (true, default) shows matching events, deny (false) hides them
  bool isAllow(void) const { return m_bAllow; };
  void setAllow(bool bAllow) { m_bAllow = bAllow; };

  /*!
    Constraints that are set, as type_xxx codes in increasing order
    (type_type is reported as part of type_class).
  */
  std::deque<uint8_t> getConstraintTypes(void) const;

  /// True if the constraint (type_xxx) is set
  bool hasConstraint(uint8_t type) const;

  /// Remove a constraint (type_xxx)
  void removeConstraint(uint8_t type);

  /// Remove all constraints
  void clear(void);

  /// Text for a constraint type as shown in the filter dialogs
  static std::string getConstraintTypeName(uint8_t type);

  /*!
    Filter as JSON (name, description and constraints)
  */
  nlohmann::json toJson(void) const;

  /*!
    Set the filter from JSON as written by toJson
    @return false if it is not a filter
  */
  bool fromJson(const nlohmann::json& j);

  /*!
    Read filters from a file (a JSON array of filters)
    @param path File to read
    @param filters Filters read
    @return true on success
  */
  static bool readFilters(const std::string& path, std::vector<CSessionFilter>& filters);

  /*!
    Write filters to a file (a JSON array of filters)
    @param path File to write
    @param filters Filters to write
    @return true on success
  */
  static bool writeFilters(const std::string& path, const std::vector<CSessionFilter>& filters);

  // Direction RX  constraint handling
  void addReceiveConstraint(bool b = true) { m_bReceive = b; };
  bool isReceiveConstraint(void) const { return m_bReceive; };

  // Direction TX constraint handling
  void addTransmitConstraint(bool b = true) { m_bTransmit = b; };
  bool isTransmitConstraint(void) const { return m_bTransmit; };

  // Level I constraint handling
  void addLevel1Constraint(bool b = true) { m_bLevel1 = b; };
  bool isLevel1Constraint(void) const { return m_bLevel1; };

  // Level II Constraint handling
  void addLevel2Constraint(bool b = true) { m_bLevel2 = b; };
  bool isLevel2Constraint(void) const { return m_bLevel2; };

  // Measurement constraint handling
  void addMeasurementConstraint(bool b = true) { m_bMeasurement = b; };
  bool isMeasurementConstraint(void) const { return m_bMeasurement; };

  /*!
    Class constraint handling. An event must have one of the classes. If
    types are selected for its class, it must also have one of them.
  */
  bool addClassConstraint(uint16_t vscp_class);
  bool removeClassConstraint(uint16_t vscp_class);
  bool isClassAccepted(const vscp_event_t* pev) const;
  std::deque<uint16_t> getClasses(void) const;
  void clearClasses(void) { m_mapClass.clear(); };

  // Type Constraint handling, type is (class << 16) + type
  bool addTypeConstraint(uint32_t type);
  bool removeTypeConstraint(uint32_t type);
  bool isTypeAccepted(const vscp_event_t* pev) const;
  std::deque<uint32_t> getTypes(void) const;
  void clearTypes(void) { m_mapType.clear(); };

  // Data constraint handling, pos (16) : op (8) : value (8)
  bool addDataConstraint(uint16_t pos, uint8_t val, constraint chk = constraint::ANY);
  void addDataConstraints(const std::deque<uint32_t>& listData);
  bool removeDataConstraint(uint16_t pos);
  std::deque<uint32_t> getDataConstraints(void) const;
  bool isDataAccepted(const vscp_event_t* pev) const;

  // Data size constraint handling
  constraint getDataSizeConstraint(void) const { return m_constraint_data_size; };
  uint16_t getDataSizeValue(void) const { return m_data_size; };
  bool addDataSizeConstraint(uint16_t size, constraint chk = constraint::ANY);

  // GUID constraint handling, pos (8) : op (8) : value (8)
  bool addGuidConstraint(uint8_t pos, uint8_t val, constraint chk = constraint::ANY);
  bool removeGuidConstraint(uint8_t pos);
  bool isGuidAccepted(const vscp_event_t* pev) const;
  std::deque<uint32_t> getGuids(void) const;

  // Date constraint handling, op (8) : value (16)
  bool addDateConstraint(uint8_t pos, uint16_t val, constraint chk = constraint::ANY);
  uint32_t getDateConstraint(uint8_t pos) const;
  bool removeDateConstraint(uint8_t pos);
  bool isDateAccepted(const vscp_event_t* pev) const;

  // OBID constraint
  bool addObidConstraint(uint32_t val, constraint chk = constraint::ANY);
  bool removeObidConstraint(void);
  bool isObidAccepted(const vscp_event_t* pev) const;
  uint32_t getObidValue(void) const { return m_obid; }
  constraint getObidConstraint(void) const { return m_constraint_obid; }

  // TimeStamp constraint
  bool addTimeStampConstraint(uint32_t val, constraint chk = constraint::ANY);
  bool removeTimeStampConstraint(void);
  bool isTimeStampAccepted(const vscp_event_t* pev) const;
  uint32_t getTimeStampValue(void) const { return m_timestamp; }
  constraint getTimeStampConstraint(void) const { return m_constraint_timestamp; }

  // Priority constraint handling
  constraint getPriorityConstraint(void) const { return m_constraint_priority; };
  uint16_t getPriorityValue(void) const { return m_priority; };
  bool addPriorityConstraint(uint8_t priority, constraint chk = constraint::ANY);

  // Sensor index constraint handling
  constraint getSensorIndexConstraint(void) const { return m_constraint_sensor_index; };
  uint16_t getSensorIndexValue(void) const { return m_sensor_index; };
  bool addSensorIndexConstraint(uint8_t size, constraint chk = constraint::ANY);

  // Measurement value constraint handling
  constraint getMeasurementValueConstraint(void) const { return m_constraint_value; };
  double getMeasurementValue(void) const { return m_value; };
  bool addMeasurementValueConstraint(double value, constraint chk = constraint::ANY);

  // Measurement unit constraint handling
  constraint getMeasurementUnitConstraint(void) const { return m_constraint_unit; };
  uint8_t getMeasurementUnit(void) const { return m_unit; };
  bool addMeasurementUnitConstraint(uint8_t unit, constraint chk = constraint::ANY);

  // Measurement data coding constraint handling
  constraint getMeasurementDataCodingConstraint(void) const { return m_constraint_data_coding; };
  uint8_t getMeasurementDataCoding(void) const { return m_data_coding; };
  bool addMeasurementDataCodingConstraint(uint8_t coding, constraint chk = constraint::ANY);

private:
  /// Name of filter
  std::string m_name;

  /// Filter description
  std::string m_description;

  /// Javascript code to execute (not used yet)
  std::string m_script;

  /// Allow (show matching events) or deny (hide them)
  bool m_bAllow;

  // Must be receive event
  bool m_bReceive;

  // Must be transmit event
  bool m_bTransmit;

  // Accepted classes (empty is all)
  std::map<uint16_t, bool> m_mapClass;

  // Accepted types, (class << 16) + type (none for a class is all types)
  std::map<uint32_t, bool> m_mapType;

  // Data compare, pos -> pos (16) : op (8) : value (8)
  std::map<uint16_t, uint32_t> m_mapData;

  // Data size constraint
  constraint m_constraint_data_size;
  uint16_t m_data_size;

  // Priority constraint
  constraint m_constraint_priority;
  uint8_t m_priority;

  // GUID, pos -> op (8) : value (8)
  std::map<uint8_t, uint16_t> m_mapGuid;

  // obid constraint
  constraint m_constraint_obid;
  uint32_t m_obid;

  // timestamp constraint
  constraint m_constraint_timestamp;
  uint32_t m_timestamp;

  // Date, pos -> op (8) : value (16)
  std::map<uint8_t, uint32_t> m_mapDateTime;

  // Must be Level I event
  bool m_bLevel1;

  // Must be level II event
  bool m_bLevel2;

  // Must be measurement
  bool m_bMeasurement;

  // sensor index constraint
  constraint m_constraint_sensor_index;
  uint32_t m_sensor_index;

  // data coding constraint
  constraint m_constraint_data_coding;
  uint32_t m_data_coding;

  // unit constraint
  constraint m_constraint_unit;
  uint32_t m_unit;

  // Measurement value constraint
  constraint m_constraint_value;
  double m_value;
};

#endif // SESSIONFILTER_H
