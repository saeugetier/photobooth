import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.2
import "content"

Pane {
    id: sidebar
    width: 400
    property alias effectListView: effectListView
    property alias downButton: downButton
    property alias upButton: upButton
    property alias scrollIndicator: effectListScrollIndicator

    background: Rectangle {
            opacity: 0.8
            color: Material.backgroundColor
        }

    z: 0.1

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
            id: effectListView
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillHeight: true
            spacing: 10
            width: sidebar.width
            clip: true
            leftMargin: 25
            transformOrigin: Item.Center
            Layout.fillWidth: true

            opacity: 1

            z: 0.5

            ScrollBar.vertical: ScrollBar {
                id: effectListScrollIndicator
                parent: effectListView
                interactive: false
                Layout.alignment: Qt.AlignLeft
            }

            model: EffectList {}
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
