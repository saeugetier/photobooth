import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtMultimedia 5.5


PageCameraForm {
    property var locale: Qt.locale()

    cameraCountdown.defaultSeconds: settingsPopup.settingCountdown

    cameraSettingsButton.onClicked:
    {
        settingsPopup.open()
    }

    cameraShutterButton.onClicked:
    {
        cameraCountdown.start()
    }

    cameraCountdown.onTimeout:
    {
        camera.imageCapture.captureToLocation(applicationSettings.foldername.substring(7, applicationSettings.foldername.length) + "/Pict_"+ new Date().toLocaleString(locale, "dd.MM.yyyy_hh:mm:ss") + ".jpg")
        if(settingsPopup.settingFlashEnable)
        {
            flash.setFlash(true)
        }
    }

    camera.imageCapture.onImageCaptured:
    {
        flash.setFlash(false)
        previewPopup.showImage(preview)
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
