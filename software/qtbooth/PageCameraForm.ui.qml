import QtQuick 2.5
import QtMultimedia 5.5
import QtQuick.Controls 2.0

Item {
    property alias cameraShutterButton: shutterButton
    property alias cameraSettingsButton: settingsButton
    property alias camera: camera
    property alias cameraCountdown: countdown
    property alias settingsButton: settingsButton

    Camera {
        id: camera

        position: Camera.FrontFace

        imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
        }
    }

    VideoOutput {
        id: output
        source: camera
        anchors.fill: parent
        focus: visible // to receive focus and capture key events when visible
    }

    ToolButton {
        id: shutterButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        text: "\uE801"
        font.family: "fontello"
        font.pointSize: 104

        opacity: 0.7

        //layer.enabled: true
        //layer.effect: ShaderEffect {
        //    blending: true
        //}
    }

    ToolButton {
        id: settingsButton
        anchors.right: parent.right
        anchors.top: parent.top
        text: "\uE80F"
        font.family: "fontello"
        font.pointSize: 36

        opacity: 0.8

        //layer.effect: ShaderEffect {
        //    blending: true
        //}
    }

    Countdown {
        id: countdown
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
