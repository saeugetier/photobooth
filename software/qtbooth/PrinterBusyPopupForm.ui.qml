import QtQuick 2.4
import QtQuick.Controls 2.3
import "content"

Popup {
    id: popup
    width: 600
    height: 400
    property alias button: button

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    Button {
        id: button
        x: 248
        y: 293
        width: 85
        height: 65
        text: qsTr("OK")
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 32
    }

    Text {
        id: element
        text: qsTr("Printer is busy currently. Please stand by.")
        anchors.top: parent.top
        anchors.topMargin: 16
        horizontalAlignment: Text.AlignHCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        wrapMode: Text.WordWrap
        font.pixelSize: 72
    }
}

/*##^##
Designer {
    D{i:2;anchors_x:8;anchors_y:16}
}
##^##*/
