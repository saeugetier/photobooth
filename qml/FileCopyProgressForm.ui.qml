import QtQuick
import QtQuick.Controls

Dialog {
    width: 600
    height: 200
    property alias copyProgressBar: copyProgressBar

    title: "Copying files..."

    ProgressBar {
        id: copyProgressBar
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        value: 0.5
    }
}
