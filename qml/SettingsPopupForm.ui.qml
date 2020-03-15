import QtQuick 2.4
import QtQuick.Controls 2.2

Popup {
    id: popup
    width: 400
    height: 400
    property alias comboBoxLanguages: comboBoxLanguages
    property alias buttonShutdown: buttonShutdown
    property alias buttonRestart: buttonRestart
    property alias buttonCopyTemplates: buttonCopyTemplates
    property alias buttonDeletePhotos: buttonDeletePhotos
    property alias buttonClose: buttonClose
    property alias switchPrinter: switchPrinter
    property alias buttonCopyPhotos: buttonCopyPhotos

    DelayButton {
        id: buttonDeletePhotos
        x: 239
        y: 321
        text: qsTr("Delete all photos")
        delay: 5000
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
    }

    Button {
        id: buttonClose
        x: 262
        text: qsTr("Close")
        anchors.top: parent.top
        anchors.right: parent.right
    }

    Text {
        id: headerText
        x: 10
        y: 10
        color: "#ffffff"
        text: qsTr("Settings")
        font.pixelSize: 24
    }

    Column {
        id: mainLayout
        y: 60
        spacing: 20

        Row {
            id: rowPrinter

            Label {
                id: labelPrinter
                text: qsTr("Enable Printing")
                anchors.verticalCenter: switchPrinter.verticalCenter
                horizontalAlignment: Text.AlignLeft
            }

            Switch {
                id: switchPrinter
                text: qsTr("enabled")
            }
        }

        Button {
            id: buttonCopyPhotos
            text: qsTr("Copy photos to removable disk")
        }

        Button {
            id: buttonCopyTemplates
            text: qsTr("Copy layout templates from removable disk")
        }

        ComboBox {
            id: comboBoxLanguages
            model: translation.languages
        }
    }

    ToolSeparator {
        id: toolSeparator
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 50
        orientation: Qt.Horizontal
    }

    Button {
        id: buttonRestart
        text: qsTr("Restart")
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
    }

    Button {
        id: buttonShutdown
        text: qsTr("Shutdown")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.left: buttonRestart.right
        anchors.leftMargin: 30
    }
}

/*##^##
Designer {
    D{i:2;anchors_y:23}D{i:5;anchors_y:23}D{i:11;anchors_x:39;anchors_y:"-69"}
}
##^##*/

