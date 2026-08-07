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
    property alias switchEnableGpio: switchEnableGpio
    property alias comboBoxBoardPreset: comboBoxBoardPreset
    property alias comboBoxGpioChip: comboBoxGpioChip
    property alias comboBoxLedEnableLine: comboBoxLedEnableLine
    property alias comboBoxLedBrightnessLine: comboBoxLedBrightnessLine
    property alias spinBoxPwmFrequency: spinBoxPwmFrequency
    property alias switchInvertPwm: switchInvertPwm
    property alias switchCameraWakeup: switchCameraWakeup
    property alias comboBoxCameraWakeupLine: comboBoxCameraWakeupLine
    property alias spinBoxCameraWakeupDelay: spinBoxCameraWakeupDelay
    property var libcamera

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
                id: buttonClose
                text: qsTr("Close Settings")
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
            TabButton {
                text: qsTr("GPIO")
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
                Flickable {
                    id: photosScrollView
                    anchors.fill: parent
                    interactive: true
                    clip: true
                    contentWidth: width
                    contentHeight: Math.max(height, photosColumnLayout.y + photosColumnLayout.height + 24)
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: photosColumnLayout
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 20
                        height: implicitHeight
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
            }

            // Camera Tab
            Item {
                Flickable {
                    id: cameraScrollView
                    anchors.fill: parent
                    interactive: true
                    clip: true
                    contentWidth: width
                    contentHeight: Math.max(height, cameraColumnLayout.y + cameraColumnLayout.height + 24)
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: cameraColumnLayout
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 20
                        height: implicitHeight
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
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
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
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                                model: [
                                    {
                                        "value": 0,
                                        "text": qsTr("Landscape 0°")
                                    },
                                    {
                                        "value": 90,
                                        "text": qsTr("Portrait 90°")
                                    },
                                    {
                                        "value": 180,
                                        "text": qsTr("Landscape 180°")
                                    },
                                    {
                                        "value": 270,
                                        "text": qsTr("Portrait 270°")
                                    }
                                ]
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Label {
                                text: qsTr("Neuroal Network Runtime")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxNeuralNetworkRuntime
                                textRole: "text"
                                valueRole: "value"
                                Layout.preferredWidth: 250
                            }
                        }
                    }
                }
            }

            // Printer Tab
            Item {
                Flickable {
                    id: printerScrollView
                    anchors.fill: parent
                    interactive: true
                    clip: true
                    contentWidth: width
                    contentHeight: Math.max(height, printerColumnLayout.y + printerColumnLayout.height + 24)
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: printerColumnLayout
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 20
                        height: implicitHeight
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
                                id: comboBoxPrinter
                                Layout.preferredWidth: 300
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
            }

            // System Tab
            Item {
                Flickable {
                    id: systemScrollView
                    anchors.fill: parent
                    interactive: true
                    clip: true
                    contentWidth: width
                    contentHeight: Math.max(height, systemColumnLayout.y + systemColumnLayout.height + 24)
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: systemColumnLayout
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 20
                        height: implicitHeight
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
                            id: buttonCopyTemplates
                            text: qsTr("Copy layout templates from removable disk")
                        }

                        ToolSeparator {
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: buttonShutdown
                                text: qsTr("Shutdown")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Button {
                                id: buttonRestart
                                text: qsTr("Restart")
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: buttonCloseProgram
                                text: qsTr("Exit Photobooth")
                            }
                        }

                        ToolSeparator {
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 10
                            Label {
                                id: labelVersionText
                                text: qsTr("Version: 1.0.0")
                            }
                        }
                    }
                }
            }

            // GPIO Tab
            Item {
                Flickable {
                    id: gpioScrollView
                    anchors.fill: parent
                    interactive: true
                    clip: true
                    contentWidth: width
                    contentHeight: Math.max(height, gpioColumnLayout.y + gpioColumnLayout.height + 24)
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: gpioColumnLayout
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 20
                        height: implicitHeight
                        spacing: 10

                        RowLayout {
                            spacing: 10
                            Label {
                                text: qsTr("Enable GPIO:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Switch {
                                id: switchEnableGpio
                            }
                        }

                        ToolSeparator {
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("Board Preset:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxBoardPreset
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                                model: [
                                    {
                                        value: "rpi",
                                        text: qsTr("Raspberry Pi 3/4/5")
                                    },
                                    {
                                        value: "orangepi3b",
                                        text: qsTr("Orange Pi 3B (RK3566)")
                                    },
                                    {
                                        value: "custom",
                                        text: qsTr("Custom")
                                    }
                                ]
                            }
                        }

                        ToolSeparator {
                            visible: switchEnableGpio.checked
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("GPIO Chip:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxGpioChip
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                                enabled: comboBoxBoardPreset.currentValue === "custom"
                            }
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("LED Enable Line:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxLedEnableLine
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                                enabled: comboBoxBoardPreset.currentValue === "custom"
                            }
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("LED Brightness Line:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxLedBrightnessLine
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                                enabled: comboBoxBoardPreset.currentValue === "custom"
                            }
                        }

                        ToolSeparator {
                            visible: switchEnableGpio.checked
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("PWM Frequency (Hz):")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: spinBoxPwmFrequency
                                from: 100
                                to: 10000
                                stepSize: 100
                                value: 1000
                                editable: true
                                enabled: comboBoxBoardPreset.currentValue === "custom"
                            }
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("Invert PWM:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Switch {
                                id: switchInvertPwm
                            }
                        }

                        ToolSeparator {
                            visible: switchEnableGpio.checked
                            orientation: Qt.Horizontal
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked
                            spacing: 10
                            Label {
                                text: qsTr("Enable Camera Wake-up:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Switch {
                                id: switchCameraWakeup
                            }
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked && switchCameraWakeup.checked
                            spacing: 10
                            Label {
                                text: qsTr("Camera Wake-up Line:")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ComboBox {
                                id: comboBoxCameraWakeupLine
                                Layout.preferredWidth: 300
                                textRole: "text"
                                valueRole: "value"
                            }
                        }

                        RowLayout {
                            visible: switchEnableGpio.checked && switchCameraWakeup.checked
                            spacing: 10
                            Label {
                                text: qsTr("Wake-up Delay (ms):")
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: spinBoxCameraWakeupDelay
                                from: 0
                                to: 5000
                                stepSize: 10
                                value: 100
                                editable: true
                            }
                        }
                    }
                }
            }
        }
    }
}
