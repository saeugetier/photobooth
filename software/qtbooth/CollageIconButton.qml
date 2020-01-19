import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Image {
    id: root
    source: "images/icon/Unknown.svg"
    signal clicked

    property int duration: 250
    //property alias text: label.text
//! [0]
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        onPressed: {
            glow.visible = true
            animation1.start()
            animation2.start()
        }
    }
//! [0]

    Rectangle {
        id: glow
        visible: false

        width: root.width
        height: root.height
        color: "#00000000"
        radius: 125
        scale: 1.05
        border.color: "#ffffff"
    }

    /*Label {
        id: label
        x: 292
        y: root.height + 10
        text: qsTr("Label")
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#443224"
        font.family: "DejaVu Serif"
        font.pixelSize: 28
    }*/

    PropertyAnimation {
        target: glow
        id: animation1
        duration: root.duration
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
                duration: root.duration
                loops: 1
                from: 0.2
                to: 1.0
                property: "opacity"
            }
            PropertyAnimation {
                target: glow
                duration: root.duration
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
                duration: root.duration
                loops: 1
                from: 20
                to: 10
                property: "border.width"
            }
        }
    }
}
