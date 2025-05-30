# Overview

This project contains yet another [Photobooth](https://github.com/saeugetier/photobooth). The software is intended to run on a Raspberry Pi 2 and upwards, but it can run on any linux PC. At least a RPi5 is recommended when using the AI background replacement. A preview image is displayed in capture mode. After countdown, a picture is taken and can be discarded or saved. Saved images can be printed immediately or printed  collage image with multiple photos. All saved images are shown on front page when the photobox is idle.

Photo capture is currently only supported via V4L2 camera. In prior version of the software DSLR via GPhoto2 was also supported. This feature has now to be backported after changing from Qt5 to Qt6. Supporting Raspberry Pi Cameras via libcamera is a planned feature.

The application is distributed via Flathub. But interested people can compile the software by themselves. Steps are documented. The photobooth application used a [Yocto Linux deployment](https://github.com/saeugetier/poky-photobooth/) in the past. This deployment is deprecated now, because the distribution via flatpak is more convinient. 

## Video

[![](https://markdown-videos-api.jorgenkh.no/youtube/fB2aQGPT-wg?width=640&height=360)](https://youtu.be/fB2aQGPT-wg)
(Link to Youtube)

# Technology
## Software
Framework: Qt 6.5 or higher - [https://qt.io](https://qt.io)

Neural network execution for background removal is done via onnxruntime.

## Tested Hardware
Camera: USB Webcam

Printer: Canon Selphy Photo Printer

LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html

20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/

Canon Selphy CP910 photo printer cpnnected via Wifi

