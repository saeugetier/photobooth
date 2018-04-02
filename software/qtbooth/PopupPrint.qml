import QtQuick 2.4

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

            if(collageImage.photoCount >= 4)
            {
                printForm.open()
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
            var filename = applicationSettings.foldername + "/collage/Collage_" + new Date().toLocaleString(locale, "dd.MM.yyyy_hh:mm:ss") + ".jpg"
            collageImage.save(filename)
            helper.printImage(filename)
            printForm.close()
        }
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
