import QtQuick 2.4
import CollageModel 1.0
import QtQuick.Controls 2.5

ApplicationFlowForm {
    id: flow
    width: parent.width
    height: parent.height

    property int animationDuration : 500

    mainMenuModel: modelFactory.getCollageIconModel()

    mainMenu.onSelectedCollageNameChanged:
    {
        if(mainMenu.selectedCollageName != "")
        {
            mainMenu.collageRenderer.imageModel = modelFactory.getCollageImageModel(mainMenu.selectedCollageName)
            mainMenu.collageRenderer.imageModel.clearImagePathes()
        }
    }

    mainMenu.onCollageSelected:
    {
        state = "snapshot"
        collageMenu.collageImage.imageModel = modelFactory.getCollageImageModel(mainMenu.selectedCollageName)
        collageMenu.collageImage.imageModel.clearImagePathes()
        mainMenu.selectedCollageName = ""
    }

    snapshotMenu.onCaptured:
    {
        state = "imagePreview"
        imagePreview.setPreviewImage(filename)
    }

    snapshotMenu.onAbort:
    {
        state = "collageSelection"
    }

    imagePreview.onAccept:
    {
        state = "collageMenu"
        collageMenu.collageImage.imageModel.addImagePath(filename)
    }

    imagePreview.onAbort:
    {
        state = "snapshot"
    }

    collageMenu.onNext:
    {
        state = "snapshot"
    }

    collageMenu.onExit:
    {
        state = "collageSelection"
    }

    Behavior on mainMenu.x {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on snapshotMenu.x {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on imagePreview.x {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on collageMenu.x {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }
}


