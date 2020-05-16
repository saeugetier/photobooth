import QtQuick 2.4
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.0

Pane {
    width: 350
    property alias spinBoxCountdownTime: spinBoxCountdownTime
    property alias sliderViewfinderBrightness: sliderViewfinderBrightness
    property alias sliderFlashBrightness: sliderFlashBrightness
    property alias switchFlashEnable: switchFlashEnable
    opacity: 0.5

    Column {
        id: columnLayout
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 5

        Row {
            id: rowFlash
            Label {
                id: labelFlash
                font.pixelSize: 32
                text: qsTr("Flash")
                font.family: "DejaVu Serif"
            }
        }

        Row {
            id: rowFlashEnable
            spacing: 20
            anchors.left: parent.left
            anchors.right: parent.right
            Switch {
                id: switchFlashEnable
                text: checked ? qsTr("enabled") : qsTr("disabled")
                font.pixelSize: 32
                font.family: "DejaVu Serif"
                width: 250
            }
        }

        Row {
            id: rowFlashBrightness
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: 0
            spacing: 20

            Slider {
                id: sliderFlashBrightness
                value: 0.5
                width: parent.width - labelFlashBrightness.width - 20
            }

            Label {
                id: labelFlashBrightness
                font.pixelSize: 32
                text: qsTr("Brightness")
                anchors.verticalCenter: sliderFlashBrightness.verticalCenter
                font.family: "DejaVu Serif"
            }
        }

        ToolSeparator {
            anchors.right: parent.right
            anchors.left: parent.left
            orientation: Qt.Horizontal
        }

        Row {
            id: rowViewfinder
            Label {
                id: labelViewfinder
                font.pixelSize: 32
                text: qsTr("Preview Light")
                font.family: "DejaVu Serif"
            }
        }

        Row {
            id: rowViewfinderBrightness
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: 0
            spacing: 20

            Slider {
                id: sliderViewfinderBrightness
                value: 0.5
                width: parent.width - labelViewfinderBrightness.width - 20
            }

            Label {
                id: labelViewfinderBrightness
                font.pixelSize: 32
                text: qsTr("Brightness")
                anchors.verticalCenter: sliderViewfinderBrightness.verticalCenter
                font.family: "DejaVu Serif"
            }
        }

        ToolSeparator {
            anchors.right: parent.right
            anchors.left: parent.left
            orientation: Qt.Horizontal
        }

        Row {
            id: countdown
            Label {
                id: labelCountdown
                font.pixelSize: 32
                text: qsTr("Countdown")
                font.family: "DejaVu Serif"
            }
        }

        Row {
            id: rowCountdownTime
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: 0
            spacing: 20

            SpinBox {
                id: spinBoxCountdownTime
                to: 15
                from: 3
                width: parent.width - labelCountdownTime.width - 20
            }

            Label {
                id: labelCountdownTime
                font.pixelSize: 32
                text: qsTr("seconds")
                anchors.verticalCenter: spinBoxCountdownTime.verticalCenter
                font.family: "DejaVu Serif"
            }
        }
    }
}
