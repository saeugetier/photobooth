import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import QtQuick.VirtualKeyboard

SettingsPasswordForm {
    id: passwordForm

    property bool unlocked: false
    property string password: "1234"
    signal passwordAccepted()

    onOpened:
    {
        passwordInput.text = ""
        console.log("Password prompt opened")
    }

    passwordInput.onVisibleChanged:
    {
        if(passwordInput.visible)
        {
            console.log("Focus prompt");
            passwordInput.focus = true;
            passwordInput.forceActiveFocus();
        }
    }

    okButton.onClicked:
    {
        passwordForm.checkPassword()
        passwordForm.close()
    }

    passwordInput.onAccepted:
    {
        passwordForm.checkPassword()
        passwordForm.close()
    }

    function checkPassword()
    {
        if(passwordInput.text == password)
        {
            unlocked = true
            console.log("Password accepted: " + passwordInput.text)
            passwordAccepted()
        }
        else
        {
            console.log("Password wrong: " + passwordInput.text)
            unlocked = false
        }

    }

}
