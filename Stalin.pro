TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L/usr/local/lib64 -lraylib -ldl -lpthread

SOURCES += \
        main.cpp
