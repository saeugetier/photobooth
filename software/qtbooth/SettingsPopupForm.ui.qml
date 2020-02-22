import QtQuick 2.4
import QtQuick.Controls 2.2

Popup {
    id: popup
    width: 400
    height: 400

    DelayButton {
        id: buttonDeletePhotos
        x: 239
        y: 321
        text: qsTr("Delete all photos")
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
            id: button
            text: qsTr("Copy photos to removable disk")
        }

        Button {
            id: button1
            text: qsTr("Copy layout templates from removable disk")
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
}

/*##^##
Designer {
    D{i:2;anchors_y:23}D{i:5;anchors_y:23}D{i:10;anchors_x:39;anchors_y:-69}
}
##^##*/

