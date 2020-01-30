import QtQuick 2.4
import Selphy 1.0

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer
    property Printer printer

    signal next
    signal exit

    printButton.enabled: !printer.busy
    property real printerHeight : printer.getPrintSize().height
    property real printerWidth : printer.getPrintSize().width
    printerRatio:  printerHeight / printerWidth

    nextButton.onClicked:
    {
        next()
    }

    printButton.onClicked:
    {
        collageRenderer.saveImage("Test.jpg", printer.getPrintSize())
        printer.printImage("Test")
        exit()
    }

    exitButton.onClicked:
    {
        exit()
    }

    onStateChanged:
    {
        if(state == "CollageFull")
        {
            if(printer.busy)
            {
                printerBusyPopup.modal = 1
                printerBusyPopup.open()
            }
        }
    }

    Connections
    {
        target: collageImage
        onCollageFull: {
            if(full) form.state = "CollageFull";
            else form.state = "CollageNotFull";
        }
    }

    Connections
    {
        target: printer
        onBusyChanged:
        {
            if(!printer.busy && printerBusyPopup.opened)
            {
                printerBusyPopup.close()
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
