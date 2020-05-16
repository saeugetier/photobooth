import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import "content"

Item {
    id: element
    width: 640
    height: 480
    property alias effectButton: effectButton
    property alias effectSelector: effectSelector
    property alias deleteButton: deleteButton
    property alias saveButton: saveButton
    property alias fileLoadIndicator: fileLoadIndicator
    property alias previewImage: previewImage
    property bool effectSelectable: true

    Image {
        id: previewImage

        asynchronous: true

        mipmap: true

        // INPUTS
        property double rightMargin: 100
        property double bottomMargin: 100
        property double leftMargin: 100
        property double topMargin: 100
        property double aimedRatio: sourceSize.height / sourceSize.width

        // SIZING
        property double availableWidth: parent.width - rightMargin - leftMargin
        property double availableHeight: parent.height - bottomMargin - topMargin

        property bool parentIsLarge: parentRatio > aimedRatio

        property double parentRatio: availableHeight / availableWidth

        height: parentIsLarge ? width * aimedRatio : availableHeight
        width: parentIsLarge ? availableWidth : height / aimedRatio

        property double verticalSpacing: (availableHeight - height) / 2
        property double horzitontalSpacing: (availableWidth - width) / 2

        anchors.top: parent.top
        anchors.topMargin: topMargin + verticalSpacing
        anchors.left: parent.left
        anchors.leftMargin: leftMargin + horzitontalSpacing

        rotation: -10
        layer.enabled: true
    }

    BorderImage {
        anchors.horizontalCenter: previewImage.horizontalCenter
        anchors.verticalCenter: previewImage.verticalCenter
        id: borderImage
        border.bottom: 65
        border.top: 25
        border.right: 25
        border.left: 25
        rotation: -10
        width: previewImage.paintedWidth + 40
        height: previewImage.paintedHeight + 60
        anchors.verticalCenterOffset: 10
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        source: "../../images/polaroid.svg.png"
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

    EffectSelector {
        id: effectSelector

        visible: false

        anchors.right: element.right
        anchors.top: element.top
        anchors.bottom: element.bottom

        previewImage: previewImage.source
    }

    ToolButton {
        id: effectButton
        text: "\uF0D0" // icon-print
        font.family: "fontello"
        font.pointSize: 75
        enabled: true

        visible: effectSelectable

        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40

        scale: hovered ? 1.1 : 1

        layer.enabled: true
        layer.effect: Glow {
            color: "black"
            samples: 20
            spread: 0.3
        }
    }

    states: [
        State {
            name: "idle"
        },
        State {
            name: "effectSelection"

            PropertyChanges {
                target: effectSelector
                visible: true
            }

            PropertyChanges {
                target: effectButton
                visible: false
            }

            PropertyChanges {
                target: saveButton
                visible: false
            }

            PropertyChanges {
                target: deleteButton
                visible: false
            }
        }
    ]
}

/*##^##
Designer {
    D{i:1;anchors_height:342;anchors_width:484;anchors_x:73;anchors_y:64}
}
##^##*/

