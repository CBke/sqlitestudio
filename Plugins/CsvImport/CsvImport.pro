#-------------------------------------------------
#
# Project created by QtCreator 2014-05-17T20:38:58
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT       -= gui

TARGET = CsvImport
TEMPLATE = lib

DEFINES += CSVIMPORT_LIBRARY

SOURCES += csvimport.cpp

HEADERS += csvimport.h\
        csvimport_global.h

FORMS += \
    CsvImportOptions.ui

OTHER_FILES += \
    csvimport.json

RESOURCES += \
    csvimport.qrc
