import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.10
import QtGraphicalEffects 1.0

Popup {
    id: popup
    property alias gridViewCollages: gridViewCollages
    property alias buttonClose: buttonClose
    property alias buttonPrint: buttonPrint
    Button {
        id: buttonClose
        text: qsTr("Close")
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 20
    }

    GridView {
        id: gridViewCollages
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 100
        anchors.bottomMargin: 100
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        cellWidth: 450
        cellHeight: 450

        interactive: true

        clip: true

        delegate: Image {
            id: collageImage
            property double rightMargin: 50
            property double bottomMargin: 150
            property double leftMargin: 50
            property double topMargin: 30
            property double aimedRatio: 2380 / 3570

            // SIZING
            property double availableWidth: 400
            property double availableHeight: 400

            property bool parentIsLarge: parentRatio > aimedRatio

            property double parentRatio: availableHeight / availableWidth

            height: parentIsLarge ? width * aimedRatio : availableHeight
            width: parentIsLarge ? availableWidth : height / aimedRatio

            property double verticalSpacing: (availableHeight - height) / 2
            property double horzitontalSpacing: (availableWidth - width) / 2

            x: leftMargin + horzitontalSpacing

            source: fileURL
            sourceSize.height: 1024
            sourceSize.width: 1024
            fillMode: Image.PreserveAspectFit

            asynchronous: true

            BorderImage {
                anchors.horizontalCenter: collageImage.horizontalCenter
                anchors.verticalCenter: collageImage.verticalCenter
                id: borderImage
                border.bottom: 65
                border.top: 25
                border.right: 25
                border.left: 25
                width: collageImage.paintedWidth + 40
                height: collageImage.paintedHeight + 60
                anchors.verticalCenterOffset: 10
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                source: "../images/polaroid.svg.png"
            }

            BrightnessContrast {
                anchors.fill: borderImage
                source: borderImage
                brightness: collageImage.GridView.isCurrentItem ? 1.0 : 0.5
                contrast: 1.0
            }
        }
    }

    Button {
        id: buttonPrint
        text: qsTr("Print")
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

