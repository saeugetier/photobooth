import QtQuick 2.5
import QtQuick.Controls 2.1

PopupPrintForm {
    id: printForm
    property string fileNameFullScale : ""
    property bool printerBusy: false
    property bool fileReady: false

    onPrinterBusyChanged:
    {
        if(printerBusy)
        {
            printerBusyState = "busy"
            printButton.enabled = false
        }
        else
        {
            printerBusyState = "idle"
            if(fileReady)
            {
                printButton.enabled = true
            }
        }
    }

    onFileReadyChanged:
    {
        if(fileReady) {
            busyIndicator.visible = false
            if(!printerBusy)
            {
                printButton.enabled = true
            }
        }
        else {
            busyIndicator.visible = true
            printButton.enabled = false
        }
    }

    function setCollageType(type)
    {
        console.log(type)
        printForm.state = type
        collageImage.clearPhotos()
    }

    fullScaleImage.onSourceChanged: {
        fileReady = true
    }

    collageImage.onSourceChanged: {
        fileReady = true
    }

    states: [
        State
        {
            name: "fullscale"
            PropertyChanges { target: collageImage; visible: false }
            PropertyChanges { target: fullScaleImage; visible: true }
        },
        State
        {
            name: "collage"
            PropertyChanges { target: collageImage; visible: true; }
            PropertyChanges { target: fullScaleImage; visible: false }
        }
    ]

    onStateChanged:
    {
        if(state == "collage")
        {
            collageImage.clearPhotos()
        }
    }

    printerBusyStates:
    [
        State
        {
            name: "busy"
            PropertyChanges {
                target: printerBusyIndicator
                visible: true
            }
        },
        State
        {
            name: "idle"
            PropertyChanges {
                target: printerBusyIndicator
                visible: false
            }
        }

    ]

    function newPhoto(filename)
    {
        fileReady = false
        if(state == "collage")
        {
            collageImage.addPhoto(filename)

            var count = collageImage.files.length

            if(count >= 4)
            {
                printForm.open()
                collageImage.showMontage()
            }
        }
        else
        {
            printForm.open()
            fileNameFullScale = filename
        }
    }

    printButton.onClicked:
    {
        if(state == "collage")
        {
            if(printer.printImage(collageImage.collageFileName) < 0)
            {
                printer_error.visible = true;
            }
            else
            {
                printerBusy = true
            }

            printForm.close()
        }
        else
        {
            if(printer.printImage(fileNameFullScale) < 0)
            {
                printer_error.visible = true;
            }
            else
            {
                printerBusy = true
            }

            printForm.close()
        }
    }

    Component.onCompleted: {
        printer.failed.connect(showPrinterError)
        printer.success.connect(printerSuccess)
    }

    function showPrinterError()
    {
        printer_error.visible = true
    }

    function printerSuccess()
    {
        printerBusy = false
        printer_error.visible = false
    }

    Dialog {
        id: printer_error
        title: "Printer"
        Label {
                text: "Printer not connected"
            }
        standardButtons: Dialog.Ok
        modal: true
        onAccepted: console.log("Ok clicked")
    }

    cancelButton.onClicked:
    {
        printForm.close()
    }

    onClosed:
    {
        collageImage.clearPhotos()
        fileNameFullScale = ""
    }

    onOpened:
    {
        /*if(state == "collage")
        {
            console.log("Canvas requested repaint")
            printButton.enabled = false
        }*/
    }
}
