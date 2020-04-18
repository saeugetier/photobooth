import QtQuick 2.4

SettingsPopupForm {
    id: form
    property alias printerEnabled: form.switchPrinter

    onOpened:
    {
        if(filesystem.layoutFilesOnRemovableDrive())
        {
            buttonCopyTemplates.enabled = true
        }
        else
        {
            buttonCopyTemplates.enabled = false
        }

        if(filesystem.removableDriveMounted())
        {
            buttonCopyPhotos.enabled = true
        }
        else
        {
            buttonCopyPhotos.enabled = false
        }
    }

    buttonClose.onClicked:
    {
        close()
    }

    buttonCopyTemplates.onClicked:
    {
        filesystem.copyLayoutFiles()
    }

    FileCopyProgress
    {
        id: copyProgressPopup
    }

    buttonCopyPhotos.onClicked:
    {
        copyProgressPopup.open()
    }

    buttonShutdown.onClicked:
    {
        system.shutdown()
    }

    buttonRestart.onClicked:
    {
        system.restart()
    }

    Timer
    {
        interval: 1000
        running: true
        repeat: true
        onTriggered:
        {
            labelTime.text = qsTr("Time: ") + Date().toString();
        }

        Component.onCompleted:
        {
            labelTime.text = qsTr("Time: ") + Date().toString();
        }
    }

}
