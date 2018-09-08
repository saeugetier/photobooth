import QtQuick 2.4
import QtQuick.Controls 2.0

AdvancedSettingsForm {
    closePolicy: Popup.CloseOnEscape
    id: form

    property alias printerPermanentEnabled: form.printerEnabled
    property alias recycleBinEnabled: form.recycleEnabled

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
