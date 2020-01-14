import QtQuick 2.0
import CollageModel 1.0

Item {
    id: renderer
    property CollageImageModel imageModel

    Image
    {
        anchors.fill: parent
        source: imageModel.backgroundImage
        fillMode: Image.PreserveAspectFit
    }

    Component
    {
        id: imageDelegate
        CollageImageDelegate
        {
            id: collageImage
            borderImageSource: borderImage
            imageSource: imagePath
            number: index
            x: imageRect.x * renderer.width
            y: imageRect.y * renderer.height
            width: imageRect.width * renderer.width
            height: imageRect.height * renderer.height
        }
    }

    Repeater
    {
        anchors.fill: parent
        model: imageModel
        delegate: imageDelegate
    }

    onImageModelChanged:
    {
        console.log("model chnaged. Size: " + Number(imageModel.rowCount()).toString())
    }

}
