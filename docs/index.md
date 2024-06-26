# Overview

This project contains yet another [Photobooth](https://github.com/saeugetier/photobooth). The software is intended to run on a Raspberry Pi 2 and upwards, but it can run on any linux PC. A preview image is displayed in capture mode. After countdown, a picture is taken and can be discarded or saved. Saved images can be printed immediately or printed  collage image with multiple photos. All saved images are shown on front page when the photobox is idle.

As image source a DSLR over GPhoto2 or a V4L2 camera (Raspberry Pi Camera or webcam) can be used.

The application can be either compiled and deployed on an existing Raspberry Pi OS installation, or a ready to go image can be build by Yocto build system: [https://github.com/saeugetier/poky-photobooth/](https://github.com/saeugetier/poky-photobooth/)

## Video

[![](https://markdown-videos-api.jorgenkh.no/youtube/fB2aQGPT-wg?width=640&height=360)](https://youtu.be/fB2aQGPT-wg)
(Link to Youtube)

# Technology
## Software
Framework: Qt 5.9 or higher - [https://qt.io](https://qt.io)

By default V4L2 are used. If you use a Raspberry Pi Camera, the v4l2 camera kernel module must be used.

In order to use GPhoto2 cameras, the [QT Multimedia Gphoto Plugin](https://github.com/saeugetier/qtmultimedia-gphoto) must be installed. A [list of supported cameras](http://www.gphoto.org/proj/libgphoto2/support.php) can be trieved from the GPhoto2 website.

The software can be build to run on a local PC. Or it can run on an Raspberry Pi. In order to get the best performance and integration, the recipes for Yocto can be used: https://github.com/saeugetier/poky-photobooth

## Tested Hardware
Camera: Canon EOS 450D

Printer: Canon Selphy Photo Printer

LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html

20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/

Canon Selphy CP910 photo printer cpnnected via Wifi

