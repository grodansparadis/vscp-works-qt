# - Find paho-mqtt
# Find the paho-mqtt includes and library from vcpkg
# It's temporary solution on Win, written issue:
# https://github.com/Microsoft/vcpkg/issues/3175
#  paho-mqtt_INCLUDE_DIR - Where to find paho-mqtt includes
#  paho-mqtt_LIBRARIES   - List of libraries when using paho-mqtt
#  paho-mqtt_FOUND       - True if paho-mqtt was found

# To find and use catch
if (WIN32)
find_path(paho-mqtt_INCLUDE_DIR MQTTAsync.h
  PATHS
  c:/devel/vcpkg/installed/x64-windows/include
  DOC "paho-mqtt - Headers"
)
else()
find_path(paho-mqtt_INCLUDE_DIR MQTTAsync.h
  PATHS
  /usr/include
  /usr/local/include/
  DOC "paho-mqtt - Headers"
)
endif()

include_directories(${paho-mqtt_INCLUDE_DIR})

if (WIN32)
SET(paho-mqtt_NAMES paho-mqtt3as.lib paho-mqtt3cs.lib)
else()
SET(paho-mqtt_NAMES paho-mqtt3as paho-mqtt3cs)
endif()

if (WIN32)
FIND_LIBRARY(paho-mqtt_LIBRARY NAMES ${paho-mqtt_NAMES}
  PATHS
  c:/devel/vcpkg/installed/x64-windows/lib
  PATH_SUFFIXES lib lib64
  DOC "paho-mqtt - Library"
)
else()
FIND_LIBRARY(paho-mqtt_LIBRARY NAMES ${paho-mqtt_NAMES}
  PATHS
  /usr/lib/x86_64-linux-gnu
  /usr/local/lib
  PATH_SUFFIXES lib lib64
  DOC "paho-mqtt - Library"
)
endif()

INCLUDE(FindPackageHandleStandardArgs)

SET(paho-mqtt_LIBRARIES ${paho-mqtt_LIBRARY})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(paho-mqtt DEFAULT_MSG paho-mqtt_LIBRARY paho-mqtt_INCLUDE_DIR)
  
MARK_AS_ADVANCED(paho-mqtt_LIBRARY paho-mqtt_INCLUDE_DIR)
  
IF(paho-mqtt_FOUND)
  SET(paho-mqtt_INCLUDE_DIRS ${paho-mqtt_INCLUDE_DIR})
ENDIF(paho-mqtt_FOUND)
