import QtQuick 2.5
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Dialog {
    id: dialog
    property alias settingPrintEnable: printEnable.checked
    property alias settingPrintEnableSwitchEnable: printEnable.enabled
    property alias settingPrintFullscale: printFullscale.checked
    property alias settingFlashEnable: flashEnable.checked
    property alias settingBrightness: brightnessSlider.value
    property alias settingFlashBrightness: flashBrightnessSlider.value
    property alias settingCountdown: countdownSpinBox.value
    property alias advancedSettings: advancedSettings

    GridLayout {
        id: grid
        anchors.bottomMargin: 120
        columns: 2

        Label {
            text: qsTr("Print: ")
        }

        RowLayout {
            Label {
                text: "\uE802"
                font.family: "fontello"
            }

            Switch {
                id: printEnable
            }
        }

        Label {
            text: qsTr("Print type: ")
        }

        RowLayout {
            Label {
                text: "\uE805"
                font.family: "fontello"
            }

            Switch {
                id: printFullscale
                checked: false
            }

            Label {
                text: "\uE806" // icon-folder-open-empty
                font.family: "fontello"
            }
        }

        Label {
            text: qsTr("Flash: ")
        }

        RowLayout {
            Label {
                text: "\uE809"
                font.family: "fontello"
            }

            Switch {
                id: flashEnable
            }
        }

        Label {
            text: qsTr("Light brightness: ")
        }

        Slider {
            id: brightnessSlider
            stepSize: 0.05
        }

        Label {
            text: qsTr("Flash brightness: ")
        }

        Slider {
            id: flashBrightnessSlider
            stepSize: 0.05
        }

        Label {
            text: qsTr("Count down timer: ")
        }

        SpinBox {
            id: countdownSpinBox
            from: 3
            to: 10
            value: 3
        }
    }

    Button {
        id: advancedSettings
        text: qsTr("Advanced Settings")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 0
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
