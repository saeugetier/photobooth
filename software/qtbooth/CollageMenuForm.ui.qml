import QtQuick 2.4
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    property alias collageRenderer: collageRenderer
    property alias printButton: printButton
    property alias busyIndicator: busyIndicator
    property alias okButton: okButton

    CollageRenderer {
        id: collageRenderer
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 150
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.top: parent.top
        anchors.topMargin: 30
    }

    ToolButton {
        id: printButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 250
        anchors.bottom: parent.bottom
        text: "\uE802" // icon-print
        font.family: "fontello"
        font.pointSize: 72
        enabled: true

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    ToolButton {
        id: okButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -250
        anchors.bottom: parent.bottom
        text: "\uE81E" // icon-ok
        font.family: "fontello"
        font.pointSize: 72
        enabled: true

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        visible: false
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1;anchors_height:300;anchors_width:400;anchors_x:103;anchors_y:74}
}
##^##*/

