import QtQuick 2.4
import Qt.labs.platform 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2
import QtQuick.Controls 2.3

FileCopyProgressForm {
    id: copyFileDialog
    closePolicy: Popup.CloseOnEscape

    Connections
    {
        target: filesystem
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
        filesystem.startCopyFilesToRemovableDrive()
    }

    function updateProgress(progress)
    {
        copyProgressBar.value = progress / 100
    }

    onRejected:
    {
        console.log("Cancel copy")
        filesystem.abortCopy()
    }

    onClosed:
    {
        filesystem.unmountRemoveableDrive()
    }
}
