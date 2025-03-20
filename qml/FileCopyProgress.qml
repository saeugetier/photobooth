import QtQuick
import QtQuick.Controls

FileCopyProgressForm {
    standardButtons: Dialog.Cancel
    id: copyFileDialog
    closePolicy: Popup.CloseOnEscape

    Connections
    {
        target: filesystem
        function onCopyProgress(progress)  {
            updateProgress(progress)
        }
        function onCopyFinished()
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
