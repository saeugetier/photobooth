import QtQuick 2.4
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.0

Pane {
    width: 300

    Row {
        id: row
        anchors.fill: parent

        Column {
            id: column
            width: 200
            height: 400

            Switch {
                id: element
                text: qsTr("Switch")
            }
        }
    }
}

/*##^##
Designer {
    D{i:1;anchors_height:400;anchors_width:200;anchors_x:50;anchors_y:15}
}
##^##*/

