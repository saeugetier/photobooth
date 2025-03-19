import QtQuick 2.5
import Qt.labs.folderlistmodel 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.1
import QtQuick.Window 2.0
import QtQml.Models 2.2
import Printer 1.0
import "content"

Item {
    property alias foldermodel: folderModel
    property alias scrollUpButton: scrollBarUpButton
    property alias scrollDownButton: scrollBarDownButton
    property alias scrollBar: galleryScrollbar
    property alias photosGrid: photosGridView
    property alias photoView: photosListView
    property alias viewState: root.state
    property alias exitButton: exitButton
    property alias photoButton: photoButton
    property alias collageButton: collageButton
    property Printer printer

    Image {
        id: root
        anchors.fill: parent

        source: "../images/cardboard.png"
        fillMode: Image.Tile

        property bool imageLoading: false

        Rectangle
        {
            id: sidebar
            width: 150
            height: parent.height
            color: "#4d60ea"

            ColumnLayout
            {
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter

                Button
                {
                    id: photoButton

                    Layout.alignment: Qt.AlignLeft
                    flat: true
                    implicitWidth: parent.width
                    implicitHeight: parent.height / 2

                    contentItem: Item {
                        anchors.fill: parent

                        Text {
                            text: qsTr("Photos")

                            color: "white"
                            font.family: "DejaVu Serif"
                            font.pixelSize: 50
                            anchors.centerIn: parent
                            font.capitalization: Font.AllUppercase

                            rotation: 270
                        }
                    }


                }

                Button
                {
                    id: collageButton

                    Layout.alignment: Qt.AlignLeft
                    flat: true
                    implicitWidth: parent.width
                    implicitHeight: parent.height / 2

                    contentItem: Item {
                        anchors.fill: parent

                        Text {
                            text: qsTr("Collages")

                            color: "white"
                            font.family: "DejaVu Serif"
                            font.pixelSize: 50
                            anchors.centerIn: parent
                            font.capitalization: Font.AllUppercase

                            rotation: 270
                        }
                    }
                }
            }
        }

        FolderListModel {
            id: folderModel
            folder: applicationSettings.foldername
            nameFilters: ["*.jpg", "*.JPG", "*.png", "*.PNG"]
            showDirs: false
            sortReversed: true //show newest photo first
        }

        DelegateModel {
            id: visualModel
            delegate: DelegateGalleryItem {
                onPrintImage:
                {
                    printerPopup.visible = true
                    printer.printImage(filename, 1)
                    printerPopup.visible = false
                }
            }
            model: folderModel
        }

        GridView {
            id: photosGridView

            x: 0
            y: 0
            cellWidth: 320
            cellHeight: 306
            anchors.left: sidebar.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            model: visualModel.parts.grid
            interactive: true
            //displayMarginBeginning: 10000
            //displayMarginEnd: 10000

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
            height: root.height
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

        NavigationButton {
            id: exitButton
            y: 515
            text: qsTr("Exit")
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.left: sidebar.right
            anchors.leftMargin: 20
            forward: false
        }

        PrinterPopup {
            id: printerPopup
            anchors.centerIn: parent
            visible: false
            isPrinting: true
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

        //MouseArea {
        //    anchors.fill: root
        //    z: root.state == 'inGrid' ? -1 : 0
        //    onClicked: {
        //        root.state = 'inGrid'
        //    }
        //}
    }

    Item {
        id: foreground
        anchors.fill: parent
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
