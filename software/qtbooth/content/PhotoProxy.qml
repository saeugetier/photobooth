import QtQuick 2.0

Item {
    property string imageFileName : ""
    property int number : 0

    id: photoProxy

    Image {
        anchors.fill: parent
        id: image
        source: imageFileName
        asynchronous: true
        sourceSize.height: 512
        sourceSize.width: 512
        fillMode: Image.PreserveAspectFit
    }

    Rectangle
    {
        id: numberRect
        anchors.fill: parent
        color: "gray"
    }

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
}
