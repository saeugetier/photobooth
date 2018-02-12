import os
from configparser import ConfigParser
config = ConfigParser()
config.read("config/config.ini")

os.environ['KIVY_CAMERA'] = config.get("Camera","input")

## TODO: everything what is