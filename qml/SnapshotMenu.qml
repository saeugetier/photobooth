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

    cameraRenderer.mirrored: applicationSettings.cameraMirrored

    GPIO
    {
        id: ledEnablePin
        pin:  23
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
            shutterButton.reset()
            ledEnablePin.value = 1.0
        }
    }

    GPIO
    {
        id: ledBrightnessPin
        pin:  18
        value: 1.0 - snapshotSettings.viewFinderBrightness
    }

    cameraRenderer.onStateChanged:
    {
        if(cameraRenderer.state == "snapshot" && snapshotSettings.flashEnabled)
        {
            ledBrightnessPin.value = 1.0 - snapshotSettings.flashBrightness
        }
        else
        {
            ledBrightnessPin.value = 1.0 - snapshotSettings.viewFinderBrightness
        }
    }

    snapshotSettings.onViewFinderBrightnessChanged:
    {
        ledBrightnessPin.value = 1.0 - snapshotSettings.viewFinderBrightness
    }

    shutterButton.onTriggerSnapshot:
    {
        cameraRenderer.takePhoto()
    }

    shutterButton.onStateChanged:
    {
        //while shutter button is triggered
        if(shutterButton.state != "idle")
        {
            form.state = "snapshot"
        }
    }

    cameraRenderer.onSavedPhoto:
    {
        captured(filename)
    }

    cameraRenderer.onFailed:
    {
        shutterButton.reset()
        failureAnimation.start()
    }

    SequentialAnimation
    {
        id: failureAnimation
        running: false
        loops: 1
        PropertyAnimation { target: failureText; property: "visible"; to: true}
        PropertyAnimation { target: failureText; property: "opacity"; to: 1.0}
        NumberAnimation { target: failureText; property: "opacity"; to: 0.0; duration: 2000}
        PropertyAnimation { target: failureText; property: "visible"; to: false}
        PropertyAnimation { target: form; property: "state"; to: "activated"}
    }

    Behavior on snapshotSettings.opacity {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
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
