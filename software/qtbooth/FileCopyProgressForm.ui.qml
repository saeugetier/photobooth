import QtQuick 2.4
import QtQuick.Controls 2.3
import QtQuick.Window 2.2

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
