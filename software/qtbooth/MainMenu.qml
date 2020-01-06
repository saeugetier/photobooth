import QtQuick 2.4
import QtQml.Models 2.11
import Qt.labs.folderlistmodel 2.1
import "scripts/utils.js" as Utils

MainMenuForm {
    id: form
    property alias iconModel: form.iconModel
    state: "IconNotSelected"
    signal collageSelected

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
        labelIconName.text = collageSelector.iconName
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
}



/*##^##
Designer {
    D{i:0;height:480;width:640}
}
##^##*/
