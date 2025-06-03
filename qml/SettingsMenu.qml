import QtQuick
import QtMultimedia
import QtQuick.Controls

SettingsMenuForm {
    id: form
    property alias printerEnabled: form.switchPrinter
    property alias mirrorCamera: form.switchMirrorCamera
    property alias comboBoxCameraOrientation: form.comboBoxCameraOrientation

    function makeCameraList()
    {
        var listModel = [];
        var i;
        var availableCameras = mediaDevices.videoInputs;
        console.log("Available Camera Count: " + Number(availableCameras.length).toString())
        for(i = 0; i < availableCameras.length; i++)
        {
            listModel.push(availableCameras[i].description)
        }
        return listModel;
    }

    MediaDevices
    {
        id: mediaDevices
    }

    function findDeviceId(cameraName)
    {
        var i;
        var availableCameras = mediaDevices.videoInputs;
        for(i = 0; i < availableCameras.length; i++)
        {
            if(availableCameras[i].description === cameraName)
            {
                return availableCameras[i].id;
            }
        }
        return mediaDevices.defaultVideoInput.id
    }

    Component.onCompleted:
    {
        versionText = "Version: " + system.getGitHash()

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

    buttonDeletePhotos.onActivated:
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
}
