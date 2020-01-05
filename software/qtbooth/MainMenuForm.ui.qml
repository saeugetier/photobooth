import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0

Item {
    width: 640
    height: 480
    property alias iconModel: collageSelector.iconModel

    ToolButton {
        id: settingsButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "\uE80F"
        font.family: "fontello"
        font.pointSize: 36

        opacity: 0.8

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        //layer.effect: ShaderEffect {
        //    blending: true
        //}
    }

    CollageSelector {
        id: collageSelector
        x: 0
        y: 0
        height: parent.height
        width: 300
    }
}
