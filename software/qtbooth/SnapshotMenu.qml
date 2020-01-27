import QtQuick 2.4
import Qt.labs.settings 1.0
import GPIO 1.0

SnapshotMenuForm {
    id: form

    signal captured(string filename)
    signal abort

    countdown: settings.countdown

    Settings
    {
        id: settings
        category: "Camera"
        property bool flashEnabled: true
        property real viewFinderBrightness: 0.1
        property real flashBrightness: 1.0
        property int countdown: 1
    }

    GPIO
    {
        id: ledEnablePin
        pin:  4
        value: 0.0
    }

    GPIO
    {
        id: ledBrightnessPin
        pin:  1
        value: 0.0
    }

    shutterButton.onTriggerSnapshot:
    {
        cameraRenderer.takePhoto()
    }

    cameraRenderer.onSavedPhoto:
    {
        captured(filename)
        shutterButton.state = "idle"
    }

    exitButton.onClicked:
    {
        abort()
    }

}
