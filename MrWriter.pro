#-------------------------------------------------
#
# Project created by QtCreator 2015-06-05T20:13:52
#
#-------------------------------------------------

QT       += core gui
QT       += xml
QT       += printsupport
QT       += concurrent
QT       += svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MrWriter
TEMPLATE = app

HEADERS  += version.h \
    pagesettingsdialog.h \
    colorbutton.h

VERSION_MAJOR = MY_MAJOR_VERSION
VERSION_MINOR = MY_MINOR_VERSION

SOURCES += main.cpp\
        mainwindow.cpp \
    widget.cpp \
    curve.cpp \
    document.cpp \
    page.cpp \
    qcompressor.cpp \
    selection.cpp \
    commands.cpp \
    tabletapplication.cpp \
    pagesettingsdialog.cpp \
    colorbutton.cpp

HEADERS  += mainwindow.h \
    widget.h \
    curve.h \
    document.h \
    page.h \
    qcompressor.h \
    selection.h \
    commands.h \
    tictoc.h \
    tabletapplication.h \
    version.h

FORMS    +=

RESOURCES += \
    myresource.qrc

LIBS += -lz

ICON = MyIcon.icns

RC_ICONS = MyIcon.ico

DISTFILES += \
    images/openIcon.png \
    images/newIcon.png \
    Info.plist

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

QMAKE_CXXFLAGS_RELEASE *= -O3

QMAKE_INFO_PLIST = Info.plist

