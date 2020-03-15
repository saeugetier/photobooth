QT += qml quick multimedia concurrent xml

CONFIG += c++17

linux {
    contains(QT_ARCH, arm):{
        DEFINES += WIRINGPI
        LIBS += -lwiringPi -lwiringPiDev
    }
}

SOURCES += main.cpp \
    collageimagemodel.cpp \
    collagemodelfactory.cpp \
    fakeprinter.cpp \
    fileio.cpp \
    filesystem.cpp \
    gpio.cpp \
    modelparser.cpp \
    selphyprinter.cpp \
    src/collageiconmodel.cpp \
    src/collageimagemodel.cpp \
    src/collagemodelfactory.cpp \
    src/fakeprinter.cpp \
    src/fileio.cpp \
    src/filesystem.cpp \
    src/gpio.cpp \
    src/main.cpp \
    src/modelparser.cpp \
    src/selphyprinter.cpp \
    src/system.cpp \
    src/translationhelper.cpp \
    system.cpp \
    translationhelper.cpp

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
    filesystem.h \
    gpio.h \
    modelparser.h \
    call_once.h \
    selphyprinter.h \
    src/abstractprinter.h \
    src/call_once.h \
    src/collageiconmodel.h \
    src/collageimagemodel.h \
    src/collagemodelfactory.h \
    src/fakeprinter.h \
    src/fileio.h \
    src/filesystem.h \
    src/gpio.h \
    src/modelparser.h \
    src/selphyprinter.h \
    src/system.h \
    src/translationhelper.h \
    system.h \
    translationhelper.h
