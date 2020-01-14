import QtQuick 2.0
import CollageModel 1.0

Item {
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
            borderImageSource: borderImage
            imageSource: imagePath
            number: index
        }
    }

    ListView
    {
        model: imageModel
        delegate: imageDelegate
    }

    onImageModelChanged:
    {
        console.log("model chnaged. Size: " + Number(imageModel.rowCount()).toString())
    }

}
