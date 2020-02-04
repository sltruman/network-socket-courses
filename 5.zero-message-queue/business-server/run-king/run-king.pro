#-------------------------------------------------
#
# Project created by QtCreator 2019-12-27T20:28:14
#
#-------------------------------------------------

QT       += core gui network opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = run-king
TEMPLATE = app
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

SOURCES += \
        json/json_reader.cpp \
        json/json_value.cpp \
        json/json_writer.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        json/autolink.h \
        json/config.h \
        json/features.h \
        json/forwards.h \
        json/json.h \
        json/json_batchallocator.h \
        json/reader.h \
        json/value.h \
        json/writer.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

unix:!mac:LIBS += -lzmq -lxml2 -lOpenThreads -losg -losgDB -losgGA -losgUtil -losgViewer -losgManipulator -losgAnimation -losgShadow -losgText
win32:LIBS     += -lAdvapi32 -L$$PWD/../OpenSceneGraph/build-win-vs2017x64/lib -lOpenThreads -losg -losgDB -losgGA -losgUtil -losgViewer -losgManipulator -losgAnimation -losgShadow -losgText

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../three-widget
DEPENDPATH += $$PWD/../three-widget

unix:!macx: LIBS += -L$$PWD/../build-three-widget-Qt5_x64-Debug/ -lthree-widget
