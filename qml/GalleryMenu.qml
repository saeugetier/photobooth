import QtQuick
import Printer

GalleryMenuForm {
    property alias foldermodel: galleryForm.foldermodel
    signal exitGallery
    property alias printer: galleryForm.printer

    id: galleryForm


    scrollDownButton.onPressed:
    {
        scrollBar.increase()
    }

    scrollUpButton.onPressed:
    {
        scrollBar.decrease()
    }

    photosGrid.onCurrentIndexChanged: {
        photoView.positionViewAtIndex(photosGrid.currentIndex, ListView.Contain)
    }

    photoView.onCurrentIndexChanged: {
        photosGrid.positionViewAtIndex(photoView.currentIndex, GridView.Contain)
    }

    function resetGallery()
    {
        viewState = "inGrid"
    }

    exitButton.onClicked:
    {
        exitGallery()
    }

    photoButton.onClicked:
    {
        foldermodel.folder = applicationSettings.foldername
    }

    collageButton.onClicked:
    {
        foldermodel.folder = applicationSettings.foldername + "/collage"
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
