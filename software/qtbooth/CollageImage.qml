import QtQuick 2.0
import Montage 1.0

Image
{
    id: collage
    anchors.fill: parent

    sourceSize.width: 1024
    sourceSize.height: 1024

    property alias files: montage.filenames
    property string collageFileName: ""

    PhotoMontage
    {
        id: montage
        onMontageReady:
        {
            collage.source = filename
            collageFileName = filename
        }
    }

    function showMontage()
    {
        var filename = applicationSettings.foldername + "/collage/Collage_" + new Date().toLocaleString(locale, "dd.MM.yyyy_hh:mm:ss") + ".jpg"
        montage.generate(filename)
    }

    function addPhoto(fileName)
    {
        console.log("Added image " + fileName + " to montage" );
        montage.addFile(fileName)
    }

    function clearPhotos()
    {
        montage.clearFiles()
        collage.source = ""
        collageFileName = ""
    }
}
