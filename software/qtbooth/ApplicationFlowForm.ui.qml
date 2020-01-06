import QtQuick 2.4
import CollageModel 1.0

Item {
    id: root
    property alias mainMenuModel: mainMenu.iconModel
    property alias collageModel: collageMenu.model
    property alias mainMenu: mainMenu
    property alias snapshotMenu: snapshotMenu
    property alias imagePreview: imagePreview
    property alias collageMenu: collageMenu

    MainMenu {
        id: mainMenu
        x: 0
        y: 0
        width: root.width
        height: root.height
    }

    SnapshotMenu {
        id: snapshotMenu
        x: root.width
        y: 0
        width: root.width
        height: root.height
    }

    ImagePreview {
        id: imagePreview
        x: root.width * 2
        y: 0
        width: root.width
        height: root.height
    }

    CollageMenu {
        id: collageMenu
        x: root.width * 3
        y: 0
        width: root.width
        height: root.height
    }
    states: [
        State {
            name: "collageSelection"
        },
        State {
            name: "snapshot"

            PropertyChanges {
                target: mainMenu
                x: -root.width
            }

            PropertyChanges {
                target: snapshotMenu
                x: 0
            }
        },
        State {
            name: "imagePreview"
        },
        State {
            name: "collageMenu"
        }
    ]
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

