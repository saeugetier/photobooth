#!python

import os
os.environ['KIVY_CAMERA'] = "opencv"
from os.path import join, dirname
from kivy.garden.androidtabs import AndroidTabsBase, AndroidTabs
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.lang import Builder
from kivy.app import App
from kivy.uix.camera import Camera
from kivy.logger import Logger
from kivy.uix.scatter import Scatter
from kivy.properties import StringProperty
from kivy.uix.behaviors import CoverBehavior
from kivy.animation import Animation

from kivy.config import Config

from RPi import GPIO

import time
from glob import glob
from random import randint


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
    canvas:
        Color:
            rgb: 1, 1, 1
        Rectangle:
            source: 'data/images/background.jpg'
            size: self.size
            
<Picture>:
    # each time a picture is created, the image can delay the loading
    # as soon as the image is loaded, ensure that the center is changed
    # to the center of the screen.
    on_size: self.center = win.Window.center
    size: image.size
    size_hint: None, None

    Image:
        id: image
        source: root.source

        # create initial image to be 400 pixels width
        size: 400, 400 / self.image_ratio

        # add shadow background
        canvas.before:
            Color:
                rgba: 1,1,1,1
            BorderImage:
                source: 'assets/shadow.png'
                border: (36,36,36,36)
                size:(self.width+72, self.height+72)
                pos: (-36,-36)

        
<MyCameraView>:
    canvas:
        Color:
            rgb: 1, 1, 1
        Rectangle:
            source: 'data/images/background.jpg'
            size: self.size

    orientation: 'vertical'
    Camera:
        id: camera
        resolution: (640,480)
        allow_stretch: True
        play: True
    ToggleButton:
        text: 'Play'
        on_press: camera.play = not camera.play
        size_hint_y: None
        height: '48dp'
    Button:
        text: 'Capture'
        size_hint_y: None
        height: '48dp'
        on_press: root.capture()
'''




if __name__ == '__main__':

    class Picture(Scatter):
        '''Picture is the class that will show the image with a white border and a
        shadow. They are nothing here because almost everything is inside the
        picture.kv. Check the rule named <Picture> inside the file, and you'll see
        how the Picture() is really constructed and used.

        The source property will be the filename to show.
        '''

        source = StringProperty(None)


    class MyCameraView(BoxLayout, AndroidTabsBase):
        def capture(self):
            camera = self.ids['camera']
            timestr = time.strftime("%Y%m%d_%H%M%S")
            camera.export_to_png("pictures/IMG_{}.png".format(timestr))

    class GalleryTab(FloatLayout, AndroidTabsBase):
        pass

    class Example(App):
        def build(self):
            Config.set('graphics', 'fullscreen', 'auto')

            Builder.load_string(kvdemo)
            android_tabs = AndroidTabs()

            self.cameraTab = MyCameraView(text="Kamera")
            android_tabs.add_widget(self.cameraTab)
            self.galleryTab = GalleryTab(text="Gallerie")
            android_tabs.add_widget(self.galleryTab)


            # get any files into images directory
            curdir = dirname(__file__)
            pattern = join(curdir + "/pictures/", '*')

            for filename in glob(pattern):
                try:
                    # load the image
                    picture = Picture(source=filename, rotation=randint(-30, 30))
                    # add to the main field
                    self.galleryTab.add_widget(picture)
                except Exception as e:
                    Logger.exception('Pictures: Unable to load <%s>' % filename)

            return android_tabs

        def on_stop(self):
            self.cameraTab.ids["camera"].on_play(None,False)

    example = Example()
    example.run()