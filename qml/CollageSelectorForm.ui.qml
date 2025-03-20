import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import QtQuick.Controls
import CollageModel

Flickable {
    id: sidebar
    width: 300
    property alias downButton: downButton
    property alias upButton: upButton
    property alias iconListView: iconListView
    property alias scrollIndicator: iconListScrollIndicator

    property CollageIconModel iconModel: ({})

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#4d60ea"
    }

    ColumnLayout {
        anchors.fill: parent

        Button {
            id: upButton
            Layout.alignment: Qt.AlignTop
            text: "\uE819"
            flat: true
            font.family: "fontello"
            font.pixelSize: 54
            Layout.fillWidth: true
            autoRepeat: true
            z: 1
        }

        ListView {
            id: iconListView
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillHeight: true
            spacing: 20
            width: sidebar.width
            height: sidebar.height
            clip: true
            leftMargin: 25
            transformOrigin: Item.Center
            Layout.fillWidth: true

            ScrollBar.vertical: ScrollBar {
                id: iconListScrollIndicator
                parent: iconListView
                interactive: false
                Layout.alignment: Qt.AlignLeft
            }

            model: iconModel
        }

        Button {
            id: downButton
            Layout.alignment: Qt.AlignBottom
            text: "\uE81C"
            flat: true
            font.family: "fontello"
            font.pixelSize: 54
            Layout.fillWidth: true
            autoRepeat: true
            z: 1
        }
    }
}
