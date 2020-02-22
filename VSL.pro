QT       += core gui multimedia
CONFIG+=sdk_no_version_check
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += vs
SOURCES += \
    common.cpp \
    highlighter.cpp \
    main.cpp \
    mainwindow.cpp \
    vs/ChordTable.cpp \
    vs/ColorIndex.cpp \
    vs/ColorScale.cpp \
    vs/FastSin.cpp \
    vs/MusicFreq.cpp \
    vs/audioout.cpp \
    vs/codeeditor.cpp \
    vs/fft.cpp \
    vs/riff.cpp \
    wavegraph.cpp

HEADERS += \
    VSL_compiler.h \
    common.h \
    highlighter.h \
    mainwindow.h \
    scaler.h \
    vs/ChordTable.h \
    vs/ColorIndex.h \
    vs/ColorScale.h \
    vs/FastSin.h \
    vs/MusicFreq.h \
    vs/Thread.h \
    vs/Timer.h \
    vs/audioout.h \
    vs/codeeditor.h \
    vs/ddc.h \
    vs/fft.h \
    vs/freqmesh.h \
    vs/riff.h \
    wavegraph.h

FORMS += \
    mainwindow.ui \
    random_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES += \
    default.etg \
    old_stuff \
    snippets \
    waves
