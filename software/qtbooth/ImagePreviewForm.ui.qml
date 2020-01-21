import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

Item {
    width: 640
    height: 480
    property alias fileLoadIndicator: fileLoadIndicator
    property alias deleteButton: deleteButton
    property alias saveButton: saveButton
    property alias previewImage: previewImage

    Image {
        id: previewImage
        x: 73
        y: 64
        width: 484
        height: 342
        fillMode: Image.PreserveAspectFit
    }

    ToolButton {
        id: saveButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -150
        anchors.bottom: parent.bottom
        text: "\uE803" // icon-save
        font.family: "fontello"
        font.pointSize: 72

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }
    ToolButton {
        id: deleteButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 150
        anchors.bottom: parent.bottom
        text: "\uE800" // icon-folder-open-empty
        font.family: "fontello"
        font.pointSize: 72

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    BusyIndicator {
        id: fileLoadIndicator
        anchors.centerIn: parent
    }
}
