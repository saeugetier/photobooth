import QtQuick 2.4
import Printer 1.0
import "content"

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer
    property Printer printer
    property bool multiplePrints: false

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
        printButton.enabled = false
        printerPopup.visible = true
        console.log("Print button pressed")
        var path = applicationSettings.foldername.toString()
        path = path.replace(/^(file:\/{2})/,"");
        var cleanPath = decodeURIComponent(path);
        console.log(cleanPath)
        var filename = cleanPath + "/collage/Coll_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".png"
        collageRenderer.saveImage(filename, printer.getPrintSize())
        console.log("Collage rendered")
    }

    collageRenderer.onSavingChanged:
    {
        if(collageRenderer.saving === false)
        {
            printerPopup.visible = false
            if(collageRenderer.savedFilename.length > 0)
            {
                if(!multiplePrints)
                {
                    printer.printImage(collageRenderer.savedFilename, 1)
                }
                else
                {
                    printer.printImage(collageRenderer.savedFilename, printCountTumbler.currentIndex + 1)
                }
                exit()
            }
        }
    }

    collageRenderer.onImagesLoadingChanged:
    {
        console.log("loading " + Number(collageRenderer.imagesLoading).toString() + " images in Collage Renderer")
        if(collageRenderer.imagesLoading > 0)
        {
            printButton.enabled = false
        }
        else
        {
            printButton.enabled = !printer.busy
        }
    }

    exitButton.onClicked:
    {
        exit()
    }

    plusButton.visible: multiplePrints
    minusButton.visible: multiplePrints
    printCountTumbler.visible: multiplePrints

    minusButton.onClicked:
    {
        if(printCountTumbler.currentIndex > 0)
        {
            printCountTumbler.currentIndex = printCountTumbler.currentIndex - 1
        }
    }

    plusButton.onClicked:
    {
        if(printCountTumbler.currentIndex < (printCountTumbler.count - 1))
        {
            printCountTumbler.currentIndex = printCountTumbler.currentIndex + 1
        }
    }

    onStateChanged:
    {
        if(state == "CollageFull")
        {
            if(printer.busy)
            {
                printerPopup.visible = true
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
            if(!printer.busy)
            {
                printerPopup.visible = false
                if(collageRenderer.imagesLoading == 0)
                {
                    printButton.enabled = true
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
