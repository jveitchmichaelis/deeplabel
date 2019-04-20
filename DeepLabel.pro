#-------------------------------------------------
#
# Project created by QtCreator 2017-10-04T17:31:00
#
#-------------------------------------------------

QT       += core gui sql testlib concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeepLabel
TEMPLATE = app

CONFIG += c++14 #Qt will ignore anything higher at the moment

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
PKG_CONFIG = /usr/local/bin/pkg-config
QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
}

unix|macx{
CONFIG += link_pkgconfig
PKGCONFIG += opencv4
}

win32{
message("Windows")
INCLUDEPATH += "C:/Users/Josh/Code/opencv/build/install/include"
LIBS += -L"C:/Users/Josh/Code/opencv/build/install/x64/vc15/lib"
CONFIG(debug, debug|release) {
LIBS += -lopencv_core400d -lopencv_highgui400d -lopencv_imgproc400d -lopencv_imgcodecs400d -lopencv_tracking400d -lopencv_video400d -lopencv_dnn400d
}else{
LIBS += -lopencv_core400 -lopencv_highgui400 -lopencv_imgproc400 -lopencv_imgcodecs400 -lopencv_tracking400 -lopencv_video400 -lopencv_dnn400
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
    src/labelproject.cpp \
    src/imagelabel.cpp \
    src/kittiexporter.cpp \
    src/darknetexporter.cpp \
    src/exportdialog.cpp \
    src/detection/detectoropencv.cpp

HEADERS += \
        mainwindow.h \
    src/labelproject.h \
    src/imagelabel.h \
    src/boundingbox.h \
    src/kittiexporter.h \
    src/darknetexporter.h \
    src/exportdialog.h \
    src/detection/detectoropencv.h

FORMS += \
        mainwindow.ui \
    exportdialog.ui

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

win32 {
    DEPLOY_COMMAND = windeployqt
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
win32 {
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}
macx {
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET} -dmg
}
