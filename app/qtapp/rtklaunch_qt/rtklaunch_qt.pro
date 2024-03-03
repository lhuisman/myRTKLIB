QT += widgets core gui

include(../../../RTKLib.pri)

TEMPLATE = app
TARGET = rtklaunch_qt
target.path = $$INSTALLROOT/bin
INSTALLS += target

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

CONFIG += c++11 debug

desktop.path   = $$INSTALLROOT/share/applications/
desktop.files += rtklaunch_qt.desktop
INSTALLS      += desktop

icons.path   = $$INSTALLROOT/share/pixmaps/
icons.files += ../icon/rtklaunch.png
INSTALLS    += icons
