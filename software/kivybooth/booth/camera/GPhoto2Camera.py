from kivy.core.camera import CameraBase
import gphoto2 as gp
import threading
import time
from queue import Queue
from kivy.core.image import Image
from kivy.base import Clock
import io

class CameraGPhoto2(CameraBase):
    '''
    Implementation of CameraBase using GPhoto2
    '''
    class CameraThread(threading.Thread):
        stop = threading.Event()

        def __init__(self, capture_device, queue):
            self._capture_device = capture_device
            self._queue = queue
            super(CameraGPhoto2.CameraThread, self).__init__()


        def run(self):
            while not self.stop.is_set():
                if self._queue.empty():
                    camera_file = gp.check_result(gp.gp_camera_capture_preview(self._capture_device))
                    file_data = gp.check_result(gp.gp_file_get_data_and_size(camera_file))
                    self._queue.put(io.BytesIO(file_data))
                else:
                    time.sleep(0.05)


    _update_ev = None

    def __init__(self, **kwargs):
        self.capture = Queue()
        self._thread = None
        self.image = None
        self._update_ev = None
        super(CameraGPhoto2, self).__init__(**kwargs)


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


    def _update(self, dt):
        print("test")
        if self.stopped:
            return


        if self.capture.empty() is False:
            if self.image is None:
                capture = self.capture.get()
                self.image = Image(capture, ext="jpg")
                self._texture = self.image.texture
                self.dispatch('on_load')
            else:
                self.image.load_memory(self.capture.get(), "jpg")
            self.dispatch('on_texture')

        #### update....

    def start(self):
        super(CameraGPhoto2, self).start()
        if self._thread is None:
            self._thread = CameraGPhoto2.CameraThread(self.capture_device, self.capture)
            self._thread.start()

        if self._update_ev is not None:
            self._update_ev.cancel()
        self._update_ev = Clock.schedule_interval(self._update, 1/10)


    def stop(self):
        if self._update_ev is not None:
            self._update_ev.cancel()
            self._update_ev = None
        if self._thread is not None:
            self._thread.stop.set()
        super(CameraGPhoto2, self).stop()



