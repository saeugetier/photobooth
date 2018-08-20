import QtQuick 2.5

PageGalleryForm {
    property alias foldermodel: galleryForm.foldermodel
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
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
