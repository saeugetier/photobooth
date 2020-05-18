import QtQuick 2.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.0

Pane {
    id: pane
    width: 80
    property alias showButton: showButton
    property alias spinBoxCountdownTime: spinBoxCountdownTime
    property alias sliderViewfinderBrightness: sliderViewfinderBrightness
    property alias sliderFlashBrightness: sliderFlashBrightness
    property alias switchFlashEnable: switchFlashEnable
    opacity: 0.5
    spacing: 5

    RowLayout {
        id: rowLayout
        height: pane.height
        Layout.fillWidth: true

        ColumnLayout {
            Layout.preferredWidth: 50
            height: rowLayout.height
            Layout.fillHeight: true
            Layout.fillWidth: true

            Item {
                id: container
                Layout.fillHeight: true
                Layout.fillWidth: true

                Text {
                    id: settingsTextLabel
                    color: "#ffffff"
                    text: qsTr("Settings")
                    transformOrigin: Item.BottomLeft
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    font.family: "DejaVu Serif"
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: 50
                    font.capitalization: Font.AllUppercase

                    rotation: 90
                }

                ToolButton {
                    id: showButton
                    y: container.height / 2
                    text: "\uF137" // icon-left-arrow
                    font.family: "fontello"
                    font.pixelSize: 50

                    Layout.alignment: Qt.AlignVCenter

                    scale: hovered ? 1.1 : 1

                    layer.enabled: true
                    layer.effect: Glow {
                        color: "black"
                        samples: 20
                        spread: 0.3
                    }
                }
            }
        }

        ColumnLayout {
            ToolSeparator {
                Layout.fillHeight: true
            }
        }

        ColumnLayout {
            id: columnLayout

            Layout.fillHeight: true

            visible: false

            spacing: 15

            Layout.alignment: Qt.AlignTop

            RowLayout {

                spacing: 20

                Label {
                    id: labelFlashIcon
                    font.pixelSize: 32
                    text: "\uE809"
                    font.family: "fontello"
                }

                Label {
                    id: labelFlash
                    font.pixelSize: 32
                    text: qsTr("Flash")
                    font.family: "DejaVu Serif"
                }

                Switch {
                    id: switchFlashEnable
                    text: checked ? qsTr("enabled") : qsTr("disabled")
                    font.pixelSize: 32
                    font.family: "DejaVu Serif"
                    width: 250
                }
            }

            RowLayout {
                id: rowFlashBrightness
                spacing: 15

                Label {
                    id: labelFlashBrightnessLow
                    font.pixelSize: 32
                    text: "\uE80B"
                    font.family: "fontello"
                }

                Slider {
                    id: sliderFlashBrightness
                    value: 0.5
                    width: 200
                }

                Label {
                    id: labelFlashBrightnessHigh
                    font.pixelSize: 32
                    text: "\uE80A"
                    font.family: "fontello"
                }
            }

            RowLayout {
                ToolSeparator {
                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                }
            }

            RowLayout {
                id: rowViewfinder

                spacing: 15

                Label {
                    id: labelViewfinderIcon
                    font.pixelSize: 32
                    text: "\uE824"
                    font.family: "fontello"
                }

                Label {
                    id: labelViewfinder
                    font.pixelSize: 32
                    text: qsTr("Preview Light")
                    font.family: "DejaVu Serif"
                }
            }

            RowLayout {
                id: rowViewfinderBrightness
                spacing: 20

                Label {
                    id: labelViewfinderBrightnessLow
                    font.pixelSize: 32
                    text: "\uE80B"
                    font.family: "fontello"
                }

                Slider {
                    id: sliderViewfinderBrightness
                    value: 0.5
                    width: 200
                }

                Label {
                    id: labelViewfinderBrightnessHigh
                    font.pixelSize: 32
                    text: "\uE80A"
                    font.family: "fontello"
                }
            }

            RowLayout {
                ToolSeparator {
                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                }
            }

            RowLayout {
                id: countdown

                spacing: 15

                Label {
                    id: labelCountdownIcon
                    font.pixelSize: 32
                    text: "\uE828"
                    font.family: "fontello"
                }

                Label {
                    id: labelCountdown
                    font.pixelSize: 32
                    text: qsTr("Countdown")
                    font.family: "DejaVu Serif"
                }
            }

            RowLayout {
                id: rowCountdownTime

                SpinBox {
                    id: spinBoxCountdownTime
                    to: 15
                    from: 3
                    width: 200
                    font.pixelSize: 32
                    font.family: "DejaVu Serif"
                }
            }
        }
    }
    states: [
        State {
            name: "idle"
        },
        State {
            name: "expanded"
            PropertyChanges {
                target: showButton
                text: "\uF138" //icon-right-arrow
            }
            PropertyChanges {
                target: pane
                width: 450
            }
            PropertyChanges {
                target: columnLayout
                visible: true
            }

            PropertyChanges {
                target: spinBoxCountdownTime
                font.family: "Dejavu Serif"
                font.pointSize: 15
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.75}
}
##^##*/

