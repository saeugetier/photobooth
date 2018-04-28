import QtQuick 2.0
import Montage 1.0

Image
{
    property alias files: montage.filenames

    PhotoMontage
    {
        id: montage
        onMontageReady:
        {
            collage.source = filename
        }
    }

    id: collage
    anchors.fill: parent

    function showMontage()
    {
        var filename = applicationSettings.foldername + "/collage/Collage_" + new Date().toLocaleString(locale, "dd.MM.yyyy_hh:mm:ss") + ".jpg"
        montage.generate(filename)
    }



    function addPhoto(fileName)
    {
        console.log("Canvas load image " + fileName );
        montage.filenames.append(fileName)
    }

    function clearPhotos()
    {
        montage.filenames.clear()
    }
}
