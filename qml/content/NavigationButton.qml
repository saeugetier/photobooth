import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Row {
    id: row

    signal clicked

    property alias text: textLabel.text

    property bool forward: true
    layoutDirection: row.forward ? Qt.LeftToRight : Qt.RightToLeft

    spacing: 17

    layer.enabled: true
    layer.effect: Glow {
        color: "black"
        samples: 20
        spread: 0.3
    }

    Text {
        id: textLabel
        color: "#ffffff"
        text: qsTr("Continue")
        font.family: "DejaVu Serif"
        wrapMode: Text.WrapAnywhere
        font.pixelSize: 64
        font.capitalization: Font.AllUppercase
    }

    Image {
        id: image

        anchors.verticalCenter: parent.verticalCenter
        source: row.forward ? "../../images/buttons/go/white.png" : "../../images/buttons/back/white.png"
        scale: mouseArea.containsMouse ? 1.1 : 1
        MouseArea {
            id: mouseArea
            hoverEnabled: true

            anchors.fill: parent
            Connections {
                target: mouseArea
                function onClicked(mouse) { row.clicked() }
            }
        }
    }
}
