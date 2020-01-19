import QtQuick 2.4
import QtQuick.Controls 2.0
import Qt.labs.platform 1.0
import QtQuick.Dialogs 1.2

FileCopyProgressForm {
    standardButtons: Dialog.Cancel
    id: copyFileDialog
    closePolicy: Popup.CloseOnEscape

    Connections
    {
        target: helper
        onCopyProgress:  {
            updateProgress(progress)
        }
        onCopyFinished:
        {
            copyFileDialog.close()
        }
    }

    onOpened:
    {
        helper.startCopyFilesToRemovableDrive()
    }

    function updateProgress(progress)
    {
        copyProgressBar.value = progress / 100
    }

    onRejected:
    {
        console.log("Cancel copy")
        helper.abortCopy()
    }

    onClosed:
    {
        helper.unmountRemoveableDrive()
    }
}
