import QtQuick 2.5
import Qt.labs.folderlistmodel 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.2
import QtQuick.Window 2.0
import QtQml.Models 2.2

Item
{
    property alias foldermodel: folderModel

    Rectangle {
            id: root
            anchors.fill: parent

            color: "grey"

            property real downloadProgress: 0
            property bool imageLoading: false

            FolderListModel {
                    id: folderModel
                    folder: applicationSettings.foldername
                    nameFilters: ["*.jpg","*.JPG"]
                    showDirs: false
                    sortReversed: true //show newest photo first
            }

            DelegateModel {
                id: visualModel
                delegate: DelegateGalleryItem {}
                model: folderModel
            }

            GridView {
                id: photosGridView

                x: 0; y: 0; cellWidth: 160; cellHeight: 153
                width: root.width; height: root.height - progressBar.height
                model: visualModel.parts.grid
                interactive: true
                displayMarginBeginning:  10000
                displayMarginEnd: 10000
                /*Rectangle {
                    anchors.centerIn: parent
                    width: parent.width-10; height: parent.height-10
                    color: "transparent"
                    border.color: "red"
                    border.width: 5
                }*/
                onCurrentIndexChanged: {
                    photosListView.positionViewAtIndex(currentIndex, ListView.Contain)
                }
            }

            ListView {
                id: photosListView;

                width: root.width; height: root.height - progressBar.height
                orientation: Qt.Horizontal
                model: visualModel.parts.list;
                interactive: false

                highlightRangeMode: ListView.StrictlyEnforceRange
                snapMode: ListView.SnapOneItem
                /*Rectangle {
                    anchors.centerIn: parent
                    width: parent.width-10; height: parent.height-10
                    color: "transparent"
                    border.color: "green"
                    border.width: 5
                }*/
                onCurrentIndexChanged: {
                    photosGridView.positionViewAtIndex(currentIndex, GridView.Contain)
                }
            }

            state: 'inGrid'
            states: [
                State {
                    name: 'inGrid'
                    PropertyChanges { target: photosGridView; interactive: true; visible: true }
                    PropertyChanges { target: photosListView; visible: false }
                },
                State {
                    name: 'fullscreen'; extend: 'inGrid'
                    PropertyChanges { target: photosGridView; interactive: false; visible: false }
                    PropertyChanges { target: photosListView;  visible: true }
                }
            ]

            MouseArea {
                anchors.fill: root
                z: root.state == 'inGrid' ? -1 : 0
                onClicked: root.state = 'inGrid'
            }
        }

        Item {
            id: foreground
            anchors.fill: parent
        }

        ProgressBar {
            id: progressBar

            width: parent.width; height: 15
            anchors.bottom: parent.bottom
            opacity: root.imageLoading

            value: root.downloadProgress
        }
}
