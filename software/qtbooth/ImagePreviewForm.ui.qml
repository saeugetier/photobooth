import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias deleteButton: deleteButton
    property alias saveButton: saveButton
    property alias fileLoadIndicator: fileLoadIndicator
    property alias previewImage: previewImage

    Image {
        id: previewImage
        anchors.topMargin: 100
        anchors.bottomMargin: 100
        anchors.leftMargin: 100
        anchors.rightMargin: 100
        anchors.fill: parent
        rotation: -10
        fillMode: Image.PreserveAspectFit

        BorderImage {
            anchors.horizontalCenter: previewImage.horizontalCenter
            anchors.verticalCenter: previewImage.verticalCenter
            id: borderImage
            border.bottom: 65
            border.top: 25
            border.right: 25
            border.left: 25
            width: previewImage.paintedWidth + 40
            height: previewImage.paintedHeight + 60
            anchors.verticalCenterOffset: 10
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            source: "images/polaroid.svg.png"
        }
    }

    BusyIndicator {
        id: fileLoadIndicator
        anchors.verticalCenterOffset: 0
        anchors.centerIn: parent
    }

    NavigationButton {
        id: saveButton
        x: 122
        y: 453
        text: "Accept"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 40
    }

    NavigationButton {
        id: deleteButton
        text: "Retry"
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40
        forward: false
    }
}

/*##^##
Designer {
    D{i:1;anchors_height:342;anchors_width:484;anchors_x:73;anchors_y:64}
}
##^##*/

