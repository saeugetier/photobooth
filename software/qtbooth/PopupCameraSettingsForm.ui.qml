import QtQuick 2.5
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3


Dialog {
    property alias settingPrintEnable: printEnable.checked
    property alias settingPrintFullscale: printFullscale.checked
    property alias settingFlashEnable: flashEnable.checked
    property alias settingBrightness: brightnessSlider.value
    property alias settingCountdown: countdownSpinBox.value
    property alias quitButton: quitButton
    property alias advancedSettings: advancedSettings

    GridLayout {
        id: grid
        columns: 2

        Label
        {
            text: qsTr("Print: ")
        }

        RowLayout
        {
            Label
            {
                text: "\uE802"
                font.family: "fontello"
            }

            Switch
            {
                id: printEnable
            }
        }

        Label
        {
            text: qsTr("Print type: ")
        }

        RowLayout
        {
            Label
            {
                text: "\uE805"
                font.family: "fontello"
            }

            Switch
            {
                id: printFullscale
                checked: false
            }

            Label
            {
                text: "\uE806" // icon-folder-open-empty
                font.family: "fontello"
            }
        }

        Label
        {
            text: qsTr("Flash: ")
        }

        RowLayout
        {
            Label
            {
                text: "\uE809"
                font.family: "fontello"
            }

            Switch
            {
                id: flashEnable
            }
        }

        Label
        {
            text: qsTr("Light brightness: ")
        }

        Slider
        {
            id: brightnessSlider
            stepSize: 0.05
        }

        Label
        {
            text: qsTr("Count down timer: ")
        }

        SpinBox
        {
            id: countdownSpinBox
            from: 3
            to: 10
            value: 3
        }

        Button
        {
            id: quitButton
            text: "Close program"
        }

        Button {
            id: advancedSettings
            text: qsTr("Advanced Settings")
        }

        Label {
            id: label
            text: qsTr("hidden")
            visible: false
        }

        RangeSlider {
            id: rangeSlider
            visible: false
            second.value: 0.75
            first.value: 0.25
        }

    }

}
