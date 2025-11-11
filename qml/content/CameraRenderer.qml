import QtQuick
import QtMultimedia
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts
import BackgroundFilter
import CaptureProcessor
import GPhotoCamera

Item {
   id: renderer
   signal savedPhoto(string filename)
   signal failed

   property bool photoProcessing: (state === "snapshot")
   property bool mirrored: true
   property string cameraName: ""
   property alias backgroundFilter: backgroundFilter
   property bool backgroundFilterEnabled: false
   property url backgroundImage: ""

   onCameraNameChanged:
   {
      print("Camera changed to " + cameraName)
   }

   CaptureProcessor
   {
      id: captureProcessor

      rotation: applicationSettings.cameraOrientation

      onCaptureSaved: fileName =>
                      {

                         if(backgroundFilterEnabled)
                         {
                            console.log("Process file: " + fileName)
                            backgroundFilter.processCapture(fileName)
                         }
                         else
                         {
                            renderer.state = "preview"
                            savedPhoto("file:" + fileName)
                            console.log("Saved: " + fileName)
                         }
                      }
   }

   CameraSource
   {
      id: cameraSource

      cameraName: renderer.cameraName

      onImageCaptured: function(image) {
         whiteOverlay.state = "released"
         renderer.state = "store"
         console.log("Captured: " + image)

         console.log(applicationSettings.foldername.toString())
         var path = applicationSettings.foldername.toString()
         if(backgroundFilterEnabled)
         {
            path = path + "/raw"
         }
         path = path.replace(/^(file:\/{2})/, "")
         var cleanPath = decodeURIComponent(path)
         console.log(cleanPath)

         captureProcessor.saveCapture(image, cleanPath + "/Pict_" + new Date().toLocaleString(
                                 locale, "dd_MM_yyyy_hh_mm_ss") + ".jpg")
      }

      onErrorOccurred: function(errorString) {
         renderer.state = "preview"
         failed()
      }
   }


   ReplaceBackgroundVideoFilter {
      id: backgroundFilter
      videoSink: cameraSource.output.videoSink
      background: backgroundImage

      onCaptureProcessingFinished: fileName =>
                                   {
                                      console.log("Capture processing finished")
                                      if (backgroundFilterEnabled) {
                                         renderer.state = "preview"
                                         savedPhoto("file:" + fileName)
                                         console.log("Saved: " + fileName)
                                      }
                                   }
   }

   VideoOutput {
      id: output

      anchors.fill: parent

      visible: false

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

      rotation: applicationSettings.cameraOrientation

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
            source: backgroundImage
            fillMode: Image.PreserveAspectCrop
         }

         property bool mirrored: renderer.mirrored
         property bool enableMask: true
         property rect contentRect: Qt.rect(maskOutput.contentRect.x / maskOutput.width, maskOutput.contentRect.y / maskOutput.height, maskOutput.contentRect.width / maskOutput.width, maskOutput.contentRect.height / maskOutput.height)
         anchors.fill: maskOutput
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

   function takePhoto() {
      if (cameraSource.readyForCapture) {
         state = "snapshot"
         cameraSource.captureImage()
      } else {
         renderer.state = "preview"
         failed()
      }
   }

   BusyIndicator {
      id: busyIndicator
      anchors.centerIn: parent
      visible: false
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
         PropertyChanges {
            target: busyIndicator
            visible: false
         }
         StateChangeScript {
            script: {
               cameraSource.start()
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
            state: "processing"
         }
         PropertyChanges {
            target: busyIndicator
            visible: true
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
               cameraSource.stop()
            }
         }
      }
   }
}
