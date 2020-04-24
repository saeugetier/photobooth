import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Item {
    property alias imageSource: proxy.imageFileName
    property alias effectSource: proxy.effectName
    property alias borderImageSource: borderImage.source
    property alias number: proxy.number
    property alias proxy: proxy
    property int borderSizeLeft : 30
    property int borderSizeRight : 30
    property int borderSizeTop : 30
    property int borderSizeBottom : 30

    signal deletePhoto(int number)

    PhotoProxy
    {
        id: proxy
        anchors.fill: parent
    }

    BorderImage {
        id: borderImage

        border.left: borderSizeLeft
        border.top: borderSizeTop
        border.bottom: borderSizeBottom
        border.right: borderSizeRight

        anchors.horizontalCenter: proxy.horizontalCenter
        anchors.verticalCenter: proxy.verticalCenter
        width: proxy.width; height: proxy.height
        visible: source != "" ? true : false
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        z: 1
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

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
