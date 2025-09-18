import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsMenu
    width: 400
    height: 600
    property alias switchHideSnapshotSettings: switchHideSnapshotSettings
    property alias switchHideEffectPopup: switchHideEffectPopup
    property alias switchMultiplePrints: switchMultiplePrints
    property alias buttonCloseProgram: buttonCloseProgram
    property alias comboWindowMode: comboWindowMode
    property alias labelTime: labelTime
    property alias comboBoxLanguages: comboBoxLanguages
    property alias buttonShutdown: buttonShutdown
    property alias buttonRestart: buttonRestart
    property alias buttonCopyTemplates: buttonCopyTemplates
    property alias buttonDeletePhotos: buttonDeletePhotos
    property alias buttonClose: buttonClose
    property alias switchPrinter: switchPrinter
    property alias switchPrintFromGallery: switchPrintFromGallery
    property alias buttonCopyPhotos: buttonCopyPhotos
    property alias buttonCopyPhotosCustomLocation: buttonCopyPhotosCustomLocation
    property alias switchMirrorCamera: switchMirrorCamera
    property alias comboBoxPrinter: comboBoxPrinter
    property alias comboBoxCamera: comboBoxCamera
    property alias switchEnableSettingsPassword: switchEnableSettingsPassword
    property alias versionText: labelVersionText.text
    property alias comboBoxCameraOrientation: comboBoxCameraOrientation
    property alias comboBoxNeuralNetworkRuntime: comboBoxNeuralNetworkRuntime
    property alias buttonSelectPhotoDirectory: buttonSelectPhotoDirectory

    ColumnLayout {
        anchors.fill: parent

        // Header Section
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10

            Text {
                text: qsTr("Settings")
                font.pixelSize: 24
                Layout.alignment: Qt.AlignLeft
                color: "#ffffff"
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Close Settings")
                id: buttonClose
            }
        }

        ToolSeparator {
            Layout.fillWidth: true
        }

        // Tabs
        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Photos")
            }
            TabButton {
                text: qsTr("Camera")
            }
            TabButton {
                text: qsTr("Printer")
            }
            TabButton {
                text: qsTr("System")
            }
        }

        // Content
        StackLayout {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // Photos Tab
            Item {
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    spacing: 20

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Photo Directory: ")
                        }
                        Label {
                            id: labelPhotoDirectory
                            Layout.fillWidth: true
                            text: applicationSettings.foldername
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Button {
                            id: buttonSelectPhotoDirectory
                            text: qsTr("Browse")
                        }
                    }

                    Button {
                        id: buttonCopyPhotos
                        text: qsTr("Copy photos to removable disk")
                    }

                    Button {
                        id: buttonCopyPhotosCustomLocation
                        text: qsTr("Copy photos to custom location")
                    }

                    ToolSeparator {
                        Layout.fillWidth: true
                        orientation: Qt.Horizontal
                    }

                    DelayButton {
                        id: buttonDeletePhotos
                        text: qsTr("Delete all photos")
                        delay: 5000
                    }
                }
            }

            // Camera Tab
            Item {
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    spacing: 10

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Camera:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: comboBoxCamera
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Mirror Camera:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchMirrorCamera
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Snapshot Settings:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchHideSnapshotSettings
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Effect Popup:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchHideEffectPopup
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Camera Orientation:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: comboBoxCameraOrientation
                            textRole: "text"
                            valueRole: "value"
                            model: [{
                                    "value": 0,
                                    "text": qsTr("Landscape 0°")
                                }, {
                                    "value": 90,
                                    "text": qsTr("Portrait 90°")
                                }, {
                                    "value": 180,
                                    "text": qsTr("Landscape 180°")
                                }, {
                                    "value": 270,
                                    "text": qsTr("Portrait 270°")
                                }]
                        }
                    }

                    RowLayout
                    {
                        spacing: 10
                        Label
                        {
                            text: qsTr("Neuroal Network Runtime")
                        }
                        Item
                        {
                            Layout.fillWidth: true
                        }
                        ComboBox
                        {
                            id: comboBoxNeuralNetworkRuntime
                            textRole: "text"
                            valueRole: "value"
                            model: [{text: "ONNX Runtime", value: "ONNX"}, {text: "NCNN Runtime", value: "NCNN"}]
                            Layout.preferredWidth: 200
                        }
                    }
                }
            }

            // Printer Tab
            Item {

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    spacing: 10

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Enable Printing:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchPrinter
                        }
                    }

                    RowLayout {
                        visible: switchPrinter.checked
                        spacing: 10
                        Label {
                            text: qsTr("Printer:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            Layout.preferredWidth: 300
                            id: comboBoxPrinter
                        }
                    }

                    ToolSeparator {
                        visible: switchPrinter.checked
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        visible: switchPrinter.checked
                        spacing: 10
                        Label {
                            text: qsTr("Allow multiple prints:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchMultiplePrints
                        }
                    }

                    ToolSeparator {
                        visible: switchPrinter.checked
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        visible: switchPrinter.checked
                        spacing: 10
                        Label {
                            text: qsTr("Print from Gallery:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchPrintFromGallery
                        }
                    }
                }
            }

            // System Tab
            Item {
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    spacing: 10

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Enable Settings Password:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: switchEnableSettingsPassword
                        }
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Current Time:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Label {
                            id: labelTime
                            text: "HH:MM"

                        }
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Language:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: comboBoxLanguages
                            model: translation.languages
                        }
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Window Mode:")
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: comboWindowMode
                            model: ["Window", "Fullscreen"]
                        }
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Copy layout templates from removable disk")
                        id: buttonCopyTemplates
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Button {
                            text: qsTr("Shutdown")
                            id: buttonShutdown
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Button {
                            text: qsTr("Restart")
                            id: buttonRestart
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Button {
                            text: qsTr("Exit Photobooth")
                            id: buttonCloseProgram
                        }
                    }

                    ToolSeparator {
                        orientation: Qt.Horizontal
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Label {
                            text: qsTr("Version: 1.0.0")
                            id: labelVersionText
                        }
                    }
                }
            }
        }
    }
}
