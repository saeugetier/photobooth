import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import QtQuick.Controls.Material 2.2
import Qt.labs.folderlistmodel 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0
import CollageModel 1.0
import Printer 1.0
import QtQuick.Window 2.2

ApplicationWindow {
    id: mainWindow
    visible: true
    visibility: applicationSettings.windowMode
    width: 640
    height: 480

    property Printer printer : printerFactory.getPrinter(applicationSettings.printerName)

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
            flow.mainMenu.settingsPopup.comboBoxPrinter.model = printerFactory.printers
        }
    }

    function findCollagesFile()
    {
        var path = StandardPaths.locate(StandardPaths.AppLocalDataLocation, "Collages.xml")
        console.log(StandardPaths.standardLocations(StandardPaths.AppLocalDataLocation))
        console.log("Path: " + path)
        if(path != "")
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
    }

    ApplicationFlow
    {
        id: flow
        height: parent.height
        width: parent.width
        collageMenu.printer : printer

        mainMenu.settingsPopup.switchPrinter.onCheckedChanged:
        {
            applicationSettings.printEnable = mainMenu.settingsPopup.switchPrinter.checked
        }

        mainMenu.settingsPopup.switchMultiplePrints.onCheckedChanged:
        {
            applicationSettings.multiplePrints = mainMenu.settingsPopup.switchMultiplePrints.checked
        }

        mainMenu.settingsPopup.switchHideSnapshotSettings.onCheckedChanged:
        {
            applicationSettings.disableSnapshotSettingsPane = mainMenu.settingsPopup.switchHideSnapshotSettings.checked
        }

        mainMenu.settingsPopup.switchHideEffectPopup.onCheckedChanged:
        {
            applicationSettings.disableEffectPopup = mainMenu.settingsPopup.switchHideEffectPopup.checked
        }

        mainMenu.settingsPopup.comboBoxLanguages.onDisplayTextChanged:
        {
            applicationSettings.language = mainMenu.settingsPopup.comboBoxLanguages.displayText
            console.log("Language changed to: " + mainMenu.settingsPopup.comboBoxLanguages.displayText)
            translation.setLanguage(applicationSettings.language)
        }

        mainMenu.settingsPopup.switchMirrorCamera.onCheckedChanged:
        {
            applicationSettings.cameraMirrored = mainMenu.settingsPopup.switchMirrorCamera.checked
        }

        mainMenu.settingsPopup.comboBoxLanguages.onCountChanged:
        {
            if(mainMenu.settingsPopup.comboBoxLanguages.count != 0)
            {
                var indexOfLanguage = mainMenu.settingsPopup.comboBoxLanguages.find(applicationSettings.language)
                mainMenu.settingsPopup.comboBoxLanguages.currentIndex = indexOfLanguage
                console.log("Language selected: " + Number(indexOfLanguage).toString() + " - " + applicationSettings.language + " - " + Number(mainMenu.settingsPopup.comboBoxLanguages.count).toString())
                translation.setLanguage(applicationSettings.language)
            }
        }

        mainMenu.settingsPopup.comboBoxPrinter.onCurrentTextChanged:
        {
            if(flow.mainMenu.settingsPopup.opened)
            {
                applicationSettings.printerName = mainMenu.settingsPopup.comboBoxPrinter.currentText
            }
        }

        mainMenu.settingsPopup.comboWindowMode.onCurrentIndexChanged:
        {
            applicationSettings.windowMode = mainMenu.settingsPopup.comboWindowMode.currentIndex == 0 ? Window.Maximized : Window.FullScreen
        }

        mainMenu.printerBusy: printer.busy
    }

    Settings
    {
        id: applicationSettings
        category: "Application"
        property url foldername: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property bool printEnable: true
        property string password: "0815"
        property string language: "en"
        property bool cameraMirrored: true
        property string printerName: "No Printer"
        property int windowMode: Window.Maximized
        property bool multiplePrints: false
        property bool disableSnapshotSettingsPane: false
        property bool disableEffectPopup: false

        Component.onCompleted:
        {
            flow.mainMenu.settingsPopup.printerEnabled.checked = printEnable
            flow.mainMenu.settingsPopup.switchMultiplePrints.checked = multiplePrints
            flow.mainMenu.settingsPinCode = password
            flow.mainMenu.settingsPopup.mirrorCamera.checked = cameraMirrored
            flow.mainMenu.settingsPopup.switchHideSnapshotSettings.checked = disableSnapshotSettingsPane
            flow.mainMenu.settingsPopup.switchHideEffectPopup = disableEffectPopup
            flow.mainMenuModel.setShowPrintable(printEnable)
            flow.collageMenu.multiplePrints = multiplePrints
            flow.snapshotMenu.hideSnapshotSettingsPane = disableSnapshotSettingsPane
            flow.imagePreview.effectButton.visible = !disableEffectPopup
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
