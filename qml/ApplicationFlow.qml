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
        collageMenu.collageIsPrintable = mainMenu.collageIsPrintable
        mainMenu.selectedCollageName = ""
    }

    snapshotMenu.onCaptured:
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

    imagePreview.onAccept:
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
}


