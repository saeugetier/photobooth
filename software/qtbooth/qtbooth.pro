QT += qml quick multimedia concurrent xml

CONFIG += c++17 qtquickcompiler

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
    gpio.cpp \
    modelparser.cpp \
    myhelper.cpp \
    photomontage.cpp \
    ledflash.cpp \
    printer.cpp

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
    collageiconmodel.h \
    collageimagemodel.h \
    collagemodelfactory.h \
    gpio.h \
    modelparser.h \
    myhelper.h \
    photomontage.h \
    ledflash.h \
    call_once.h \
    singleton.h \
    printer.h
