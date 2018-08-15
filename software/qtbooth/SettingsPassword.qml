import QtQuick 2.4
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtQuick.VirtualKeyboard 2.1

SettingsPasswordForm {
    id: passwordForm

    property bool unlocked: false
    property string password: "1234"

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
        }
        else
        {
            unlocked = false
        }
        console.log("Password accepted: " + unlocked)
    }

}
