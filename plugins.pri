TEMPLATE = lib
CONFIG += plugin

QT += network bluetooth dbus

QMAKE_CXXFLAGS += -Werror -std=c++11 -g
QMAKE_LFLAGS += -std=c++11

INCLUDEPATH += /usr/include/guh
LIBS += -lguh
HEADERS += $${OUT_PWD}/plugininfo.h


PLUGIN_PATH=/usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')/guh/plugins/

# Check if this is a snap build
snappy{
    INCLUDEPATH+=$$(SNAPCRAFT_STAGE)/usr/include/guh
}

# Create plugininfo file
plugininfo.commands = guh-generateplugininfo --filetype i --jsonfile $$PWD/$${TARGET}/deviceplugin"$$TARGET".json --output plugininfo.h --builddir $$OUT_PWD; \
                      guh-generateplugininfo --filetype e --jsonfile $$PWD/$${TARGET}/deviceplugin"$$TARGET".json --output extern-plugininfo.h --builddir $$OUT_PWD;
plugininfo.depends = FORCE
QMAKE_EXTRA_TARGETS += plugininfo
PRE_TARGETDEPS += plugininfo

# Install translation files
TRANSLATIONS *= $$files($${PWD}/$${TARGET}/translations/*ts, true)
lupdate.depends = FORCE
lupdate.depends += plugininfo
lupdate.commands = lupdate -recursive -no-obsolete $$PWD/"$$TARGET"/"$$TARGET".pro;
QMAKE_EXTRA_TARGETS += lupdate

translations.path = /usr/share/guh/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm

# Install plugin
target.path = $$PLUGIN_PATH
target.depends += plugininfo
INSTALLS += target translations

