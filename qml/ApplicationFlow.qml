import QtQuick
import CollageModel
import QtQuick.Controls

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
        collageMenu.collageIsPrintable = mainMenu.collageIsPrintable
        mainMenu.selectedCollageName = ""
    }

    mainMenu.onGalleryEnter:
    {
        state = "gallery"
    }

    snapshotMenu.onCaptured: filename =>
    {
        state = "imagePreview"
        imagePreview.effectSelectable = applicationSettings.printEnable && collageMenu.collageImage.imageModel.nextImageIsEffectSelectable()

        if(applicationSettings.printEnable)
        {
            imagePreview.effectPreset = collageMenu.collageImage.imageModel.nextImageEffectPreset()
        }
        else
        {
            imagePreview.effectPreset = ""
        }

        imagePreview.setPreviewImage(filename)
    }

    snapshotMenu.onAbort:
    {
        state = "collageSelection"
    }

    imagePreview.onAccept: (filename, effect) =>
    {
        if(applicationSettings.printEnable)
        {
            state = "collageMenu"
            collageMenu.collageImage.imageModel.addImagePath(filename, effect)
        }
        else
        {
            state = "collageSelection"
        }
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

    collageMenu.collageImage.onCollageImagesChanged: count =>
    {
        snapshotMenu.snapshotTimeoutEnable = (count === 0)
    }

    galleryMenu.onExitGallery:
    {
        state = "collageSelection"
    }

    Behavior on slideValueCollageSelection {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on slideValueSnapshotMenu {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on slideValuePreviewMenu {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on slideValueCollageMenu {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on slideValueGalleryMenu {
        PropertyAnimation {
            duration: flow.animationDuration
            easing.type: Easing.InOutQuad
        }
    }
}


