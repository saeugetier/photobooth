import QtQuick 2.4
import Qt.labs.settings 1.0

SnapshotSettingsForm {
    property alias flashEnabled: settings.flashEnabled
    property alias viewFinderBrightness: settings.viewFinderBrightness
    property alias flashBrightness: settings.flashBrightness
    property alias countdown: settings.countdown

    Settings
    {
        id: settings
        category: "Camera"
        property bool flashEnabled: true
        property real viewFinderBrightness: 0.1
        property real flashBrightness: 1.0
        property int countdown: 1
    }

    sliderFlashBrightness.value: settings.flashBrightness
    sliderFlashBrightness.onValueChanged:
    {
        if(sliderFlashBrightness.value < sliderViewfinderBrightness.value)
        {
            sliderFlashBrightness.value = sliderViewfinderBrightness.value
        }
        settings.flashBrightness = sliderFlashBrightness.value
    }

    sliderViewfinderBrightness.value: settings.viewFinderBrightness
    sliderViewfinderBrightness.onValueChanged:
    {
        if(sliderFlashBrightness.value < sliderViewfinderBrightness.value)
        {
            sliderFlashBrightness.value = sliderViewfinderBrightness.value
        }
        settings.viewFinderBrightness = sliderViewfinderBrightness.value
    }

    switchFlashEnable.checked: settings.flashEnabled
    switchFlashEnable.onCheckedChanged:
    {
        settings.flashEnabled = switchFlashEnable.checked
    }

    spinBoxCountdownTime.value: settings.countdown
    spinBoxCountdownTime.onValueChanged:
    {
        settings.countdown = spinBoxCountdownTime.value
    }
}
