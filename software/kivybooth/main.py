#!python

import os
os.environ['KIVY_CAMERA'] = "gphoto2"
from kivy.garden.androidtabs import AndroidTabsBase, AndroidTabs
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.lang import Builder
from kivy.app import App
from kivy.uix.camera import Camera


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
            

<GalleryTab>:
    Button:
        text: root.text
        
<MyCameraView>:
    orientation: 'vertical'
    Camera:
        id: camera
        resolution: (640,480)
        play: True
    ToggleButton:
        text: 'Play'
        on_press: camera.play = not camera.play
        size_hint_y: None
        height: '48dp'
'''


if __name__ == '__main__':

    class MyCameraView(BoxLayout, AndroidTabsBase):
        pass

    class GalleryTab(FloatLayout, AndroidTabsBase):
        pass

    class Example(App):

        def build(self):

            Builder.load_string(kvdemo)
            android_tabs = AndroidTabs()

            tab = MyCameraView(text="Kamera")
            android_tabs.add_widget(tab)
            tab = GalleryTab(text="Gallerie")
            android_tabs.add_widget(tab)


            return android_tabs

    Example().run()

