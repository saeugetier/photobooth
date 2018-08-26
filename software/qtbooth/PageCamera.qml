import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtMultimedia 5.5


PageCameraForm {
    property var locale: Qt.locale()

    onFocusChanged:
    {
        console.log("Focus of camera page changed. Focus: " + focus)
        if(focus)
        {
            cameraTimeoutTimer.restart()
            flash.triggerFocus()
            delay(1000, function() {
                camera.start()
            });
            flash.setBrightness(settingsPopup.cameraPrintSettings.brightness)
        }
        else if(swipeView.currentIndex != 0) //ignore popups in current swipe view
        {
            flash.setBrightness(0)
            cameraTimeoutTimer.stop()
            camera.stop()
        }
    }

    Timer
    {
        id: cameraTimeoutTimer
        interval: 1000 * 60 * 4  //going inactive after 4 minutes

        Component.onCompleted:
        {
            cameraTimeoutTimer.start()
        }

        onTriggered:
        {
            console.log("Focus Camera")
            camera.stop()
            swipeView.setCurrentIndex(swipeView.count - 1)
        }
    }

    cameraCountdown.defaultSeconds: settingsPopup.settingCountdown

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
    }

    cameraCountdown.onTimeout:
    {
        if(camera.imageCapture.ready)
        {
            camera.imageCapture.captureToLocation(applicationSettings.foldername.substring(7, applicationSettings.foldername.length) + "/Pict_"+ new Date().toLocaleString(locale, "dd.MM.yyyy_hh:mm:ss") + ".jpg")
            if(settingsPopup.settingFlashEnable)
            {
                flash.setFlash(true)
            }
        }
        else
        {
            flash.setFlash(false)
            failureText.visible = true;
            failureTimeout.start()
            tabBar.enabled = true
            settingsButton.enabled = true
            cameraShutterButton.enabled = true
        }
    }

    Timer
    {
        id: delayTimer
    }

    function delay(delayTime, cb) {
        delayTimer.interval = delayTime;
        delayTimer.repeat = false;
        delayTimer.triggered.connect(cb);
        delayTimer.start();
    }

    camera.onError:
    {
        console.log("Camera Error: " + camera.errorString)
        camera.stop()
        flash.triggerFocus()
        delay(1000, function() {
            camera.start()
        });
    }

    camera.imageCapture.onImageCaptured:
    {
        flash.setFlash(false)
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
        flash.setFlash(false)
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
            helper.removeFile(filename)
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
