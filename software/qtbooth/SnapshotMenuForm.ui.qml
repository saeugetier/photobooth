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
}

/*##^##
Designer {
    D{i:1;anchors_height:138;anchors_width:350;anchors_x:174;anchors_y:129}D{i:3;anchors_x:19;anchors_y:15}
}
##^##*/

