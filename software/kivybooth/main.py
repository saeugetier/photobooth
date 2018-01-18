#!python


import platform
print(platform.python_version())

import os
os.environ['KIVY_CAMERA'] = ""
from kivy.garden.androidtabs import AndroidTabsBase, AndroidTabs
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.floatlayout import FloatLayout
from kivy.lang import Builder
from kivy.app import App
from kivy.core.camera import Camera as CameraContainer
from kivy.core.camera.camera_opencv import CameraOpenCV
from kivy.uix.camera import Camera
from kivy.uix.label import Label

from booth.camera.GPhoto2Camera import CameraGPhoto2



kvdemo = '''
#:import get_color_from_hex kivy.utils.get_color_from_hex
<AndroidTabsBar>:
    canvas.before:
        Color:
            rgba: get_color_from_hex('#03A9F4')
        Rectangle:
            pos: self.pos
            size: self.size
        # you can add a bit of shade if you want
        Color:
            rgba: 0,0,0,.3
        Rectangle:
            pos: self.pos[0], self.pos[1] - 1
            size: self.size[0], 1
        Color:
            rgba: 0,0,0,.2
        Rectangle:
            pos: self.pos[0], self.pos[1] - 2
            size: self.size[0], 1
        Color:
            rgba: 0,0,0,.05
        Rectangle:
            pos: self.pos[0], self.pos[1] - 3
            size: self.size[0], 1
            
<MyCameraView>:
    Label:
        text: root.text


<MyTab>:
    Button:
        text: root.text
'''


if __name__ == '__main__':

    try:
        CameraContainer = CameraGPhoto2()
    except:
        CameraContainer = CameraOpenCV()

    class MyCameraView(Camera):
        pass

    class MyTab(FloatLayout, AndroidTabsBase):

        pass

    class GalleryTab(FloatLayout, AndroidTabsBase):

        pass

    class Example(App):

        def build(self):

            Builder.load_string(kvdemo)
            android_tabs = AndroidTabs()

            tab = MyTab(text="Kamera")
            android_tabs.add_widget(tab)
            tab = GalleryTab(text="Gallerie")
            android_tabs.add_widget(tab)

            return android_tabs

    Example().run()

