import QtQuick 2.0

Item {
    property alias imageSource: proxy.imageFileName
    property alias borderImageSource: border.source
    property alias number: proxy.number
    property alias proxy: proxy
    //property rect imageRect

    //sanchors.fill: parent

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
        //x: parent.x * imageRect.x
        //y: parent.y * imageRect.y
        //width: parent.width * imageRect.width
        //height: parent.height * imageRect.height
    }

}