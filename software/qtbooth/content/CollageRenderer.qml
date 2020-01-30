import QtQuick 2.0
import CollageModel 1.0

Item {
    id: renderer
    property CollageImageModel imageModel
    property rect backgroundRect : Qt.rect(0, 0, renderer.width, renderer.height) // initial value
    property bool saving: false

    signal collageFull(bool full)

    Rectangle
    {
        color: "white"
        anchors.fill: parent
    }

    Image
    {
        id: background
        anchors.fill: renderer
        //source: imageModel.backgroundImage //set onImageModeChanged
        fillMode: Image.PreserveAspectFit

        sourceSize.height: 2048
        sourceSize.width: 2048

        onStatusChanged:
        {
            if(background.paintedHeight != 0 && background.paintedWidth != 0)
            {
                backgroundRect.x = background.x
                backgroundRect.y = background.y
                backgroundRect.width = background.paintedWidth
                backgroundRect.height = background.paintedHeight
            }
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
                x: imageRect.x * backgroundRect.width + (renderer.width - backgroundRect.width) / 2
                y: imageRect.y * backgroundRect.height + (renderer.height - backgroundRect.height) / 2
                width: imageRect.width * backgroundRect.width
                height: imageRect.height * backgroundRect.height

                Component.onCompleted:
                {
                    console.log("Picture placed at: " + Number(x).toString() + " " + Number(y).toString())
                }

                onImageSourceChanged:
                {
                    console.log(imageSource)
                }

                onDeletePhoto:
                {
                    console.log("Delete photo: " + Number(number).toString())
                    imageModel.clearImagePath(number)
                }
            }
        }

        Repeater
        {
            anchors.fill: background
            model: imageModel
            delegate: imageDelegate
        }
    }

    onImageModelChanged:
    {
        background.source = imageModel.backgroundImage
        console.log("model chnaged. Size: " + Number(imageModel.rowCount()).toString())
    }

    onBackgroundRectChanged:
    {
        console.log("background rect: " + Number(backgroundRect.width).toString()
                    + " " + Number(backgroundRect.height))
    }

    Connections
    {
        target: imageModel
        onCollageFullChanged: {
            collageFull(full)
            console.log("Collage Full Changed to " + Boolean(full).toString());
        }
    }

    function saveImage(filename, size)
    {
        saving = true;
        // TODO clip the image and bringing everything into right format...
        renderer.grabToImage(function(result) {
            result.saveToFile(filename, Qt.size(backgroundRect.width, backgroundRect.height));
            saving = false;
        }, size);
    }
}
