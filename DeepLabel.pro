#-------------------------------------------------
#
# Project created by QtCreator 2017-10-04T17:31:00
#
#-------------------------------------------------

QT       += core gui sql

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

win32{
    INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/opencv/include"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/opencv/lib"
}

mac|unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
}

CONFIG(debug, debug|release) {
    message("Debug mode")
    win32{
    LIBS += -lopencv_ximgproc310d -lopencv_core310d -lopencv_highgui310d -lopencv_calib3d310d -lopencv_imgproc310d -lopencv_imgcodecs310d
    }
}else {
    message("Release mode")
    win32{
    LIBS += -lopencv_ximgproc310 -lopencv_core310 -lopencv_highgui310 -lopencv_calib3d310 -lopencv_imgproc310 -lopencv_imgcodecs310
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
    src/labelproject.cpp

HEADERS += \
        mainwindow.h \
    src/labelproject.h

FORMS += \
        mainwindow.ui

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
#QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
