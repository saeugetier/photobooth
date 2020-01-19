import QtQuick 2.5
import QtMultimedia 5.5
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.2
import "content"

Item {
    property alias cameraShutterButton: shutterButton
    property alias cameraSettingsButton: settingsButton
    property alias camera: camera
    property alias cameraCountdown: countdown
    property alias settingsButton: settingsButton
    property alias printerBusyIndicator: printerBusyIndicator
    property alias animatedProgressBar: animatedProgressBar
    property alias whiteOverlay: whiteOverlay

    Camera {
        id: camera

        position: Camera.FrontFace

        //imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
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

    WhiteOverlay
    {
        id: whiteOverlay
        x: output.x
        y: output.y
        width: output.width
        height: output.height
    }

    ToolButton {
        id: shutterButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        text: "\uE801"
        font.family: "fontello"
        font.pointSize: 104

        opacity: 0.7

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

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

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        //layer.effect: ShaderEffect {
        //    blending: true
        //}
    }

    Countdown {
        id: countdown
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    Item {
        id: printerBusyIndicator
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 120
        anchors.bottomMargin: 60
        visible: false

        RowLayout {
            BusyIndicator {
                width: 30
                height: 30
            }
            Text {
                text: "\uE802" // icon-print
                font.family: "fontello"
                font.pointSize: 48
                color: "white"
            }
        }
    }

    AnimatedProgressBar {
        id: animatedProgressBar
        width: parent.width
        anchors.bottom: parent.bottom
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
