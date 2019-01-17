#-------------------------------------------------
#
# Project created by QtCreator 2015-06-05T20:13:52
#
#-------------------------------------------------

#QMAKE_POST_LINK=make_doc.sh

QT       += core gui
QT       += xml
QT       += printsupport
QT       += concurrent
#QT       += svg
#QT       += webenginewidgets

system(touch src/version.h)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MrWriter
TEMPLATE = app

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target.path = $$PREFIX/bin

    INSTALLS += target
}

#VERSION_MAJOR = MY_MAJOR_VERSION
#VERSION_MINOR = MY_MINOR_VERSION

SOURCES += \
    src/main.cpp\
    src/mainwindow.cpp \
    src/widget.cpp \
    src/document.cpp \
    src/page.cpp \
    src/qcompressor.cpp \
    src/selection.cpp \
    src/commands.cpp \
    src/tabletapplication.cpp \
    src/pagesettingsdialog.cpp \
    src/colorbutton.cpp \
    src/stroke.cpp \
    src/tools.cpp \
    src/element.cpp

HEADERS  += \
    src/pagesettingsdialog.h \
    src/colorbutton.h \
    src/stroke.h \
    src/mrdoc.h \
    src/mainwindow.h \
    src/widget.h \
    src/document.h \
    src/page.h \
    src/qcompressor.h \
    src/selection.h \
    src/commands.h \
    src/tictoc.h \
    src/tabletapplication.h \
    src/version.h \
    src/tools.h \
    src/element.h

FORMS    +=

RESOURCES += \
    myresource.qrc

unix {
    LIBS += -lz
}
win32 {
    LIBS += -lzlib
}
win64 {
    LIBS += -lzlib
}

ICON = MrWriter.icns

RC_ICONS = MrWriter.ico

DISTFILES += \
    images/openIcon.png \
    images/newIcon.png \
    Info.plist \
    COPYING \
    MrWriter.icns

CONFIG += c++17

#QMAKE_CXXFLAGS_RELEASE -= -O
#QMAKE_CXXFLAGS_RELEASE -= -O1
#QMAKE_CXXFLAGS_RELEASE -= -O2

#QMAKE_CXXFLAGS_RELEASE *= -O3

QMAKE_INFO_PLIST = Info.plist

