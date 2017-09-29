TRANSLATIONS =  translations/en_US.ts \
                translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(plugins.pri)

QT += serialport

TARGET = $$qtLibraryTarget(guh_devicepluginserialportcommander)

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")


SOURCES += \
    devicepluginserialportcommander.cpp \


HEADERS += \
    devicepluginserialportcommander.h \
