import QtQuick 2.5
import QtQuick.Controls 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.2
import QtMultimedia 5.5

PopupCameraSettingsForm {
    property alias cameraPrintSettings: settings
    id: dialog
    standardButtons: Dialog.Ok | Dialog.Cancel

    // sync settings and UI
    settingPrintEnable: settings.printEnable & settings.printerPermanentEnable
    settingPrintEnableSwitchEnable:  settings.printerPermanentEnable
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

        flash.setBrightness(settings.brightness)
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

    advancedSettings.onClicked:
    {
        passwordPopup.open()
    }

    SettingsPassword
    {
        x: (parent.width - width) / 2
        id: passwordPopup
        modal: true
        transformOrigin: Popup.Center
        closePolicy: Popup.CloseOnEscape

        password: settings.password

        onUnlockedChanged:
        {
            console.log("Settings unlock state changed")
            if(passwordPopup.unlocked)
            {
                advancedSettings.open()
            }
        }
    }

    AdvancedSettings
    {
        id: advancedSettings

        onOpened:
        {
            advancedSettings.printerPermanentEnabled.checked = settings.printerPermanentEnable
            advancedSettings.recycleBinEnabled.checked = applicationSettings.recycleBinEnabled
            advancedSettings.setWhiteBalance(settings.whiteBalance)
        }

        onClosed:
        {
            passwordPopup.unlocked = false
            settings.printerPermanentEnable = advancedSettings.printerPermanentEnabled.checked
            applicationSettings.recycleBinEnabled = advancedSettings.recycleBinEnabled.checked
            settings.whiteBalance = advancedSettings.getWhiteBalance()
        }
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
        property string password: "0815"
        property bool printerPermanentEnable: true
        property int whiteBalance: CameraImageProcessing.WhiteBalanceAuto

        Component.onCompleted:
        {
            if(!settings.printerPermanentEnable)
                settings.printEnable = false
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
