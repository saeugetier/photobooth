import QtQuick 2.0
import QtQuick.Controls 2.2

Dialog {
    standardButtons: Dialog.Yes | Dialog.No

    Label
    {
        text: qsTr("Delete all images?")
    }

    onAccepted:
    {
        helper.deleteAllImages()
    }
}
