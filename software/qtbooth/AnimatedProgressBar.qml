import QtQuick 2.0
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

ProgressBar {
    id: control

    Behavior on value { PropertyAnimation { duration:  1000 } }
    property color barColor: "#17a81a"
    onValueChanged:
    {
        barColor = Qt.rgba(0.1 + 0.7*value, 0.8 - 0.7*value, 0.1, 1.0)
    }

    value: 0
    height: 20
    clip: true
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 6
        border.color: "#999999"
        radius: 5
        color: "#333333"
    }
    contentItem: Item {
        implicitWidth: 200
        implicitHeight: 4

        Rectangle {
            id: bar
            width: control.visualPosition * parent.width
            height: parent.height
            radius: 5
        }

        LinearGradient {
            anchors.fill: bar
            start: Qt.point(0, 0)
            end: Qt.point(bar.width, 0)
            source: bar
            gradient: Gradient {
                GradientStop { position: 0.0; color: barColor }
                GradientStop { position: 1.0; color: Qt.lighter(barColor, 1.5) }
            }
        }
        LinearGradient {
            anchors.fill: bar
            start: Qt.point(0, 0)
            end: Qt.point(0, bar.height)
            source: bar
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(0,0,0,0) }
                GradientStop { position: 0.5; color: Qt.rgba(1,1,1,0.3) }
                GradientStop { position: 1.0; color: Qt.rgba(0,0,0,0.05) }
            }
        }
    }
}
