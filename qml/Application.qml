import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import Qt.labs.folderlistmodel
import QtCore
import Qt.labs.platform
import CollageModel
import Printer
import QtQuick.Window
import Libcamera

ApplicationWindow {
    id: mainWindow
    visible: true
    visibility: applicationSettings.windowMode
    width: 640
    height: 480

    property Printer printer : printerFactory.getPrinter(applicationSettings.printerName)

    Component.onCompleted:
    {
        filesystem.checkImageFolders()
    }

/*  FontLoader
    {
        name: "fontello"
        source: "qrc:/font/fontello/fontello.ttf"
    }

    FontLoader
    {
        name: "DejaVu Serif"
        source: "qrc:/font/DejaVuSerif/DejaVuSerif.ttf"
    }*/

    title: qsTr("QML Photo Booth")

    PrinterFactory
    {
        id: printerFactory
        Component.onCompleted:
        {
            flow.settingsMenu.comboBoxPrinter.model = printerFactory.printers
        }
    }

    function findCollagesFile()
    {
        var path = StandardPaths.locate(StandardPaths.AppLocalDataLocation, "Collages.xml")
        console.log(StandardPaths.standardLocations(StandardPaths.AppLocalDataLocation))
        console.log("Path: " + path)
        if(path.len > 0)
            return path
        else
            return "qrc:/XmlData.xml"
    }

    CollageModelFactory
    {
        id: modelFactory
        source: findCollagesFile()
    }

    onPrinterChanged:
    {
        flow.collageMenu.printer = printer
        flow.galleryMenu.printer = printer
    }

    Libcamera {
       id: libcamera
    }

    ApplicationFlow
    {
        id: flow
        height: parent.height
        width: parent.width
        collageMenu.printer : printer
        libcamera: libcamera

        // Set up camera wake-up GPIO when settings change
        onSnapshotMenuChanged: {
            if (snapshotMenu && snapshotMenu.cameraRenderer) {
                var cameraDevice = snapshotMenu.cameraRenderer.camera
                if (cameraDevice && applicationSettings.gpioCameraWakeupEnabled && 
                    applicationSettings.gpioCameraWakeupLine >= 0) {
                    // This would require exposing setCameraWakeupGpio from C++
                    // For now, this is a placeholder for future implementation
                    console.log("Camera wake-up GPIO configured: line " + applicationSettings.gpioCameraWakeupLine)
                }
            }
        }

        settingsMenu.switchPrinter.onCheckedChanged:
        {
            applicationSettings.printEnable = settingsMenu.switchPrinter.checked
        }

        settingsMenu.switchMultiplePrints.onCheckedChanged:
        {
            applicationSettings.multiplePrints = settingsMenu.switchMultiplePrints.checked
        }

        settingsMenu.switchPrintFromGallery.onCheckedChanged:
        {
            applicationSettings.printFromGallery = settingsMenu.switchPrintFromGallery.checked
        }

        settingsMenu.switchHideSnapshotSettings.onCheckedChanged:
        {
            applicationSettings.disableSnapshotSettingsPane = settingsMenu.switchHideSnapshotSettings.checked
        }

        settingsMenu.switchHideEffectPopup.onCheckedChanged:
        {
            applicationSettings.disableEffectPopup = settingsMenu.switchHideEffectPopup.checked
        }

        settingsMenu.comboBoxLanguages.onDisplayTextChanged:
        {
            applicationSettings.language = settingsMenu.comboBoxLanguages.displayText
            console.log("Language changed to: " + settingsMenu.comboBoxLanguages.displayText)
            translation.setLanguage(applicationSettings.language)
        }

        settingsMenu.switchMirrorCamera.onCheckedChanged:
        {
            applicationSettings.cameraMirrored = settingsMenu.switchMirrorCamera.checked
        }

        settingsMenu.comboBoxLanguages.onCountChanged:
        {
            if(settingsMenu.comboBoxLanguages.count !== 0)
            {
                var indexOfLanguage = settingsMenu.comboBoxLanguages.find(applicationSettings.language)
                settingsMenu.comboBoxLanguages.currentIndex = indexOfLanguage
                console.log("Language selected: " + Number(indexOfLanguage).toString() + " - " + applicationSettings.language + " - " + Number(settingsMenu.comboBoxLanguages.count).toString())
                translation.setLanguage(applicationSettings.language)
            }
        }

        settingsMenu.comboBoxPrinter.onCurrentTextChanged:
        {
            if(flow.settingsMenu.opened)
            {
                applicationSettings.printerName = settingsMenu.comboBoxPrinter.currentText
            }
        }

        settingsMenu.comboWindowMode.onCurrentIndexChanged:
        {
            applicationSettings.windowMode = settingsMenu.comboWindowMode.currentIndex === 0 ? Window.Maximized : Window.FullScreen
            console.log("Window mode changed to: " + applicationSettings.windowMode)
        }

        settingsMenu.comboBoxCamera.onCurrentValueChanged:
        {
            applicationSettings.cameraName = settingsMenu.comboBoxCamera.currentValue
        }

        settingsMenu.switchEnableSettingsPassword.onCheckedChanged:
        {
            applicationSettings.enableSettingsPassword = settingsMenu.switchEnableSettingsPassword.checked
        }

        settingsMenu.comboBoxCameraOrientation.onCurrentValueChanged:
        {
            if(slideValueSettingsMenu !== 0)
            {
                return
            }
            applicationSettings.cameraOrientation = Number(settingsMenu.comboBoxCameraOrientation.currentValue)
            console.log("Camera orientation changed to: " + applicationSettings.cameraOrientation)
        }

        settingsMenu.comboBoxNeuralNetworkRuntime.onCurrentValueChanged:
        {
            if(slideValueSettingsMenu !== 0)
            {
                return
            }
            applicationSettings.neuralNetworkRuntime = String(settingsMenu.comboBoxNeuralNetworkRuntime.currentValue)
            console.log("Neural network runtime changed to: " + applicationSettings.neuralNetworkRuntime)
        }

        settingsMenu.switchEnableGpio.onCheckedChanged:
        {
            applicationSettings.gpioEnabled = settingsMenu.switchEnableGpio.checked
        }

        settingsMenu.comboBoxGpioChip.onCurrentValueChanged:
        {
            applicationSettings.gpioChip = String(settingsMenu.comboBoxGpioChip.currentValue)
        }

        settingsMenu.comboBoxLedEnableLine.onCurrentValueChanged:
        {
            applicationSettings.gpioLedEnableLine = Number(settingsMenu.comboBoxLedEnableLine.currentValue)
        }

        settingsMenu.comboBoxLedBrightnessLine.onCurrentValueChanged:
        {
            applicationSettings.gpioLedBrightnessLine = Number(settingsMenu.comboBoxLedBrightnessLine.currentValue)
        }

        settingsMenu.spinBoxPwmFrequency.onValueChanged:
        {
            applicationSettings.gpioPwmFrequency = settingsMenu.spinBoxPwmFrequency.value
        }

        settingsMenu.comboBoxBoardPreset.onCurrentValueChanged:
        {
            applicationSettings.gpioBoardPreset = String(settingsMenu.comboBoxBoardPreset.currentValue)
        }

        settingsMenu.switchInvertPwm.onCheckedChanged:
        {
            applicationSettings.gpioInvertPwm = settingsMenu.switchInvertPwm.checked
        }

        settingsMenu.switchCameraWakeup.onCheckedChanged:
        {
            applicationSettings.gpioCameraWakeupEnabled = settingsMenu.switchCameraWakeup.checked
        }

        settingsMenu.comboBoxCameraWakeupLine.onCurrentValueChanged:
        {
            applicationSettings.gpioCameraWakeupLine = Number(settingsMenu.comboBoxCameraWakeupLine.currentValue)
        }

        settingsMenu.spinBoxCameraWakeupDelay.onValueChanged:
        {
            applicationSettings.gpioCameraWakeupDelayMs = settingsMenu.spinBoxCameraWakeupDelay.value
        }

        mainMenu.printerBusy: printer ? printer.busy : false
    }

    Settings
    {
        id: applicationSettings
        category: "Application"
        property url foldername: StandardPaths.writableLocation(StandardPaths.PicturesLocation) + "/photobooth"
        property bool printEnable: true
        property string password: "0815"
        property string language: "en"
        property bool cameraMirrored: true
        property string printerName: "No Printer"
        property int windowMode: Number(Window.Maximized)
        property bool multiplePrints: false
        property bool disableSnapshotSettingsPane: false
        property bool disableEffectPopup: false
        property string cameraName: ""
        property bool printFromGallery: true
        property bool enableSettingsPassword: false
        property int cameraOrientation: 0
        property string neuralNetworkRuntime: "ONNX"
        property bool gpioEnabled: false
        property string gpioBoardPreset: "rpi"
        property string gpioChip: "/dev/gpiochip0"
        property int gpioLedEnableLine: 23
        property int gpioLedBrightnessLine: 18
        property int gpioPwmFrequency: 1000
        property bool gpioInvertPwm: true
        property bool gpioCameraWakeupEnabled: false
        property int gpioCameraWakeupLine: -1
        property int gpioCameraWakeupDelayMs: 100

        Component.onCompleted:
        {
            flow.settingsMenu.printerEnabled.checked = printEnable
            flow.settingsMenu.switchPrintFromGallery.checked = printFromGallery
            flow.settingsMenu.switchMultiplePrints.checked = multiplePrints
            flow.mainMenu.settingsPinCode = password
            flow.settingsMenu.mirrorCamera.checked = cameraMirrored
            flow.settingsMenu.switchHideSnapshotSettings.checked = disableSnapshotSettingsPane
            flow.settingsMenu.switchHideEffectPopup.checked = disableEffectPopup
            flow.settingsMenu.switchEnableSettingsPassword.checked = enableSettingsPassword
            flow.mainMenuModel.setShowPrintable(printEnable)
            flow.collageMenu.multiplePrints = multiplePrints
            flow.snapshotMenu.hideSnapshotSettingsPane = disableSnapshotSettingsPane
            flow.imagePreview.effectButton.visible = !disableEffectPopup

            // GPIO settings
            flow.settingsMenu.switchEnableGpio.checked = gpioEnabled
            flow.settingsMenu.switchInvertPwm.checked = gpioInvertPwm
            flow.settingsMenu.switchCameraWakeup.checked = gpioCameraWakeupEnabled
            flow.settingsMenu.spinBoxCameraWakeupDelay.value = gpioCameraWakeupDelayMs
        }

        onPrinterNameChanged:
        {
            printer = printerFactory.getPrinter(printerName)
        }

        onPrintEnableChanged:
        {
            flow.mainMenuModel.setShowPrintable(printEnable)
        }

        onMultiplePrintsChanged:
        {
            flow.collageMenu.multiplePrints = multiplePrints
        }

        onDisableSnapshotSettingsPaneChanged:
        {
            flow.snapshotMenu.hideSnapshotSettingsPane = disableSnapshotSettingsPane
        }

        onDisableEffectPopupChanged:
        {
            flow.imagePreview.effectButton.visible = !disableEffectPopup
        }
    }
}
