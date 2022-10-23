import QtQuick 2.4
import QtMultimedia 5.5
import QtQuick.Controls 2.4

SettingsPopupForm {
    id: form
    property alias printerEnabled: form.switchPrinter
    property alias mirrorCamera: form.switchMirrorCamera

    Component.onCompleted:
    {
        versionText = "Version: " + system.getGitHash()
    }

    function makeCameraList()
    {
        var listModel = [];
        var i;
        var availableCameras = QtMultimedia.availableCameras;
        console.log("Available Camera Count: " + Number(availableCameras.length).toString())
        for(i = 0; i < availableCameras.length; i++)
        {
            listModel.push(availableCameras[i].displayName)
        }
        return listModel;
    }

    function findDeviceId(cameraName)
    {
        var i;
        var availableCameras = QtMultimedia.availableCameras;
        for(i = 0; i < availableCameras.length; i++)
        {
            if(availableCameras[i].displayName === cameraName)
            {
                return availableCameras[i].deviceId;
            }
        }
        return QtMultimedia.defaultCamera.deviceId
    }

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

        comboBoxCamera.model = makeCameraList();
        var indexCamera = comboBoxCamera.find(applicationSettings.cameraName)
        comboBoxCamera.currentIndex = indexCamera

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


    TimeSettings
    {
        id: timeSettings

        onSetTime:
        {
            system.setTime(time)
        }
    }

}
