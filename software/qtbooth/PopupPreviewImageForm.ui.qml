import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Popup {
    property alias imageSaveButton: saveButton
    property alias imageDeleteButton: deleteButton
    property alias previewImage: popupImage
    property alias imageFileSaveIndicator: fileSaveIndicator

    Image {
        anchors.fill: parent
        id: popupImage
        fillMode: Image.PreserveAspectFit
    }

    ToolButton {
        id: saveButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -50
        anchors.bottom: parent.bottom
        text: "\uE803" // icon-save
        font.family: "fontello"
        font.pointSize: 72
    }
    ToolButton {
        id: deleteButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 50
        anchors.bottom: parent.bottom
        text: "\uE800" // icon-folder-open-empty
        font.family: "fontello"
        font.pointSize: 72
    }

    BusyIndicator
    {
        id: fileSaveIndicator
        anchors.centerIn: parent
    }
}