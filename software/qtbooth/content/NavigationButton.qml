import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Row {
    id: row

    signal clicked

    property alias text: brewLabel.text

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
        id: brewLabel
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
        source: row.forward ? "../images/buttons/go/white.png" : "../images/buttons/back/white.png"
        scale: mouseArea.containsMouse ? 1.1 : 1
        MouseArea {
            id: mouseArea
            hoverEnabled: true

            anchors.fill: parent
            Connections {
                target: mouseArea
                onClicked: row.clicked()
            }
        }
    }
}
