import QtQuick 2.0

Canvas
{
    property alias photoCount: images.count
    ListModel
    {
        id: images
    }

    id: collage
    anchors.fill: parent
    onPaint: {
            console.log("Canvas paint started")
            var ctx = getContext("2d");
            ctx.fillStyle = Qt.rgba(1, 0, 0, 1);
            ctx.fillRect(0, 0, width, height);

            for(var i=0; i < images.count; i++)
            {
                ctx.drawImage(images.get(i).fileName, 50*i + 50, 50*i + 50, 100, 100)
                console.log("drawed images: " + images.get(i).fileName)
            }
        }
    onImageLoaded:
        collage.requestPaint()

    function addPhoto(fileName)
    {
        console.log("Canvas load image " + fileName );
        collage.loadImage(fileName)
        images.append({"fileName": fileName})
    }

    function clearPhotos()
    {
        for(var i = 0; i < images.count; i++)
        {
            console.log("Canvas unload image" + images.get(i).fileName )
            collage.unloadImage(images.get(i).fileName)
        }
        images.clear()
    }
}
