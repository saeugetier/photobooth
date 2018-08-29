import QtQuick 2.5

PageGalleryForm {
    property alias foldermodel: galleryForm.foldermodel
    property alias galleryShuffleTimer: galleryShuffleTimer

    id: galleryForm

    Timer
    {
        id: galleryShuffleTimer
        interval: 1000 * 5
        repeat: true

        onTriggered:
        {
            var index = Math.floor(photosGrid.count*Math.random())
            console.log("random picture: " + index)
            photoView.positionViewAtIndex(index, ListView.Contain)
            viewState = "fullscreen"
        }
    }

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
