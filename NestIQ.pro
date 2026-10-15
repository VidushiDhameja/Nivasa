QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = NestIQ
TEMPLATE = app

SOURCES += \
    main.cpp \
    Property.cpp \
    Seller.cpp \
    Buyer.cpp

HEADERS += \
    Property.h \
    Seller.h \
    Buyer.h

FORMS +=

RESOURCES +=

QMAKE_CXXFLAGS += -std=c++17
