import QtQuick 2.4

SettingsPopupForm {
    id: form
    property alias printerEnabled: form.switchPrinter
    property alias mirrorCamera: form.switchMirrorCamera

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

        var index = comboBoxPrinter.find(applicationSettings.printerName)
        comboBoxPrinter.currentIndex = index
        console.log("index: " + Number(index).toString())
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

    buttonCloseProgram.onClicked:
    {
        Qt.quit()
    }

    buttonDeletePhotos.onClicked:
    {
        filesystem.deleteAllImages()
    }

    Timer
    {
        interval: 1000
        running: true
        repeat: true
        onTriggered:
        {
            var date = new Date()
            var dateString = date.toLocaleString(Qt.locale(), Locale.NarrowFormat);
            labelTime.text = qsTr("Time: ") + dateString;
        }

        Component.onCompleted:
        {
            var date = new Date()
            var dateString = date.toLocaleString(Qt.locale(), Locale.NarrowFormat);
            labelTime.text = qsTr("Time: ") + dateString;
        }
    }

    buttonSetTime.onClicked:
    {
        timeSettings.open()
    }

    buttonReprint.onClicked:
    {
        collageReprint.open()
    }

    CollageReprint
    {
        id: collageReprint
        modal: true
        focus: true
        x: 20 - form.x
        y: 20 - form.y
        width: mainWindow.width - 60
        height: mainWindow.height - 60
    }


    TimeSettings
    {
        id: timeSettings

        onSetTime:
        {
            system.setTime(time)
        }
    }

}
