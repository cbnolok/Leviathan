#-------------------------------------------------
#
# Project created by QtCreator 2016-08-03T20:33:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Leviathan
TEMPLATE = app


SOURCES += \
    main.cpp\
    spherescript/scriptparser.cpp \
    uofiles/uoclientcom.cpp \
    spherescript/scriptobjects.cpp \
    forms/mainwindow.cpp \
    globals.cpp \
    spherescript/scriptutils.cpp \
    common.cpp \
    forms/maintab_chars.cpp \
    forms/maintab_log.cpp \
    forms/maintab_items.cpp \
    forms/dlg_parseprogress.cpp \
    qtutils/delayedexecutiontimer.cpp \
    qtutils/checkableproxymodel.cpp \
    forms/dlg_profilescripts_options.cpp \
    spherescript/scriptsprofile.cpp \
    qtutils/modelutils.cpp

HEADERS  += \
    forms/mainwindow.h \
    spherescript/scriptparser.h \
    uofiles/uoclientcom.h \
    spherescript/scriptobjects.h \
    globals.h \
    spherescript/scriptutils.h \
    common.h \
    forms/maintab_chars.h \
    forms/maintab_log.h \
    forms/maintab_items.h \
    forms/dlg_parseprogress.h \
    qtutils/checkableproxymodel.h \
    qtutils/delayedexecutiontimer.h \
    forms/dlg_profilescripts_options.h \
    spherescript/scriptsprofile.h \
    qtutils/modelutils.h

FORMS    += \
    forms/mainwindow.ui \
    forms/maintab_chars.ui \
    forms/maintab_log.ui \
    forms/maintab_items.ui \
    forms/dlg_parseprogress.ui \
    forms/dlg_profilescripts_options.ui
