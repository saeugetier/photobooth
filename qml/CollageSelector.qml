import QtQuick 2.4
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0

CollageSelectorForm {
    id: form
    property alias iconModel: form.iconModel
    property string iconName: ""
    signal selected

    upButton.onClicked:
    {
        scrollIndicator.increase()
    }

    downButton.onClicked:
    {
        scrollIndicator.decrease()
    }

    iconListView.delegate: iconDelegate

    Component
    {
        id: iconDelegate
        CollageIconButton {
            id: iconButton
            source:
            {
                var filename = filesystem.findFile(icon, StandardPaths.standardLocations(StandardPaths.AppDataLocation), true)
                if(filename.toString().length === 0)
                {
                    filename = "../images/icon/Unknown.svg"
                }
                filename
            }
            onClicked:
            {
                iconListView.currentIndex = index
                console.log("Selected index: " + Number(index).toString())
                iconName = name
                selected()
            }
        }
    }


    NumberAnimation on scrollIndicator.position {
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation on iconListView.contentY {
        duration: 1000
        easing.type: Easing.InOutQuad
    }

   /* scrollIdicator.transitions: Transition {
        NumberAnimation {
            property: "position"
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }*/
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
