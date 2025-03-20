import QtQuick
import QtMultimedia
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts

Item {
    id: renderer
    signal savedPhoto(string filename)
    signal failed

    property bool photoProcessing: (state === "snapshot")
    property bool mirrored: true
    property string deviceId: camera.deviceId

    function printDevicesToConsole(devices)
    {
        console.log("Found " + devices.length + " camera devices!")
        for(var i = 0; i < devices.length; i++)
        {
            console.log("Found device: " + devices[i].deviceId + " with number " + i);
        }
    }

    MediaDevices
    {
        id: mediaDevices
    }

    CaptureSession {

        camera:  Camera
        {
            id: camera
            //cameraDevice: mediaDevices.defaultVideoInput
        }

        id: cameraSession

        videoOutput: output

        //position: Camera.UnspecifiedPosition

        //imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
        /*exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }*/

        //flash.mode: Camera.FlashRedEyeReduction

        Component.onCompleted:
        {
            //for(var filterType in imageProcessing.supportedColorFilters)
            //{
            //    console.log("Filter: " + Number(filterType).toString())
            //}
        }

        imageCapture : ImageCapture {
            id: imageCapture

            //videoOutput: output

            onImageSaved: (_, fileName) =>
            {
                renderer.state = "preview"
                savedPhoto("file:" + fileName)
                console.log("Saved: " + fileName)
            }
            onImageCaptured:
            {
                whiteOverlay.state = "released"
                renderer.state = "store"
                console.log("Captured")
            }
            onErrorOccurred:
            {
                renderer.state = "preview"
                failed()
            }
            onErrorStringChanged:
            {
                console.log("Camera error: " + errorString)
            }
        }

        /*onError:
        {
            console.log("Camera Error: " + errorString)
        }*/

        /*onCameraStateChanged:
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
        }*/

    }

    VideoOutput {
        id: output

        anchors.fill: parent

        layer.enabled: true
        layer.effect: ShaderEffect
        {
            id: mirrorEffect
            property variant source: ShaderEffectSource {
                sourceItem: output
                hideSource: true
            }
            anchors.fill: output
            fragmentShader: mirrored ? "qrc:/shaders/vmirror.frag.qsb" : "qrc:/shaders/passthrough.frag.qsb"
        }

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

   /* Timer
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
    }*/

    function takePhoto()
    {
        if(cameraSession.imageCapture.readyForCapture)
        {
            state  = "snapshot"
            console.log(applicationSettings.foldername.toString())
            var path = applicationSettings.foldername.toString()
            path = path.replace(/^(file:\/{2})/,"");
            var cleanPath = decodeURIComponent(path);
            console.log(cleanPath)
            cameraSession.imageCapture.captureToFile(cleanPath + "/Pict_"+ new Date().toLocaleString(locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg")
        }
        else
        {
            renderer.state = "preview"
            failed()
        }
    }

    states: [
        State {
            name: "idle"
        },
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
            StateChangeScript
            {
                script: {
                    camera.start()
                }
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
    transitions: Transition {
        to: "idle"
        SequentialAnimation {
            NumberAnimation { duration: 2000 }
            ScriptAction {
               script: {
                  camera.stop()
              }
            }
        }
    }
}
