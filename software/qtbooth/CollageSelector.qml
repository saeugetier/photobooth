import QtQuick 2.4

CollageSelectorForm {
    id: form
    property alias iconModel: form.iconModel

    upButton.onClicked:
    {
        scrollIdicator.increase()
    }

    downButton.onClicked:
    {
        scrollIdicator.decrease()
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
