QT += qml quick multimedia concurrent xml svg printsupport

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
    src/noprinter.cpp \
    src/printerfactory.cpp \
    src/selphyprinter.cpp \
    src/standardprinter.cpp \
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

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

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
    src/noprinter.h \
    src/printerfactory.h \
    src/selphyprinter.h \
    src/standardprinter.h \
    src/system.h \
    src/translationhelper.h

contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

DEFINES += GIT_CURRENT_SHA1="$(shell git -C \""$$_PRO_FILE_PWD_"\" describe)"
