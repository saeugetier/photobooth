import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import "../styles"

Item {
    property int countDownTime: 3
    signal triggerSnapshot

    function reset()
    {
        state = "idle"
    }

    ProgressBar
    {
        id: timerProgress

        anchors.fill: parent
        visible: false

        property color barColor: "#17a81a"
        onValueChanged:
        {
            barColor = Qt.rgba(0.1 + 0.7*value, 0.8 - 0.7*value, 0.1, 1.0)
        }

        background:
            Rectangle
            {
               id: outerRing
               z: 0
               anchors.fill: parent
               radius: Math.max(width, height) / 2
               color: "transparent"
               border.color: "gray"
               border.width: 16
            }

        contentItem:
            Rectangle
            {
               id: innerRing
               z: 1
               anchors.fill: parent
               anchors.margins: (outerRing.border.width - border.width) / 2
               radius: outerRing.radius
               color: "transparent"
               border.color: "darkgray"
               border.width: 8

               ConicalGradient
               {
                  source: innerRing
                  anchors.fill: parent
                  gradient: Gradient
                  {
                     GradientStop { position: 0.00; color: timerProgress.barColor }
                     GradientStop { position: timerProgress.value - 0.01; color: timerProgress.barColor }
                     GradientStop { position: timerProgress.value; color: "transparent" }
                     GradientStop { position: 1.00; color: "transparent" }
                  }
               }
            }
    }

    ToolButton {
        id: shutterButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        text: "\uE801"
        font.family: "fontello"
        font.pixelSize: parent.width / 2
        enabled: true

        opacity: 0.7

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }

        onClicked:
        {
            parent.state = "triggered"
            console.log("Test")
            countdown.start()
        }
    }

    Countdown
    {
        id: countdown
        seconds: countDownTime
        anchors.fill: parent

        onTimeout:
        {
            parent.state = "snapshot"
            triggerSnapshot()
        }
    }

    states: [
        State {
            name: "idle"
            PropertyChanges {
                target: shutterButton
                enabled: true
            }
            PropertyChanges {
                target: timerProgress
                visible: false
                value: 0.0
            }
        },
        State {
            name: "triggered"
            PropertyChanges {
                target: shutterButton
                enabled: false
            }
            PropertyChanges {
                target: timerProgress
                visible: true
                value: 1.0
            }
        },
        State {
            name: "snapshot"
            PropertyChanges {
                target: shutterButton
                enabled: false
            }
            PropertyChanges {
                target: timerProgress
                visible: false
                value: 0.0
            }
        }

    ]

    transitions: Transition {
        to: "triggered"
        reversible: false

        PropertyAnimation {
            target: timerProgress
            property: "value"
            duration: countDownTime * 1000
        }
    }
}
