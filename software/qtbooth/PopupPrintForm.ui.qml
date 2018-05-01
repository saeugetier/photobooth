import QtQuick 2.5
import QtQuick.Controls 2.0

Popup {
    property alias fullScaleImage: fullScaleImage
    property alias collageImage : collageImage
    property alias printBusyIndicator: busyIndicator
    property alias states: printTypeProxy.states
    property alias state: printTypeProxy.state
    property alias busyIndicator: busyIndicator

    property alias printButton: printButton
    property alias cancelButton: cancelButton

    Item
    {
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

        CollageImage
        {
            id: collageImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit

            onSourceChanged:
            {
                    busyIndicator.running = false
            }
        }
    }

    ToolButton {
        id: printButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 50
        anchors.bottom: parent.bottom
        text: "\uE802" // icon-print
        font.family: "fontello"
        font.pointSize: 72
        enabled: true
    }

    ToolButton {
        id: cancelButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -50
        anchors.bottom: parent.bottom
        text: "\uE81F" // icon-cross
        font.family: "fontello"
        font.pointSize: 72
        enabled: true
    }

    BusyIndicator
    {
        id: busyIndicator
        anchors.centerIn: parent
    }
}
