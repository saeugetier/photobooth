import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

Item {
    width: 640
    height: 480
    property alias fileLoadIndicator: fileLoadIndicator
    property alias deleteButton: deleteButton
    property alias saveButton: saveButton
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
    }

    DropShadow {
        anchors.fill: previewImage
        horizontalOffset: 3
        verticalOffset: 0
        radius: 30.0
        samples: 17
        rotation: previewImage.rotation
        color: "#80000000"
        source: previewImage
    }

    ToolButton {
        id: saveButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -150
        anchors.bottom: parent.bottom
        text: "\uE803" // icon-save
        font.family: "fontello"
        font.pointSize: 72

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }
    ToolButton {
        id: deleteButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 150
        anchors.bottom: parent.bottom
        text: "\uE800" // icon-folder-open-empty
        font.family: "fontello"
        font.pointSize: 72

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    BusyIndicator {
        id: fileLoadIndicator
        anchors.centerIn: parent
    }
}

/*##^##
Designer {
    D{i:1;anchors_height:342;anchors_width:484;anchors_x:73;anchors_y:64}
}
##^##*/
