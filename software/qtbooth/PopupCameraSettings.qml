import QtQuick 2.5
import QtQuick.Controls 2.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0

PopupCameraSettingsForm {
    property alias cameraPrintSettings: settings
    id: dialog
    standardButtons: Dialog.Ok | Dialog.Cancel

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

    quitButton.onClicked:
    {
        mainWindow.close()
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
                console.log("Visible")
                Label.visible = true;
                RangeSlider.visible = true;
                Label.visibleChildren = true;
                RangeSlider.visibleChildren = true;
            }
            else
            {
                console.log("Hidden")
                Label.visible = false;
                RangeSlider.visible = false;
                Label.visibleChildren = false;
                RangeSlider.visibleChildren = false;
            }
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
        property string password: "1234"

        Component.onCompleted:
        {
            flash.setBrightness(brightness)
        }
    }
}
