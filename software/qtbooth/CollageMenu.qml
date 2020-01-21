import QtQuick 2.4

CollageMenuForm {
    id: form
    property alias collageImage: form.collageRenderer

    signal ok

    okButton.onClicked:
    {
        ok()
    }
}
