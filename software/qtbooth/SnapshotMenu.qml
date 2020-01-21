import QtQuick 2.4

SnapshotMenuForm {
    id: form

    signal captured(string filename)

    shutterButton.onTriggerSnapshot:
    {
        cameraRenderer.takePhoto()
    }

    cameraRenderer.onSavedPhoto:
    {
        captured(filename)
        shutterButton.state = "idle"
    }

}
