import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "content"

Item {
    id: menu
    property alias collageRenderer: collageRenderer
    property alias printButton: printButton
    property alias busyIndicator: busyIndicator
    property alias nextButton: nextButton
    property alias exitButton: exitButton
    property real printerRatio: 3 / 4
    property alias printerPopup: printerPopup
    property alias plusButton: plusButton
    property alias minusButton: minusButton
    property alias printCountTumbler: printCountTumbler
    property bool showPrintButton : true

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
        property double calculatedWidth: parent.width - rightMargin - leftMargin
        property double calculatedHeight: parent.height - bottomMargin - topMargin

        property double availableWidth: calculatedWidth > 10 ? calculatedWidth : 10
        property double availableHeight: calculatedHeight > 10 ? calculatedHeight : 10

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
            text: showPrintButton ? qsTr("Print") : qsTr("Save")
            font.family: "DejaVu Serif"
            wrapMode: Text.WrapAnywhere
            font.pixelSize: 64
            font.capitalization: Font.AllUppercase
        }

        ToolButton {
            id: printButton
            text: showPrintButton ? "\uE802" : "\uE803" // icon-print or icon-floppy
            font.family: "fontello"
            font.pixelSize: 64
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
        text: qsTr("Next Photo")
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        forward: false
    }

    NavigationButton {
        id: exitButton
        y: 515
        text: qsTr("Exit")
        visible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        forward: false
    }

    ToolButton {
        id: minusButton
        text: "\uE814" // icon-minus
        font.family: "fontello"
        font.pixelSize: 48
        enabled: true

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 25
        anchors.rightMargin: 15
        anchors.right: printCountTumbler.left

        scale: hovered ? 1.1 : 1

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    Tumbler {
        height: 80
        id: printCountTumbler
        model: ["1", "2", "3", "4"]
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        wrap: false
        visibleItemCount: 1
        delegate: Text {
            text: modelData
            font.pixelSize: 64
            color: "#ffffff"
            font.family: "DejaVu Serif"
            opacity: 0.4 + Math.max(0, 1 - Math.abs(Tumbler.displacement)) * 0.6
        }
    }

    ToolButton {
        id: plusButton
        text: "\uE813" // icon-plus
        font.family: "fontello"
        font.pixelSize: 48
        enabled: true

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 25
        anchors.left: printCountTumbler.right

        scale: hovered ? 1.1 : 1

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    PrinterPopup {
        id: printerPopup
        anchors.centerIn: parent
        visible: false
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
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

