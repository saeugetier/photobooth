import QtQuick 2.4
import QtQuick.Controls 2.4
import "content"

EffectSelectionPopupForm {
    id: form

    property url previewImage: ""

    effectGridView.delegate: effectDelegate

    signal effectSelected(string effect)

    enter: Transition {
              NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
          }

    exit: Transition {
              NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
          }

    closeButton.onClicked:
    {
        close()
    }

    onOpened:
    {
        effectGridScrollBar.position = 0
    }

    upButton.onClicked:
    {
        effectGridScrollBar.decrease()
    }

    downButton.onClicked:
    {
        effectGridScrollBar.increase()
    }

    Component
    {
        id: effectDelegate

        Item {

            id: item

            width: effectGridView.cellWidth
            height: effectGridView.cellHeight


            Image {
                id: image
                source: previewImage

                fillMode: Image.PreserveAspectFit

                sourceSize.height: 256
                sourceSize.width: 256

                property size imageSize : filesystem.getImageSize(source)
                property double rightMargin: 20
                property double bottomMargin: 20 + label.height
                property double leftMargin: 20
                property double topMargin: 20
                property double aimedRatio: imageSize.height / imageSize.width

                // SIZING
                property double availableWidth: parent.width - rightMargin - leftMargin
                property double availableHeight: parent.height - bottomMargin - topMargin

                property bool parentIsLarge: parentRatio > aimedRatio

                property double parentRatio: availableHeight / availableWidth

                height: parentIsLarge ? width * aimedRatio : availableHeight
                width: parentIsLarge ? availableWidth : height / aimedRatio

                property double verticalSpacing: (availableHeight - height) / 2
                property double horzitontalSpacing: (availableWidth - width) / 2

                anchors.top: parent.top
                anchors.topMargin: topMargin + verticalSpacing
                anchors.left: parent.left
                anchors.leftMargin: leftMargin + horzitontalSpacing

                layer.enabled: true
                layer.effect: ShaderEffect
                {
                    clip: true
                    property variant source: ShaderEffectSource {
                        sourceItem: image
                        hideSource: true
                    }
                    fragmentShader: effectSource
                }

                property int duration: 250
                MouseArea {
                    anchors.fill: parent
                    onClicked:
                    {
                        effectGridView.currentIndex = index
                        effectSelected(effectSource)
                        form.close()
                    }
                    onPressed: {
                        glow.visible = true
                        animation1.start()
                        animation2.start()
                    }
                }
            }

            BorderImage {
                anchors.horizontalCenter: image.horizontalCenter
                anchors.verticalCenter: image.verticalCenter
                id: borderImage
                border.bottom: 65
                border.top: 25
                border.right: 25
                border.left: 25
                width: image.paintedWidth + 40
                height: image.paintedHeight + 60
                anchors.verticalCenterOffset: 10
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                source: "qrc:/images/polaroid.svg.png"
            }

            Rectangle {
                id: glow
                visible: false

                anchors.verticalCenter: image.verticalCenter
                anchors.horizontalCenter: image.horizontalCenter

                width: image.width
                height: image.height
                color: "#00000000"
                scale: 1.05
                border.color: "#ffffff"
            }

            Label {
                id: label
                text: effectText
                visible: true
                anchors.topMargin: -7
                anchors.top: image.bottom
                anchors.horizontalCenter: image.horizontalCenter
                color: "#443224"
                font.family: "DejaVu Serif"
                font.pixelSize: 28
            }

            PropertyAnimation {
                target: glow
                id: animation1
                duration: image.duration
                loops: 1
                from: 1.05
                to: 1.2
                property: "scale"
            }

            ParallelAnimation {
                id: animation2
                SequentialAnimation {
                    PropertyAnimation {
                        target: glow
                        duration: image.duration
                        loops: 1
                        from: 0.2
                        to: 1.0
                        property: "opacity"
                    }
                    PropertyAnimation {
                        target: glow
                        duration: image.duration
                        loops: 1
                        from: 1.0
                        to: 0.0
                        property: "opacity"
                    }

                    PropertyAction {
                        target: glow
                        property: "visible"
                        value: false
                    }
                }

                SequentialAnimation {
                    PropertyAction {
                        target: glow
                        property: "border.width"
                        value: 20
                    }

                    PauseAnimation {
                        duration: 200
                    }

                    PropertyAnimation {
                        target: glow
                        duration: image.duration
                        loops: 1
                        from: 20
                        to: 10
                        property: "border.width"
                    }
                }
            }
        }
    }

}
