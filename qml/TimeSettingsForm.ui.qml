import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.10
import QtQuick.Controls

Popup {
    property alias buttonApply: buttonApply
    property alias buttonCancel: buttonCancel
    //property alias calendar: calendar
    property alias tumblerHour: tumblerHour
    property alias tumblerMinute: tumblerMinute
    property alias tumblerSecond: tumblerSecond

    ColumnLayout {
        anchors.fill: parent
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignCenter
        spacing: 5
        //Calendar {
        //    id: calendar
        //}
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Label {
                id: labelTime
                text: qsTr("Time: ")
            }

            Tumbler {
                id: tumblerHour
                height: 60
                model: 24
            }

            Tumbler {
                id: tumblerMinute
                height: 60
                model: 60
            }

            Tumbler {
                id: tumblerSecond
                height: 60
                model: 60
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Button {
                id: buttonCancel
                text: qsTr("Cancel")
            }

            Button {
                id: buttonApply
                text: qsTr("Apply")
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

