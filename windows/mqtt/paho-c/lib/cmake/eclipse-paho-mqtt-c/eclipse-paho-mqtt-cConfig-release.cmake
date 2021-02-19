#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "eclipse-paho-mqtt-c::paho-mqtt3c" for configuration "Release"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3c APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3c PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/paho-mqtt3c.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/paho-mqtt3c.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3c )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3c "${_IMPORT_PREFIX}/lib/paho-mqtt3c.lib" "${_IMPORT_PREFIX}/bin/paho-mqtt3c.dll" )

# Import target "eclipse-paho-mqtt-c::paho-mqtt3a" for configuration "Release"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3a APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3a PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/paho-mqtt3a.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/paho-mqtt3a.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3a )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3a "${_IMPORT_PREFIX}/lib/paho-mqtt3a.lib" "${_IMPORT_PREFIX}/bin/paho-mqtt3a.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
