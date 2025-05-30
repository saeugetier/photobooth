import QtQuick
import CollageModel
import Qt.labs.platform

Item {
    id: renderer
    property CollageImageModel imageModel
    property rect backgroundRect : Qt.rect(0, 0, renderer.width, renderer.height) // initial value
    property bool saving: false
    property string savedFilename : ""
    property int imagesLoading : 0

    signal collageFull(bool full)
    signal collageImagesChanged(int count)

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
            if(background.paintedHeight !== 0 && background.paintedWidth !== 0)
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
                borderImageSource: filesystem.findFile(borderImage, StandardPaths.standardLocations(StandardPaths.AppDataLocation), true)
                imageSource: imagePath
                effectSource: effect
                borderSizeLeft: borderRect.left
                borderSizeTop: borderRect.top
                borderSizeBottom: borderRect.bottom
                borderSizeRight: borderRect.right
                number: index
                x: imageRect.x * backgroundRect.width + (renderer.width - backgroundRect.width) / 2
                y: imageRect.y * backgroundRect.height + (renderer.height - backgroundRect.height) / 2
                width: imageRect.width * backgroundRect.width
                height: imageRect.height * backgroundRect.height
                transform: Rotation { origin.x: width/2; origin.y: height/2; axis { x: 0; y: 0; z: 1 } angle: imageRotation }

                Component.onCompleted:
                {
                    console.log("Picture placed at: " + Number(x).toString() + " " + Number(y).toString() + " Rotation: " + Number(rotation).toString())
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

                proxy.onLoadingChanged:
                {
                    if(proxy.loading === true)
                    {
                        imagesLoading = imagesLoading + 1
                    }
                    else
                    {
                        imagesLoading = imagesLoading - 1

                        if(imagesLoading < 0)
                        {
                            imagesLoading = 0
                        }
                    }
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

    Image
    {
        id: foreground
        sourceSize.height: 2048
        sourceSize.width: 2048

        anchors.fill: background
        fillMode: Image.PreserveAspectFit
    }

    onImageModelChanged:
    {
        imagesLoading = 0
        background.source = filesystem.findFile(imageModel.backgroundImage, StandardPaths.standardLocations(StandardPaths.AppDataLocation), true)
        foreground.source = filesystem.findFile(imageModel.foregroundImage, StandardPaths.standardLocations(StandardPaths.AppDataLocation), true)
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
        function onCollageFullChanged(full) {
            collageFull(full)
            console.log("Collage Full Changed to " + Boolean(full).toString());
        }

        function onCountImagePathSetChanged(count)
        {
            collageImagesChanged(count)
            console.log("Images in model set: " + Number(count).toString());
        }
    }

    function saveImage(filename, size)
    {
        saving = true;

        var aspectRatio = size.height / size.width;
        if(size.height > 2048 || size.width > 2048)
        {
            if(size.width > size.height)
            {
                size.width = 2048
                size.height = 2048 * aspectRatio
            }
            else
            {
                size.width = 2048 / aspectRatio
                size.height = 2048
            }
        }

        // TODO clip the image and bringing everything into right format...
        renderer.grabToImage(function(result) {
            if (result) {
                console.log("Image grabbed with size " + result.image.width + "x" + result.image.height + " successfully");
                var success = result.saveToFile(filename);
                if (success) {
                    console.log("Image saved successfully to " + filename);
                    savedFilename = filename;
                } else {
                    console.log("Failed to save image to " + filename);
                }
            } else {
                console.log("Failed to grab image");
            }
            saving = false;
        }, size);
    }
}
