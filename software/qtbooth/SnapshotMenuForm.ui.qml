import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias exitButton: exitButton
    property alias cameraRenderer: cameraRenderer
    property alias shutterButton: shutterButton
    property alias countdown: shutterButton.countDownTime

    CameraRenderer {
        id: cameraRenderer
        anchors.fill: parent
    }

    ShutterButton {
        id: shutterButton
        x: 268
        y: 356
        width: 150
        height: 150
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
    }

    NavigationButton {
        id: exitButton
        text: "Abort"
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 30
        forward: false
    }
    states: [
        State {
            name: "deactivated"
        },
        State {
            name: "activated"
        }
    ]
}



