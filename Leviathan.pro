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
    common.cpp \
    globals.cpp \
    main.cpp \
    sysio.cpp \
    forms/dlg_profileclient_options.cpp \
    forms/dlg_profilescripts_options.cpp \
    forms/maintab_chars.cpp \
    forms/maintab_items.cpp \
    forms/maintab_log.cpp \
    forms/mainwindow.cpp \
    qtutils/checkableproxymodel.cpp \
    qtutils/delayedexecutiontimer.cpp \
    qtutils/modelutils.cpp \
    settings/clientprofile.cpp \
    settings/scriptsprofile.cpp \
    spherescript/scriptobjects.cpp \
    spherescript/scriptparser.cpp \
    spherescript/scriptutils.cpp \
    uofiles/uoppackage/UOPError.cpp \
    uofiles/uoppackage/UOPBlock.cpp \
    uofiles/uoppackage/UOPFile.cpp \
    uofiles/uoppackage/UOPHeader.cpp \
    uofiles/uoppackage/UOPPackage.cpp \
    uofiles/uoanim.cpp \
    uofiles/uoanimmul.cpp \
    uofiles/uoart.cpp \
    uofiles/uohues.cpp \
    uofiles/uoidx.cpp \
    uofiles/uoanimuop.cpp \
    keystrokesender/keystrokesender_common.cpp \
    keystrokesender/keystrokesender_windows.cpp \
    keystrokesender/keystrokesender_linux.cpp \
    keystrokesender/keystrokesender.cpp \
    forms/subdlg_taskprogress.cpp

HEADERS  += \
    common.h \
    globals.h \
    sysio.h \
    forms/dlg_profileclient_options.h \
    forms/dlg_profilescripts_options.h \
    forms/maintab_chars.h \
    forms/maintab_items.h \
    forms/maintab_log.h \
    forms/mainwindow.h \
    qtutils/checkableproxymodel.h \
    qtutils/delayedexecutiontimer.h \
    qtutils/modelutils.h \
    settings/clientprofile.h \
    settings/scriptsprofile.h \
    spherescript/scriptobjects.h \
    spherescript/scriptparser.h \
    spherescript/scriptutils.h \
    uofiles/uoppackage/UOPBlock.h \
    uofiles/uoppackage/UOPCompression.h \
    uofiles/uoppackage/UOPError.h \
    uofiles/uoppackage/UOPFile.h \
    uofiles/uoppackage/UOPHeader.h \
    uofiles/uoppackage/UOPPackage.h \
    uofiles/uoppackage/zconf.h \
    uofiles/uoppackage/zlib.h \
    uofiles/uoanim.h \
    uofiles/uoanimmul.h \
    uofiles/uoart.h \
    uofiles/uohues.h \
    uofiles/uoidx.h \
    uofiles/uoanimuop.h \
    keystrokesender/keystrokesender_common.h \
    keystrokesender/keystrokesender_windows.h \
    keystrokesender/keystrokesender_linux.h \
    keystrokesender/keystrokesender.h \
    forms/subdlg_taskprogress.h

FORMS    += \
    forms/mainwindow.ui \
    forms/maintab_chars.ui \
    forms/maintab_log.ui \
    forms/maintab_items.ui \
    forms/dlg_profilescripts_options.ui \
    forms/dlg_profileclient_options.ui \
    forms/subdlg_taskprogress.ui


Release:DESTDIR     = release
Release:OBJECTS_DIR = release/obj
Release:MOC_DIR     = release/moc
Release:RCC_DIR     = release/rcc
Release:UI_DIR      = release/ui

Debug:DESTDIR       = debug
Debug:OBJECTS_DIR   = debug/obj
Debug:MOC_DIR       = debug/moc
Debug:RCC_DIR       = debug/rcc
Debug:UI_DIR        = debug/ui


# windows
win32:!unix
{
    contains(QMAKE_CC, gcc) {
        # MinGW
        #   Copy zlib dll to the build directory and dynamically link it
        contains(QT_ARCH, x86_64) {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\winlibs\\64\\*.dll\" \"$$DESTDIR\"
            LIBS += -L\"$$PWD\\winlibs\\64\" -lz
        } else {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\winlibs\\32\\*.dll\" \"$$DESTDIR\"
            LIBS += -L\"$$PWD\\winlibs\\32\" -lz
        }
    }
    contains(QMAKE_CC, cl) {
        # Visual Studio
        #   dynamically link zlib and user32 (the latter for postmessage and such in KeystrokeSender...)
        #   (is user32 automatically dynamically linked by MinGW?)
        contains(QT_ARCH, x86_64) {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\winlibs\\64\\*.dll\" \"$$DESTDIR\"
            LIBS += user32.lib \"$$PWD\\winlibs\\64\\zlib1.lib\"
        } else {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\winlibs\\32\\*.dll\" \"$$DESTDIR\"
            LIBS += user32.lib \"$$PWD\\winlibs\\32\\zlib1.lib\"
        }
    }
}
# linux: dynamically link zlib and xlib (the latter for KeystrokeSender)
unix {
    LIBS += -lz -L/usr/X11R6/lib -lX11
}

