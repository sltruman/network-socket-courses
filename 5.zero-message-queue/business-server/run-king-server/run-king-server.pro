TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

HEADERS += \
    server-of-asio.h \
    server-of-basic.h \
    server-of-epoll.h \
    server-of-poll.h \
    server-of-select.h

LIBS += -lpthread -lzmq
