import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import QtQuick.Controls.Material 2.2
import Qt.labs.folderlistmodel 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0
import CollageModel 1.0
import Selphy 1.0

ApplicationWindow {
    id: mainWindow
    visible: true
    visibility: "Maximized"
    width: 640
    height: 480

    FontLoader
    {
        name: "fontello"
        source: "../font/fontello/fontello.ttf"
    }

    FontLoader
    {
        name: "DejaVu Serif"
        source: "../font/DejaVuSerif/DejaVuSerif.ttf"
    }

    title: qsTr("QML Photo Booth")

    Printer
    {
        id: printer
    }

    function findCollagesFile()
    {
        var path = StandardPaths.locate(StandardPaths.AppLocalDataLocation, "Collages.xml")
        console.log(StandardPaths.standardLocations(StandardPaths.AppLocalDataLocation))
        console.log("Path: " + path)
        if(path != "")
            return path
        else
            return "../XmlData.xml"
    }

    CollageModelFactory
    {
        id: modelFactory
        source: findCollagesFile()
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

        mainMenu.settingsPopup.comboBoxLanguages.onDisplayTextChanged:
        {
            applicationSettings.language = mainMenu.settingsPopup.comboBoxLanguages.displayText
            console.log("Language changed to: " + mainMenu.settingsPopup.comboBoxLanguages.displayText)
            translation.setLanguage(applicationSettings.language)
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
    }

    Settings
    {
        id: applicationSettings
        category: "Application"
        property url foldername: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property bool printEnable: true
        property string password: "0815"
        property string language: "en"

        Component.onCompleted:
        {
            flow.mainMenu.settingsPopup.printerEnabled.checked = printEnable
            flow.mainMenu.settingsPinCode = password
        }
    }
}
