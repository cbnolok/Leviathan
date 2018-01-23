#-------------------------------------------------
#
# Project created by QtCreator 2016-08-03T20:33:21
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Leviathan
TEMPLATE = app

CONFIG += c++11

RESOURCES = leviathan_resources.qrc

SOURCES += \
    globals.cpp \
    main.cpp \
    forms/mainwindow.cpp \
    forms/maintab_chars.cpp \
    forms/maintab_items.cpp \
    forms/maintab_log.cpp \
    forms/dlg_profileclient_options.cpp \
    forms/dlg_profilescripts_options.cpp \
    forms/dlg_settings.cpp \
    forms/subdlg_searchobj.cpp \
    forms/subdlg_spawn.cpp \
    forms/subdlg_taskprogress.cpp \
    qtutils/checkableproxymodel.cpp \
    qtutils/delayedexecutiontimer.cpp \
    qtutils/modelutils.cpp \
    settings/clientprofile.cpp \
    settings/scriptsprofile.cpp \
    spherescript/scriptobjects.cpp \
    spherescript/scriptparser.cpp \
    spherescript/scriptsearch.cpp \
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
    forms/dlg_huepicker.cpp \
    settings/appsettings.cpp \
    cpputils.cpp \
    cpputils_sysio.cpp \
    forms/maintab_tools.cpp

HEADERS  += \
    globals.h \
    forms/mainwindow.h \
    forms/maintab_chars.h \
    forms/maintab_items.h \
    forms/maintab_log.h \
    forms/dlg_profileclient_options.h \
    forms/dlg_profilescripts_options.h \
    forms/dlg_settings.h \
    forms/subdlg_searchobj.h \
    forms/subdlg_spawn.h \
    forms/subdlg_taskprogress.h \
    qtutils/checkableproxymodel.h \
    qtutils/delayedexecutiontimer.h \
    qtutils/modelutils.h \
    settings/clientprofile.h \
    settings/scriptsprofile.h \
    spherescript/scriptobjects.h \
    spherescript/scriptparser.h \
    spherescript/scriptsearch.h \
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
    version.h \
    forms/dlg_huepicker.h \
    settings/appsettings.h \
    cpputils.h \
    cpputils_sysio.h \
    forms/maintab_tools.h

FORMS    += \
    forms/mainwindow.ui \
    forms/maintab_chars.ui \
    forms/maintab_log.ui \
    forms/maintab_items.ui \
    forms/dlg_profilescripts_options.ui \
    forms/dlg_profileclient_options.ui \
    forms/dlg_settings.ui \
    forms/subdlg_taskprogress.ui \
    forms/subdlg_searchobj.ui \
    forms/subdlg_spawn.ui \
    forms/dlg_huepicker.ui \
    forms/maintab_tools.ui


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


# Add the application icon
win32:RC_ICONS += icons/leviathan.ico


###### Compiler/Linker settings

# windows
win32:!unix
{
    contains(QMAKE_CC, gcc) {
        # MinGW

        #   Dynamically link zlib
        contains(QT_ARCH, x86_64) {
            LIBS += -L\"$$PWD\\..\\winlibs\\64\" -lz
        } else {
            LIBS += -L\"$$PWD\\..\\winlibs\\32\" -lz
        }

        #QMAKE_CXXFLAGS += -Wno-unknown-pragmas  # disable unknown pragma warning
        QMAKE_CXXFLAGS += -fopenmp      # enable OpenMP pragmas
    }

    contains(QMAKE_CC, cl) {
        # Visual Studio

        #   Dynamically link zlib and user32 (the latter for postmessage and such in KeystrokeSender...)
        #   (is user32 automatically dynamically linked by MinGW?)
        contains(QT_ARCH, x86_64) {
            LIBS += -luser32 -L\"$$PWD\\..\\winlibs\\64\" -lzlibwapi
        } else {
            LIBS += -luser32 -L\"$$PWD\\..\\winlibs\\32\" -lzlibwapi
        }
        DEFINES += "ZLIB_WINAPI=1"
        DEFINES += "ZLIB_DLL=1"
        #contains(QT_ARCH, x86_64) {
        #    LIBS += user32.lib \"$$PWD\\..\\winlibs\\64\\zlibwapi.lib\"
        #} else {
        #    LIBS += user32.lib \"$$PWD\\..\\winlibs\\32\\zlibwapi.lib\"
        #}

        #QMAKE_CXXFLAGS += /wd4068  # disable unknown pragma warning
        QMAKE_CXXFLAGS += /openmp   # enable OpenMP pragmas
    }

    contains(QMAKE_COPY, copy) {    # When it's compiled by AppVeyor, QMAKE_COPY is cp -f, not copy
                                    # this way, we'll copy dlls to build directory only when building locally on Windows
        contains(QT_ARCH, x86_64) {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\..\\winlibs\\64\\*.dll\" \"$$DESTDIR\"
        } else {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\..\\winlibs\\32\\*.dll\" \"$$DESTDIR\"
        }
    }
}

# linux
unix {
    LIBS += -lz -L/usr/X11R6/lib -lX11      # dynamically link zlib and xlib (the latter for KeystrokeSender)
    QMAKE_CXXFLAGS += -fopenmp              # enable OpenMP pragmas

    # disable some specific warnings
    QMAKE_CXXFLAGS += -Wmisleading-indentation #-Wno-unknown-pragmas
}

LIBS += -fopenmp    # link against OpenMP library
