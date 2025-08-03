// CustomFolderDialog.qml
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Qt.labs.folderlistmodel 2.2
import QtQuick.VirtualKeyboard 6.5
import QtCore

Dialog {
    id: root
    modal: true
    width: 800
    height: 600
    standardButtons: Dialog.NoButton

    property url currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)

    signal canceled()

    property bool createMode: false

    Rectangle {
        anchors.fill: parent
        color: Material.background
        border.color: Material.foreground
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Text {
                text: "Select Folder"
                font.pixelSize: 20
                Layout.alignment: Qt.AlignHCenter
            }

            // Path display field (read-only)
            Label {
                text: currentFolder
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WrapAnywhere
                font.pixelSize: 14
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "Go Up"
                    enabled: currentFolder !== "file:///"
                    onClicked: {
                        var path = currentFolder.toString().replace("file://", "")
                        if (path.endsWith("/")) path = path.slice(0, -1)
                        var parts = path.split("/")

                        if (parts.length > 1) {
                            parts.pop() // Go up one folder
                        } else {
                            parts = [""] // Ensure root "/"
                        }

                        var newPath = "file://" + (parts.join("/") || "/")
                        currentFolder = newPath
                        folderModel.folder = currentFolder
                    }
                }

                Button {
                    text: createMode ? "Cancel" : "New Folder"
                    onClicked: {
                        createMode = !createMode
                        if (!createMode)
                            newFolderNameField.text = ""
                    }
                }
            }

            // Inline New Folder Creation
            RowLayout {
                visible: createMode
                Layout.fillWidth: true
                spacing: 10

                TextField {
                    id: newFolderNameField
                    placeholderText: "New folder name"
                    Layout.fillWidth: true
                    inputMethodHints: Qt.ImhNoPredictiveText
                    focus: createMode
                    onActiveFocusChanged: {
                        if (activeFocus && createMode) {
                            Qt.inputMethod.show()
                        } else {
                            Qt.inputMethod.hide()
                        }
                    }
                }

                Button {
                    text: "Create"
                    Button {
                        text: "Create"
                        onClicked: {
                                const name = newFolderNameField.text.trim()
                                if (name.length > 0) {
                                    var basePath = currentFolder.toString().replace("file://", "")
                                    var newPath = basePath + "/" + name
                                    var success = filesystem.createFolder(newPath)
                                    console.log("Folder created:", success, newPath)

                                    if (success) {
                                        createMode = false
                                        newFolderNameField.text = ""
                                        Qt.inputMethod.hide()
                                    }
                                }
                            }
                    }
                }
            }

            ListView {
                id: folderList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                model: FolderListModel {
                    id: folderModel
                    folder: currentFolder
                    showDirs: true
                    showFiles: false
                    sortField: FolderListModel.Name
                }

                delegate: Rectangle {
                    width: folderList.width
                    height: 48
                    color: model.fileURL === currentFolder ? "#ddeeff" : "transparent"

                    Text {
                        text: fileName
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        font.pixelSize: 16
                        color: Material.foreground
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.currentFolder = model.fileURL
                            folderModel.folder = model.fileURL
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Button {
                    text: "Select"
                    onClicked: {
                        root.accepted()
                        root.close()
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: {
                        root.close()
                        root.canceled()
                    }
                }
            }
        }

        // Virtual Keyboard - only visible if TextField is active & createMode is on
        InputPanel {
            id: inputPanel
            y: parent.height - height
            width: parent.width
            visible: Qt.inputMethod.visible && createMode
            z: 100
        }
    }
}
