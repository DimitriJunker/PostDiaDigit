#-------------------------------------------------
#
# Project created by QtCreator 2021-05-19T23:34:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PostDiaDigit
TRANSLATIONS =pdd_fr.ts\
    pdd_en.ts\
    pdd_xx.ts
TEMPLATE = app
LIBS += -lQt5Concurrent


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
        DiaDubl.cpp \
        DiasCut.cpp \
        cpddcopy.cpp \
        dias_rename.cpp \
        diasopt.cpp \
        main.cpp \
        postdiadigit.cpp \
        rahmen.cpp

HEADERS += \
        DiaDubl.h \
        DiasCut.h \
        cpddcopy.h \
        dias_rename.h \
        diasopt.h \
        postdiadigit.h \
        rahmen.h

FORMS += \
        DiaDubl.ui \
        DiasCut.ui \
        cpddcopy.ui \
        dias_rename.ui \
        diasopt.ui \
        postdiadigit.ui \
        rahmen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
