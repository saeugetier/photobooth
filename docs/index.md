# Overview

This project contains yet another [Photobooth](https://github.com/saeugetier/photobooth). The software is intended to run on a Raspberry Pi 2/3, but it can run on any linux PC. A preview image is displayed in capture mode. After countdown, a picture is taken and can be discarded or saved. Saved images can be printed immediately or printed as 2 by 2 collage image. All saved images are shown in the gallery page.

As image source a DSLR over GPhoto2 or a V4L2 camera (Raspberry Pi Camera or webcam) can be used.

The application can be either compiled and deployed on an existing Raspbian installation, or a ready to go image can be build by Yocto build system: [https://github.com/saeugetier/poky-photobooth/](https://github.com/saeugetier/poky-photobooth/)

## Video

[![Video](https://img.youtube.com/vi/Z9pVK-X5Wz4/0.jpg)](https://youtu.be/Z9pVK-X5Wz4)

# Technology
## Software
Framework: Qt 5.9 or higher - https://qt.io

By default V4L2 are used. If you use a Raspberry Pi Camera, the v4l2 camera kernel module must be used.

In order to use GPhoto2 cameras, the [QT Multimedia Gphoto Plugin](https://github.com/saeugetier/qtmultimedia-gphoto) must be installed. A [list of supported cameras](http://www.gphoto.org/proj/libgphoto2/support.php) can be trieved from the GPhoto2 website.

The software can be build to run on a local PC. Or it can run on an Raspberry Pi. In order to get the best performance and integration, the recipes for Yocto can be used: https://github.com/saeugetier/poky-photobooth

## Hardware
Camera: Canon EOS 450D

Printer: Canon Selphy Photo Printer

LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html

20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Wii Nunchuck: https://www.aliexpress.com/item/Nunchuck-Nunchuk-Video-Game-Controller-Remote-For-Nintendo-For-Wii-Console-5-Colors-R179T-Drop-shipping/32809515241.html

Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/

