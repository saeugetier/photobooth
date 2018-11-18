import QtQuick 2.5
import Qt.labs.folderlistmodel 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.1
import QtQuick.Window 2.0
import QtQml.Models 2.2
import "content"

Item {
    property alias foldermodel: folderModel
    property alias scrollUpButton: scrollBarUpButton
    property alias scrollDownButton: scrollBarDownButton
    property alias scrollBar: galleryScrollbar
    property alias photosGrid: photosGridView
    property alias photoView: photosListView
    property alias viewState: root.state

    Rectangle {
        id: root
        anchors.fill: parent

        color: "grey"

        property real downloadProgress: 0
        property bool imageLoading: false

        FolderListModel {
            id: folderModel
            folder: applicationSettings.foldername
            nameFilters: ["*.jpg", "*.JPG"]
            showDirs: false
            sortReversed: true //show newest photo first
        }

        DelegateModel {
            id: visualModel
            delegate: DelegateGalleryItem {
            }
            model: folderModel
        }

        GridView {
            id: photosGridView

            x: 0
            y: 0
            cellWidth: 160
            cellHeight: 153
            width: root.width
            height: root.height - progressBar.height
            model: visualModel.parts.grid
            interactive: true
            displayMarginBeginning: 10000
            displayMarginEnd: 10000

            //rightMargin: 100


            /*Rectangle {
                    anchors.centerIn: parent
                    width: parent.width-10; height: parent.height-10
                    color: "transparent"
                    border.color: "red"
                    border.width: 5
                }*/
            ScrollBar.vertical: ScrollBar {
                id: galleryScrollbar
                contentItem.implicitWidth: 50
                bottomPadding: 50
                topPadding: 50
                Button {
                    id: scrollBarUpButton
                    text: "\uE819" // icon-folder-open-empty
                    font.family: "fontello"
                    width: 50
                    height: 50
                    anchors.top: parent.top
                    font.pointSize: 30
                }
                Button {
                    id: scrollBarDownButton
                    text: "\uE81C" // icon-folder-open-empty
                    font.family: "fontello"
                    anchors.bottom: parent.bottom
                    width: 50
                    height: 50
                    font.pointSize: 30
                }
            }
        }

        ListView {
            id: photosListView

            width: root.width
            height: root.height - progressBar.height
            orientation: Qt.Horizontal
            model: visualModel.parts.list
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
        }

        state: 'inGrid'
        states: [
            State {
                name: 'inGrid'
                PropertyChanges {
                    target: photosGridView
                    interactive: true
                    visible: true
                }
                PropertyChanges {
                    target: photosListView
                    visible: false
                }
            },
            State {
                name: 'fullscreen'
                extend: 'inGrid'
                PropertyChanges {
                    target: photosGridView
                    interactive: false
                    visible: false
                }
                PropertyChanges {
                    target: photosListView
                    visible: true
                }
            }
        ]

        MouseArea {
            anchors.fill: root
            z: root.state == 'inGrid' ? -1 : 0
            onClicked: {
                root.state = 'inGrid'
                galleryShuffleTimer.stop()
            }
        }
    }

    Item {
        id: foreground
        anchors.fill: parent
    }

    ProgressBar {
        id: progressBar

        width: parent.width
        height: 15
        anchors.bottom: parent.bottom
        opacity: root.imageLoading

        value: root.downloadProgress
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
