import QtQuick 2.5
import QtQuick.Controls 2.1

PopupPrintForm {
    id: printForm
    property var fileNameFullScale : ""


    function setCollageType(type)
    {
        console.log(type)
        printForm.state = type
        collageImage.clearPhotos()
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

    function newPhoto(filename)
    {

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
            fileNameFullScale = filename
            printForm.open()
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

            printForm.close()
        }
        else
        {
            if(printer.printImage(fileNameFullScale) < 0)
            {
                printer_error.visible = true;
            }

            printForm.close()
        }
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
        if(state == "collage")
        {
            busyIndicator.running = true;
            console.log("Canvas requested repaint")
        }
    }
}
