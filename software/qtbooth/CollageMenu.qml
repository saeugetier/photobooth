import QtQuick 2.4
import Selphy 1.0

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer
    property Printer printer

    signal next
    signal exit

    nextButton.onClicked:
    {
        next()
    }

    printButton.onClicked:
    {
        printer.printImage("Test")
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
