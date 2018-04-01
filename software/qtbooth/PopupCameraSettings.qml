import QtQuick 2.4
import QtQuick.Controls 2.2
import Qt.labs.settings 1.0

PopupCameraSettingsForm {
    id: dialog
    standardButtons: Dialog.Ok | Dialog.Cancel

    signal changedPrintFullscreen()

    // sync settings and UI
    settingPrintEnable: settings.printEnable
    settingPrintFullscale: settings.printFullscale
    settingFlashEnable: settings.flashEnable
    settingBrightness: settings.brightness
    settingCountdown: settings.countdown

    onAccepted: {
        settings.printEnable = dialog.settingPrintEnable
        settings.printFullscale = dialog.settingPrintFullscale
        settings.flashEnable = dialog.settingFlashEnable
        settings.brightness = dialog.settingBrightness
        settings.countdown = dialog.settingCountdown
        console.log("Settings accepted")
    }

    onRejected: {
        dialog.settingPrintEnable = settings.printEnable
        dialog.settingPrintFullscale = settings.printFullscale
        dialog.settingFlashEnable = settings.flashEnable
        dialog.settingBrightness = settings.brightness
        dialog.settingCountdown = settings.countdown
        console.log("Settings rejected")
    }

    Settings //settings with default values
    {
        id: settings
        category: "Camera"
        property bool printEnable: true
        property bool printFullscale: true
        property bool flashEnable: true
        property real brightness: 0.1
        property int countdown: 3

        onPrintEnableChanged:
        {
           dialog.changedPrintFullscreen()
        }
    }
}
