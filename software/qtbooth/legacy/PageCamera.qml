import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtMultimedia 5.5


PageCameraForm {
    id: cameraPage

    property var locale: Qt.locale()
    property alias settingsPopup: settingsPopup
    cameraCountdown.defaultSeconds: settingsPopup.settingCountdown

    Timer
    {
        id: cameraDiscoveryTimer

        interval: 1000
        repeat: true

        onTriggered:
        {
            var availableCameras = QtMultimedia.availableCameras
            printDevicesToConsole(availableCameras)

            if(availableCameras.length > 0)
            {
                camera.deviceId = availableCameras[0].deviceId
                camera.start()
            }

        }
    }

    cameraSettingsButton.onClicked:
    {
        settingsPopup.open()
    }

    cameraShutterButton.onClicked:
    {
        cameraTimeoutTimer.restart()
        cameraCountdown.start()
        tabBar.enabled = false
        settingsButton.enabled = false
        cameraShutterButton.enabled = false
        animatedProgressBar.value = 1 / cameraCountdown.defaultSeconds
    }

    cameraCountdown.onTimeout:
    {
        if(camera.imageCapture.ready)
        {
            camera.imageCapture.captureToLocation(applicationSettings.foldername.substring(7, applicationSettings.foldername.length) + "/Pict_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg")
            if(settingsPopup.settingFlashEnable)
            {
                //flash.setFlash(true)
                flash.setBrightness(settingsPopup.settingFlashBrightness)
                whiteOverlay.state = "triggered"
            }
        }
        else
        {
            //flash.setFlash(false)
            flash.setBrightness(settingsPopup.settingBrightness)
            whiteOverlay.state = "released"
            failureText.visible = true;
            failureTimeout.start()
            tabBar.enabled = true
            settingsButton.enabled = true
            cameraShutterButton.enabled = true
        }
    }

    cameraCountdown.onSecondsChanged:
    {
        if(cameraCountdown.seconds === cameraCountdown.defaultSeconds)
        {
            animatedProgressBar.value = 0.0
        }
        else
        {
            var progress = 1 - (cameraCountdown.seconds -1)/cameraCountdown.defaultSeconds
            if(progress > 1.0)
                progress = 1.0
            animatedProgressBar.value = progress
        }
    }

    camera.onError:
    {
        console.log("Camera Error: " + errorString)
    }

    camera.imageProcessing.whiteBalanceMode: settingsPopup.cameraPrintSettings.whiteBalance

    camera.imageProcessing.onWhiteBalanceModeChanged:
    {
        console.log("Changed camera white balance to: " + camera.imageProcessing.whiteBalanceMode)
        console.log("Setting is " + settingsPopup.cameraPrintSettings.whiteBalance)
    }

    camera.onCameraStateChanged:
    {
        if(camera.cameraState == Camera.UnloadedState)
        {
            console.log("Camera State Changed: Unloaded")
            printDevicesToConsole(QtMultimedia.availableCameras)
            camera.stop()
            cameraDiscoveryTimer.start()
        }
        else if(camera.cameraState == Camera.LoadedState)
        {
            console.log("Camera State Changed: Loaded")
            printDevicesToConsole(QtMultimedia.availableCameras)
        }
        else if(camera.cameraState == Camera.ActiveState)
        {
            console.log("Camera State Changed: Active");
            cameraDiscoveryTimer.stop()
        }
        else
        {
            console.log("Camera State Changed: Unknown");
        }
    }

    function printDevicesToConsole(devices)
    {
        console.log("Found " + devices.length + " camera devices!")
        for(var i = 0; i < devices.length; i++)
        {
            console.log("Found device: " + devices[i].deviceId + " with number " + i);
        }
    }

    camera.imageCapture.onImageCaptured:
    {
        //flash.setFlash(false)
        flash.setBrightness(settingsPopup.settingBrightness)
        whiteOverlay.state = "released"
        previewPopup.showImage(preview)
        tabBar.enabled = true
        settingsButton.enabled = true
        cameraShutterButton.enabled = true
    }

    camera.imageCapture.onImageSaved:
    {
        previewPopup.setFileName(camera.imageCapture.capturedImagePath)
        console.log(camera.imageCapture.capturedImagePath)
    }

    camera.imageCapture.onCaptureFailed:
    {
        //flash.setFlash(false)
        flash.setBrightness(settingsPopup.settingBrightness)
        whiteOverlay.state = "released"
        failureText.visible = true;
        failureTimeout.start()
        tabBar.enabled = true
        settingsButton.enabled = true
        cameraShutterButton.enabled = true
    }

    Text
    {
        id: failureText
        anchors.centerIn: parent
        visible: false
        text: qsTr("Please try again")
        font.pointSize: 72
        color: "white"

        Timer
        {
            id: failureTimeout
            interval: 1000
            onTriggered: {
                failureText.visible = false
            }
        }
    }

    PopupPreviewImage{
        id: previewPopup
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        onImageAccepted:
        {
            if(settingsPopup.settingPrintEnable == true)
                printPopup.newPhoto("file://" + previewPopup.currentFileName)
            console.log("Accepted: " + previewPopup.currentFileName)
        }

        onImageRejected:
        {
            helper.removeFile(filename, applicationSettings.recycleBinEnabled)
        }
    }

    PopupPrint {
        id: printPopup
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        state: settingsPopup.settingPrintFullscale ? "fullscale" : "collage"

        onPrinterBusyStateChanged:
        {
            console.log("Printer State changed " + printPopup.printerBusyState)
            if(printPopup.printerBusyState == "busy")
            {
                cameraPage.printerBusyIndicator.visible = true
            }
            else
            {
                cameraPage.printerBusyIndicator.visible = false
            }
        }
    }


    PopupCameraSettings
    {
        id: settingsPopup
        x: 0
        y: 0
        width: parent.width
        height: parent.height
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/