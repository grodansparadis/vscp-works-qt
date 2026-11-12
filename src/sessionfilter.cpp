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

#ifdef WIN32
#include <pch.h>
#endif

#include <vscp.h>
#include <vscphelper.h>

#include "sessionfilter.h"

#include <fstream>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

CSessionFilter::CSessionFilter()
{
  clear();
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

CSessionFilter::~CSessionFilter()
{
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

void
CSessionFilter::clear(void)
{
  m_bReceive     = false;
  m_bTransmit    = false;
  m_bLevel1      = false;
  m_bLevel2      = false;
  m_bMeasurement = false;

  m_mapClass.clear();
  m_mapType.clear();
  m_mapData.clear();
  m_mapGuid.clear();
  m_mapDateTime.clear();

  m_constraint_data_size = constraint::ANY;
  m_data_size            = 0;

  m_constraint_priority = constraint::ANY;
  m_priority            = 0;

  m_constraint_obid = constraint::ANY;
  m_obid            = 0;

  m_constraint_timestamp = constraint::ANY;
  m_timestamp            = 0;

  m_constraint_sensor_index = constraint::ANY;
  m_sensor_index            = 0;

  m_constraint_data_coding = constraint::ANY;
  m_data_coding            = 0;

  m_constraint_unit = constraint::ANY;
  m_unit            = 0;

  m_constraint_value = constraint::ANY;
  m_value            = 0;

  m_script.clear();
  m_bAllow = true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addClassConstraint
//

bool
CSessionFilter::addClassConstraint(uint16_t vscp_class)
{
  m_mapClass[vscp_class] = true;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClassConstraint
//

bool
CSessionFilter::removeClassConstraint(uint16_t vscp_class)
{
  m_mapClass.erase(vscp_class);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isClassAccepted
//

bool
CSessionFilter::isClassAccepted(const vscp_event_t* pev) const
{
  if (nullptr == pev) {
    return false;
  }
  if (m_mapClass.empty()) {
    return true;
  }
  return m_mapClass.end() != m_mapClass.find(pev->vscp_class);
}

///////////////////////////////////////////////////////////////////////////////
// getClasses
//

std::deque<uint16_t>
CSessionFilter::getClasses(void) const
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

bool
CSessionFilter::addTypeConstraint(uint32_t type)
{
  m_mapType[type] = true;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTypeConstraint
//

bool
CSessionFilter::removeTypeConstraint(uint32_t type)
{
  m_mapType.erase(type);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isTypeAccepted
//
// Types are selected per class. A class without selected types accepts all
// its types.
//

bool
CSessionFilter::isTypeAccepted(const vscp_event_t* pev) const
{
  if (nullptr == pev) {
    return false;
  }

  const uint32_t first = ((uint32_t) pev->vscp_class) << 16;
  auto it              = m_mapType.lower_bound(first);
  if ((m_mapType.end() == it) || ((it->first & 0xffff0000) != first)) {
    return true; // No types selected for this class
  }

  return m_mapType.end() != m_mapType.find(first + pev->vscp_type);
}

///////////////////////////////////////////////////////////////////////////////
// getTypes
//

std::deque<uint32_t>
CSessionFilter::getTypes(void) const
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

bool
CSessionFilter::addGuidConstraint(uint8_t pos, uint8_t val, constraint chk)
{
  if (pos > 15) {
    return false;
  }
  if (constraint::ANY == chk) {
    m_mapGuid.erase(pos);
    return true;
  }
  // pos(8) -> chk(8) : guid(8)
  m_mapGuid[pos] = (((uint16_t) chk) << 8) + val;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeGuidConstraint
//

bool
CSessionFilter::removeGuidConstraint(uint8_t pos)
{
  m_mapGuid.erase(pos);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isGuidAccepted
//

bool
CSessionFilter::isGuidAccepted(const vscp_event_t* pev) const
{
  for (auto const& item : m_mapGuid) {
    uint8_t pos = item.first;
    auto chk    = static_cast<constraint>((item.second >> 8) & 0xff);
    uint8_t val = item.second & 0xff;
    if ((pos < 16) && !checkValue(val, pev->GUID[pos], chk)) {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// getGuids
//

std::deque<uint32_t>
CSessionFilter::getGuids(void) const
{
  std::deque<uint32_t> listGuid;

  for (auto const& item : m_mapGuid) {
    uint32_t val = (((uint32_t) item.first) << 16) + item.second;
    listGuid.push_back(val);
  }

  return listGuid;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addDateConstraint
//

bool
CSessionFilter::addDateConstraint(uint8_t pos, uint16_t val, constraint chk)
{
  if (pos > date_pos_second) {
    return false;
  }
  if (constraint::ANY == chk) {
    m_mapDateTime.erase(pos);
    return true;
  }
  m_mapDateTime[pos] = (static_cast<uint32_t>(chk) << 16) + val;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// getDateConstraint
//

uint32_t
CSessionFilter::getDateConstraint(uint8_t pos) const
{
  auto it = m_mapDateTime.find(pos);
  return (m_mapDateTime.end() == it) ? 0 : it->second;
}

///////////////////////////////////////////////////////////////////////////////
// removeDateConstraint
//

bool
CSessionFilter::removeDateConstraint(uint8_t pos)
{
  m_mapDateTime.erase(pos);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isDateAccepted
//

bool
CSessionFilter::isDateAccepted(const vscp_event_t* pev) const
{
  for (auto const& item : m_mapDateTime) {

    auto chk     = static_cast<constraint>((item.second >> 16) & 0xff);
    uint16_t val = item.second & 0xffff;
    uint32_t evval;

    switch (item.first) {
      case date_pos_year:
        evval = pev->year;
        break;
      case date_pos_month:
        evval = pev->month;
        break;
      case date_pos_day:
        evval = pev->day;
        break;
      case date_pos_hour:
        evval = pev->hour;
        break;
      case date_pos_minute:
        evval = pev->minute;
        break;
      case date_pos_second:
        evval = pev->second;
        break;
      default:
        continue;
    }

    if (!checkValue(val, evval, chk)) {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addObidConstraint
//

bool
CSessionFilter::addObidConstraint(uint32_t val, constraint chk)
{
  m_obid            = val;
  m_constraint_obid = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeObidConstraint
//

bool
CSessionFilter::removeObidConstraint(void)
{
  m_constraint_obid = constraint::ANY;
  m_obid            = 0;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isObidAccepted
//

bool
CSessionFilter::isObidAccepted(const vscp_event_t* pev) const
{
  return checkValue(m_obid, pev->obid, m_constraint_obid);
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addTimeStampConstraint
//

bool
CSessionFilter::addTimeStampConstraint(uint32_t val, constraint chk)
{
  m_timestamp            = val;
  m_constraint_timestamp = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTimeStampConstraint
//

bool
CSessionFilter::removeTimeStampConstraint(void)
{
  m_constraint_timestamp = constraint::ANY;
  m_timestamp            = 0;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isTimeStampAccepted
//

bool
CSessionFilter::isTimeStampAccepted(const vscp_event_t* pev) const
{
  return checkValue(m_timestamp, pev->timestamp, m_constraint_timestamp);
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// addDataConstraints
//

void
CSessionFilter::addDataConstraints(const std::deque<uint32_t>& listData)
{
  m_mapData.clear();

  // listData: pos (16) : op (8) : value (8)
  for (auto const& item : listData) {
    uint16_t pos   = (item >> 16) & 0x1ff;
    m_mapData[pos] = item & 0x1ffffff;
  }
}

///////////////////////////////////////////////////////////////////////////////
// getDataConstraints
//

std::deque<uint32_t>
CSessionFilter::getDataConstraints(void) const
{
  std::deque<uint32_t> listData;
  for (auto const& item : m_mapData) {
    // pos (16) : op (8) : value (8)
    listData.push_back(item.second);
  }
  return listData;
}

///////////////////////////////////////////////////////////////////////////////
// addDataConstraint
//

bool
CSessionFilter::addDataConstraint(uint16_t pos, uint8_t val, constraint chk)
{
  if (pos > 511) {
    return false;
  }
  // pos (16) : op (8) : value (8)
  m_mapData[pos] = (((uint32_t) pos) << 16) + (((uint32_t) chk) << 8) + val;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeDataConstraint
//

bool
CSessionFilter::removeDataConstraint(uint16_t pos)
{
  m_mapData.erase(pos);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// isDataAccepted
//

bool
CSessionFilter::isDataAccepted(const vscp_event_t* pev) const
{
  for (auto const& item : m_mapData) {

    auto chk = static_cast<constraint>((item.second >> 8) & 0xff);
    if (constraint::ANY == chk) {
      continue;
    }

    // An event without data at this position does not match
    if ((nullptr == pev->pdata) || (pev->sizeData <= item.first)) {
      return false;
    }

    if (!checkValue(item.second & 0xff, pev->pdata[item.first], chk)) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addDataSizeConstraint
//

bool
CSessionFilter::addDataSizeConstraint(uint16_t size, constraint chk)
{
  if (size > 512) {
    size = 512;
  }
  m_data_size            = size;
  m_constraint_data_size = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addPriorityConstraint
//

bool
CSessionFilter::addPriorityConstraint(uint8_t priority, constraint chk)
{
  m_priority            = (priority > 7) ? 7 : priority;
  m_constraint_priority = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addSensorIndexConstraint
//

bool
CSessionFilter::addSensorIndexConstraint(uint8_t sensor_index, constraint chk)
{
  m_sensor_index            = sensor_index;
  m_constraint_sensor_index = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementValueConstraint
//

bool
CSessionFilter::addMeasurementValueConstraint(double value, constraint chk)
{
  m_value            = value;
  m_constraint_value = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementUnitConstraint
//

bool
CSessionFilter::addMeasurementUnitConstraint(uint8_t unit, constraint chk)
{
  m_unit            = unit;
  m_constraint_unit = chk;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addMeasurementDataCodingConstraint
//

bool
CSessionFilter::addMeasurementDataCodingConstraint(uint8_t coding, constraint chk)
{
  m_data_coding            = coding;
  m_constraint_data_coding = chk;
  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// checkValue
//

bool
CSessionFilter::checkValue(uint32_t val, uint32_t evval, constraint chk)
{
  switch (chk) {
    case constraint::EQ:
      return evval == val;
    case constraint::NEQ:
      return evval != val;
    case constraint::LT:
      return evval < val;
    case constraint::LTEQ:
      return evval <= val;
    case constraint::GT:
      return evval > val;
    case constraint::GTEQ:
      return evval >= val;
    case constraint::ANY:
    default:
      return true;
  }
}

///////////////////////////////////////////////////////////////////////////////
// check
//

bool
CSessionFilter::check(const vscp_event_t* pev, bool bReceived) const
{
  if (nullptr == pev) {
    return false;
  }
  return m_bAllow ? matches(pev, bReceived) : !matches(pev, bReceived);
}

///////////////////////////////////////////////////////////////////////////////
// matches
//

bool
CSessionFilter::matches(const vscp_event_t* pev, bool bReceived) const
{
  if (nullptr == pev) {
    return false;
  }

  // Direction
  if (m_bReceive && !bReceived) {
    return false;
  }
  if (m_bTransmit && bReceived) {
    return false;
  }

  // Level I / Level II
  if (m_bLevel1 && (pev->vscp_class >= 1024)) {
    return false;
  }
  if (m_bLevel2 && (pev->vscp_class < 1024)) {
    return false;
  }

  // Class and type
  if (!m_mapClass.empty() && !isClassAccepted(pev)) {
    return false;
  }
  if (!m_mapType.empty() && !isTypeAccepted(pev)) {
    return false;
  }

  // Priority
  if (!checkValue(m_priority, vscp_getEventPriority(pev), m_constraint_priority)) {
    return false;
  }

  // GUID
  if (!isGuidAccepted(pev)) {
    return false;
  }

  // OBID
  if (!checkValue(m_obid, pev->obid, m_constraint_obid)) {
    return false;
  }

  // Timestamp
  if (!checkValue(m_timestamp, pev->timestamp, m_constraint_timestamp)) {
    return false;
  }

  // Date
  if (!isDateAccepted(pev)) {
    return false;
  }

  // Data size
  if (!checkValue(m_data_size, pev->sizeData, m_constraint_data_size)) {
    return false;
  }

  // Data
  if (!isDataAccepted(pev)) {
    return false;
  }

  // Measurements. Any measurement constraint requires a measurement event.
  const bool bMeasurementConstraint =
    m_bMeasurement || (constraint::ANY != m_constraint_sensor_index) ||
    (constraint::ANY != m_constraint_unit) || (constraint::ANY != m_constraint_data_coding) ||
    (constraint::ANY != m_constraint_value);

  if (!bMeasurementConstraint) {
    return true;
  }

  if (!vscp_isMeasurement(pev)) {
    return false;
  }

  if (constraint::ANY != m_constraint_sensor_index) {
    int idx = vscp_getMeasurementSensorIndex(pev);
    if ((idx < 0) || !checkValue(m_sensor_index, (uint32_t) idx, m_constraint_sensor_index)) {
      return false;
    }
  }

  if (constraint::ANY != m_constraint_unit) {
    int unit = vscp_getMeasurementUnit(pev);
    if ((unit < 0) || !checkValue(m_unit, (uint32_t) unit, m_constraint_unit)) {
      return false;
    }
  }

  if (constraint::ANY != m_constraint_data_coding) {
    if (!checkValue(m_data_coding, vscp_getMeasurementDataCoding(pev), m_constraint_data_coding)) {
      return false;
    }
  }

  if (constraint::ANY != m_constraint_value) {
    double mvalue;
    if (!vscp_getMeasurementAsDouble(&mvalue, pev)) {
      return false;
    }

    switch (m_constraint_value) {
      case constraint::EQ:
        return mvalue == m_value;
      case constraint::NEQ:
        return mvalue != m_value;
      case constraint::LT:
        return mvalue < m_value;
      case constraint::LTEQ:
        return mvalue <= m_value;
      case constraint::GT:
        return mvalue > m_value;
      case constraint::GTEQ:
        return mvalue >= m_value;
      default:
        break;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// hasConstraint
//

bool
CSessionFilter::hasConstraint(uint8_t type) const
{
  switch (type) {
    case type_must_be_receive:
      return m_bReceive;
    case type_must_be_transmit:
      return m_bTransmit;
    case type_must_be_level1:
      return m_bLevel1;
    case type_must_be_level2:
      return m_bLevel2;
    case type_must_be_measurement:
      return m_bMeasurement;
    case type_class:
    case type_type:
      return !m_mapClass.empty() || !m_mapType.empty();
    case type_priority:
      return constraint::ANY != m_constraint_priority;
    case type_guid:
      return !m_mapGuid.empty();
    case type_obid:
      return constraint::ANY != m_constraint_obid;
    case type_timestamp:
      return constraint::ANY != m_constraint_timestamp;
    case type_date:
      return !m_mapDateTime.empty();
    case type_data_size:
      return constraint::ANY != m_constraint_data_size;
    case type_data:
      return !m_mapData.empty();
    case type_sensor_index:
      return constraint::ANY != m_constraint_sensor_index;
    case type_unit:
      return constraint::ANY != m_constraint_unit;
    case type_data_coding:
      return constraint::ANY != m_constraint_data_coding;
    case type_value:
      return constraint::ANY != m_constraint_value;
    case type_script:
      return !m_script.empty();
    default:
      return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// getConstraintTypes
//

std::deque<uint8_t>
CSessionFilter::getConstraintTypes(void) const
{
  std::deque<uint8_t> types;
  for (uint8_t type = type_must_be_receive; type <= type_script; type++) {
    if ((type_type != type) && hasConstraint(type)) {
      types.push_back(type);
    }
  }
  return types;
}

///////////////////////////////////////////////////////////////////////////////
// removeConstraint
//

void
CSessionFilter::removeConstraint(uint8_t type)
{
  switch (type) {
    case type_must_be_receive:
      m_bReceive = false;
      break;
    case type_must_be_transmit:
      m_bTransmit = false;
      break;
    case type_must_be_level1:
      m_bLevel1 = false;
      break;
    case type_must_be_level2:
      m_bLevel2 = false;
      break;
    case type_must_be_measurement:
      m_bMeasurement = false;
      break;
    case type_class:
    case type_type:
      m_mapClass.clear();
      m_mapType.clear();
      break;
    case type_priority:
      m_constraint_priority = constraint::ANY;
      break;
    case type_guid:
      m_mapGuid.clear();
      break;
    case type_obid:
      removeObidConstraint();
      break;
    case type_timestamp:
      removeTimeStampConstraint();
      break;
    case type_date:
      m_mapDateTime.clear();
      break;
    case type_data_size:
      m_constraint_data_size = constraint::ANY;
      break;
    case type_data:
      m_mapData.clear();
      break;
    case type_sensor_index:
      m_constraint_sensor_index = constraint::ANY;
      break;
    case type_unit:
      m_constraint_unit = constraint::ANY;
      break;
    case type_data_coding:
      m_constraint_data_coding = constraint::ANY;
      break;
    case type_value:
      m_constraint_value = constraint::ANY;
      break;
    case type_script:
      m_script.clear();
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// getConstraintTypeName
//

std::string
CSessionFilter::getConstraintTypeName(uint8_t type)
{
  switch (type) {
    case type_must_be_receive:
      return "Must be received event";
    case type_must_be_transmit:
      return "Must be transmitted event";
    case type_must_be_level1:
      return "Must be Level I event";
    case type_must_be_level2:
      return "Must be Level II event";
    case type_must_be_measurement:
      return "Must be measurement";
    case type_class:
    case type_type:
      return "Specific class/type";
    case type_priority:
      return "Priority";
    case type_guid:
      return "GUID";
    case type_obid:
      return "OBID";
    case type_timestamp:
      return "Timestamp";
    case type_date:
      return "Date/Time";
    case type_data_size:
      return "Data size";
    case type_data:
      return "Data value(s)";
    case type_sensor_index:
      return "Sensor index";
    case type_unit:
      return "Measurement unit";
    case type_data_coding:
      return "Measurement data coding";
    case type_value:
      return "Measurement value";
    case type_script:
      return "Script";
    default:
      return "Unknown";
  }
}

// ----------------------------------------------------------------------------

namespace {

json
opValue(CSessionFilter::constraint chk, double value)
{
  return json{ { "op", static_cast<int>(chk) }, { "value", value } };
}

bool
readOpValue(const json& j, const char* key, CSessionFilter::constraint& chk, double& value)
{
  if (!j.contains(key) || !j[key].is_object()) {
    return false;
  }
  const json& o = j[key];
  int op        = o.value("op", 0);
  if ((op < 0) || (op > static_cast<int>(CSessionFilter::constraint::GTEQ))) {
    return false;
  }
  chk   = static_cast<CSessionFilter::constraint>(op);
  value = o.value("value", 0.0);
  return true;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// toJson
//

json
CSessionFilter::toJson(void) const
{
  json j;
  j["name"]        = m_name;
  j["description"] = m_description;
  if (!m_bAllow) {
    j["deny"] = true;
  }

  if (m_bReceive) {
    j["receive"] = true;
  }
  if (m_bTransmit) {
    j["transmit"] = true;
  }
  if (m_bLevel1) {
    j["level1"] = true;
  }
  if (m_bLevel2) {
    j["level2"] = true;
  }
  if (m_bMeasurement) {
    j["measurement"] = true;
  }

  if (!m_mapClass.empty()) {
    j["classes"] = getClasses();
  }
  if (!m_mapType.empty()) {
    json types = json::array();
    for (auto const& item : m_mapType) {
      types.push_back({ { "class", item.first >> 16 }, { "type", item.first & 0xffff } });
    }
    j["types"] = types;
  }

  if (!m_mapGuid.empty()) {
    json guid = json::array();
    for (auto const& item : m_mapGuid) {
      guid.push_back({ { "pos", item.first }, { "op", item.second >> 8 }, { "value", item.second & 0xff } });
    }
    j["guid"] = guid;
  }

  if (!m_mapDateTime.empty()) {
    json date = json::array();
    for (auto const& item : m_mapDateTime) {
      date.push_back(
        { { "pos", item.first }, { "op", (item.second >> 16) & 0xff }, { "value", item.second & 0xffff } });
    }
    j["date"] = date;
  }

  if (!m_mapData.empty()) {
    json data = json::array();
    for (auto const& item : m_mapData) {
      data.push_back(
        { { "pos", item.first }, { "op", (item.second >> 8) & 0xff }, { "value", item.second & 0xff } });
    }
    j["data"] = data;
  }

  if (constraint::ANY != m_constraint_priority) {
    j["priority"] = opValue(m_constraint_priority, m_priority);
  }
  if (constraint::ANY != m_constraint_obid) {
    j["obid"] = opValue(m_constraint_obid, m_obid);
  }
  if (constraint::ANY != m_constraint_timestamp) {
    j["timestamp"] = opValue(m_constraint_timestamp, m_timestamp);
  }
  if (constraint::ANY != m_constraint_data_size) {
    j["data-size"] = opValue(m_constraint_data_size, m_data_size);
  }
  if (constraint::ANY != m_constraint_sensor_index) {
    j["sensor-index"] = opValue(m_constraint_sensor_index, m_sensor_index);
  }
  if (constraint::ANY != m_constraint_unit) {
    j["unit"] = opValue(m_constraint_unit, m_unit);
  }
  if (constraint::ANY != m_constraint_data_coding) {
    j["data-coding"] = opValue(m_constraint_data_coding, m_data_coding);
  }
  if (constraint::ANY != m_constraint_value) {
    j["value"] = opValue(m_constraint_value, m_value);
  }
  if (!m_script.empty()) {
    j["script"] = m_script;
  }

  return j;
}

///////////////////////////////////////////////////////////////////////////////
// fromJson
//

bool
CSessionFilter::fromJson(const json& j)
{
  if (!j.is_object() || !j.contains("name") || !j["name"].is_string()) {
    return false;
  }

  try {
    clear();

    m_name         = j["name"].get<std::string>();
    m_description  = j.value("description", "");
    m_bReceive     = j.value("receive", false);
    m_bTransmit    = j.value("transmit", false);
    m_bLevel1      = j.value("level1", false);
    m_bLevel2      = j.value("level2", false);
    m_bMeasurement = j.value("measurement", false);
    m_script       = j.value("script", "");
    m_bAllow       = !j.value("deny", false);

    if (j.contains("classes") && j["classes"].is_array()) {
      for (auto const& c : j["classes"]) {
        addClassConstraint(c.get<uint16_t>());
      }
    }

    if (j.contains("types") && j["types"].is_array()) {
      for (auto const& t : j["types"]) {
        addTypeConstraint((((uint32_t) t.value("class", 0)) << 16) + (t.value("type", 0) & 0xffff));
      }
    }

    auto op = [](const json& item) {
      int v = item.value("op", 0);
      if ((v < 0) || (v > static_cast<int>(constraint::GTEQ))) {
        v = 0;
      }
      return static_cast<constraint>(v);
    };

    if (j.contains("guid") && j["guid"].is_array()) {
      for (auto const& g : j["guid"]) {
        addGuidConstraint(g.value("pos", 0), g.value("value", 0), op(g));
      }
    }

    if (j.contains("date") && j["date"].is_array()) {
      for (auto const& d : j["date"]) {
        addDateConstraint(d.value("pos", 0), d.value("value", 0), op(d));
      }
    }

    if (j.contains("data") && j["data"].is_array()) {
      for (auto const& d : j["data"]) {
        addDataConstraint(d.value("pos", 0), d.value("value", 0), op(d));
      }
    }

    constraint chk;
    double value;
    if (readOpValue(j, "priority", chk, value)) {
      addPriorityConstraint((uint8_t) value, chk);
    }
    if (readOpValue(j, "obid", chk, value)) {
      addObidConstraint((uint32_t) value, chk);
    }
    if (readOpValue(j, "timestamp", chk, value)) {
      addTimeStampConstraint((uint32_t) value, chk);
    }
    if (readOpValue(j, "data-size", chk, value)) {
      addDataSizeConstraint((uint16_t) value, chk);
    }
    if (readOpValue(j, "sensor-index", chk, value)) {
      addSensorIndexConstraint((uint8_t) value, chk);
    }
    if (readOpValue(j, "unit", chk, value)) {
      addMeasurementUnitConstraint((uint8_t) value, chk);
    }
    if (readOpValue(j, "data-coding", chk, value)) {
      addMeasurementDataCodingConstraint((uint8_t) value, chk);
    }
    if (readOpValue(j, "value", chk, value)) {
      addMeasurementValueConstraint(value, chk);
    }
  }
  catch (const json::exception&) {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// readFilters
//

bool
CSessionFilter::readFilters(const std::string& path, std::vector<CSessionFilter>& filters)
{
  std::ifstream in(path);
  if (!in) {
    return false;
  }

  json j;
  try {
    in >> j;
  }
  catch (const json::exception&) {
    return false;
  }

  if (!j.is_array()) {
    return false;
  }

  filters.clear();
  for (auto const& item : j) {
    CSessionFilter filter;
    if (filter.fromJson(item)) {
      filters.push_back(filter);
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeFilters
//

bool
CSessionFilter::writeFilters(const std::string& path, const std::vector<CSessionFilter>& filters)
{
  json j = json::array();
  for (auto const& filter : filters) {
    j.push_back(filter.toJson());
  }

  std::ofstream out(path, std::ios::trunc);
  if (!out) {
    return false;
  }
  out << j.dump(2) << std::endl;
  return out.good();
}
