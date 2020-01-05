import QtQuick 2.4

CollageSelectorForm {
    id: form
    property alias iconModel: form.iconModel
    property string iconName: ""
    signal selected

    upButton.onClicked:
    {
        scrollIdicator.increase()
    }

    downButton.onClicked:
    {
        scrollIdicator.decrease()
    }

    iconListView.delegate: iconDelegate

    Component
    {
        id: iconDelegate
        CollageIconButton {
            onClicked:
            {
                iconListView.currentIndex = index
                console.log("Selected index: " + Number(index).toString())
                iconName = name
                selected()
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
