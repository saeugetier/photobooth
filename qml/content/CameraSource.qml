Item
{
   id: cameraSource

   property alias output: output
   property string cameraName: ""

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
            gphotoCamera.setCameraByName(cameraName)
            console.log("CameraSource using GPhoto camera device: " + cameraName)
            return
         }
      }
      console.log("CameraSource could not find camera device: " + cameraName)
      cameraSource.state = "noCamera"
   }

   function captureImage()
   {
      if(state === "StandardCamera")
      {
         imageCapture.capture()
      }
      else if(state === "GPhotoCamera")
      {
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
         errorOccured(errorString)
      }
      onImageCaptured: function(image) {
         imageCaptured(image)
      }
      onCaptureError: function(errorString) {
         errorOccured(errorString)
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

   CaptureSession {
      
      id: cameraSession

      videoOutput: output

      imageCapture: ImageCapture {
         id: imageCapture

         onImageCaptured:
         {
            imageCaptured(image)
         }
         onErrorOccurred: {
            errorOccurred(errorString)
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
