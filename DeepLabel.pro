#-------------------------------------------------
#
# Project created by QtCreator 2017-10-04T17:31:00
#
#-------------------------------------------------

QT       += core gui sql testlib concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeepLabel
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx{
message("Mac")
#PKG_CONFIG = /usr/local/bin/pkg-config
CONFIG += c++17

# Build without FFMPEG unless you want a lot of pain later
#CONFIG += link_pkgconfig
#PKGCONFIG += opencv4

# If you use pkg-config, *everything* gets linked. Wasteful.
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/opt/opencv/lib/
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_tracking -lopencv_video -lopencv_videoio -lopencv_dnn
}

unix:!macx{
message("Linux")
#CONFIG += link_pkgconfig
#PKGCONFIG += opencv4
CONFIG += c++17
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_tracking -lopencv_video -lopencv_videoio -lopencv_dnn
}

win32{
message("Windows")
INCLUDEPATH += "C:/Users/Josh/Code/opencv/build/install/include"
LIBS += -L"C:/Users/Josh/Code/opencv/build/install/x64/vc15/lib"
QMAKE_CXXFLAGS += "/std:c++17"
CONFIG(debug, debug|release) {
LIBS += -lopencv_world453d
}else{
LIBS += -lopencv_world453
}
}

# For building in a single folder
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = .obj_debug
    MOC_DIR     = .moc_debug
}else {
    DESTDIR = release
    OBJECTS_DIR = .obj
    MOC_DIR     = .moc
}

INCLUDEPATH += "$$_PRO_FILE_PWD_/src"
VPATH = "$$_PRO_FILE_PWD_/src"

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        labelproject.cpp \
        imagelabel.cpp \
        kittiexporter.cpp \
        darknetexporter.cpp \
        exportdialog.cpp \
        multitracker.cpp \
    src/baseexporter.cpp \
    src/baseimporter.cpp \
    src/cliparser.cpp \
    src/cocoexporter.cpp \
    src/darknetimporter.cpp \
    src/gcpexporter.cpp \
    src/imagedisplay.cpp \
    src/importdialog.cpp \
    src/pascalvocexporter.cpp \
    detection/detectoropencv.cpp \
    detection/detectorsetupdialog.cpp \
    src/refinerangedialog.cpp \
    src/cocoimporter.cpp

HEADERS += \
        mainwindow.h \
        labelproject.h \
        imagelabel.h \
        boundingbox.h \
        kittiexporter.h \
        darknetexporter.h \
        exportdialog.h \
        multitracker.h \
    src/baseexporter.h \
    src/baseimporter.h \
    src/cliparser.h \
    src/cocoexporter.h \
    src/darknetimporter.h \
    src/exporter.h \
    src/gcpexporter.h \
    src/imagedisplay.h \
    src/importdialog.h \
    src/pascalvocexporter.h \
    detection/detectoropencv.h \
    detection/detectorsetupdialog.h \
    src/refinerangedialog.h \
    src/cocoimporter.h

FORMS += \
    src/importdialog.ui \
        src/mainwindow.ui \
    src/exportdialog.ui \
    src/imagedisplay.ui \
    detection/detectorsetupdialog.ui \
    src/refinerangedialog.ui

include(QtAwesome/QtAwesome/QtAwesome.pri)

# Deploy apps in OS X and Windows
isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

win32 {
    DEPLOY_COMMAND = windeployqt.exe
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
win32 {
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}
macx {
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}
