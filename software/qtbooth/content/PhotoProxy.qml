import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Item {
    property url imageFileName : ""
    property int number : 0
    property bool enableDeleteButton : true

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

    state: "number"

    states:
    [
        State
        {
            name: "number"
            PropertyChanges { target: numberRect; visible: true }
            PropertyChanges { target: image; visible: false }
        },
        State
        {
            name: "image"
            PropertyChanges { target: numberRect; visible: false }
            PropertyChanges { target: image; visible: true }
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
