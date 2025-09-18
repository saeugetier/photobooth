import QtQuick
import QtMultimedia
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.platform
import QtQml
import "content"

SettingsMenuForm {
    id: form
    property alias printerEnabled: form.switchPrinter
    property alias mirrorCamera: form.switchMirrorCamera
    property alias comboBoxCameraOrientation: form.comboBoxCameraOrientation
    signal exitSettings

    comboWindowMode.currentIndex: applicationSettings.windowMode === Window.Maximized ? 0 : 1

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

    function delay(delayTime, cb) {
        timer = new Timer();
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }

    CustomFolderDialog {
        id: pictureFolderDialog
        title: qsTr("Select Pictures Folder")
        anchors.centerIn: parent
        width: parent.width - 100
        height: parent.height - 100

        Timer
        {
            id: folderCheckTimer
            interval: 1000
            running: false
            repeat: false
            onTriggered: function() {
                filesystem.checkImageFolders()
                console.log("Checked folder: " + applicationSettings.foldername)
            }
        }

        onAccepted: function()
        {
            console.log("Selected folder: " + pictureFolderDialog.currentFolder)
            applicationSettings.foldername = pictureFolderDialog.currentFolder
            applicationSettings.sync()

            folderCheckTimer.start()
        }
    }

    CustomFolderDialog
    {
        id: pictureCopyFolderDialog
        title: qsTr("Select Copy Pictures Folder")
        anchors.centerIn: parent
        width: parent.width - 100
        height: parent.height - 100

        onAccepted: function()
        {
            console.log("Selected copy folder: " + pictureCopyFolderDialog.currentFolder)
            copyProgressPopup.targetPath = pictureCopyFolderDialog.currentFolder
            copyProgressPopup.open()
        }
    }

    buttonSelectPhotoDirectory.onClicked:
    {
        pictureFolderDialog.currentFolder = applicationSettings.foldername
        console.log("selecting pictures folder: " + pictureFolderDialog.currentFolder)
        pictureFolderDialog.open()
    }

    buttonCopyPhotosCustomLocation.onClicked:
    {
        pictureCopyFolderDialog.currentFolder = StandardPaths.writableLocation(StandardPaths.HomeLocation)
        pictureCopyFolderDialog.open()
    }

    buttonClose.onClicked:
    {
        exitSettings()
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
        copyProgressPopup.targetPath = filesystem.getRemovableDrivePath()
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
