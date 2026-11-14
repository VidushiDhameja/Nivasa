QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = NIVASA
TEMPLATE = app

SOURCES += \
    main.cpp \
    Property.cpp \
    Seller.cpp \
    Buyer.cpp \
    Admin.cpp \
    PropertyManager.cpp \
    ImageManager.cpp \
    user.cpp \
    Authentication.cpp

HEADERS += \
    Property.h \
    Seller.h \
    Buyer.h \
    Admin.h \
    PropertyManager.h \
    ImageManager.h \
    user.h \
    Authentication.h

FORMS +=

RESOURCES +=

QMAKE_CXXFLAGS += -std=c++17
