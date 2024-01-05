#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui

TARGET = rtklaunch_qt
TEMPLATE = app

INCLUDEPATH += ../../../src/

SOURCES += \
    launchmain.cpp \
    main.cpp \
    launchoptdlg.cpp

HEADERS  += \
    launchmain.h \
    launchoptdlg.h

FORMS    += \
    launchmain.ui \
    launchoptdlg.ui

RESOURCES += \
    ../appcmn_qt/appcmn_qt.qrc \
    ../icon/resources.qrc

CONFIG += c++11
