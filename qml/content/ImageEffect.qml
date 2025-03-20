import QtQuick
import FileIO

ShaderEffect {
    property variant source
    property ListModel parameters: ListModel { }
    property string fragmentShaderFilename

    QtObject {
        id: d
        property string fragmentShaderCommon: "
            #ifdef GL_ES
                precision mediump float;
            #else
            #   define lowp
            #   define mediump
            #   define highp
            #endif // GL_ES
        "
    }

    // The following is a workaround for the fact that ShaderEffect
    // doesn't provide a way for shader programs to be read from a file,
    // rather than being inline in the QML file

    FileIO
    {
        id: file
    }

    onFragmentShaderFilenameChanged:
    {
        if(fragmentShaderFilename.length == 0)
        {
            fragmentShaderFilename = "qrc:/shaders/passthrough.qsb"
        }

        let filename = "qrc:/shaders/" + fragmentShaderFilename
        file.source = filename
        console.log("Shader file: " + filename)
        fragmentShader = d.fragmentShaderCommon + file.read()
    }
}
