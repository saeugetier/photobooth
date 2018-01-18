from kivy.core.camera import CameraBase
import gphoto2 as gp

class CameraGPhoto2(CameraBase):
    '''
    Implementation of CameraBase using GPhoto2
    '''
    _update_ev = None

    def __init__(self, **kwargs):
        super(CameraGPhoto2, self).__init__(**kwargs)

    def init_camera(self):
        gp.check_result(gp.use_python_logging())
        camera = gp.check_result(gp.gp_camera_new())
        try:
            gp.check_result(gp.gp_camera_init(camera))
        except gp.GPhoto2Error:
            pass

        self._config = gp.check_result(gp.gp_camera_get_config(camera))

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
            gp.check_result(gp.gp_camera_set_config(camera, config))



    def _update(self, dt):
        pass