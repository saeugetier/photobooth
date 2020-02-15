import QtQuick 2.4
import GPIO 1.0

SnapshotMenuForm {
    id: form

    signal captured(string filename)
    signal abort

    countdown: snapshotSettings.countdown

    cameraRenderer.onPhotoProcessingChanged:
    {
        console.log("photo processing: " + Boolean(cameraRenderer.photoProcessing).toString())
    }

    GPIO
    {
        id: ledEnablePin
        pin:  4
        value: 0.0
    }

    onStateChanged:
    {
        console.log("Snapshot menu state changed: " + state)
        if(state == "deactivated")
        {
            ledEnablePin.value = 0.0
        }
        else
        {
            ledEnablePin.value = 1.0
        }
    }

    GPIO
    {
        id: ledBrightnessPin
        pin:  1
        value: snapshotSettings.viewFinderBrightness
    }

    cameraRenderer.onStateChanged:
    {
        if(cameraRenderer.state == "snapshot" && snapshotSettings.flashEnabled)
        {
            ledBrightnessPin.value = snapshotSettings.flashBrightness
        }
        else
        {
            ledBrightnessPin.value = snapshotSettings.viewFinderBrightness
        }
    }

    shutterButton.onTriggerSnapshot:
    {
        cameraRenderer.takePhoto()
    }

    shutterButton.onStateChanged:
    {
        if(shutterButton.state != "idle")
        {
            form.state = "snapshot"
        }
    }

    cameraRenderer.onSavedPhoto:
    {
        captured(filename)
        shutterButton.state = "idle"
    }

    cameraRenderer.onFailed:
    {
        shutterButton.state = "idle"
        failureAnimation.start()
    }

    SequentialAnimation
    {
        id: failureAnimation
        running: false
        loops: 1
        PropertyAnimation { target: failureText; property: "visible"; to: true}
        PropertyAnimation { target: failureText; property: "opacity"; to: 1.0}
        NumberAnimation { target: failureText; property: "opacity"; to: 0.0; duration: 1000}
        PropertyAnimation { target: failureText; property: "visible"; to: false}
    }

    exitButton.onClicked:
    {
        abort()
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
