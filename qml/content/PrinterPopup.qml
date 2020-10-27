import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

Item {
    id: container

    width: 300
    height: 300

    Rectangle {
        anchors.fill: parent
        radius: 20
        color: "black"
        opacity: 0.5
    }

    BusyIndicator
    {
        anchors.centerIn: parent

        width: 200
        height: 200
        running: true
    }

    Text {
        anchors.centerIn: parent

        color: "white"
        text: "\uE802" // icon-print
        font.family: "fontello"
        font.pixelSize: 80

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }
}
