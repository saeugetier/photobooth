import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Item {
    property alias imageSource: proxy.imageFileName
    property alias borderImageSource: border.source
    property alias number: proxy.number
    property alias proxy: proxy

    signal deletePhoto(int number)

    BorderImage {
        id: border
        width: 100; height: 100
        border.left: 5; border.top: 5
        border.right: 5; border.bottom: 5
    }

    PhotoProxy
    {
        id: proxy
        anchors.fill: parent
    }

    ToolButton {
        id: deleteButton
        anchors.right: proxy.right
        anchors.top: proxy.top
        anchors.rightMargin: 20
        anchors.topMargin: 20
        text: "\uE81F"  // cancel button
        font.family: "fontello"
        font.pixelSize: 32
        enabled: true
        z: 1 // on top
        visible: imageArea.containsMouse && (proxy.state == "image")

        Material.foreground: Material.Red

        opacity: 0.7

        onClicked:
        {
            deletePhoto(number)
        }
    }

    MouseArea
    {
        id: imageArea
        anchors.fill: proxy
        hoverEnabled: true
    }
}
