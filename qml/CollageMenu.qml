import QtQuick 2.4
import Printer 1.0
import "content"

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer
    property Printer printer
    property bool multiplePrints: false
    property alias collageIsPrintable: form.showPrintButton

    signal next
    signal exit

    printButton.enabled: printer ? !printer.busy : false
    property real printerHeight : form.collageImage.imageModel ? form.collageImage.imageModel.collagePixelSize.height : 0
    property real printerWidth : form.collageImage.imageModel ? form.collageImage.imageModel.collagePixelSize.width : 0
    printerRatio:  printerHeight / printerWidth

    nextButton.onClicked:
    {
        next()
    }

    printButton.onClicked:
    {
        printButton.enabled = false
        printerPopup.isPrinting = collageIsPrintable
        printerPopup.visible = true
        console.log("Print button pressed")
        var path = applicationSettings.foldername.toString()
        path = path.replace(/^(file:\/{2})/,"");
        var cleanPath = decodeURIComponent(path);
        var filename = cleanPath + "/collage/Coll_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".png"
        console.log("Save files to: " + filename)
        collageRenderer.saveImage(filename, form.collageImage.imageModel.collagePixelSize)
        console.log("Collage rendered width: " + Number(form.collageImage.imageModel.collagePixelSize.width).toString()
                    + " height: " + form.collageImage.imageModel.collagePixelSize.height)
    }

    collageRenderer.onSavingChanged:
    {
        if(collageRenderer.saving === false)
        {
            printerPopup.visible = false
            if(collageRenderer.savedFilename.length > 0)
            {
                if(collageIsPrintable)
                {
                    if(!multiplePrints)
                    {
                        printer.printImage(collageRenderer.savedFilename, 1)
                    }
                    else
                    {
                        printer.printImage(collageRenderer.savedFilename, printCountTumbler.currentIndex + 1)
                        printCountTumbler.currentIndex = 0
                    }
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
            printButton.enabled = !printer.busy || !collageIsPrintable
        }
    }

    exitButton.onClicked:
    {
        exit()
    }

    // selector for multiple prints should only show if multiple prints are enabled in settings menu and collage is finished
    plusButton.visible: multiplePrints && (form.state == "CollageFull")
    minusButton.visible: multiplePrints && (form.state == "CollageFull")
    printCountTumbler.visible: multiplePrints && collageIsPrintable && (form.state == "CollageFull")

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
        if(state === "CollageFull")
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
        function onCollageFull(full) {
            if(full) form.state = "CollageFull";
            else form.state = "CollageNotFull";
        }
    }

    Connections
    {
        target: printer
        function onBusyChanged()
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
