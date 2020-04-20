import QtQuick 2.4
import QtQuick.Controls 2.4
import "content"

EffectSelectorForm {    
    id: form

    property url previewImage: ""

    signal effectSelected(string effect)

    property int duration: 250

    upButton.onClicked:
    {
        scrollIndicator.increase()
    }

    downButton.onClicked:
    {
        scrollIndicator.decrease()
    }

    NumberAnimation on scrollIndicator.position {
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation on effectListView.contentY {
        duration: 1000
        easing.type: Easing.InOutQuad
    }

    effectListView.delegate: effectDelegate


    Component
    {
        id: effectDelegate

        Item {

            id: item

            width: image.width
            height: image.height + 50


        Image {
            id: image
            source: previewImage

            fillMode: Image.PreserveAspectFit

            sourceSize.height: 256
            sourceSize.width: 256

            width: 300

            layer.enabled: true
            layer.effect: ImageEffect
            {
                clip: true
                source: image
                fragmentShaderFilename: effectSource
            }

            property int duration: 250
            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    effectListView.currentIndex = index
                    effectSelected(effectSource)
                }
                onPressed: {
                    glow.visible = true
                    animation1.start()
                    animation2.start()
                }
            }


            }

        Rectangle {
            id: glow
            visible: false

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
            anchors.topMargin: 10
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
