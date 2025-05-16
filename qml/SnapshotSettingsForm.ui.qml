import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Material

Pane {
    id: pane
    width: 80
    property alias showButton: showButton
    property alias spinBoxCountdownTime: spinBoxCountdownTime
    property alias sliderViewfinderBrightness: sliderViewfinderBrightness
    property alias sliderFlashBrightness: sliderFlashBrightness
    property alias switchFlashEnable: switchFlashEnable
    property alias switchBackgroundFilterEnable: switchBackgroundFilterEnable
    property alias switchChromaKeyEnable: switchChromaKeyEnable
    property alias sliderChromaKeyFilterColor: sliderChromaKeyFilterColor
    property alias backgroundImageSelectorModel: backgroundSelector.model
    property alias backgroundImageSelector: backgroundSelector
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

        ScrollView
        {
            id: settingsScrollView
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: false

            //ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            ColumnLayout {
                id: columnLayout

                Layout.fillHeight: true

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
                        width: 250
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
                        width: 250
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
                        Layout.fillWidth: true
                        width: 200
                        font.pixelSize: 32
                        font.family: "DejaVu Serif"
                    }
                }

                RowLayout {
                    ToolSeparator {
                        Layout.fillWidth: true
                        orientation: Qt.Horizontal
                    }
                }

                RowLayout
                {
                    id: backgroundFilterLabel

                    spacing: 15

                    Label {
                        id: labelBackgrounddFilterIcon
                        font.pixelSize: 32
                        text: "\uF0D0"
                        font.family: "fontello"
                    }

                    Label {
                        id: labelBackgroundFilter
                        font.pixelSize: 32
                        text: qsTr("Background Filter")
                        font.family: "DejaVu Serif"
                    }
                }

                RowLayout
                {
                    Label {
                        id: labelBackgroundFilterEnable
                        font.pixelSize: 32
                        text: "Filter: "
                    }

                    Switch {
                        id: switchBackgroundFilterEnable
                        text: checked ? qsTr("enabled") : qsTr("disabled")
                        font.pixelSize: 32
                        font.family: "DejaVu Serif"
                        width: 250
                    }
                }

                RowLayout
                {
                    visible: switchBackgroundFilterEnable.checked

                    Label {
                        id: labelChromaKeyFilterEnable
                        font.pixelSize: 32
                        text: "Type: "
                    }

                    Switch {
                        id: switchChromaKeyEnable
                        text: checked ? qsTr("chroma") : qsTr("neural")
                        font.pixelSize: 32
                        font.family: "DejaVu Serif"
                        width: 250
                    }

                }

                RowLayout {
                    id: rowChromaKeyFilterColor
                    spacing: 20

                    visible: switchChromaKeyEnable.checked & switchBackgroundFilterEnable.checked

                    Rectangle
                    {
                        color: "Green"
                        width: 32
                        height: 32
                    }

                    Slider {
                        id: sliderChromaKeyFilterColor
                        value: 0.0
                        from: 0.0
                        to: 1.0
                        width: 250
                    }

                    Rectangle
                    {
                        color: "Blue"
                        width: 32
                        height: 32
                    }
                }

                RowLayout {
                    ToolSeparator {
                        Layout.fillWidth: true
                        orientation: Qt.Horizontal
                    }
                }

                RowLayout
                {
                    visible: switchBackgroundFilterEnable.checked

                    Label {
                        id: labelBackground
                        font.pixelSize: 32
                        text: "Background:"
                    }
                }

                RowLayout
                {
                    visible: switchBackgroundFilterEnable.checked

                    GridView
                    {
                        id: backgroundSelector
                        cellWidth: 100
                        cellHeight: 120
                        Layout.fillWidth: true

                        focus: true

                        delegate: Column {
                            spacing: 5

                            Rectangle {
                                width: backgroundSelector.cellWidth - 10
                                height: backgroundSelector.cellHeight - 30
                                border.color: GridView.isCurrentItem ? "blue" : "gray"
                                border.width: 5
                                radius: 8

                                Image {
                                    anchors.fill: parent
                                    source: model.fileUrl
                                    fillMode: Image.PreserveAspectCrop
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: backgroundSelector.currentIndex = index
                                }
                            }

                            Label {
                                width: backgroundSelector.cellWidth - 10
                                text: model.fileBaseName
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
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
                target: settingsScrollView
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

