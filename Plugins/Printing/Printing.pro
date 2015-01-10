#-------------------------------------------------
#
# Project created by QtCreator 2014-08-03T20:25:45
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += printsupport

TARGET = Printing
TEMPLATE = lib

DEFINES += PRINTING_LIBRARY

SOURCES += printing.cpp \
    printingexport.cpp

HEADERS += printing.h\
        printing_global.h \
    printingexport.h

OTHER_FILES += \
    printing.json

INCLUDEPATH += $$PLUGINSDIR/PdfExport
DEPENDPATH += $$PLUGINSDIR/PdfExport

win32|macx: {
#    LIBS += -lPdfExport
    pluginDep(PdfExport)
}

RESOURCES += \
    printing.qrc


TRANSLATIONS += Printing_pl.ts \
		Printing_en.ts

