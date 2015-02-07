#-------------------------------------------------
#
# Project created by QtCreator 2014-09-15T15:02:05
#
#-------------------------------------------------

QT += widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = ConfigMigration
TEMPLATE = lib

DEFINES += CONFIGMIGRATION_LIBRARY

SOURCES += configmigration.cpp \
    configmigrationwizard.cpp

HEADERS += configmigration.h\
        configmigration_global.h \
    configmigrationwizard.h \
    configmigrationitem.h

OTHER_FILES += \
    configmigration.json

FORMS += \
    configmigrationwizard.ui

RESOURCES += \
    configmigration.qrc


TRANSLATIONS += ConfigMigration_sk.ts \
		ConfigMigration_de.ts \
		ConfigMigration_ru.ts \
		ConfigMigration_pt_BR.ts \
		ConfigMigration_fr.ts \
		ConfigMigration_es.ts \
		ConfigMigration_pl.ts











