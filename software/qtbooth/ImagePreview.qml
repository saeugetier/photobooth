import QtQuick 2.4

ImagePreviewForm {
    signal abort
    signal accept(string filename)

    function setPreviewImage(filename)
    {
        previewImage.source = filename
    }

    previewImage.onStatusChanged:
    {
        if(previewImage.status == Image.Loading)
            fileLoadIndicator.running = true;
        else
            fileLoadIndicator.running = false;
    }

    saveButton.onClicked:
    {
        accept(previewImage.source)
    }

    deleteButton.onClicked:
    {
        // TODO delete file
        abort()
    }
}
