#-------------------------------------------------
#
# Project created by QtCreator 2011-10-14T14:02:24
#
#-------------------------------------------------

QT       += core gui

TARGET = Seebeck
TEMPLATE = app

LIBS += -L../QSCPIDev -lQSCPIDev \
    -L../msdptool/build -lmsdptool

SOURCES += main.cpp\
        mainwindow.cpp \
    experiment.cpp \
    config.cpp \
    configui.cpp

HEADERS  += mainwindow.h \
    experiment.h \
    config.h \
    configui.h

FORMS    += mainwindow.ui \
    configui.ui
