import QtQuick 2.4
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.2
import "content"

Popup {
    id: popup

    property alias effectGridView: effectGridView
    property alias effectGridScrollBar: scrollbar
    property alias closeButton: closeButton
    property alias downButton: downButton
    property alias upButton: upButton
    modal: true
    padding: 10

    Material.elevation: 6
    Material.theme: Material.Light
    Material.background: Material.Grey

    closePolicy: Popup.CloseOnEscape

    ColumnLayout {
        anchors.fill: parent

        Button {
            id: upButton
            Layout.alignment: Qt.AlignTop
            text: "\uE819"
            flat: true
            font.family: "fontello"
            font.pixelSize: 24
            Layout.fillWidth: true
            autoRepeat: true
            z: 1
        }

        GridView {
            id: effectGridView

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillHeight: true
            Layout.fillWidth: true

            anchors.leftMargin: 40
            anchors.bottomMargin: 40
            anchors.topMargin: 40
            anchors.rightMargin: 40
            clip: true
            transformOrigin: Item.Center

            cellHeight: height / 2
            cellWidth: width / 3

            opacity: 1

            z: 0.5

            ScrollBar.vertical: ScrollBar {
                id: scrollbar
                parent: effectGridView
                interactive: false
            }

            model: EffectList {}
        }

        Button {
            id: downButton
            Layout.alignment: Qt.AlignBottom
            text: "\uE81C"
            flat: true
            font.family: "fontello"
            font.pixelSize: 24
            Layout.fillWidth: true
            autoRepeat: true
            z: 1
        }
    }

    ToolButton {
        id: closeButton
        anchors.right: parent.right
        anchors.top: parent.top
        text: "\uE81F" // cancel button
        font.family: "fontello"
        font.pixelSize: 64
        enabled: true
        z: 1
        visible: true

        Material.foreground: Material.Red

        opacity: 1
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.25;height:480;width:640}
}
##^##*/

