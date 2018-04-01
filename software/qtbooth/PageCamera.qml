import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import Qt.labs.platform 1.0
import QtMultimedia 5.9

PageCameraForm {
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
        camera.imageCapture.captureToLocation(applicationSettings.foldername.substring(7, applicationSettings.foldername.length) + "/Pict "+ new Date().toLocaleString(Locale.NarrowFormat) + ".jpg")
    }

    camera.imageCapture.onImageCaptured:
    {
        previewPopup.showImage(preview)
    }

    camera.imageCapture.onImageSaved:
    {
        previewPopup.setFileName(camera.imageCapture.capturedImagePath)
        console.log(camera.imageCapture.capturedImagePath)
    }

    camera.imageCapture.onCaptureFailed:
    {
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
                printPopup.open()
            console.log("Accepted")
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

    }

    PopupCameraSettings
    {
        id: settingsPopup
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        onChangedPrintFullscreen:
        {

        }
    }
}
