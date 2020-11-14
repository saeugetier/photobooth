import QtQuick 2.4
import QtQml.Models 2.11
import Qt.labs.folderlistmodel 2.1

CollageReprintForm {

    FolderListModel
    {
        id: folderModel
        folder: applicationSettings.foldername + "/collage"
        nameFilters: ["*.jpg", "*.JPG"]
        showDirs: false
        sortField: FolderListModel.Unsorted
    }

    gridViewCollages.model: folderModel

    buttonClose.clicked:
    {
        close()
    }

    closePolicy: "NoAutoClose"
}
