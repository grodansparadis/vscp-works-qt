# CMakeLists.txt
#
# Build instructions for VSCP Works.
#
# Copyright (c) 2000-2025 Åke Hedman, Grodans Paradis AB
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# VSCP_PATH - set to path to local vscp repository. Default is "../vscp". Set with
# -DVSCP_PATH="path"
#
# windows
# -------
# cmake .. -G "Visual Studio 17 2022" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=G:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH=G:\Qt\6.7.2\msvc2019_64\
# cmake --build . --config Debug
# set PATH=G:\Qt\Qt-6.7.2\msvc2019_64\bin;%PATH%
#

cmake_minimum_required(VERSION 3.5)
project(vscp-works-qt LANGUAGES CXX C)

option(USE_SSL "Use SSL" TRUE)

## --- C++14 build flags ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Generate position-independent code (-fPIC on UNIX)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${PROJECT_NAM} PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE ON
)

# --- System Libraries ---
include(GNUInstallDirs) 

# Added extensions for curl lib find
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a")

# --- Path to VSCP --- 
if (NOT DEFINED VSCP_PATH)
  set (VSCP_PATH "${PROJECT_SOURCE_DIR}/third_party/vscp")
endif()

file(TO_CMAKE_PATH "$ENV{VSCP_ROOT}" VSCP_PATH)
message(STATUS "VSCP path = $ENV{VSCP_ROOT}")

if (DEFINED QT_PATH)
file(TO_CMAKE_PATH "$ENV{QT_ROOT_DIR}" QT_PATH)
message(STATUS "QT_ROOT_DIR = $ENV{QT_ROOT_DIR}")
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DWIN32_LEAN_AND_MEANS)
  add_definitions(-D_POSIX_C_SOURCE)
  add_definitions(-DVCPKG_TARGET_TRIPLET=x64-windows)
  add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
  add_definitions(-D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
  set(LIBS_SYSTEM c stdc++)
endif()

if (MSVC)
  # use precompiled headers
  set (CMAKE_AUTOMOC_MOC_OPTIONS "-bpch.h ")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# qt - Charts Quick SerialBus SerialPort  
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Sql Network Qml)
qt_standard_project_setup()

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)

# libwebsockets
#find_package(libwebsockets CONFIG REQUIRED)
include(FindLibWebSockets)

#set(TS_FILES vscp-works-qt_en_150.ts)

# Windows Libs
if (MSVC)
  message("Windows build")
  #include(g:/akhe/development/vcpkg/scripts/buildsystems/vcpkg.cmake)
  
  # pthread
  find_package(PThreads4W REQUIRED)
  #find_package(pthread REQUIRED)

  # Paho MQTT
  #find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)
  #get_target_property(PAHO_INCLUDE_DIR eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_INCLUDE_DIRECTORIES)
  #get_target_property(PAHO_LIBRARIES eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_LINK_LIBRARIES)

  # Moongose
  #find_package(unofficial-mongoose CONFIG REQUIRED)

  # Mosquitto lib
  #find_package(mosquitto REQUIRED)
  include(FindMosquitto)

  # SQLite3
  #find_package(unofficial-sqlite3 CONFIG REQUIRED)

  # Expat C library
  find_package(expat CONFIG REQUIRED)

  # dlfcn
  find_package(dlfcn-win32 CONFIG REQUIRED)
else()
  # expat
  include(FindEXPAT)
  find_package(EXPAT REQUIRED)
  include_directories(${EXPAT_INCLUDE_DIRS})
  # paho
  #include(Findpaho-mqtt)
  # Mosquitto lib
  include(FindMosquitto)

endif()

find_package(CURL REQUIRED) 
IF(CURL_FOUND)
  message(STATUS Found cURL)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library.")
ENDIF(CURL_FOUND)

set(OPENSSL_VER "OPENSSL_API_${OPENSSL_VERSION_MAJOR}_${OPENSSL_VERSION_MINOR}")
message(STATUS "OPENSSL Version: ${OPENSSL_VER} - ${OPENSSL_VERSION}")
#add_definitions(-D${OPENSSL_VER}) 

# 1.1.0 / 1.1.1 / 3.0 / 3.1 / 3.2 / 3.3df -h
string(FIND "${OPENSSL_VERSION}" "1.0" SSL_VER_POS)
if(NOT ${SSL_VER_POS} EQUAL -1)
  add_definitions(-DOPENSSL_API_1_0)
  message(STATUS "Open ssl version is 1.0")
endif()    

string(FIND "${OPENSSL_VERSION}" "1.1" SSL_VER_POS)
if(NOT ${SSL_VER_POS} EQUAL -1)
  add_definitions(-DOPENSSL_API_1_1)
  message(STATUS "Open ssl version is 1.1")
endif()

string(FIND "${OPENSSL_VERSION}" "3.0" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_0)
  message(STATUS "Open ssl version is 3.0")
endif()  

string(FIND "${OPENSSL_VERSION}" "3.1" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_1)
  message(STATUS "Open ssl version is 3.1")
endif()

string(FIND "${OPENSSL_VERSION}" "3.2" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_2)
  message(STATUS "Open ssl version is 3.2")
endif()

string(FIND "${OPENSSL_VERSION}" "3.3" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_3)
  message(STATUS "Open ssl version is 3.3")
endif()


qt_add_executable(${PROJECT_NAME}
  src/vscpworks.qrc
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
  src/filedownloader.h
  src/filedownloader.cpp

  src/cdlgnewconnection.ui
  src/cdlgnewconnection.h
  src/cdlgnewconnection.cpp

  src/cdlgmainsettings.ui
  src/cdlgmainsettings.h
  src/cdlgmainsettings.cpp

  src/cdlgconnsettingscanal.ui
  src/cdlgconnsettingscanal.h
  src/cdlgconnsettingscanal.cpp

  src/cdlgconnsettingstcpip.ui
  src/cdlgconnsettingstcpip.h
  src/cdlgconnsettingstcpip.cpp

  src/cdlgconnsettingssocketcan.ui
  src/cdlgconnsettingssocketcan.h
  src/cdlgconnsettingssocketcan.cpp

  src/cdlgsocketcanflags.ui
  src/cdlgsocketcanflags.h
  src/cdlgsocketcanflags.cpp

  src/cdlgconnsettingsmqtt.ui
  src/cdlgconnsettingsmqtt.h
  src/cdlgconnsettingsmqtt.cpp

  src/cdlgmqttpublish.ui
  src/cdlgmqttpublish.h
  src/cdlgmqttpublish.cpp

  src/cdlgmqttsubscribe.ui
  src/cdlgmqttsubscribe.h
  src/cdlgmqttsubscribe.cpp

  src/cdlgconnsettingsws1.ui
  src/cdlgconnsettingsws1.h
  src/cdlgconnsettingsws1.cpp

  src/cdlgconnsettingsws2.ui
  src/cdlgconnsettingsws2.h
  src/cdlgconnsettingsws2.cpp

  src/cdlgconnsettingsudp.ui
  src/cdlgconnsettingsudp.h
  src/cdlgconnsettingsudp.cpp

  src/cdlgconnsettingsmulticast.ui
  src/cdlgconnsettingsmulticast.h
  src/cdlgconnsettingsmulticast.cpp

  src/cdlgknownguid.ui
  src/cdlgknownguid.h
  src/cdlgknownguid.cpp

  src/cdlglevel1filter.ui
  src/cdlglevel1filter.h
  src/cdlglevel1filter.cpp

  src/cdlglevel1filterwizard.ui
  src/cdlglevel1filterwizard.h
  src/cdlglevel1filterwizard.cpp

  src/cdlglevel2filter.ui
  src/cdlglevel2filter.h
  src/cdlglevel2filter.cpp

  src/cdlgcanfilter.ui
  src/cdlgcanfilter.h
  src/cdlgcanfilter.cpp

  src/cdlgtls.ui
  src/cdlgtls.h
  src/cdlgtls.cpp  

  src/cfrmsession.h
  src/cfrmsession.cpp

  src/vscpworks.h
  src/vscpworks.cpp
  
  src/canalconfigwizard.h
  src/canalconfigwizard.cpp

  src/eventlistmodel.h
  src/eventlistmodel.cpp

  src/cfrmnodeconfig.h
  src/cfrmnodeconfig.cpp
  src/cfrmnodeconfig.ui

  src/cdlgeditdm.ui
  src/cdlgeditdm.h
  src/cdlgeditdm.cpp

  src/cdlgactionparam.ui
  src/cdlgactionparam.h
  src/cdlgactionparam.cpp

  src/cfrmnodescan.h
  src/cfrmnodescan.cpp
  src/cfrmnodescan.ui

  src/cdlgeditguid.ui
  src/cdlgeditguid.h
  src/cdlgeditguid.cpp

  src/cdlgsensorindex.ui
  src/cdlgsensorindex.h
  src/cdlgsensorindex.cpp

  src/cdlgeditsensorindex.ui
  src/cdlgeditsensorindex.h
  src/cdlgeditsensorindex.cpp

  src/cdlgtxedit.ui
  src/cdlgtxedit.h
  src/cdlgtxedit.cpp

  src/ctxevent.h
  src/ctxevent.cpp

  src/sessionfilter.h
  src/sessionfilter.cpp

  src/cdlgsessionfilter.ui
  src/cdlgsessionfilter.h
  src/cdlgsessionfilter.cpp

  src/cdlgeditsessionfilter.ui
  src/cdlgeditsessionfilter.h
  src/cdlgeditsessionfilter.cpp

  src/cdlgselectclass.ui
  src/cdlgselectclass.h
  src/cdlgselectclass.cpp

  src/cdlgselectguid.ui
  src/cdlgselectguid.h
  src/cdlgselectguid.cpp

  src/cdlgselectobid.ui
  src/cdlgselectobid.h
  src/cdlgselectobid.cpp

  src/cdlgselectdata.ui
  src/cdlgselectdata.h
  src/cdlgselectdata.cpp

  src/cdlgselectdatasize.ui
  src/cdlgselectdatasize.h
  src/cdlgselectdatasize.cpp

  src/cdlgselectdatacoding.ui
  src/cdlgselectdatacoding.h
  src/cdlgselectdatacoding.cpp

  src/cdlgselectmeasurementvalue.ui
  src/cdlgselectmeasurementvalue.h
  src/cdlgselectmeasurementvalue.cpp

  src/cdlgselectpriority.ui
  src/cdlgselectpriority.h
  src/cdlgselectpriority.cpp

  src/cdlgselecttimestamp.ui
  src/cdlgselecttimestamp.h
  src/cdlgselecttimestamp.cpp

  src/cdlgselectunit.ui
  src/cdlgselectunit.h
  src/cdlgselectunit.cpp

  src/cdlgselectsensorindex.ui
  src/cdlgselectsensorindex.h
  src/cdlgselectsensorindex.cpp

  src/cdlgselectdate.ui
  src/cdlgselectdate.h
  src/cdlgselectdate.cpp

  src/cdlgselectconstraint.ui
  src/cdlgselectconstraint.h
  src/cdlgselectconstraint.cpp

  #src/widgetgraph.h
  #src/widgetgraph.cpp 

  #src/xyseries.h
  #src/xyseries.cpp

  src/cdlgdatepicker.ui
  src/cdlgdatepicker.cpp 
  src/cdlgdatepicker.h 

  src/cfrmmdf.ui
  src/cfrmmdf.h
  src/cfrmmdf.cpp

  src/cdlgmdfmodule.ui
  src/cdlgmdfmodule.cpp 
  src/cdlgmdfmodule.h 

  src/cdlgmdfdescription.ui
  src/cdlgmdfdescription.cpp
  src/cdlgmdfdescription.h

  src/cdlgmdfinfourl.ui
  src/cdlgmdfinfourl.cpp
  src/cdlgmdfinfourl.h

  src/cdlgeditmap.ui
  src/cdlgeditmap.cpp
  src/cdlgeditmap.h

  src/cdlgmdfmanufacturer.ui
  src/cdlgmdfmanufacturer.cpp
  src/cdlgmdfmanufacturer.h

  src/cdlgmdfcontact.ui
  src/cdlgmdfcontact.cpp 
  src/cdlgmdfcontact.h

  src/cdlgmdfcontactlist.ui
  src/cdlgmdfcontactlist.cpp 
  src/cdlgmdfcontactlist.h

  src/cdlgmdfbootloader.ui
  src/cdlgmdfbootloader.cpp 
  src/cdlgmdfbootloader.h

  src/cdlgmdffilepicture.ui
  src/cdlgmdffilepicture.cpp 
  src/cdlgmdffilepicture.h

  src/cdlgmdffilevideo.ui
  src/cdlgmdffilevideo.cpp 
  src/cdlgmdffilevideo.h

  src/cdlgmdffilesetup.ui
  src/cdlgmdffilesetup.cpp 
  src/cdlgmdffilesetup.h

  src/cdlgmdffilemanual.ui
  src/cdlgmdffilemanual.cpp
  src/cdlgmdffilemanual.h

  src/cdlgmdffilefirmware.ui
  src/cdlgmdffilefirmware.cpp
  src/cdlgmdffilefirmware.h

  src/cdlgmdffiledriver.ui
  src/cdlgmdffiledriver.cpp
  src/cdlgmdffiledriver.h

  src/cdlgmdffile.ui
  src/cdlgmdffile.cpp
  src/cdlgmdffile.h

  src/cdlgmdfregister.ui
  src/cdlgmdfregister.cpp
  src/cdlgmdfregister.h

  src/cdlgmdfregisterlist.ui
  src/cdlgmdfregisterlist.cpp
  src/cdlgmdfregisterlist.h

  src/cdlgmdfbit.ui
  src/cdlgmdfbit.cpp
  src/cdlgmdfbit.h

  src/cdlgmdfbitlist.ui
  src/cdlgmdfbitlist.cpp
  src/cdlgmdfbitlist.h

  src/cdlgmdfvalue.ui
  src/cdlgmdfvalue.cpp
  src/cdlgmdfvalue.h

  src/cdlgmdfvaluelist.ui
  src/cdlgmdfvaluelist.cpp
  src/cdlgmdfvaluelist.h

  src/cdlgmdfremotevar.ui
  src/cdlgmdfremotevar.cpp
  src/cdlgmdfremotevar.h

  src/cdlgmdfremotevarlist.ui
  src/cdlgmdfremotevarlist.cpp
  src/cdlgmdfremotevarlist.h

  src/cdlgmdfdm.ui
  src/cdlgmdfdm.cpp
  src/cdlgmdfdm.h

  src/cdlgmdfdmaction.ui
  src/cdlgmdfdmaction.cpp
  src/cdlgmdfdmaction.h

  src/cdlgmdfdmactionparam.ui
  src/cdlgmdfdmactionparam.cpp
  src/cdlgmdfdmactionparam.h

  src/cdlgmdfevent.ui
  src/cdlgmdfevent.cpp
  src/cdlgmdfevent.h

  src/cdlgmdfeventdata.ui
  src/cdlgmdfeventdata.cpp
  src/cdlgmdfeventdata.h

  src/cdlgtxtsearch.ui
  src/cdlgtxtsearch.cpp
  src/cdlgtxtsearch.h

  src/cdlgselectmqtttopics.ui
  src/cdlgselectmqtttopics.cpp
  src/cdlgselectmqtttopics.h

  src/bootloaderwizard.h
  src/bootloaderwizard.cpp

  src/cdlgbootfirmware.ui
  src/cdlgbootfirmware.h
  src/cdlgbootfirmware.cpp

  src/cdlgselectbootalgorithm.ui
  src/cdlgselectbootalgorithm.h
  src/cdlgselectbootalgorithm.cpp

  ./third_party/vscp/src/vscp/common/version.h
  ./third_party/vscp/src/vscp/common/vscp.h
  ./third_party/vscp/src/vscp/common/vscpremotetcpif.h
  ./third_party/vscp/src/vscp/common/vscpremotetcpif.cpp
  ./third_party/vscp/src/vscp/common/vscpdatetime.h
  ./third_party/vscp/src/vscp/common/vscpdatetime.cpp
  ./third_party/vscp/src/vscp/common/guid.h
  ./third_party/vscp/src/vscp/common/guid.cpp
  ./third_party/vscp/src/vscp/common/mdf.h
  ./third_party/vscp/src/vscp/common/mdf.cpp
  ./third_party/vscp/src/vscp/common/vscphelper.h
  ./third_party/vscp/src/vscp/common/vscphelper.cpp
  ./third_party/vscp/src/vscp/common/canal.h
  ./third_party/vscp/src/vscp/common/canaldlldef.h
  ./third_party/vscp/src/vscp/common/vscpcanaldeviceif.h
  ./third_party/vscp/src/vscp/common/vscpcanaldeviceif.cpp
  ./third_party/vscp/src/vscp/common/canal-xmlconfig.h
  ./third_party/vscp/src/vscp/common/canal-xmlconfig.cpp
  ./third_party/vscp/src/vscp/common/register.h
  ./third_party/vscp/src/vscp/common/register.cpp
  ./third_party/vscp/src/vscp/common/vscp-client-base.h
  ./third_party/vscp/src/vscp/common/vscp-client-base.cpp
  ./third_party/vscp/src/vscp/common/vscp-client-canal.h
  ./third_party/vscp/src/vscp/common/vscp-client-canal.cpp    
  ./third_party/vscp/src/vscp/common/vscp-client-mqtt.h
  ./third_party/vscp/src/vscp/common/vscp-client-mqtt.cpp
  ./third_party/vscp/src/vscp/common/vscp-client-tcp.h
  ./third_party/vscp/src/vscp/common/vscp-client-tcp.cpp
  ./third_party/vscp/src/vscp/common/vscp-client-socketcan.h
  ./third_party/vscp/src/vscp/common/vscp-client-socketcan.cpp
  ./third_party/vscp/src/vscp/common/vscp-client-ws1.h
  ./third_party/vscp/src/vscp/common/vscp-client-ws1.cpp 
  ./third_party/vscp/src/vscp/common/vscp-client-ws2.h
  ./third_party/vscp/src/vscp/common/vscp-client-ws2.cpp 
  ./third_party/vscp/src/vscp/common/vscp-client-udp.h
  ./third_party/vscp/src/vscp/common/vscp-client-udp.cpp 
  ./third_party/vscp/src/vscp/common/vscp-client-multicast.h
  ./third_party/vscp/src/vscp/common/vscp-client-multicast.cpp 
  ./third_party/vscp/src/vscp/common/vscp-bootdevice.h
  ./third_party/vscp/src/vscp/common/vscp-bootdevice.cpp
  ./third_party/vscp/src/vscp/common/vscp-bootdevice-pic1.h
  ./third_party/vscp/src/vscp/common/vscp-bootdevice-pic1.cpp
  ./third_party/vscp/src/vscp/common/vscp-bootdevice-vscp.h
  ./third_party/vscp/src/vscp/common/vscp-bootdevice-vscp.cpp
  ./third_party/vscp/src/vscp/common/vscpunit.h
  ./third_party/vscp/src/vscp/common/vscpunit.cpp     
  ./third_party/vscp/src/common/sockettcp.h
  ./third_party/vscp/src/common/sockettcp.c
  ./third_party/vscp/src/common/vscpbase64.h
  ./third_party/vscp/src/common/vscpbase64.c
  ./third_party/vscp/src/common/vscp-aes.h
  ./third_party/vscp/src/common/vscp-aes.c
  ./third_party/vscp/src/common/crc.h
  ./third_party/vscp/src/common/crc.c
  ./third_party/vscp/src/common/crc8.h
  ./third_party/vscp/src/common/crc8.c
  ./third_party/vscp/src/common/vscpmd5.h
  ./third_party/vscp/src/common/vscpmd5.c
  ./third_party/mustache/mustache.hpp
  ./third_party/spdlog/include/
  ./third_party/crypto-algorithms/sha256.h
  ./third_party/crypto-algorithms/sha256.c    
  install/${PROJECT_NAME}.desktop
  ${TS_FILES}   
) 

if (MSVC)
  find_path(PTHREAD_INCLUDE_DIR pthread.h)
  find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
  message(STATUS "Include: ${MOSQUITTO_INCLUDE_DIR}")

  include_directories( PUBLIC 
    ./src 
    ./build
    ./ui
    ./third_party/vscp/src/vscp/common/  
    ./third_party/vscp/src/common
    ./third_party
    ./third_party/nlohmann/include/
    ./third_party/spdlog/include/
    ./third_party/Mustache/
    ./third_party/maddy/include/
    ./third_party/crypto-algorithms/
    ${PThreads4W_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${MOSQUITTO_INCLUDE_DIRS}cd bui
    ${LIBWEBSOCKETS_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    ${DLFCN_INCLUDES}
    ${LOCAL_WIN_INCLUDE}
    ${LOCAL_SYSLOG_INCLUDE} 
    ${CURL_INCLUDE_DIR}
  )
else()
  include_directories( PUBLIC 
    ./src 
    ./build
    ./ui
    ./third_party/mqtt
    ./third_party/mqtt/clib
    ./third_party/vscp/src/vscp/common/
    ./third_party/vscp/src/common
    ./third_party
    ./third_party/nlohmann/include/
    ./third_party/spdlog/include/
    ./third_party/mustache/
    ./third_party/maddy/include/
    ./third_party/crypto-algorithms/s
    ${CMAKE_BINARY_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${MOSQUITTO_INCLUDE_DIRS}
    ${LIBWEBSOCKETS_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    ${DLFCN_INCLUDES}
    ${LOCAL_WIN_INCLUDE}
    ${LOCAL_SYSLOG_INCLUDE} 
    ${CURL_INCLUDE_DIR}
  )
 endif()   

if (MSVC) 
  find_path(MOSQUITTO_LIB_DIR mosquitto.lib HINTS "${VCPKG_ROOT}/installed/x64-windows/lib")
  message(STATUS "Mosquitto lib: ${MOSQUITTO_LIB_DIR}  --- ${VCPKG_ROOT}/installed/x64-windows/lib/mosquitto.lib")
  link_directories(${MOSQUITTO_LIB_DIR})

  message(STATUS "lib websockets lib: ${LIBWEBSOCKETS_LIBRARY_DIR}  --- ${VCPKG_ROOT}/installed/x64-windows/lib/${LIBWEBSOCKETS_LIBRARIES}")
  link_directories(${LIBWEBSOCKETS_LIBRARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Qt6::Widgets
  Qt6::Qml
  Qt6::Sql
  #Qt6::Charts    
  PThreads4W::PThreads4W
  OpenSSL::SSL
  OpenSSL::Crypto
  OpenSSL::SSL OpenSSL::Crypto        
  ${MOSQUITTO_LIBRARIES}
  ${LIBWEBSOCKETS_LIBRARIES}
  expat::expat
  CURL::libcurl
  dlfcn-win32::dl
)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE     
  Qt6::Widgets
  Qt6::Qml
  Qt6::Sql
  #Qt6::Charts
  m
  dl
  Threads::Threads
  OpenSSL::SSL 
  OpenSSL::Crypto
  ${OPENSSL_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  ${LIBWEBSOCKETS_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${CURL_LIBRARIES}
  ${DLFCN_LIBS}
)
endif()


if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DWIN32_LEAN_AND_MEANS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
  set(LIBS_SYSTEM c stdc++)
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME}.desktop
  DESTINATION ${INSTALL_PATH}/share/applications/) 

# icons
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/icons/ DESTINATION ${INSTALL_PATH}/share/icons/) 

# Event database etc
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/share/ DESTINATION ${INSTALL_PATH}/share/${PROJECT_NAME}/)
