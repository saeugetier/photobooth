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

    title: qsTr("QML Photo Booth")

    Printer
    {
        id: printer
    }

    CollageModelFactory
    {
        id: modelFactory
        source: "XmlData.xml"
    }

    ApplicationFlow
    {
        height: parent.height
        width: parent.width
        collageMenu.printer : printer
    }

    Settings
    {
        id: applicationSettings
        category: "Application"
        property string foldername: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property bool recycleBinEnabled: false
        property bool printEnable: true
        property string password: "0815"
    }
}
