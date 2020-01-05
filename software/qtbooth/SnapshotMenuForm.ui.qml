import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0

Item {
    width: 640
    height: 480

    ToolButton {
        id: shutterButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        text: "\uE801"
        font.family: "fontello"
        font.pointSize: 104

        opacity: 0.7

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        //layer.enabled: true
        //layer.effect: ShaderEffect {
        //    blending: true
        //}
    }
}
