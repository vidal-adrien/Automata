#-------------------------------------------------
#
# Project created by QtCreator 2016-10-24T18:46:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = automata
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    gamewidget.cpp \
    infodialog.cpp

HEADERS  += mainwindow.h \
    gamewidget.h \
    infodialog.h

FORMS    += mainwindow.ui \
    infodialog.ui

OTHER_FILES += \
    README.md \
    default.ini \
    rulesets.ini \
    head.ico

RESOURCES += \
    resources.qrc
	
win32:RC_ICONS += head.ico

DISTFILES += \
    rootpath.ini
