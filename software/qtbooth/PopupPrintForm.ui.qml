import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0

Popup {
    id: popup
    property alias fullScaleImage: fullScaleImage
    property alias collageImage: collageImage
    property alias states: printTypeProxy.states
    property alias state: printTypeProxy.state
    property alias busyIndicator: busyIndicator

    property alias printButton: printButton
    property alias cancelButton: cancelButton

    property alias printerBusyIndicator: printerBusyIndicator
    property alias printerBusyState: printerBusyIndicator.state
    property alias printerBusyStates: printerBusyIndicator.states

    Item {
        anchors.fill: parent

        id: printTypeProxy
        Image {
            anchors.fill: parent
            id: fullScaleImage
            source: fileNameFullScale
            asynchronous: true
            sourceSize.height: 1024
            sourceSize.width: 1024
            fillMode: Image.PreserveAspectFit
        }

        CollageImage {
            id: collageImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
        }
    }

    ToolButton {
        id: printButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 150
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
        id: cancelButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -150
        anchors.bottom: parent.bottom
        text: "\uE81F" // icon-cross
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

    Item {
        id: printerBusyIndicator
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 400
        visible: false

        RowLayout {
            BusyIndicator {
                width: 50
            }
            Text {
                text: qsTr("Printer is busy")
                font.pointSize: 32
                color: "white"
            }
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
