import QtQuick 2.4
import FileIO 1.0
import "content"

ImagePreviewForm {
    id: form
    signal abort
    signal accept(string filename, string effect)

    property var shaderName: ""
    property string effectPreset: ""

    function setPreviewImage(filename)
    {
        previewImage.source = filename
    }

    Component
    {
        id: effectComponent
        ImageEffect {
                   id: effectPreview
                   source: previewImage
                   fragmentShaderFilename: shaderName

                   Connections
                   {
                       target: form
                       onShaderNameChanged: fragmentShaderFilename = form.shaderName
                   }
                }


    }

    previewImage.layer.effect: effectComponent

    previewImage.onStatusChanged:
    {
        if(previewImage.status == Image.Loading)
        {
            fileLoadIndicator.running = true;
        }
        else
        {
            fileLoadIndicator.running = false;
            shaderName = effectPreset
        }
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
