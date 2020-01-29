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
    property real printerRatio: 3 / 4

    Rectangle {
        color: "white"
        anchors.verticalCenter: collageRenderer.verticalCenter
        anchors.horizontalCenter: collageRenderer.horizontalCenter
        width: collageRenderer.width + 20
        height: collageRenderer.height + 20
    }

    CollageRenderer {
        id: collageRenderer
        // INPUTS
        property double rightMargin: 50
        property double bottomMargin: 150
        property double leftMargin: 50
        property double topMargin: 30
        property double aimedRatio: printerRatio

        // SIZING
        property double availableWidth: parent.width - rightMargin - leftMargin
        property double availableHeight: parent.height - bottomMargin - topMargin

        property bool parentIsLarge: parentRatio > aimedRatio

        property double parentRatio: availableHeight / availableWidth

        height: parentIsLarge ? width * aimedRatio : availableHeight
        width: parentIsLarge ? availableWidth : height / aimedRatio

        property double verticalSpacing: (availableHeight - height) / 2
        property double horzitontalSpacing: (availableWidth - width) / 2

        anchors.top: parent.top
        anchors.topMargin: topMargin + verticalSpacing
        anchors.left: parent.left
        anchors.leftMargin: leftMargin + horzitontalSpacing
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

    Text {
        id: element1
        x: 200
        y: 197
        text: qsTr("Printer is still busy")
        visible: false
        font.pixelSize: 12
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

            PropertyChanges {
                target: element1
                visible: printer.busy
            }
        }
    ]
}
/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

