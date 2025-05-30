import QtQuick
import QtCore
import Qt.labs.folderlistmodel

SnapshotSettingsForm {
    property alias flashEnabled: settings.flashEnabled
    property alias viewFinderBrightness: settings.viewFinderBrightness
    property alias flashBrightness: settings.flashBrightness
    property alias countdown: settings.countdown
    property alias backgroundFilterEnabled: settings.backgroundFilterEnabled
    property alias chromaKeyEnabled: settings.chromaKeyEnabled
    property alias chromaKeyColor: settings.chromaKeyColor
    property alias backgroundImage: settings.backgroundImage

    Settings
    {
        id: settings
        category: "Camera"
        property bool flashEnabled: true
        property real viewFinderBrightness: 0.1
        property real flashBrightness: 1.0
        property int countdown: 1
        property bool backgroundFilterEnabled: false
        property bool chromaKeyEnabled: false
        property real chromaKeyColor: 0.5
        property url backgroundImage: "qrc:/images/backgrounds/Brickwall.jpg"
    }

    function findBackgroundFiles()
    {
        // use default backgrounds if no user backgrounds are found
        var path = StandardPaths.locate(StandardPaths.AppLocalDataLocation, "backgrounds", StandardPaths.LocateDirectory)

        if(path.len > 0)
        {
            // check if the path is a valid directory and display this custom path in log
            console.log("Background images path: " + path)
            return path
        }
        else
        {
            // use default backgrounds if no custom backgrounds are provided
            return "qrc:/images/backgrounds/"
        }
    }

    FolderListModel
    {
        id: backgroundImageModel
        folder: findBackgroundFiles()
        nameFilters: ["*.jpg", "*.png", "*.JPG", "*.PNG"]
        showDirs: false
        sortField: FolderListModel.Name
    }

    backgroundImageSelectorModel: backgroundImageModel

    showButton.onClicked:
    {
        if(state !== "expanded")
        {
            state = "expanded"
        }
        else
        {
            state = "idle"
        }
    }

    sliderFlashBrightness.value: settings.flashBrightness
    sliderFlashBrightness.onValueChanged:
    {
        if(sliderFlashBrightness.value < sliderViewfinderBrightness.value)
        {
            sliderFlashBrightness.value = sliderViewfinderBrightness.value
        }
        settings.flashBrightness = sliderFlashBrightness.value
    }

    sliderViewfinderBrightness.value: settings.viewFinderBrightness
    sliderViewfinderBrightness.onValueChanged:
    {
        if(sliderFlashBrightness.value < sliderViewfinderBrightness.value)
        {
            sliderFlashBrightness.value = sliderViewfinderBrightness.value
        }
        settings.viewFinderBrightness = sliderViewfinderBrightness.value
    }

    switchFlashEnable.checked: settings.flashEnabled
    switchFlashEnable.onCheckedChanged:
    {
        settings.flashEnabled = switchFlashEnable.checked
    }

    spinBoxCountdownTime.value: settings.countdown
    spinBoxCountdownTime.onValueChanged:
    {
        settings.countdown = spinBoxCountdownTime.value
    }

    switchBackgroundFilterEnable.checked: settings.backgroundFilterEnabled
    switchBackgroundFilterEnable.onCheckedChanged:
    {
        settings.backgroundFilterEnabled = switchBackgroundFilterEnable.checked
    }

    switchChromaKeyEnable.checked: settings.chromaKeyEnabled
    switchChromaKeyEnable.onCheckedChanged:
    {
        settings.chromaKeyEnabled = switchChromaKeyEnable.checked
    }

    sliderChromaKeyFilterColor.value: settings.chromaKeyColor
    sliderChromaKeyFilterColor.onValueChanged:
    {
        settings.chromaKeyColor = sliderChromaKeyFilterColor.value
    }

    backgroundImageSelector.currentIndex: backgroundImageSelectorModel.indexOf(settings.backgroundImage)
    backgroundImageSelector.onCurrentIndexChanged:
    {
        if(backgroundImageSelectorModel.count > 0)
        {
            console.log("selected index " + backgroundImageSelector.currentIndex)
            if (backgroundImageSelector.currentIndex >= 0)
            {
                settings.backgroundImage = backgroundImageSelectorModel.get(backgroundImageSelector.currentIndex, "fileUrl")
                console.log("selected background " + settings.backgroundImage)
            }
        }
    }

    Behavior on width
    {
        PropertyAnimation
        {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }
}
