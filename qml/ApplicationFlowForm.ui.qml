import QtQuick
import CollageModel
import "content"

Item {
    id: root
    property alias mainMenuModel: mainMenu.iconModel
    property alias mainMenu: mainMenu
    property alias snapshotMenu: snapshotMenu
    property alias imagePreview: imagePreview
    property alias collageMenu: collageMenu
    property alias galleryMenu: galleryMenu
    property alias settingsMenu: settingsMenu

    property real slideValueCollageSelection: 0.0
    property real slideValueSnapshotMenu: 1.0
    property real slideValuePreviewMenu: 1.0
    property real slideValueCollageMenu: 1.0
    property real slideValueGalleryMenu: 1.0
    property real slideValueSettingsMenu: 1.0

    MainMenu {
        id: mainMenu
        x: root.width * slideValueCollageSelection
        y: 0
        z: 5
        width: root.width
        height: root.height
    }

    SnapshotMenu {
        id: snapshotMenu
        x: root.width * slideValueSnapshotMenu
        y: 0
        z: 4
        width: root.width
        height: root.height
        state: "deactivated"
    }

    ImagePreview {
        id: imagePreview
        x: root.width * slideValuePreviewMenu
        y: 0
        z: 3
        width: root.width
        height: root.height
    }

    CollageMenu {
        id: collageMenu
        x: root.width * slideValueCollageMenu
        y: 0
        z: 2
        width: root.width
        height: root.height
    }

    GalleryMenu
    {
        id: galleryMenu
        x: root.width * slideValueGalleryMenu
        y: 0
        z: 1
        width: root.width
        height: root.height
    }

    SettingsMenu {
        id: settingsMenu
        x: root.width * slideValueSettingsMenu
        y: 0
        z: 0
        width: root.width
        height: root.height
    }

    states: [
        State {
            name: "collageSelection"
            PropertyChanges {
                target: mainMenu
                state: "NoIconSelected"
            }
        },
        State {
            name: "snapshot"

            PropertyChanges {
                target: root
                slideValueCollageSelection: -1.0
                slideValueSnapshotMenu: 0.0
            }

            PropertyChanges {
                target: snapshotMenu
                state: "activated"
            }
            PropertyChanges {
                target: imagePreview
                visible: false
            }
        },
        State {
            name: "imagePreview"

            PropertyChanges {
                target: root
                slideValueCollageSelection: -1.0
                slideValueSnapshotMenu: -1.0
                slideValuePreviewMenu: 0.0
            }

            PropertyChanges {
                target: mainMenu
                visible: false
            }
        },
        State {
            name: "collageMenu"

            PropertyChanges {
                target: root
                slideValueCollageSelection: -1.0
                slideValueSnapshotMenu: -1.0
                slideValuePreviewMenu: -1.0
                slideValueCollageMenu: 0.0
            }

            PropertyChanges {
                target: mainMenu
                visible: false
            }

            PropertyChanges {
                target: snapshotMenu
                visible: false
            }
        },
        State {
            name: "gallery"

            PropertyChanges {
                target: root
                slideValueCollageSelection: -1.0
                slideValueSnapshotMenu: -1.0
                slideValuePreviewMenu: -1.0
                slideValueCollageMenu: -1.0
                slideValueGalleryMenu: 0.0
            }

            PropertyChanges {
                target: snapshotMenu
                visible: false
            }

            PropertyChanges {
                target: imagePreview
                visible: false
            }

            PropertyChanges {
                target: collageMenu
                visible: false
            }
        },
        State {
            name: "settings"

            PropertyChanges {
                target: root
                slideValueCollageSelection: -1.0
                slideValueSnapshotMenu: -1.0
                slideValuePreviewMenu: -1.0
                slideValueCollageMenu: -1.0
                slideValueGalleryMenu: -1.0
                slideValueSettingsMenu: 0.0
            }

            PropertyChanges {
                target: snapshotMenu
                visible: false
            }

            PropertyChanges {
                target: imagePreview
                visible: false
            }

            PropertyChanges {
                target: collageMenu
                visible: false
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

