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

# TODO Check SLL version and set variable accordingly
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(vscp-works-qt
    src/vscpworks.qrc
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/cdlgnewconnection.h
    src/cdlgnewconnection.cpp
    src/cfrmsession.cpp
    src/connection.h
    src/connection.cpp
    src/vscpworks.h
    src/vscpworks.cpp
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
    vscp/src/vscp/common/vscp_client_base.h
    vscp/src/vscp/common/vscp_client_base.cpp
    vscp/src/vscp/common/vscp_client_mqtt.h
    vscp/src/vscp/common/vscp_client_mqtt.cpp
    vscp/src/vscp/common/vscp_client_canal.h
    vscp/src/vscp/common/vscp_client_canal.cpp
    vscp/src/vscp/common/vscp_client_tcp.h
    vscp/src/vscp/common/vscp_client_tcp.cpp 
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

include_directories( PUBLIC ./src 
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
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${MOSQUITTO_LIBRARIES}
    ${EXPAT_LIBRARIES})

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
qt5_add_resources(src/vscpworks.qrc) 

message(" Qt5Widgets_INCLUDE_DIRS =${Qt5Widgets_INCLUDE_DIRS}     ")
message(" Qt5Widgets_LIBRARIES =${Qt5Widgets_LIBRARIES}   ")

message(" Qt5Core_INCLUDE_DIRS =${Qt5Core_INCLUDE_DIRS}   ")
message(" Qt5Core_LIBRARIES =${Qt5Core_LIBRARIES}         ")

message(" Qt5SerialPort_INCLUDE_DIRS =${Qt5SerialPort_INCLUDE_DIRS}")
message(" Qt5SerialPort_LIBRARIES =${Qt5SerialPort_LIBRARIES}")