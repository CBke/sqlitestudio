#-------------------------------------------------
#
# Project created by QtCreator 2013-04-05T00:10:49
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += testlib
QT       -= gui

TARGET = tst_selectresolvertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_selectresolvertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
