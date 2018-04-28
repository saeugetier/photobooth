QT += qml quick multimedia printsupport concurrent

CONFIG += c++11

SOURCES += main.cpp \
    myhelper.cpp \
    photomontage.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lMagick++-6.Q16 -lMagickWand-6.Q16 -lMagickCore-6.Q16

DISTFILES +=

HEADERS += \
    myhelper.h \
    photomontage.h

INCLUDEPATH += /usr/include/ImageMagick-6
