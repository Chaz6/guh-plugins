TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginmodbuscommander)

QT += serialport

LIBS += -lmodbus

SOURCES += \
    devicepluginmodbuscommander.cpp \
    modbusrtuclient.cpp \
    modbustcpclient.cpp

HEADERS += \
    devicepluginmodbuscommander.h \
    modbusrtuclient.h \
    modbustcpclient.h
