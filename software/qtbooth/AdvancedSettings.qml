import QtQuick 2.4
import QtQuick.Controls 2.0
import QtMultimedia 5.5

AdvancedSettingsForm {
    closePolicy: Popup.CloseOnEscape
    id: form

    property alias printerPermanentEnabled: form.printerEnabled
    property alias recycleBinEnabled: form.recycleEnabled

    comboBoxWhiteBalance.onCurrentIndexChanged:
    {
        console.log("White Balance Selected: " + comboBoxWhiteBalance.textAt(comboBoxWhiteBalance.currentIndex))
    }

    function setWhiteBalance(value)
    {
        var text
        switch(value)
        {
        case CameraImageProcessing.WhiteBalanceAuto:
            text = "Auto"
            break;
        case CameraImageProcessing.WhiteBalanceCloudy:
            text = "Cloudy"
            break;
        case CameraImageProcessing.WhiteBalanceFlash:
            text = "Flash"
            break;
        case CameraImageProcessing.WhiteBalanceFluorescent:
            text = "Flourescent"
            break;
        case CameraImageProcessing.WhiteBalanceShade:
            text = "Shade"
            break;
        case CameraImageProcessing.WhiteBalanceSunlight:
            text = "Sunlight"
            break;
        case CameraImageProcessing.WhiteBalanceSunset:
            text = "Sunset"
            break;
        case CameraImageProcessing.WhiteBalanceTungsten:
            text = "Thungsten"
            break;
        default:
            text = "Auto"
        }

        var index = comboBoxWhiteBalance.find(text)
        if(index !== -1)
        {
            comboBoxWhiteBalance.currentIndex = index
        }
    }

    function getWhiteBalance()
    {
        var whitebalance = CameraImageProcessing.WhiteBalanceAuto
        var text = comboBoxWhiteBalance.textAt(comboBoxWhiteBalance.currentIndex)
        switch(text)
        {
        case "Auto":
            whitebalance = CameraImageProcessing.WhiteBalanceAuto
            break;
        case "Cloudy":
            whitebalance = CameraImageProcessing.WhiteBalanceCloudy
            break;
        case "Flash":
            whitebalance = CameraImageProcessing.WhiteBalanceFlash
            break;
        case "Flourescent":
            whitebalance = CameraImageProcessing.WhiteBalanceFluorescent
            break;
        case "Shade":
            whitebalance = CameraImageProcessing.WhiteBalanceShade
            break;
        case "Sunlight":
            whitebalance = CameraImageProcessing.WhiteBalanceSunlight
            break;
        case "Sunset":
            whitebalance = CameraImageProcessing.WhiteBalanceSunset
            break;
        case "Thungsten":
            whitebalance = CameraImageProcessing.WhiteBalanceTungsten
            break;
        default:
            whitebalance = CameraImageProcessing.WhiteBalanceAuto
        }

        return whitebalance
    }

    onOpened:
    {
        if(helper.removableDriveMounted())
        {
            copyButton.enabled = true
        }
        else
        {
            copyButton.enabled = false
        }
    }

    cancelButton.onClicked:
    {
        form.close()
    }

    closeButton.onClicked:
    {
        mainWindow.close()
    }

    shutDownButton.onClicked:
    {
        flash.setBrightness(0);
        mainWindow.close()
        helper.shutdown()
    }

    restartButton.onClicked:
    {
        flash.setBrightness(0);
        mainWindow.close()
        helper.restart()
    }

    printerEnabled.onCheckedChanged:
    {
    }

    copyButton.onClicked:
    {
        progressForm.open()
        copyButton.enabled = false
    }

    FileCopyProgress
    {
        id: progressForm
    }

    deleteButton.onActivated:
    {
        console.log("Deletion promp")
        deleteButton.progress = 0
        deleteDialog.open()
    }

    DeleteConfirmDialog
    {
        id: deleteDialog
    }

}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
