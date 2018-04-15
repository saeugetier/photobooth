import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtMultimedia 5.0
import QtQuick.Controls.Material 2.0
import Qt.labs.folderlistmodel 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("QML Photo Booth")

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        PageCamera {
            id: camera
        }


        PageGallery
        {
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
        property string foldername
    }
}
