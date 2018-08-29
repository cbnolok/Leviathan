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
    forms/dlg_huepicker.cpp \
    forms/dlg_profileclient_options.cpp \
    forms/dlg_profilescripts_options.cpp \
    forms/dlg_settings.cpp \
    forms/subdlg_searchobj.cpp \
    forms/subdlg_spawn.cpp \
    forms/subdlg_taskprogress.cpp \
    forms/maintab_tools.cpp \
    qtutils/checkableproxymodel.cpp \
    qtutils/delayedexecutiontimer.cpp \
    qtutils/modelutils.cpp \
    settings/appsettings.cpp \
    settings/clientprofile.cpp \
    settings/scriptsprofile.cpp \
    spherescript/scriptobjects.cpp \
    spherescript/scriptparser.cpp \
    spherescript/scriptsearch.cpp \
    spherescript/scriptutils.cpp \
    uoclientfiles/libsquish/alpha.cpp \
    uoclientfiles/libsquish/clusterfit.cpp \
    uoclientfiles/libsquish/colourblock.cpp \
    uoclientfiles/libsquish/colourfit.cpp \
    uoclientfiles/libsquish/colourset.cpp \
    uoclientfiles/libsquish/maths.cpp \
    uoclientfiles/libsquish/rangefit.cpp \
    uoclientfiles/libsquish/singlecolourfit.cpp \
    uoclientfiles/libsquish/singlecolourlookup.inl \
    uoclientfiles/libsquish/squish.cpp \
    uoclientfiles/ddsinfo.cpp \
    uoclientfiles/helpers.cpp \
    uoclientfiles/uonimmul.cpp \
    uoclientfiles/uoanim.cpp \
    uoclientfiles/uoart.cpp \
    uoclientfiles/uohues.cpp \
    uoclientfiles/uoidx.cpp \
    uoclientfiles/uoanimuop.cpp \
    keystrokesender/keystrokesender_common.cpp \
    keystrokesender/keystrokesender_windows.cpp \
    keystrokesender/keystrokesender_linux.cpp \
    keystrokesender/keystrokesender.cpp \
    uoppackage/uopblock.cpp \
    uoppackage/uopcompression.cpp \
    uoppackage/uoperror.cpp \
    uoppackage/uopfile.cpp \
    uoppackage/uophash.cpp \
    uoppackage/uopheader.cpp \
    uoppackage/uoppackage.cpp \
    cpputils/strings.cpp \
    cpputils/sysio.cpp

HEADERS  += \
    globals.h \
    version.h \
    forms/mainwindow.h \
    forms/maintab_chars.h \
    forms/maintab_items.h \
    forms/maintab_log.h \
    forms/maintab_tools.h \
    forms/dlg_huepicker.h \
    forms/dlg_profileclient_options.h \
    forms/dlg_profilescripts_options.h \
    forms/dlg_settings.h \
    forms/subdlg_searchobj.h \
    forms/subdlg_spawn.h \
    forms/subdlg_taskprogress.h \
    qtutils/checkableproxymodel.h \
    qtutils/delayedexecutiontimer.h \
    qtutils/modelutils.h \
    settings/appsettings.h \
    settings/clientprofile.h \
    settings/scriptsprofile.h \
    spherescript/scriptobjects.h \
    spherescript/scriptparser.h \
    spherescript/scriptsearch.h \
    spherescript/scriptutils.h \
    uoppackage/zconf.h \
    uoppackage/zlib.h \
    uoclientfiles/libsquish/alpha.h \
    uoclientfiles/libsquish/clusterfit.h \
    uoclientfiles/libsquish/colourblock.h \
    uoclientfiles/libsquish/colourfit.h \
    uoclientfiles/libsquish/colourset.h \
    uoclientfiles/libsquish/maths.h \
    uoclientfiles/libsquish/rangefit.h \
    uoclientfiles/libsquish/simd.h \
    uoclientfiles/libsquish/simd_float.h \
    uoclientfiles/libsquish/simd_sse.h \
    uoclientfiles/libsquish/simd_ve.h \
    uoclientfiles/libsquish/singlecolourfit.h \
    uoclientfiles/libsquish/squish.h \
    uoclientfiles/ddsinfo.h \
    uoclientfiles/helpers.h \
    uoclientfiles/uoanim.h \
    uoclientfiles/uoanimmul.h \
    uoclientfiles/uoanimuop.h \
    uoclientfiles/uoart.h \
    uoclientfiles/uohues.h \
    uoclientfiles/uoidx.h \
    keystrokesender/keystrokesender_common.h \
    keystrokesender/keystrokesender_windows.h \
    keystrokesender/keystrokesender_linux.h \
    keystrokesender/keystrokesender.h \
    cpputils/strings.h \
    cpputils/sysio.h \
    uoppackage/uopblock.h \
    uoppackage/uopcompression.h \
    uoppackage/uoperror.h \
    uoppackage/uopfile.h \
    uoppackage/uophash.h \
    uoppackage/uopheader.h \
    uoppackage/uoppackage.h \
    cpputils/maps.h

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

SUBDIRS += \
    uoclientfiles/libsquish/libSquish.pro
