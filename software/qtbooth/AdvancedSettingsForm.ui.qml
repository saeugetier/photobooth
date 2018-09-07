import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0

Popup {
    id: advancedSettingsPopup
    property alias cancelButton: cancelButton
    property alias restartButton: restartButton
    property alias shutDownButton: shutDownButton
    property alias closeButton: closeButton
    property alias printerEnabled: printerEnabled
    property alias copyButton: copyButton
    property alias deleteButton: deleteButton
    property alias recycleEnabled: recycleEnabled
    modal: true

    ColumnLayout {

        Button {
            id: cancelButton
            x: 576
            text: qsTr("X")
            anchors.right: parent.right
            anchors.rightMargin: 0
        }

        GridLayout {
            id: gridLayout
            width: 100
            height: 100

            Label {
                id: label
                text: qsTr("Printer:")
            }

            Switch {
                id: printerEnabled
                text: qsTr("enabled")
            }
        }

        GridLayout {
            width: 100
            height: 100

            Label {
                text: qsTr("Recycle Bin:")
            }

            Switch {
                id: recycleEnabled
                text: qsTr("enabled")
            }
        }

        Button {
            id: copyButton
            text: qsTr("Copy images to removable drive")
        }

        DelayButton {
            id: deleteButton
            text: "Delete Images"
            delay: 5000
        }

        RowLayout {
            Button {
                id: closeButton
                text: qsTr("Close Program")
                Layout.preferredHeight: 48
                Layout.preferredWidth: 131
            }

            Button {
                id: shutDownButton
                text: qsTr("Shut Down")
                Layout.preferredHeight: 48
                Layout.preferredWidth: 131
            }

            Button {
                id: restartButton
                text: qsTr("Restart")
                Layout.preferredHeight: 48
                Layout.preferredWidth: 131
            }
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
