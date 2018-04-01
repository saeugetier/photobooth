import QtQuick 2.4
import QtQuick.Controls 2.1

Popup {
    property alias previewImage: popupImage
    property alias printBusyIndicator: busyIndicator

    Image {
        anchors.fill: parent
        id: popupImage
    }

    ToolButton {
        id: deleteButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 50
        anchors.bottom: parent.bottom
        text: "\uE802" // icon-print
        font.family: "fontello"
        font.pointSize: 72
        enabled: false
    }

    ToolButton {
        id: cameraButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -50
        anchors.bottom: parent.bottom
        text: "\uE801" // icon-camera
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
