QT += qml quick multimedia printsupport concurrent

CONFIG += c++14

linux {
    contains(QT_ARCH, arm):{
        DEFINES += WIRINGPI
        LIBS += -lwiringPi -lwiringPiDev
    }
}

SOURCES += main.cpp \
    myhelper.cpp \
    photomontage.cpp \
    ledflash.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


DISTFILES +=

HEADERS += \
    myhelper.h \
    photomontage.h \
    ledflash.h \
    call_once.h \
    singleton.h
