import QtQuick 2.4

PopupPreviewImageForm {
    id: imageForm
    property string currentFileName: ""
    signal imageAccepted(string filename)
    signal imageRejected(string filename)

    imageDeleteButton.onClicked: {
        imageForm.close()
        imageForm.imageRejected(currentFileName)
    }

    imageSaveButton.onClicked: {
        imageForm.close()
        imageForm.imageAccepted(currentFileName)
    }

    function showImage(source)
    {
        previewImage.source = source
        imageFileSaveIndicator.running = true
        open()
    }

    function setFileName(filename)
    {
        currentFileName = filename
        imageFileSaveIndicator.running = false
    }
}
