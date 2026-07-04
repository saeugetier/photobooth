import QtQuick
import QtQuick.Controls

FileCopyProgressForm {
    standardButtons: Dialog.Cancel
    id: copyFileDialog
    closePolicy: Popup.CloseOnEscape
    property string targetPath : ""

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
        filesystem.startCopyFilesToPath(targetPath)
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
}
