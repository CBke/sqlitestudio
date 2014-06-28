#-------------------------------------------------
#
# Project created by QtCreator 2014-06-20T22:57:37
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = HtmlExport
TEMPLATE = lib

DEFINES += HTMLEXPORT_LIBRARY

SOURCES += htmlexport.cpp

HEADERS += htmlexport.h\
        htmlexport_global.h

OTHER_FILES += \
    htmlexport.json \
    htmlexport.css

RESOURCES += \
    htmlexport.qrc

FORMS += \
    htmlexport.ui
