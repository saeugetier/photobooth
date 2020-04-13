QT += qml quick multimedia concurrent xml svg

CONFIG += c++17 qml_debug

SOURCES += src/collageiconmodel.cpp \
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
    src/translationhelper.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

TRANSLATIONS = "tr_de.ts"

DISTFILES +=

INCLUDEPATH += src/

HEADERS += \
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
    src/translationhelper.h
