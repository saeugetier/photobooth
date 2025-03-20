import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias printerBusy: printerPopup.visible
    property alias settingsButton: settingsButton
    property alias collageRenderer: collageRenderer
    property alias backButton: backButton
    property alias continueButton: continueButton
    property alias imageSlider: imageSlider
    property alias collageSelector: collageSelector
    property alias iconModel: collageSelector.iconModel
    property alias galleryButton: galleryButton

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
        source: "../images/cardboard.png"
        fillMode: Image.Tile

        ListView {
            id: imageSlider
            interactive: false
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            spacing: height / 8

            delegate: Image {
                id: randomImage
                property double rightMargin: 50
                property double bottomMargin: 150
                property double leftMargin: 50
                property double topMargin: 30
                property double aimedRatio: 2380 / 3570

                // SIZING
                property double availableWidth: imageSlider.width - rightMargin - leftMargin
                property double availableHeight: imageSlider.height - bottomMargin - topMargin

                property bool parentIsLarge: parentRatio > aimedRatio

                property double parentRatio: availableHeight / availableWidth

                height: parentIsLarge ? width * aimedRatio : availableHeight
                width: parentIsLarge ? availableWidth : height / aimedRatio

                property double verticalSpacing: (availableHeight - height) / 2
                property double horzitontalSpacing: (availableWidth - width) / 2

                x: leftMargin + horzitontalSpacing

                source: fileURL
                sourceSize.height: 512
                sourceSize.width: 512
                fillMode: Image.PreserveAspectFit
                transform: Rotation {
                    origin.x: imageSlider.width / 4
                    origin.y: imageSlider.height / 4
                    axis {
                        x: 0
                        y: 0
                        z: 1
                    }
                    angle: (Math.random() * 60) - 30
                }
                BorderImage {
                    anchors.horizontalCenter: randomImage.horizontalCenter
                    anchors.verticalCenter: randomImage.verticalCenter
                    id: borderImage
                    border.bottom: 65
                    border.top: 25
                    border.right: 25
                    border.left: 25
                    width: randomImage.paintedWidth + 40
                    height: randomImage.paintedHeight + 60
                    anchors.verticalCenterOffset: 10
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    source: "../images/polaroid.svg.png"
                }
            }
        }

        CollageRenderer {
            id: collageRenderer

            property double rightMargin: 50
            property double bottomMargin: 120
            property double leftMargin: 50
            property double topMargin: 120
            property double aimedRatio: 2380 / 3570

            // SIZING
            property double availableWidth: parent.width - rightMargin - leftMargin
            property double availableHeight: parent.height - bottomMargin - topMargin

            property bool parentIsLarge: parentRatio > aimedRatio

            property double parentRatio: availableHeight / availableWidth

            height: parentIsLarge ? availableWidth * aimedRatio : availableHeight
            width: parentIsLarge ? availableWidth : availableHeight / aimedRatio

            property double verticalSpacing: (availableHeight - height) / 2
            property double horzitontalSpacing: (availableWidth - width) / 2

            anchors.top: parent.top
            anchors.topMargin: topMargin + verticalSpacing
            anchors.left: parent.left
            anchors.leftMargin: leftMargin + horzitontalSpacing

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
        visible: true
        font.family: "fontello"
        font.pixelSize: 42

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
        id: galleryButton
        text: qsTr("Gallery")
        anchors.bottom: parent.bottom
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

    PrinterPopup {
        id: printerPopup
        anchors.centerIn: parent
    }

    states: [
        State {
            name: "NoIconSelected"
            PropertyChanges {
                target: imageSlider
                visible: true
            }

            PropertyChanges {
                target: settingsButton
                visible: true
            }

            PropertyChanges {
                target: galleryButton
                visible: true
            }
        },
        State {
            name: "IconSelected"
            PropertyChanges {
                target: imageSlider
                visible: false
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

            PropertyChanges {
                target: collageRenderer
                visible: true
            }

            PropertyChanges {
                target: settingsButton
                visible: false
            }

            PropertyChanges {
                target: galleryButton
                visible: false
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:480;width:640}D{i:15}D{i:14}D{i:7}
}
##^##*/

