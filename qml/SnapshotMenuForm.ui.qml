import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias failureText: failureText
    property alias snapshotSettings: snapshotSettings
    property alias exitButton: exitButton
    property alias cameraRenderer: cameraRenderer
    property alias shutterButton: shutterButton
    property alias countdown: shutterButton.countDownTime

    CameraRenderer {
        id: cameraRenderer
        anchors.fill: parent

        Text {
            id: failureText
            x: 308
            y: 233
            text: qsTr("Please Try Again")
            font.bold: true
            visible: false
            font.family: "DejaVu Serif"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 42
        }
    }

    ShutterButton {
        id: shutterButton
        width: 150
        height: 150
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
    }

    NavigationButton {
        id: exitButton
        text: "Abort"
        visible: false
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 30
        forward: false
    }

    SnapshotSettings {
        id: snapshotSettings
        visible: false
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
    }
    states: [
        State {
            name: "deactivated"
        },
        State {
            name: "activated"

            PropertyChanges {
                target: snapshotSettings
                visible: true
            }

            PropertyChanges {
                target: exitButton
                visible: true
            }
        },
        State {
            name: "snapshot"
        }
    ]
}
