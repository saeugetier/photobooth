import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias effectButton: effectButton
    property alias effectSelector: effectSelector
    property alias deleteButton: deleteButton
    property alias saveButton: saveButton
    property alias fileLoadIndicator: fileLoadIndicator
    property alias previewImage: previewImage
    property string shaderName: "passthrough.fsh"

    Image {
        id: previewImage
        anchors.topMargin: 100
        anchors.bottomMargin: 100
        anchors.leftMargin: 100
        anchors.rightMargin: 100
        anchors.fill: parent
        rotation: -10
        fillMode: Image.PreserveAspectFit
        layer.enabled: true
        layer.effect: ImageEffect {
            clip: true
            source: previewImage
            fragmentShaderFilename: shaderName
        }
    }

    BorderImage {
        anchors.horizontalCenter: previewImage.horizontalCenter
        anchors.verticalCenter: previewImage.verticalCenter
        id: borderImage
        border.bottom: 65
        border.top: 25
        border.right: 25
        border.left: 25
        rotation: -10
        width: previewImage.paintedWidth + 40
        height: previewImage.paintedHeight + 60
        anchors.verticalCenterOffset: 10
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        source: "../../images/polaroid.svg.png"
    }

    BusyIndicator {
        id: fileLoadIndicator
        anchors.verticalCenterOffset: 0
        anchors.centerIn: parent
    }

    NavigationButton {
        id: saveButton
        x: 122
        y: 453
        text: "Accept"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 40
    }

    NavigationButton {
        id: deleteButton
        text: "Retry"
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40
        forward: false
    }

    EffectSelector {
        id: effectSelector

        visible: false

        anchors.right: element.right
        anchors.top: element.top
        anchors.bottom: element.bottom

        previewImage: previewImage.source
    }

    Row {
        id: effectButtonRow
        visible: true
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40

        spacing: 10

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        Text {
            id: textLabelEffectButton
            color: "#ffffff"
            text: qsTr("Effect")
            font.family: "DejaVu Serif"
            wrapMode: Text.WrapAnywhere
            font.pixelSize: 64
            font.capitalization: Font.AllUppercase
        }

        ToolButton {
            id: effectButton
            text: "\uF0D0" // icon-print
            font.family: "fontello"
            font.pointSize: 82
            enabled: true

            scale: hovered ? 1.1 : 1

            layer.enabled: true
            layer.effect: Glow {
                color: "black"
                samples: 20
                spread: 0.3
            }
        }
    }

    states: [
        State {
            name: "idle"
        },
        State {
            name: "effectSelection"

            PropertyChanges {
                target: effectSelector
                visible: true
            }

            PropertyChanges {
                target: effectButtonRow
                visible: false
            }

            PropertyChanges {
                target: saveButton
                visible: false
            }

            PropertyChanges {
                target: deleteButton
                visible: false
            }
        }
    ]
}

/*##^##
Designer {
    D{i:1;anchors_height:342;anchors_width:484;anchors_x:73;anchors_y:64}
}
##^##*/

