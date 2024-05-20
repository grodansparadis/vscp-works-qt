#-------------------------------------------------
#
# Project created by QtCreator 2021-02-18T17:35:00
#
#-------------------------------------------------

QT = core gui printsupport qml serialbus serialport widgets help network

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT OPENSSL_API_1_1



CONFIG += c++14
CONFIG += NO_UNIT_TESTS

DEFINES += QCUSTOMPLOT_USE_OPENGL
DEFINES += OPENSSL_API_1_1
DEFINES += USE_WEBSOCKET

TARGET = vscpworks
TEMPLATE = app

SOURCES +=  src/main.cpp \
    src/mainwindow.cpp \
    src/filedownloader.cpp \
    src/cdlgnewconnection.cpp \
    src/cdlgmainsettings.cpp \
    src/cdlgconnsettingslocal.cpp \
    src/cdlgconnsettingscanal.cpp \
    src/cdlgconnsettingstcpip.cpp \
    src/cdlgconnsettingssocketcan.cpp \
    src/cdlgsocketcanflags.cpp \
    src/cdlgconnsettingsmqtt.cpp \
    src/cdlgmqttpublish.cpp \
    src/cdlgconnsettingsws1.cpp \
    src/cdlgconnsettingsws2.cpp \
    src/cdlgconnsettingsudp.cpp \
    src/cdlgconnsettingsmulticast.cpp \
    src/cdlgconnsettingsrest.cpp \
    src/cdlglevel1filter.cpp \
    src/cdlglevel1filterwizard.cpp \
    src/cdlglevel2filter.cpp \
    src/cdlgcanfilter.cpp \
    src/cdlgtls.cpp \
    src/cfrmsession.cpp \
    src/vscpworks.cpp \
    src/canalconfigwizard.cpp \
    vscp/src/vscp/common/vscpremotetcpif.cpp \
    vscp/src/vscp/common/vscpdatetime.cpp \
    vscp/src/vscp/common/guid.cpp \
    vscp/src/vscp/common/mdf.cpp \
    vscp/src/vscp/common/vscphelper.cpp \
    vscp/src/vscp/common/vscpcanaldeviceif.cpp \
    vscp/src/vscp/common/canal_xmlconfig.cpp \
    vscp/src/vscp/common/vscp-client-base.cpp \
    vscp/src/vscp/common/vscp-client-local.cpp \
    vscp/src/vscp/common/vscp-client-canal.cpp \  
    vscp/src/vscp/common/vscp-client-mqtt.cpp \
    vscp/src/vscp/common/vscp-client-tcp.cpp \
    vscp/src/vscp/common/vscp-client-socketcan.cpp \
    vscp/src/vscp/common/vscp-client-ws1.cpp \
    vscp/src/vscp/common/vscp-client-ws2.cpp \
    vscp/src/vscp/common/vscp-client-udp.cpp \
    vscp/src/vscp/common/vscp-client-multicast.cpp \
    vscp/src/vscp/common/vscp-client-rest.cpp \
    vscp/src/vscp/common/vscp-client-rawcan.cpp \
    vscp/src/vscp/common/vscp-client-rawmqtt.cpp \
    vscp/src/vscp/common/vscp_bootloader.cpp \
    vscp/src/common/third_party/civetweb-1.13/src/civetweb.c \ 
    vscp/src/common/sockettcp.c \
    vscp/src/common/vscpbase64.c \
    vscp/src/common/vscp_aes.c \
    vscp/src/common/crc.c \
    vscp/src/common/crc8.c \
    vscp/src/common/vscpmd5.c \
    vscp/src/common/fastpbkdf2.c    

HEADERS  += src/vscpworks.h \
    src/mainwindow.h \
    src/filedownloader.h \
    src/cdlgnewconnection.h \
    src/cdlgmainsettings.h \
    src/cdlgconnsettingslocal.h \
    src/cdlgconnsettingscanal.h \
    src/cdlgconnsettingstcpip.h \
    src/cdlgconnsettingssocketcan.h \
    src/cdlgsocketcanflags.h \
    src/cdlgconnsettingsmqtt.h \
    src/cdlgmqttpublish.h \
    src/cdlgconnsettingsws1.h \
    src/cdlgconnsettingsws2.h \
    src/cdlgconnsettingsudp.h \
    src/cdlgconnsettingsmulticast.h \
    src/cdlgconnsettingsrawcan.h \
    src/cdlgconnsettingsrawmqtt.h \
    src/cdlgconnsettingsrest.h \
    src/cdlglevel1filter.h \
    src/cdlglevel1filterwizard.h \
    src/cdlglevel2filter.h \
    src/cdlgcanfilter.h \
    src/cdlgtls.h \
    src/cfrmsession.h \
    src/canalconfigwizard.h \
    vscp/src/vscp/common/version.h \
    vscp/src/vscp/common/vscp.h \
    vscp/src/vscp/common/vscpremotetcpif.h \
    vscp/src/vscp/common/vscpdatetime.h \
    vscp/src/vscp/common/guid.h \
    vscp/src/vscp/common/mdf.h \
    vscp/src/vscp/common/vscphelper.h \
    vscp/src/vscp/common/canal.h \
    vscp/src/vscp/common/canaldlldef.h \
    vscp/src/vscp/common/vscpcanaldeviceif.h \
    vscp/src/vscp/common/canal_xmlconfig.h \
    vscp/src/vscp/common/vscp-client-base.h \
    vscp/src/vscp/common/vscp-client-local.h \
    vscp/src/vscp/common/vscp-client-canal.h \
    vscp/src/vscp/common/vscp-client-mqtt.h \
    vscp/src/vscp/common/vscp-client-tcp.h \
    vscp/src/vscp/common/vscp-client-socketcan.h \
    vscp/src/vscp/common/vscp-client-ws1.h \
    vscp/src/vscp/common/vscp-client-ws2.h \
    vscp/src/vscp/common/vscp-client-udp.h \
    vscp/src/vscp/common/vscp-client-multicast.h \
    vscp/src/vscp/common/vscp-client-rest.h \
    vscp/src/vscp/common/vscp-client-rawcan.h \
    vscp/src/vscp/common/vscp-client-rawmqtt.h \
    vscp/src/vscp/common/vscp_bootloader.h \
    vscp/src/common/third_party/mustache.hpp \
    vscp/src/common/third_party/civetweb-1.13/include/civetweb.h \
    vscp/src/common/sockettcp.h \
    vscp/src/common/vscpbase64.h \
    vscp/src/common/vscp_aes.h \
    vscp/src/common/crc.h \
    vscp/src/common/crc8.h \
    vscp/src/common/vscpmd5.h \
    vscp/src/common/fastpbkdf2.h

FORMS += src/mainwindow.ui \
    src/cdlgmainsettings.ui \
    src/cdlgconnsettingslocal.ui \
    src/cdlgconnsettingscanal.ui \
    src/cdlgconnsettingstcpip.ui \
    src/cdlgconnsettingssocketcan.ui \
    src/cdlgconnsettingsmqtt.ui \
    src/cdlgconnsettingsws1.ui \
    src/cdlgconnsettingsws2.ui \
    src/cdlgconnsettingsudp.ui \
    src/cdlgconnsettingsmulticast.ui \
    src/cdlgconnsettingsrest.ui \
    src/cdlgconnsettingsrawcan.ui \
    src/cdlgconnsettingsrawmqtt.ui \
    src/cfrmsession.ui \
    src/cdlgnewconnection.ui \
    src/cdlglevel1filter.ui \
    src/cdlglevel1filterwizard.ui \
    src/cdlglevel2filter.ui \
    src/cdlgcanfilter.ui \
    src/cdlgsocketcanflags.ui \
    src/cdlgtls.ui \
    src/cdlgmqttpublish.ui

INCLUDEPATH += ./src \
    ./build \
    vscp/src/vscp/common/ \
    vscp/src/common \
    vscp/src/common/third_party \
    vscp/src/common/third_party/nlohmann \
    vscp/src/common/third_party/civetweb-1.13/include
    
RESOURCES += \
    src/vscpworks.qrc


win32 {
LIBS += -lws2_32 -lComdlg32 -lUser32 -lShell32 -lAdvapi32 -lopengl32.lib
} else {
LIBS += -lexpat -lssl -lz -lrt -lm -lcrypto -lpthread -ldl
}

unix {
   isEmpty(PREFIX)
   {
      PREFIX=/usr/local
   }
   target.path = $$PREFIX/bin
   shortcutfiles.files=vscp-works-qt.desktop
   shortcutfiles.path = $$PREFIX/share/applications
   INSTALLS += shortcutfiles
   DISTFILES += vscp-works-qt.desktop
}

examplefiles.files=examples
examplefiles.path = $$PREFIX/share/vscpworks/examples
INSTALLS += examplefiles

iconfiles.files=icons
iconfiles.path = $$PREFIX/share/icons
INSTALLS += iconfiles

helpfiles.files=help/*
helpfiles.path = $$PREFIX/bin/help
INSTALLS += helpfiles

INSTALLS += target


unix:!macx: LIBS += -L$$PWD/../../../../../lib/x86_64-linux-gnu/ -lmosquitto

INCLUDEPATH += $$PWD/../../../../../lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../lib/x86_64-linux-gnu

unix:!macx: LIBS += -L$$PWD/../../../../../lib/x86_64-linux-gnu/ -lexpat

INCLUDEPATH += $$PWD/../../../../../lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../lib/x86_64-linux-gnu

unix:!macx: LIBS += -lssl

unix:!macx: LIBS += -lcrypt

unix:!macx: LIBS += -lpthread
