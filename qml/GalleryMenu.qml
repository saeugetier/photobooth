import QtQuick 2.5

GalleryMenuForm {
    property alias foldermodel: galleryForm.foldermodel
    signal exitGallery

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
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
