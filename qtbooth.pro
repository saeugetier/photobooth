QT += qml quick multimedia concurrent xml svg printsupport multimedia-private network

CONFIG += c++20 qml_debug

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4 libcamera libgpiod
} else {
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lcamera -lcamera-base -lcamera-controls -lgpiod
}

SOURCES += src/collageiconmodel.cpp \
    src/captureprocessor.cpp \
    src/collageimagemodel.cpp \
    src/collagemodelfactory.cpp \
    src/fakeprinter.cpp \
    src/fileio.cpp \
    src/filesystem.cpp \
    src/gphotocamera.cpp \
    src/gpio.cpp \
    src/libcameracamera.cpp \
    src/main.cpp \
    src/modelparser.cpp \
    src/noprinter.cpp \
    src/printerfactory.cpp \
    src/replacebackgroundvideofilter.cpp \
    src/segmentation.cpp \
    src/selphyprinter.cpp \
    src/standardprinter.cpp \
    src/system.cpp \
    src/translationhelper.cpp \
    src/yolo11segncnn.cpp \
    src/yolo11segonnx.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

TRANSLATIONS = \
    tr_de.ts \
    tr_fr.ts \
    tr_es.ts \
    tr_it.ts

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

INCLUDEPATH += src/ \
    libs/onnxruntime/include/ \
    libs/ncnn/include/ \

HEADERS += \
    src/abstractprinter.h \
    src/call_once.h \
    src/captureprocessor.h \
    src/collageiconmodel.h \
    src/collageimagemodel.h \
    src/collagemodelfactory.h \
    src/fakeprinter.h \
    src/fileio.h \
    src/filesystem.h \
    src/gphotocamera.h \
    src/gpio.h \
    src/libcameracamera.h \
    src/modelparser.h \
    src/noprinter.h \
    src/printerfactory.h \
    src/replacebackgroundvideofilter.h \
    src/segmentation.h \
    src/selphyprinter.h \
    src/standardprinter.h \
    src/system.h \
    src/translationhelper.h \
    src/yolo11segncnn.h \
    src/yolo11segonnx.h \
    src/yolobackend.h

contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

DEFINES += GIT_CURRENT_SHA1="$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)"
DEFINES += GIT_CURRENT_TAG="$(shell git -C \""$$_PRO_FILE_PWD_"\" tag --points-at HEAD)"

LIBS += -L"$$PWD/libs/onnxruntime/lib" -lonnxruntime
LIBS += -L"$$PWD/libs/ncnn/lib" -lncnn
LIBS += -lgphoto2 -lgphoto2_port

# RKNN NPU support — only on AArch64 targets (RK3566/RK3568/RK3588 etc.)
contains(QT_ARCH, arm64) | contains(QT_ARCH, aarch64) {
    DEFINES += HAS_RKNN

    exists(/app/include) { INCLUDEPATH += /app/include }
    exists($$PWD/libs/rknn/include) { INCLUDEPATH += $$PWD/libs/rknn/include }

    # Ensure -L paths are added before -l so the linker can resolve librknnrt.
    LIBS += -L"$$PWD/libs/rknn/lib"
    exists(/app/lib) { LIBS += -L/app/lib }
    exists(/app/lib64) { LIBS += -L/app/lib64 }
    LIBS += -lrknnrt

    SOURCES += src/yolo11segrknn.cpp
    HEADERS += src/yolo11segrknn.h
}

# Flatpak runtime libraries are typically staged in /app/lib or /app/lib64.
exists(/app/lib) {
    LIBS += -L/app/lib
}
exists(/app/lib64) {
    LIBS += -L/app/lib64
}

!isEmpty(PREFIX) {
    INSTALLS += target
    target.path = $$PREFIX/bin
    # Flatpak/aarch64 can install third-party libs into lib64.
    LIBS += -L$$PREFIX/lib -L$$PREFIX/lib64
}
else {
    INSTALLS += target
       target.path = /usr/local/bin
}
