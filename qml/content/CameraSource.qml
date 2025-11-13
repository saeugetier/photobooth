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
            cameraSource.state = "GPhotoCamera"
            gphotoCamera.cameraName = cameraName
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
         cameraSource.errorOccurred(errorString)
      }
      onImageCaptured: function(image) {
         cameraSource.imageCaptured(image)
      }
      onCaptureError: function(errorString) {
         cameraSource.errorOccurred(errorString)
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
         console.log("Camera Error: " + errorString)
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
            cameraSource.errorOccurred(errorString)
         }
      }
   }

   VideoOutput {
      id: output
      anchors.fill: parent
      visible: false
   }

   states: [
      State {
         name: "noCamera"
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
      }
   ]
}
