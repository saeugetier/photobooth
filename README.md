# Photobooth

## Overview

The goal of the project is to provide a configurable photobooth software for Raspberry Pi or a PC. 

Main features are:
- User Interface with touchscreen input (input via mouse is not recommended). Control via buttons may be added in future.
- Photo preview and capture via V4L2 (Raspberry Pi Camera or Webcam) or via GPhoto2 supported cameras.
- Printout with Canon Selphy photo printer or standard printer via CUPS.
- Configurable image collages with own templates. User can select current template in application.
- Password protected settings menu:
	- Copy all photos to USB storage.
	- Copy collage templates form USB storage.
	- Disable printing.
- Brightness of LED preview and flash lights can be adjusted.

                                 .------------. Current .---------.
                                 | LED Driver |-------->| 20W LED |
                                 '------------'         '---------'
                                    ^
                                    |
                                 PWM|          .-----------------------.
                                    |          |     DSLR Camera       |
                                    |          | connected via GPhoto2 |
                    .------------------.       |     or raspi cam      |
                    |      Raspi       |<------|        via v4l2       |          -.-.-,~ .   
                    |    Application   |       |                       |          )     (     
                    |                  |       '-----------------------'          |_    |     
                    |     based on     |                                          /(_)---`\   
                    |     QT/Quick     |                                         (_      -'   
                    |                  |           .-------------.                ]      |    
                    |                  |           | Touchscreen |                |    _,')   
                    |                  |  USB/VGA  |             |                [_,-'_-'(   
                    |                  |---------->|             |               (_).-'    \  
                    |                  |           |             |               / /        \ 
                    '------------------'           |             |
                             |                     '-------------'
                         USB |
                             |
                             v 
                          Storage
                         _.-----._   
                       .-         -.
                       |-_       _-|
                       |  ~-----~  |
                       |           |
                       `._       _.'
                          "-----"   
                          
## Technology
### Software
Framework: Qt 5.11 or higher - https://qt.io

Used camera: Photo camera connected via USB with GPhoto2 - See supported cameras: http://www.gphoto.org/proj/libgphoto2/support.php
Qt GPhoto2 plugin: https://github.com/dept2/qtmultimedia-gphoto

Yocto Linux: The main target of this software is the deployment with Yocto Linux. With Yocto Linux a ready to use image for the SD card can be built. The image is very small and contains only needed binaries to run the application.

### Tested Hardware
Platform: Tested on PC plattform, Raspberry Pi 3B and Raspbarry Pi 4 (Raspberry Pi 3 might also work)

Camera: Tested with Canon EOS 450D and Raspberry Pi camera. Every camera compatible with gPhoto2 or v4l2 will do.

Printer: Tested Canon Selphy Photo Printer CP910 over wifi via https://github.com/saeugetier/go-selphy-cp. Tested with standard inkjet printer via CUPS.

Light/Flash: 
- LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html
- 20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Display: A touchscreen connected via HDMI is highly recommended.

I2C RTC: If using a Raspberry Pi, it is recommended to use a realtime clock.

### Housing / Electronics
My own housing is documented in a seperate git repository: https://github.com/saeugetier/photobooth_hardware

Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/

## Deployment / Installation
### Yocto Linux on a Raspberry Pi

In order to get the best performance and integration, the recipes for Yocto can be used: https://github.com/saeugetier/poky-photobooth
 
Prebuild images will be available soon. For more information how to create an own image for deployment on SD card, please look into the repository.

### Raspbian an a Raspberry Pi

Minimum Raspbian Buster must be used, which provides QT 5.11 development packages. In order to use GPhoto2 cameras, the Qt GPhoto2 plugin (https://github.com/saeugetier/qtmultimedia-gphoto) must be installed.

For now there is no further support for deployment on Raspbian. Please compile the program yourself from sources.

### Local PC

At least QT 5.11 development packages must be installed in order to compile the application. In order to use GPhoto2 cameras, the Qt GPhoto2 plugin (https://github.com/saeugetier/qtmultimedia-gphoto) must be installed.

For now there is no further support for deployment on local pc. Please compile the program yourself from sources.

## Configuration

### Configuration file
The local configuration file is stored in /home/<user>/.config/saeugetier/qtbooth.conf
The file contains all application settings and the pin code for the settings menu password protection (only numbers are supported). Default pin code is: 0815

### Template files
The local template files for your collage images are stored in /home/<user>/.local/share/saeugetier/qtbooth

It contains:
- Background images for the image collages
- The "Collages.xml" describing all image collages
- Border images

The templates can be imported from USB storage. All files with extension "xml,jpg,png,svg" in the folder "layout" will be copied to local template folder.
