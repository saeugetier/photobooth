import QtQuick 2.4
import FileIO 1.0

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

    FileIO
    {
        id: fileio
    }

    deleteButton.onClicked:
    {
        // TODO delete file
        fileio.source = previewImage.source
        fileio.remove()
        abort()
    }

    effectButton.onClicked:
    {
        state = "effectSelection"
    }

    effectSelector.onEffectSelected:
    {
        shaderName = effect
        state = "idle"
    }
}
