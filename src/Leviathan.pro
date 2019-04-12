#-------------------------------------------------
#
# Project created by QtCreator 2016-08-03T20:33:21
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Leviathan
TEMPLATE = app

CONFIG += c++14

unix:!macx {
    DEFINES += BUILD_LINUX=1 BUILD_MACOS=0 BUILD_WINDOWS=0
}
win32|win64 {
    DEFINES += BUILD_WINDOWS=1 BUILD_LINUX=0 BUILD_MACOS=0
}
macx {
    DEFINES += BUILD_MACOS=1 BUILD_LINUX=0 BUILD_WINDOWS=0
}

RESOURCES = leviathan_resources.qrc

SOURCES += \
    globals.cpp \
    main.cpp \
    cpputils/strings.cpp \
    cpputils/sysio.cpp \
    qtutils/checkableproxymodel.cpp \
    qtutils/delayedexecutiontimer.cpp \
    qtutils/modelutils.cpp \
    forms/base_mapview.cpp \
    forms/mainwindow.cpp \
    forms/maintab_chars.cpp \
    forms/maintab_items.cpp \
    forms/maintab_log.cpp \
    forms/maintab_travel.cpp \
    forms/dlg_huepicker.cpp \
    forms/dlg_profileclient_options.cpp \
    forms/dlg_profilescripts_options.cpp \
    forms/dlg_settings.cpp \
    forms/dlg_worldmap.cpp \
    forms/subdlg_searchobj.cpp \
    forms/subdlg_spawn.cpp \
    forms/subdlg_taskprogress.cpp \
    forms/maintab_tools.cpp \
    settings/appsettings.cpp \
    settings/clientprofile.cpp \
    settings/scriptsprofile.cpp \
    spherescript/scriptobjects.cpp \
    spherescript/scriptparser.cpp \
    spherescript/scriptsearch.cpp \
    spherescript/scriptutils.cpp \
    uoppackage/uopblock.cpp \
    uoppackage/uopcompression.cpp \
    uoppackage/uoperror.cpp \
    uoppackage/uopfile.cpp \
    uoppackage/uophash.cpp \
    uoppackage/uoppackage.cpp \
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
    uoclientfiles/exceptions.cpp \
    uoclientfiles/helpers.cpp \
    uoclientfiles/uoradarcol.cpp \
    uoclientfiles/uomap.cpp \
    uoclientfiles/uostatics.cpp \
    uoclientfiles/colors.cpp \
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
    keystrokesender/keystrokesender_mac.cpp

HEADERS  += \
    globals.h \
    logging.h \
    version.h \
    cpputils/maps.h \
    cpputils/strings.h \
    cpputils/sysio.h \
    qtutils/checkableproxymodel.h \
    qtutils/delayedexecutiontimer.h \
    qtutils/modelutils.h \
    forms/base_mapview.h \
    forms/mainwindow.h \
    forms/maintab_chars.h \
    forms/maintab_items.h \
    forms/maintab_log.h \
    forms/maintab_tools.h \
    forms/maintab_travel.h \
    forms/dlg_huepicker.h \
    forms/dlg_profileclient_options.h \
    forms/dlg_profilescripts_options.h \
    forms/dlg_settings.h \
    forms/dlg_worldmap.h \
    forms/subdlg_searchobj.h \
    forms/subdlg_spawn.h \
    forms/subdlg_taskprogress.h \
    settings/appsettings.h \
    settings/clientprofile.h \
    settings/scriptsprofile.h \
    spherescript/scriptobjects.h \
    spherescript/scriptparser.h \
    spherescript/scriptsearch.h \
    spherescript/scriptutils.h \
    uoppackage/uopblock.h \
    uoppackage/uopcompression.h \
    uoppackage/uoperror.h \
    uoppackage/uopfile.h \
    uoppackage/uophash.h \
    uoppackage/uoppackage.h \
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
    uoclientfiles/colors.h \
    uoclientfiles/ddsinfo.h \
    uoclientfiles/exceptions.h \
    uoclientfiles/helpers.h \
    uoclientfiles/uoradarcol.h \
    uoclientfiles/uomap.h \
    uoclientfiles/uostatics.h \
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
    keystrokesender/keystrokesender_mac.h

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
    forms/maintab_tools.ui \
    forms/dlg_worldmap.ui \
    forms/maintab_travel.ui

SUBDIRS += \
    uoclientfiles/libsquish/libSquish.pro

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
win32:!unix {
    contains(QMAKE_CC, gcc) {
        # MinGW

        #   Dynamically link zlib
        contains(QT_ARCH, x86_64) {
            LIBS += -L\"$$PWD/../winlibs/64\" -lz
        } else {
            LIBS += -L\"$$PWD/../winlibs/32\" -lz
        }

        #QMAKE_CXXFLAGS += -Wno-unknown-pragmas  # disable unknown pragma warning
        QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
        QMAKE_CXXFLAGS += -fopenmp      # enable OpenMP pragmas
    }

    contains(QMAKE_CC, cl) {
        # Visual Studio

        #   Dynamically link zlib and user32 (the latter for postmessage and such in KeystrokeSender...)
        #   (is user32 automatically dynamically linked by MinGW?)
        contains(QT_ARCH, x86_64) {
            LIBS += -luser32 -L\"$$PWD/../winlibs/64\" -lzlib
        } else {
            LIBS += -luser32 -L\"$$PWD/../winlibs/32\" -lzlib
        }
        DEFINES += "ZLIB_WINAPI=1"
        DEFINES += "ZLIB_DLL=1"

        #QMAKE_CXXFLAGS += /wd4068  # disable unknown pragma warning
        QMAKE_CXXFLAGS += /openmp   # enable OpenMP pragmas
    }

    LIBS += -fopenmp

    contains(QMAKE_COPY, copy) {    # When it's compiled by AppVeyor, QMAKE_COPY is cp -f, not copy
                                    # this way, we'll copy dlls to build directory only when building locally on Windows
        contains(QT_ARCH, x86_64) {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\..\\winlibs\\64\\*.dll\" \"$$DESTDIR\"
        } else {
            QMAKE_PRE_LINK += $$QMAKE_COPY \"$$PWD\\..\\winlibs\\32\\*.dll\" \"$$DESTDIR\"
        }
    }
}

# linux and mac
unix:!win32 {
    !mac {
        LIBS += -lz                             # dynamically link zlib...
        LIBS += -L/usr/X11R6/lib -lX11          # ...and xlib (for KeystrokeSender)

        QMAKE_CXXFLAGS += -fopenmp              # enable OpenMP pragmas as default flag for Linux
        LIBS += -fopenmp                        # link against OpenMP library
    } else {
        LIBS += -L/usr/lib -lz                  # dynamically link zlib

        # UNTESTED
        # looks like OpenMP support here began only with recent LLVM versions?
        #  (Apple's LLVM versions have a different numeration from the official LLVM branch)
        #LLVMVER = system(g++ -x c++ -dM -E - < /dev/null | grep -Eo "__apple_build_version__  [0-9]{1,7}" | grep -Eo "[0-9]{1,7}")
        # OpenMP support began with "standard" version 3.8.0, which may be Apple 703.0.29 or 703.0.31. To be sure, check for 3.9.0 (>= Apple 800.0.38 ?)
        #greaterThan(LLVMVER, 80000037) {
        #    QMAKE_CXXFLAGS += -fopenmp             # enable OpenMP pragmas
        #    # or should we use -fopenmp=libomp ?
        #    #LIBS += -fopenmp
        #} else {
            ##QMAKE_CXXFLAGS += -fopenmp            # no openmp for you
            ##LIBS += -fopenmp
        #}
    }

    # disable some specific warnings
    QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
    QMAKE_CXXFLAGS += -Wmisleading-indentation #-Wno-unknown-pragmas
}
