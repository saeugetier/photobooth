import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.10

Popup {
    id: popup
    width: 400
    height: 500
    property alias labelTime: labelTime
    property alias comboBoxLanguages: comboBoxLanguages
    property alias buttonShutdown: buttonShutdown
    property alias buttonRestart: buttonRestart
    property alias buttonCopyTemplates: buttonCopyTemplates
    property alias buttonDeletePhotos: buttonDeletePhotos
    property alias buttonClose: buttonClose
    property alias switchPrinter: switchPrinter
    property alias buttonCopyPhotos: buttonCopyPhotos
    property alias buttonSetTime: buttonSetTime
    property alias switchMirrorCamera: switchMirrorCamera
    property alias comboBoxPrinter: comboBoxPrinter

    Button {
        id: buttonClose
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        text: qsTr("Close")
    }

    Text {
        id: headerText
        anchors.topMargin: 5
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.left: parent.left
        color: "#ffffff"
        text: qsTr("Settings")
        font.pixelSize: 24
    }

    ToolSeparator {
        id: toolSeparator
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 50
        orientation: Qt.Horizontal
    }

    TabBar {
        id: tabBar
        width: 240
        anchors.topMargin: 0
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        anchors.top: toolSeparator.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        TabButton {
            id: tabButtonPhotos
            text: qsTr("Photos")
        }

        TabButton {
            id: tabButtonCamera
            text: qsTr("Camera")
        }

        TabButton {
            id: tabButtonPrinter
            text: qsTr("Printer")
        }

        TabButton {
            id: tabButtonSystem
            text: qsTr("System")
        }
    }

    StackLayout {
        id: stackView

        anchors.topMargin: 5
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabBar.currentIndex

        Item {
            id: elementPhotos
            Column {
                spacing: 5
                Button {
                    id: buttonCopyPhotos
                    text: qsTr("Copy photos to removable disk")
                }

                DelayButton {
                    id: buttonDeletePhotos
                    text: qsTr("Delete all photos")
                    delay: 5000
                }
            }
        }

        Item {
            id: elementCamera
            Column {
                spacing: 5
                Row {
                    spacing: 5
                    Label {
                        id: labelCamera
                        text: qsTr("Camera:")
                        anchors.verticalCenter: comboBoxCamera.verticalCenter
                    }
                    ComboBox {
                        id: comboBoxCamera
                    }
                }

                Row {
                    id: rowMirrorCamera
                    spacing: 5
                    Label {
                        id: labelMirrorCamera
                        text: qsTr("Mirror Camera")
                        anchors.verticalCenter: switchMirrorCamera.verticalCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    Switch {
                        id: switchMirrorCamera
                        text: qsTr("mirror")
                    }
                }
            }
        }

        Item {
            id: elementPrinter
            Column {
                spacing: 5
                Row {
                    id: rowPrinterEnable
                    spacing: 5
                    Label {
                        id: labelPrinterEnable
                        text: qsTr("Enable Printing")
                        anchors.verticalCenter: switchPrinter.verticalCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    Switch {
                        id: switchPrinter
                        text: qsTr("enabled")
                    }
                }

                Row {
                    id: rowPrinter
                    spacing: 5
                    Label {
                        id: labelPrinter
                        text: qsTr("Printer: ")
                        anchors.verticalCenter: comboBoxPrinter.verticalCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    ComboBox {
                        id: comboBoxPrinter
                    }

                    Switch {

                        text: qsTr("enabled")
                    }
                }
            }
        }

        Item {
            id: elementSystem

            Column {
                spacing: 5

                Row {
                    spacing: 5

                    Label {
                        id: labelTime
                        text: qsTr("Current time")
                        anchors.verticalCenter: buttonSetTime.verticalCenter
                    }

                    Button {
                        id: buttonSetTime
                        text: qsTr("Set time")
                    }
                }

                Row {
                    spacing: 5
                    Label {
                        id: labelLanguage
                        text: qsTr("Language:")
                        anchors.verticalCenter: comboBoxLanguages.verticalCenter
                    }
                    ComboBox {
                        id: comboBoxLanguages
                        model: translation.languages
                    }
                }

                Button {
                    id: buttonCopyTemplates
                    text: qsTr("Copy layout templates from removable disk")
                }

                Row {
                    spacing: 10
                    Button {
                        id: buttonShutdown
                        text: qsTr("Shutdown")
                    }

                    Button {
                        id: buttonRestart
                        text: qsTr("Restart")
                    }
                }
            }
        }
    }
}
