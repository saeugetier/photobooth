import QtQuick 2.5
import QtMultimedia 5.5
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.2

Item {
    id: renderer
    signal savedPhoto(string filename)
    signal failed

    property bool photoProcessing: (state == "snapshot")

    function printDevicesToConsole(devices)
    {
        console.log("Found " + devices.length + " camera devices!")
        for(var i = 0; i < devices.length; i++)
        {
            console.log("Found device: " + devices[i].deviceId + " with number " + i);
        }
    }

    Camera {
        id: camera

        position: Camera.FrontFace

        //imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        Component.onCompleted:
        {
            for(var filterType in imageProcessing.supportedColorFilters)
            {
                console.log("Filter: " + Number(filterType).toString())
            }
        }

        imageCapture {
            onImageSaved:
            {
                renderer.state = "preview"
                savedPhoto("file:" + camera.imageCapture.capturedImagePath)
                console.log("Saved: " + camera.imageCapture.capturedImagePath)
            }
            onImageCaptured:
            {
                whiteOverlay.state = "released"
                renderer.state = "store"
                console.log("Captured")
            }
            onCaptureFailed:
            {
                renderer.state = "preview"
                failed()
            }
            onErrorStringChanged:
            {
                console.log("Camera error: " + errorString)
            }
        }

        onError:
        {
            console.log("Camera Error: " + errorString)
        }

        onCameraStateChanged:
        {
            if(camera.cameraState == Camera.UnloadedState)
            {
                console.log("Camera State Changed: Unloaded")
                printDevicesToConsole(QtMultimedia.availableCameras)
                camera.stop()
                cameraDiscoveryTimer.start()
            }
            else if(camera.cameraState == Camera.LoadedState)
            {
                console.log("Camera State Changed: Loaded")
                printDevicesToConsole(QtMultimedia.availableCameras)
            }
            else if(camera.cameraState == Camera.ActiveState)
            {
                console.log("Camera State Changed: Active");
                cameraDiscoveryTimer.stop()
            }
            else
            {
                console.log("Camera State Changed: Unknown");
            }
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
        if(camera.imageCapture.ready)
        {
            state  = "snapshot"
            console.log(applicationSettings.foldername.toString())
            var path = applicationSettings.foldername.toString()
            path = path.replace(/^(file:\/{2})/,"");
            var cleanPath = decodeURIComponent(path);
            console.log(cleanPath)
            camera.imageCapture.captureToLocation(cleanPath + "/Pict_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg")
        }
        else
        {
            renderer.state = "preview"
            failed()
        }
    }

    states: [
        State {
            name: "preview"
            PropertyChanges {
                target: whiteOverlay
                state: "released"
            }
            PropertyChanges {
                target: shutterButton
                state: "idle"
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
            PropertyChanges {
                target: whiteOverlay
                state: "released"
            }
        }
    ]
}
