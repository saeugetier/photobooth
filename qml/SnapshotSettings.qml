import QtQuick 2.4
import Qt.labs.settings 1.0

SnapshotSettingsForm {
    property alias flashEnabled: settings.flashEnabled
    property alias viewFinderBrightness: settings.viewFinderBrightness
    property alias flashBrightness: settings.flashBrightness
    property alias countdown: settings.countdown
    property alias chromaKeyEnabled: settings.chromaKeyEnabled
    property alias chromaKeyStrength: settings.chromaKeyEnabled
    property alias chromaKeyColor: settings.chromaKeyColor

    Settings
    {
        id: settings
        category: "Camera"
        property bool flashEnabled: true
        property real viewFinderBrightness: 0.1
        property real flashBrightness: 1.0
        property int countdown: 1
        property bool chromaKeyEnabled: false
        property real chromaKeyStrength: 0.5
        property real chromaKeyColor: 0.5
    }

    showButton.onClicked:
    {
        if(state !== "expanded")
        {
            state = "expanded"
        }
        else
        {
            state = "idle"
        }
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

    switchChromaKeyEnable.checked: settings.chromaKeyEnabled
    switchChromaKeyEnable.onCheckedChanged:
    {
        settings.chromaKeyEnabled = switchChromaKeyEnable.checked
    }

    sliderChromaKeyFilterStrength.value: settings.chromaKeyStrength
    sliderChromaKeyFilterStrength.onValueChanged:
    {
        settings.chromaKeyStrength = sliderChromaKeyFilterStrength.value
    }

    sliderChromaKeyFilterColor.value: settings.chromaKeyColor
    sliderChromaKeyFilterColor.onValueChanged:
    {
        settings.chromaKeyColor = sliderChromaKeyFilterColor.value
    }

    Behavior on width
    {
        PropertyAnimation
        {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }
}
