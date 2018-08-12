import QtQuick 2.4
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtQuick.VirtualKeyboard 2.1
import QtQuick.Layouts 1.0

Popup {
    id: passwordPopup
    dim: true
    modal: true
    height: 80
    property alias okButton: okButton
    property alias passwordInput: passwordInput
    width: 250

    RowLayout {
        x: 0
        y: 0

        Label {
            id: label
            text: qsTr("Password: ")
        }

        TextInput {
            id: passwordInput
            width: 80
            text: ""
            font.wordSpacing: 0
            cursorVisible: true
            inputMethodHints: Qt.ImhDigitsOnly
            echoMode: TextInput.Password
            font.pixelSize: 32
            focus: true
        }


        Button {
            id: okButton
            x: 85
            text: qsTr("Button")
            highlighted: true
        }

    }
}
