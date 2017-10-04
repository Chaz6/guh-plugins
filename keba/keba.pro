TRANSLATIONS =  translations/en_US.ts \
                translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../plugins.pri)

TARGET = guh_devicepluginkeba

SOURCES += \
    devicepluginkeba.cpp \

HEADERS += \
    devicepluginkeba.h \
