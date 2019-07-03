#-------------------------------------------------
#
# Project created by QtCreator 2019-05-15T12:24:59
#
#-------------------------------------------------

QT += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZabbixDesktopClient
TEMPLATE = app
win32: RC_ICONS = $$PWD/ZabbixIco.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11


CONFIG(release, debug|release) {
    DESTDIR = $$OUT_PWD/../../ClientRelease
}

CONFIG(release, debug|release) {
    QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../ClientRelease
}

MOC_DIR = ../common/build/moc
RCC_DIR = ../common/build/rcc
UI_DIR = ../common/build/ui
OBJECTS_DIR = ../common/build/o

SOURCES += \
        json.cpp \
        main.cpp \
        mainwindow.cpp \
        settingwindow.cpp

HEADERS += \
        json.h \
        mainwindow.h \
        settingwindow.h

FORMS += \
        mainwindow.ui \
        settingwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rec.qrc



