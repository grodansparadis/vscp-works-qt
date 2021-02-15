cmake_minimum_required(VERSION 3.5)

project(vscp-works-qt LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED Core Network Quick Sql Qml SerialPort)

find_package(OpenSSL REQUIRED)

set(TS_FILES vscp-works-qt_en_150.ts)

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

include(FindMosquitto REQUIRED)
find_package(EXPAT REQUIRED)

# DELETE Check SLL version and set variable accordingly
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
#set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# Set civetweb build flags
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUSE_IPV6 -DUSE_WEBSOCKET")
string(FIND "${OPENSSL_VERSION}" "1.1." SSL_VER_POS)
if(${SSL_VER_POS})
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_0")
else()
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")
endif()

# set(UI )
qt5_wrap_ui(UI src/mainwindow.ui
    src/cdlgmainsettings.ui
    src/cdlgconnsettingslocal.ui
    src/cdlgconnsettingscanal.ui
    src/cdlgconnsettingstcpip.ui
    src/cdlgconnsettingssocketcan.ui
    src/cdlgconnsettingsmqtt.ui
    src/cdlgconnsettingsws1.ui
    src/cdlgconnsettingsws2.ui
    src/cdlgconnsettingsudp.ui
    src/cdlgconnsettingsmulticast.ui
    src/cdlgconnsettingsrest.ui
    src/cdlgconnsettingsrawcan.ui
    src/cdlgconnsettingsrawmqtt.ui
    src/cfrmsession.ui
    src/cdlglevel1filter.ui
    src/cdlglevel1filterwizard.ui
    src/cdlglevel2filter.ui
    src/cdlgcanfilter.ui
    src/cdlgsocketcanflags.ui
    src/cdlgtls.ui
)

add_executable(vscp-works-qt
    src/vscpworks.qrc
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/filedownloader.h
    src/filedownloader.cpp    
    src/cdlgnewconnection.h
    src/cdlgnewconnection.cpp
    build/ui_cdlgmainsettings.h
    src/cdlgmainsettings.h
    src/cdlgmainsettings.cpp

    build/ui_cdlgconnsettingslocal.h
    src/cdlgconnsettingslocal.h
    src/cdlgconnsettingslocal.cpp

    build/ui_cdlgconnsettingscanal.h
    src/cdlgconnsettingscanal.h
    src/cdlgconnsettingscanal.cpp

    build/ui_cdlgconnsettingstcpip.h
    src/cdlgconnsettingstcpip.h
    src/cdlgconnsettingstcpip.cpp

    build/ui_cdlgconnsettingssocketcan.h
    src/cdlgconnsettingssocketcan.h
    src/cdlgconnsettingssocketcan.cpp

    build/ui_cdlgsocketcanflags.h
    src/cdlgsocketcanflags.h
    src/cdlgsocketcanflags.cpp

    build/ui_cdlgconnsettingsmqtt.h
    src/cdlgconnsettingsmqtt.h
    src/cdlgconnsettingsmqtt.cpp

    build/ui_cdlgconnsettingsws1.h
    src/cdlgconnsettingsws1.h
    src/cdlgconnsettingsws1.cpp

    build/ui_cdlgconnsettingsws2.h
    src/cdlgconnsettingsws2.h
    src/cdlgconnsettingsws2.cpp

    build/ui_cdlgconnsettingsudp.h
    src/cdlgconnsettingsudp.h
    src/cdlgconnsettingsudp.cpp

    build/ui_cdlgconnsettingsmulticast.h
    src/cdlgconnsettingsmulticast.h
    src/cdlgconnsettingsmulticast.cpp

    build/ui_cdlgconnsettingsrawcan.h
    src/cdlgconnsettingsrawcan.h
    src/cdlgconnsettingsrawcan.cpp

    build/ui_cdlgconnsettingsrawmqtt.h
    src/cdlgconnsettingsrawmqtt.h
    src/cdlgconnsettingsrawmqtt.cpp

    build/ui_cdlgconnsettingsrest.h
    src/cdlgconnsettingsrest.h
    src/cdlgconnsettingsrest.cpp

    build/ui_cfrmsession.h
    
    src/cdlglevel1filter.h
    src/cdlglevel1filter.cpp
    build/ui_cdlglevel1filter.h

    src/cdlglevel1filterwizard.h
    src/cdlglevel1filterwizard.cpp
    build/ui_cdlglevel1filterwizard.h

    src/cdlglevel1filter.h
    src/cdlglevel1filter.cpp
    build/ui_cdlglevel1filter.h

    src/cdlgcanfilter.h
    src/cdlgcanfilter.cpp
    src/cdlgcanfilter.ui

    src/cdlgtls.h
    src/cdlgtls.cpp
    src/cdlgtls.ui

    src/cfrmsession.h
    src/cfrmsession.cpp
    src/vscpworks.h
    src/vscpworks.cpp
    src/canalconfigwizard.h
    src/canalconfigwizard.cpp
    vscp/src/vscp/common/version.h
    vscp/src/vscp/common/vscp.h
    vscp/src/vscp/common/vscpremotetcpif.h
    vscp/src/vscp/common/vscpremotetcpif.cpp
    vscp/src/vscp/common/vscpdatetime.h
    vscp/src/vscp/common/vscpdatetime.cpp
    vscp/src/vscp/common/guid.h
    vscp/src/vscp/common/guid.cpp
    vscp/src/vscp/common/mdf.h
    vscp/src/vscp/common/mdf.cpp
    vscp/src/vscp/common/vscphelper.h
    vscp/src/vscp/common/vscphelper.cpp
    vscp/src/vscp/common/canal.h
    vscp/src/vscp/common/canaldlldef.h
    vscp/src/vscp/common/vscpcanaldeviceif.h
    vscp/src/vscp/common/vscpcanaldeviceif.cpp
    vscp/src/vscp/common/canal_xmlconfig.h
    vscp/src/vscp/common/canal_xmlconfig.cpp
    vscp/src/vscp/common/vscp_client_base.h
    vscp/src/vscp/common/vscp_client_base.cpp
    vscp/src/vscp/common/vscp_client_local.h
    vscp/src/vscp/common/vscp_client_local.cpp
    vscp/src/vscp/common/vscp_client_canal.h
    vscp/src/vscp/common/vscp_client_canal.cpp    
    vscp/src/vscp/common/vscp_client_mqtt.h
    vscp/src/vscp/common/vscp_client_mqtt.cpp
    vscp/src/vscp/common/vscp_client_tcp.h
    vscp/src/vscp/common/vscp_client_tcp.cpp
    vscp/src/vscp/common/vscp_client_socketcan.h
    vscp/src/vscp/common/vscp_client_socketcan.cpp
    vscp/src/vscp/common/vscp_client_ws1.h
    vscp/src/vscp/common/vscp_client_ws1.cpp 
    vscp/src/vscp/common/vscp_client_ws2.h
    vscp/src/vscp/common/vscp_client_ws2.cpp 
    vscp/src/vscp/common/vscp_client_udp.h
    vscp/src/vscp/common/vscp_client_udp.cpp 
    vscp/src/vscp/common/vscp_client_multicast.h
    vscp/src/vscp/common/vscp_client_multicast.cpp 
    vscp/src/vscp/common/vscp_client_rest.h
    vscp/src/vscp/common/vscp_client_rest.cpp
    vscp/src/vscp/common/vscp_client_rawcan.h
    vscp/src/vscp/common/vscp_client_rawcan.cpp
    vscp/src/vscp/common/vscp_client_rawmqtt.h
    vscp/src/vscp/common/vscp_client_rawmqtt.cpp
    vscp/src/vscp/common/vscp_bootloader.h
    vscp/src/vscp/common/vscp_bootloader.cpp
    vscp/src/common/third_party/mustache.hpp
    vscp/src/common/third_party/civetweb-1.13/include/civetweb.h
    vscp/src/common/third_party/civetweb-1.13/src/civetweb.c
    vscp/src/common/sockettcp.h
    vscp/src/common/sockettcp.c
    vscp/src/common/vscpbase64.h
    vscp/src/common/vscpbase64.c
    vscp/src/common/vscp_aes.h
    vscp/src/common/vscp_aes.c
    vscp/src/common/crc.h
    vscp/src/common/crc.c
    vscp/src/common/crc8.h
    vscp/src/common/crc8.c
    vscp/src/common/vscpmd5.h
    vscp/src/common/vscpmd5.c
    vscp/src/common/fastpbkdf2.h
    vscp/src/common/fastpbkdf2.c    
    ${TS_FILES}
  ) 

include_directories( PUBLIC ./src ./build
    vscp/src/vscp/common/
    vscp/src/common
    vscp/src/common/third_party
    vscp/src/common/third_party/nlohmann
    vscp/src/common/third_party/civetweb-1.13/include
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5SerialBus_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${MOSQUITTO_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS} )

target_link_libraries(vscp-works-qt PRIVATE 
    Qt5::Widgets
    Qt5::Core 
    Qt5::Network
    Qt5::Quick 
    Qt5::Sql 
    Qt5::SerialPort
    m
    dl
    pthread
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${MOSQUITTO_LIBRARIES}
    ${EXPAT_LIBRARIES})

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
qt5_add_resources(src/vscpworks.qrc) 

#message(" Qt5Widgets_INCLUDE_DIRS =${Qt5Widgets_INCLUDE_DIRS}     ")
#message(" Qt5Widgets_LIBRARIES =${Qt5Widgets_LIBRARIES}   ")

#message(" Qt5Core_INCLUDE_DIRS =${Qt5Core_INCLUDE_DIRS}   ")
#message(" Qt5Core_LIBRARIES =${Qt5Core_LIBRARIES}         ")

#message(" Qt5SerialPort_INCLUDE_DIRS =${Qt5SerialPort_INCLUDE_DIRS}")
#message(" Qt5SerialPort_LIBRARIES =${Qt5SerialPort_LIBRARIES}")