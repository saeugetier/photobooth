import QtQuick 2.4

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer

    signal next

    nextButton.onClicked:
    {
        next()
    }

    Connections
    {
        target: collageImage
        onCollageFull: {
            if(full) form.state = "CollageFull";
            else form.state = "CollageNotFull";
        }
    }
}
