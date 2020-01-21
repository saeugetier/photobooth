import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    property url imageFileName : ""
    property int number : 0

    signal deletePhoto(int number)

    id: photoProxy

    Image {
        anchors.fill: parent
        id: image
        source: imageFileName
        asynchronous: true
        cache: false
        sourceSize.height: 2048
        sourceSize.width: 2048
        visible: false
        fillMode: Image.PreserveAspectCrop
    }

    Rectangle
    {
        id: numberRect
        anchors.fill: parent
        color: "gray"
        z: 0.5
        Text
        {
            anchors.fill: parent
            text: Number(number + 1).toString()
            fontSizeMode: Text.Fit
            font.pixelSize: 144
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    ToolButton {
        id: deleteButton
        anchors.horizontalCenter: image.right
        anchors.verticalCenter: image.top
        text: "\uE81F"  // cancel button
        font.family: "fontello"
        font.pixelSize: 32
        enabled: true
        z: 1 // on top
        visible: false

        opacity: 0.7

        onClicked:
        {
            deletePhoto(number)
        }
    }

    states:
    [
        State
        {
            name: "number"
            PropertyChanges { target: numberRect; visible: true }
            PropertyChanges { target: image; visible: false }
            PropertyChanges { target: deleteButton; visible: false }
        },
        State
        {
            name: "image"
            PropertyChanges { target: numberRect; visible: false }
            PropertyChanges { target: image; visible: true }
            PropertyChanges { target: deleteButton; visible: true }
        }
    ]

    onImageFileNameChanged:
    {
        if(imageFileName == "")
        {
            state = "number"
        }
        else
        {
            state = "image"
        }
    }
}
