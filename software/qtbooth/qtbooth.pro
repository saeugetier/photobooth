QT += qml quick multimedia printsupport

CONFIG += c++11

SOURCES += main.cpp \
    myhelper.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lcups

DISTFILES +=

HEADERS += \
    myhelper.h

INCLUDEPATH += /usr/include/ImageMagick-6 /usr/include/x86_64-linux-gnu/ImageMagick-6
