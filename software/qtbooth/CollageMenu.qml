import QtQuick 2.4

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer

    signal next
    signal exit

    nextButton.onClicked:
    {
        next()
    }

    printButton.onClicked:
    {
        exit()
    }

    exitButton.onClicked:
    {
        exit()
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
