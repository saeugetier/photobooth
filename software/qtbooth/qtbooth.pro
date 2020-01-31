QT += qml quick multimedia concurrent xml

CONFIG += c++17

linux {
    contains(QT_ARCH, arm):{
        DEFINES += WIRINGPI
        LIBS += -lwiringPi -lwiringPiDev
    }
}

SOURCES += main.cpp \
    collageiconmodel.cpp \
    collageimagemodel.cpp \
    collagemodelfactory.cpp \
    fakeprinter.cpp \
    fileio.cpp \
    gpio.cpp \
    modelparser.cpp \
    myhelper.cpp \
    selphyprinter.cpp \
    system.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

TRANSLATIONS = "tr_de.ts"

DISTFILES +=

HEADERS += \
    abstractprinter.h \
    collageiconmodel.h \
    collageimagemodel.h \
    collagemodelfactory.h \
    fakeprinter.h \
    fileio.h \
    gpio.h \
    modelparser.h \
    myhelper.h \
    call_once.h \
    selphyprinter.h \
    singleton.h \
    system.h
