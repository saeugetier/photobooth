import QtQuick 2.4
import FileIO 1.0

ImagePreviewForm {
    signal abort
    signal accept(string filename, string effect)

    function setPreviewImage(filename)
    {
        previewImage.source = filename
        shaderName = ""
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
        accept(previewImage.source, shaderName)
    }

    FileIO
    {
        id: fileio
    }

    deleteButton.onClicked:
    {
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
        console.log("Current effect: " + effect)
        shaderName = effect
        state = "idle"
    }
}
