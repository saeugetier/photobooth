import QtQuick 2.4
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import "content"

Popup {
    id: popup

    property alias effectGridView: effectGridView
    property alias closeButton: closeButton
    modal: true
    padding: 10

    Material.elevation: 6
    Material.theme: Material.Light
    Material.background: Material.Grey

    closePolicy: Popup.CloseOnEscape

    GridView {
        id: effectGridView

        anchors.fill: parent
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

        model: EffectList {}
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
    D{i:0;autoSize:true;formeditorZoom:3;height:480;width:640}
}
##^##*/

