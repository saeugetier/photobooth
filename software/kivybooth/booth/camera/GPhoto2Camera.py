from kivy.core.camera import CameraBase
import gphoto2 as gp
import threading
import time
from queue import Queue
from PIL import Image
import io

class CameraGPhoto2(CameraBase):
    '''
    Implementation of CameraBase using GPhoto2
    '''
    class CameraThread(threading.Thread):
        stop = threading.Event()

        def __init__(self, capture_device, queue):
            super(CameraGPhoto2.CameraThread, self).__init__()
            self._capture_device = capture_device
            self._queue = queue

        def run(self):
            while not self.stop.is_set():
                if self._queue.empty():
                    camera_file = gp.check_result(gp.gp_camera_capture_preview(self._capture_device))
                    file_data = gp.check_result(gp.gp_file_get_data_and_size(camera_file))
                    image = Image.open(io.BytesIO(file_data))
                    self._queue.put(image)
                else:
                    time.sleep(0.05)


    _update_ev = None

    def __init__(self, **kwargs):
        super(CameraGPhoto2, self).__init__(**kwargs)
        self.capture = Queue()
        self._thread = None

    def __del__(self):
        if self.capture_device is not None:
            gp.check_result(gp.gp_camera_exit(self.capture_device))

    def init_camera(self):
        if self.capture_device is not None:
            gp.check_result(gp.gp_camera_exit(self.capture_device))

        gp.check_result(gp.use_python_logging())
        self.capture_device = gp.check_result(gp.gp_camera_new())
        try:
            gp.check_result(gp.gp_camera_init(self.capture_device))
        except gp.GPhoto2Error:
            pass

        self._config = gp.check_result(gp.gp_camera_get_config(self.capture_device))

        OK, image_format = gp.gp_widget_get_child_by_name(self._config, 'imageformat')
        if OK >= gp.GP_OK:
            # get current setting
            value = gp.check_result(gp.gp_widget_get_value(image_format))
            # make sure it's not raw
            if 'raw' in value.lower():
                print('Cannot preview raw images')
                return False

        OK, capture_size_class = gp.gp_widget_get_child_by_name(
            self._config, 'capturesizeclass')
        if OK >= gp.GP_OK:
            # set value
            value = gp.check_result(gp.gp_widget_get_choice(capture_size_class, 2))
            gp.check_result(gp.gp_widget_set_value(capture_size_class, value))
            # set config
            gp.check_result(gp.gp_camera_set_config(self.capture_device, self._config))


    def start(self):
        super(CameraGPhoto2, self).start()
        if self._thread is None:
            self._thread = CameraGPhoto2.CameraThread(self.capture, self.capture_device)
            self._thread.start()


    def stop(self):
        super(CameraGPhoto2, self).stop()
        if self._thread is not None:
            self._thread.stop.set()

    def _update(self, dt):
        if self.stopped:
            return
        if self._texture is None:
            # Create the texture
            self._texture = Texture.create(self._resolution)
            self._texture.flip_vertical()
            self.dispatch('on_load')

        #### update....
