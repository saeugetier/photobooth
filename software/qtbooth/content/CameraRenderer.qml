import QtQuick 2.5
import QtMultimedia 5.5
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.2

Item {
    Camera {
        id: camera

        position: Camera.FrontFace

        //imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
        }
    }

    VideoOutput {
        id: output
        source: camera
        anchors.fill: parent
        //focus: visible // to receive focus and capture key events when visible
    }

    WhiteOverlay
    {
        id: whiteOverlay
        x: output.x
        y: output.y
        width: output.width
        height: output.height
    }

    Timer
    {
        id: cameraDiscoveryTimer

        interval: 1000
        repeat: true

        onTriggered:
        {
            //camera discovery is delayed
            var availableCameras = QtMultimedia.availableCameras
            printDevicesToConsole(availableCameras)

            if(availableCameras.length > 0)
            {
                camera.deviceId = availableCameras[0].deviceId
                camera.start()
            }

        }
    }

    function takePhoto()
    {
        state  = "snapshot"
    }

    states: [
        State {
            name: "preview"
            PropertyChanges {
                target: whiteOverlay
                state: "released"
            }
        },
        State {
            name: "snapshot"
            PropertyChanges {
                target: whiteOverlay
                state: "triggered"
            }
        },
        State {
            name: "store"
        }
    ]
}
