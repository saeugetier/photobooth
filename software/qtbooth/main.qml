import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import QtMultimedia 5.5
import QtQuick.Controls.Material 2.2
import Qt.labs.folderlistmodel 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0

ApplicationWindow {
    id: mainWindow
    visible: true
    //visibility: "FullScreen"
    width: 640
    height: 480
    title: qsTr("QML Photo Booth")

    //Material.theme: Material.Dark
    //Material.accent: Material.Blue

    Shortcut {
        sequence: StandardKey.Quit
        context: Qt.ApplicationShortcut
        onActivated:
        {
            console.log("Catched application quit...")
            Qt.quit()
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex
        interactive: false

        PageCamera {
            id: camera
        }


        PageGallery
        {
            id: gallery
        }

        onCurrentIndexChanged:
        {
            if(swipeView.currentItem == camera)
            {
                gallery.resetGallery()
            }
            else
            {
                cameraTimeoutTimer.stop()
            }
        }
    }

    Timer
    {
        id: cameraTimeoutTimer
        interval: 1000 * 60 * 5  //going inactive after 5 minutes

        Component.onCompleted:
        {
            cameraTimeoutTimer.start()
        }

        onTriggered:
        {
            console.log("Inactive changing to gallery tab...")
            swipeView.setCurrentIndex(swipeView.count - 1)
            gallery.galleryShuffleTimer.start()
        }
    }

    header: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex

        TabButton {
            text: qsTr("Camera")
        }
        TabButton {
            text: qsTr("Gallery")
        }
    }

    Settings
    {
        id: applicationSettings
        category: "Application"
        property string foldername: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
    }
}
