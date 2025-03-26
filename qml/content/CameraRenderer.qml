import QtQuick
import QtMultimedia
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts
import BackgroundFilter

Item {
   id: renderer
   signal savedPhoto(string filename)
   signal failed

   property bool photoProcessing: (state === "snapshot")
   property bool mirrored: true
   property string deviceId: camera.deviceId
   property alias backgroundFilter: backgroundFilter

   function printDevicesToConsole(devices) {
      console.log("Found " + devices.length + " camera devices!")
      for (var i = 0; i < devices.length; i++) {
         console.log(
                  "Found device: " + devices[i].deviceId + " with number " + i)
      }
   }

   MediaDevices {
      id: mediaDevices
   }

   onDeviceIdChanged: id => {
                         // get the camera device with id from mediaDevices
                         console.log("Selected camera: " + id)
                         var availableCameras = mediaDevices.videoInputs
                         for (var i = 0; i < availableCameras.length; i++) {
                            if (availableCameras[i].deviceId === id) {
                               camera.cameraDevice = availableCameras[i]
                               break
                            }
                         }
                      }

   CaptureSession {

      camera: Camera {
         id: camera
         cameraDevice: mediaDevices.defaultVideoInput
         exposureMode: Camera.ExposurePortrait
         exposureCompensation: -1.0
         whiteBalanceMode: Camera.WhiteBalanceAuto
         flashMode: Camera.FlashAuto
         torchMode: Camera.TorchAuto
      }

      id: cameraSession

      videoOutput: output

      imageCapture: ImageCapture {
         id: imageCapture

         onImageSaved: (_, fileName) => {
                          renderer.state = "preview"
                          savedPhoto("file:" + fileName)
                          console.log("Saved: " + fileName)
                       }
         onImageCaptured: {
            whiteOverlay.state = "released"
            renderer.state = "store"
            console.log("Captured")
         }
         onErrorOccurred: {
            renderer.state = "preview"
            failed()
         }
         onErrorStringChanged: {
            console.log("Camera error: " + errorString)
         }
      }


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

   ReplaceBackgroundVideoFilter {
      id: backgroundFilter
      videoSink: output.videoSink
   }

   Connections {
      id: cameraErrorListener
      target: camera
      function errorOccured(_, errorString) {
         console.log("Camera Error: " + errorString)
      }
   }

   VideoOutput {
      id: output

      anchors.fill: parent

      visible: false

      //filters: [backgroundFilter]

      //focus: visible // to receive focus and capture key events when visible
   }

   CaptureSession
   {
      id: maskSession
      videoFrameInput: backgroundFilter
      videoOutput: maskOutput
   }


   VideoOutput {
      id: maskOutput

      anchors.fill: parent

      onContentRectChanged: {
         console.log("Content rect changed: " + Qt.rect(contentRect.x / width, contentRect.y /height, contentRect.width / width, contentRect.height / height))
      }

      layer.enabled: true
      layer.effect: ShaderEffect {
         id: mirrorEffect
         property variant source: ShaderEffectSource {
            sourceItem: maskOutput
            hideSource: true
         }

         property variant bgSource : Image {
            id: bgImage
            source: "qrc:/images/backgrounds/pexels-pixabay-259915.jpg"
            fillMode: Image.PreserveAspectCrop
         }

         property bool mirrored: renderer.mirrored
         property bool enableMask: true
         anchors.fill: maskSession
         fragmentShader: "qrc:/shaders/previewshader.frag.qsb"
      }
   }

   WhiteOverlay {
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
   function takePhoto() {
      if (cameraSession.imageCapture.readyForCapture) {
         state = "snapshot"
         console.log(applicationSettings.foldername.toString())
         var path = applicationSettings.foldername.toString()
         path = path.replace(/^(file:\/{2})/, "")
         var cleanPath = decodeURIComponent(path)
         console.log(cleanPath)
         cameraSession.imageCapture.captureToFile(
                  cleanPath + "/Pict_" + new Date().toLocaleString(
                     locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg")
      } else {
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
         StateChangeScript {
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
         NumberAnimation {
            duration: 2000
         }
         ScriptAction {
            script: {
               camera.stop()
            }
         }
      }
   }
}
