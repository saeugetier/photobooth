import QtQuick 2.4
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    property alias collageRenderer: collageRenderer
    property alias printButton: printButton
    property alias busyIndicator: busyIndicator
    property alias nextButton: nextButton
    property alias exitButton: exitButton

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

    Row {
        id: printButtonRow
        visible: false
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        spacing: 10

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        Text {
            id: textLabel
            color: "#ffffff"
            text: qsTr("Print")
            font.family: "DejaVu Serif"
            wrapMode: Text.WrapAnywhere
            font.pixelSize: 64
            font.capitalization: Font.AllUppercase
        }

        ToolButton {
            id: printButton
            text: "\uE802" // icon-print
            font.family: "fontello"
            font.pointSize: 82
            enabled: true

            scale: hovered ? 1.1 : 1

            layer.enabled: true
            layer.effect: Glow {
                color: "black"
                samples: 20
                spread: 0.3
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        visible: false
    }

    NavigationButton {
        id: nextButton
        y: 524
        text: "Next Photo"
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        forward: false
    }

    NavigationButton {
        id: exitButton
        y: 515
        text: "Exit"
        visible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        forward: false
    }
    states: [
        State {
            name: "CollageNotFull"
        },
        State {
            name: "CollageFull"

            PropertyChanges {
                target: exitButton
                visible: true
            }

            PropertyChanges {
                target: printButtonRow
                visible: true
            }

            PropertyChanges {
                target: nextButton
                visible: false
            }
        }
    ]
}
/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1;anchors_height:300;anchors_width:400;anchors_x:103;anchors_y:74}
D{i:8;anchors_x:175}D{i:9;anchors_x:165}
}
##^##*/

