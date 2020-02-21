import QtQuick 2.4
import QtQml.Models 2.11
import Qt.labs.folderlistmodel 2.1
import "scripts/utils.js" as Utils

MainMenuForm {
    id: form
    property alias collageRenderer: form.collageRenderer
    property alias iconModel: form.iconModel
    property string selectedCollageName : ""
    state: "IconNotSelected"
    signal collageSelected

    property real scrollPosition: 0.0
    property real scrollDuration: 50000.0

    FolderListModel
    {
        id: folderModel
        folder: applicationSettings.foldername
        nameFilters: ["*.jpg", "*.JPG"]
        showDirs: false
        sortField: FolderListModel.Unsorted
    }

    collageSelector.onSelected:
    {
        state = "IconSelected"
        selectedCollageName = collageSelector.iconName
    }

    continueButton.onClicked:
    {
        collageSelected()
    }

    backButton.onClicked:
    {
        state = "IconNotSelected"
    }

    imageSlider.model: folderModel //Utils.shuffle(folderModel)

    onScrollPositionChanged:
    {
        imageSlider.contentY = scrollPosition * (imageSlider.contentHeight - imageSlider.height)
    }

    imageSlider.onModelChanged:
    {
        console.log("image list changed")
    }

    imageSlider.onContentHeightChanged:
    {
        console.log("list content height: " + Number(imageSlider.contentHeight).toString())
        scrollDuration = imageSlider.contentHeight * 10
        scrollAnimation.restart()
    }

    SettingsPassword {
        id: settingsPassword
        anchors.centerIn: parent

        onPasswordAccepted:
        {
            settingsPopup.open()
        }
    }

    SettingsPopup
    {
        id: settingsPopup
        anchors.centerIn: parent

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
    }

    settingsButton.onClicked:
    {
        settingsPassword.open()
    }

    SequentialAnimation
    {
        id: scrollAnimation
        running: true
        loops: Animation.Infinite
        NumberAnimation{
            target: form
            property: "scrollPosition"
            from: 0.0
            to: 1.0
            duration: scrollDuration
        }
        NumberAnimation
        {
            target: form
            property: "scrollPosition"
            from: 1.0
            to: 0.0
            duration: scrollDuration
        }
    }
}



/*##^##
Designer {
    D{i:0;height:480;width:640}
}
##^##*/
