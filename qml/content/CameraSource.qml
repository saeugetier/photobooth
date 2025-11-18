import GPhotoCamera
import QtQuick
import QtMultimedia
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts

Item
{
   id: cameraSource

   property alias output: output
   property string cameraName: ""
   property bool readyForCapture: ((cameraSession.imageCapture.readyForCapture) || (cameraSource.state === "GPhotoCamera"))

   signal imageCaptured(var image)
   signal errorOccurred(var errorString)

   onCameraNameChanged: {
      stop()
      console.log("CameraSource selected cameraName: " + cameraName)
      var availableCameras = mediaDevices.videoInputs
      for (var i = 0; i < availableCameras.length; i++) {
         if (availableCameras[i].description === cameraName) {
            systemCamera.cameraDevice = availableCameras[i]
            cameraSource.state = "StandardCamera"
            console.log("CameraSource using standard camera device: " + cameraName)
            return
         }
      }
      var gphotoCameras = gphotoCamera.availableCameras()
      for (var j = 0; j < gphotoCameras.length; j++) {
         if (gphotoCameras[j] === cameraName) {
            gphotoCamera.cameraName = cameraName
            cameraSource.state = "GPhotoCamera"
            console.log("CameraSource using GPhoto camera device: " + cameraName)
            return
         }
      }
      console.log("CameraSource could not find camera device: " + cameraName)
      cameraSource.state = "noCamera"
   }

   function start()
   {
      if(state === "StandardCamera")
      {
         systemCamera.start()
      }
      else if(state === "GPhotoCamera")
      {
         //gphotoCamera.startCamera()
      }
      else
      {
         console.log("No camera available to start!")
      }
   }

   function stop()
   {
      if(state === "StandardCamera")
      {
         systemCamera.stop()
      }
      else if(state === "GPhotoCamera")
      {
         //gphotoCamera.stopCamera()
      }
      else
      {
         console.log("No camera available to stop!")
      }
   }

   function captureImage()
   {
      if(state === "StandardCamera")
      {
         console.log("Standard camera capture")
         cameraSession.imageCapture.capture()
      }
      else if(state === "GPhotoCamera")
      {
         console.log("GPhoto capture")
         gphotoCamera.captureImage()
      }
      else
      {
         console.log("No camera available to capture image!")
      }
   }

   MediaDevices {
      id: mediaDevices
   }

   GPhotoCamera {
      id: gphotoCamera

      onErrorOccurred: function(errorString) {
         if(state === "GPhotoCamera")
         {
            cameraSource.errorOccurred(errorString)
         }
      }
      onImageCaptured: function(image) {
         cameraSource.imageCaptured(image)
      }
      onCaptureError: function(errorString) {
         if(state === "GPhotoCamera")
         {
            cameraSource.errorOccurred(errorString)
         }
      }
   }

   Camera {
      id: systemCamera
      cameraDevice: mediaDevices.defaultVideoInput
      exposureMode: Camera.ExposurePortrait
      exposureCompensation: -1.0
      whiteBalanceMode: Camera.WhiteBalanceAuto
      flashMode: Camera.FlashAuto
      torchMode: Camera.TorchAuto
   }

   Connections {
      id: cameraErrorListener
      target: systemCamera
      function errorOccured(_, errorString) {
         if(state === "StandardCamera")
         {
            cameraSource.errorOccurred(errorString)
         }
      }
   }

   CaptureSession {
      
      id: cameraSession

      videoOutput: output

      imageCapture: ImageCapture {
         id: imageCapture

         onImageCaptured: function(requestId, preview)
         {
            cameraSource.imageCaptured(preview)
         }
         onErrorOccurred: {
            if(state === "StandardCamera")
            {
               cameraSource.errorOccurred(errorString)
            }
         }
      }
   }

   VideoOutput {
      id: output
      anchors.fill: parent
      visible: false  // video is displayed in masked output after applying the filter
   }

   property string lastError: ""

   onErrorOccurred: function(errorString) {
      lastError = errorString
      cameraSource.state = "Error"
   }

   Rectangle {
      id: errorDisplay
      anchors.fill: parent
      color: "#99000000"
      radius: 6
      visible: false

      Text {
         id: errorText
         text: lastError
         color: "white"
         font.pixelSize: 36
         wrapMode: Text.WordWrap
         horizontalAlignment: Text.AlignHCenter
         verticalAlignment: Text.AlignVCenter
         anchors.centerIn: parent
         width: parent.width * 0.9
      }

   }

   states: [
      State {
         name: "noCamera"
         PropertyChanges {
            target: errorDisplay
            visible: true
         }
         PropertyChanges {
            target: cameraSource
            lastError: qsTr("No camera found with the name: ") + cameraName
         }
      },
      State {
         name: "StandardCamera"
         PropertyChanges {
            target: cameraSession
            camera: systemCamera
         }
      },
      State {
         name: "GPhotoCamera"
         PropertyChanges {
            target: cameraSession
            videoFrameInput: gphotoCamera
         }
      },
      State {
         name: "Error"
         PropertyChanges {
            target: errorDisplay
            visible: true
         }
      }

   ]
}
