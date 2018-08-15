import QtQuick 2.4
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtQuick.VirtualKeyboard 2.1
import QtQuick.Layouts 1.0

Popup {
    id: passwordPopup
    modal: true
    property alias okButton: okButton
    property alias passwordInput: passwordInput
    width: 800
    height: 350

    RowLayout {
        anchors.horizontalCenter: parent.horizontalCenter

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
            inputMethodHints: Qt.ImhDialableCharactersOnly
            echoMode: TextInput.Password
            font.pixelSize: 32
            focus: true
        }

        Button {
            id: okButton
            text: qsTr("Ok")
            highlighted: true
        }

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle {
                anchors.fill: parent
            } // to visualize the spacer
        }
    }

    InputPanel {
        id: inputPanel
        y: parent.height // position the top of the keyboard to the bottom of the screen/display

        anchors.left: parent.left
        anchors.right: parent.right
        Component.onCompleted: {
            inputPanel.keyboard.style.languagePopupListEnabled = false
        }
        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                // position the top of the keyboard to the bottom of the text input field
                y: parent.height - inputPanel.height
            }
        }
    }
}
