import QtQuick
import FileIO
import "content"

ImagePreviewForm {
    id: form
    signal abort
    signal accept(string filename, string effect)

    property string shaderName: ""
    property string effectPreset: ""

    function setPreviewImage(filename)
    {
        previewImage.imageSize = filesystem.getImageSize(filename)
        previewImage.source = filename
    }

    Component
    {
        id: effectComponent
        ShaderEffect {
                   id: effectPreview
                   property variant source: ShaderEffectSource {
                       sourceItem: previewImage
                       hideSource: true
                   }
                   fragmentShader: shaderName

                   Connections
                   {
                       target: form
                       function onShaderNameChanged(name) { fragmentShader = form.shaderName }
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
        popup.open()
    }

    EffectSelectionPopup
    {
        id: popup
        x: 50
        y: 50
        height: parent.height - 100
        width: parent.width - 100
        previewImage: form.previewImage.source

        onEffectSelected: effect =>
        {
            console.log("Current effect: " + effect)
            shaderName = effect
            state = "idle"
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
