TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginamazondash)

LIBS += -lpcap

SOURCES += \
    amazondashplugin.cpp

HEADERS += \
    amazondashplugin.h
