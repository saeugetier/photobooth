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
        console.log("Print button pressed")
        var path = applicationSettings.foldername.toString()
        path = path.replace(/^(file:\/{2})/,"");
        var cleanPath = decodeURIComponent(path);
        console.log(cleanPath)
        var filename = cleanPath + "/collage/Coll_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg"
        collageRenderer.saveImage(filename, printer.getPrintSize())
        console.log("Collage rendered")
        printer.printImage(filename)
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
