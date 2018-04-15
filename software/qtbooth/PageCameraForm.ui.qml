import QtQuick 2.5
import QtMultimedia 5.5
import QtQuick.Controls 2.0

Item {
    property alias cameraShutterButton: shutterButton
    property alias cameraSettingsButton: settingsButton
    property alias camera: camera
    property alias cameraCountdown: countdown

    Camera {
        id: camera

        imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture
        {

        }
    }

    VideoOutput {
        id: output
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible
    }

    ToolButton {
        id: shutterButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        text: "\uE801" // icon-folder-open-empty
        font.family: "fontello"
        font.pointSize: 72
        //onClicked: camera.imageCapture.capture()
    }

    ToolButton {
        id: settingsButton
        anchors.right: parent.right
        anchors.top: parent.top
        text: "\uE80F" // icon-folder-open-empty
        font.family: "fontello"
        font.pointSize: 36
    }

    Countdown
    {
        id: countdown
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

}
