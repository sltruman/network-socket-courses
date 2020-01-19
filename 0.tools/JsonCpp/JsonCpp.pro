#-------------------------------------------------
#
# Project created by QtCreator 2019-07-31T15:59:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = JsonCpp
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    json/json_writer.cpp \
    json/json_valueiterator.inl \
    json/json_value.cpp \
    json/json_reader.cpp \
    json/json_internalmap.inl \
    json/json_internalarray.inl

HEADERS += \
    json/writer.h \
    json/value.h \
    json/reader.h \
    json/json_batchallocator.h \
    json/json.h \
    json/forwards.h \
    json/features.h \
    json/config.h \
    json/autolink.h

unix { \
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    sconscript \
    json/sconscript
