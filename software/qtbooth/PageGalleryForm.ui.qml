import QtQuick 2.4
import Qt.labs.folderlistmodel 2.1
import QtQuick.Layouts 1.2

GridView {
    cellWidth: 160; cellHeight: 160

    FolderListModel {
        id: folderModel
        folder: applicationSettings.foldername
        nameFilters: ["*.jpg"]
        showDirs: false
    }


    model: folderModel
    delegate: Item {
        width: 160; height: 160

        id: fileDelegate

        RowLayout{

            Image {
                asynchronous: true
                source: applicationSettings.foldername + "/" + fileName
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: fileName
            }
        }
    }
}
