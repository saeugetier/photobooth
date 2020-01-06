import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0

Item {
    id: element
    width: 640
    height: 480
    property alias backButton: backButton
    property alias continueButton: continueButton
    property alias labelIconName: labelIconName
    property alias imageSlider: imageSlider
    property alias collageSelector: collageSelector
    property alias iconModel: collageSelector.iconModel

    CollageSelector {
        id: collageSelector
        x: 0
        y: 0
        height: parent.height
        width: 300
    }

    Image {
        id: background
        anchors.top: parent.top
        anchors.left: collageSelector.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        source: "images/cardboard.png"
        fillMode: Image.Tile

        ListView {
            id: imageSlider
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            spacing: height / 8

            delegate: Image {
                height: imageSlider.height / 2
                width: imageSlider.width / 2
                x: imageSlider.width / 4
                source: fileURL
                fillMode: Image.PreserveAspectFit
                transform: Rotation {
                    origin.x: imageSlider.width / 4
                    origin.y: imageSlider.height / 4
                    axis {
                        x: 0
                        y: 0
                        z: 1
                    }
                    angle: (Math.random() * 90) - 45
                }
            }
        }

        Label {
            id: labelIconName
            x: 141
            y: 232
            text: qsTr("Label")
            visible: false
        }
    }

    DropShadow {
        anchors.fill: collageSelector
        horizontalOffset: 3
        verticalOffset: 0
        radius: 8.0
        samples: 17
        color: "#80000000"
        source: collageSelector
    }

    ToolButton {
        id: settingsButton
        anchors.right: parent.right
        anchors.top: parent.top
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

    NavigationButton {
        id: continueButton
        text: qsTr("Continue")
        anchors.bottom: parent.bottom
        visible: false
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 30
    }

    NavigationButton {
        id: backButton
        text: qsTr("Back")
        forward: false
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: background.left
        anchors.leftMargin: 20
        visible: false
    }
    states: [
        State {
            name: "NoIconSelected"
            PropertyChanges {
                target: imageSlider
                visible: true
            }

            PropertyChanges {
                target: labelIconName
                visible: false
            }
        },
        State {
            name: "IconSelected"
            PropertyChanges {
                target: imageSlider
                visible: false
            }

            PropertyChanges {
                target: labelIconName
                visible: true
            }

            PropertyChanges {
                target: continueButton
                visible: true
            }

            PropertyChanges {
                target: backButton
                anchors.leftMargin: 20
                visible: true
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:480;width:640}D{i:11;anchors_x:139;anchors_y:394}
}
##^##*/

