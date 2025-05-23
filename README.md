# Photobooth

## Overview

The goal of the project is to provide a configurable photobooth software for Raspberry Pi or a PC. 

Main features are:
- User Interface with touchscreen input (input via mouse is not recommended). Control via buttons may be added in future.
- Photo preview and capture via V4L2 (Raspberry Pi Camera or Webcam) ~~or via GPhoto2 supported cameras.~~ (currently GPhoto2 is not supported. Will be backported later.)
- Printout with Canon Selphy photo printer or standard printer via CUPS.
- Configurable image collages with own templates. User can select current template in application.
- Password protected settings menu:
	- Copy all photos to USB storage. (currently not supported via flatpak)
	- Copy collage templates form USB storage. (currently not supported via flatpak)
	- Disable printing.
- Brightness of LED preview and flash lights can be adjusted.

                                 .------------. Current .---------.
                                 | LED Driver |-------->| 20W LED |
                                 '------------'         '---------'
                                    ^
                                    |
                          ENABLE/PWM|          .-----------------------.
                                    |          |                       |
                                    |          |                       |
                    .------------------.       |       USB webcam      |
                    |      Raspi       |<------|        via v4l2       |          -.-.-,~ .   
                    |    Application   |       |                       |          )     (     
                    |                  |       '-----------------------'          |_    |     
                    |     based on     |                                          /(_)---`\   
                    |     QT/Quick     |                                         (_      -'   
                    |                  |           .-------------.                ]      |    
                    |                  |           | Touchscreen |                |    _,')   
                    |                  |  USB/HDMI |             |                [_,-'_-'(   
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
Framework: Qt 6.5 or higher - https://qt.io

~~Used camera: Photo camera connected via USB with GPhoto2 - See supported cameras: http://www.gphoto.org/proj/libgphoto2/support.php
Qt GPhoto2 plugin: https://github.com/dept2/qtmultimedia-gphoto~~ (will be backported later)

Flatpak: The main target of this software is the distribution via flatpak. Flatpak can be installed on most of the x86_64 and Aarch64 Linux distributions. It comes with all dependencies needed for execution.

### Tested Hardware
Platform: Tested on PC plattform, Raspberry Pi 3B and Raspbarry Pi 4 (Raspberry Pi 3 might also work)

Camera: Tested with ~~Canon EOS 450D and Raspberry Pi camera~~ USB Webca,. Every camera compatible ~~with gPhoto2 ~~or~~ v4l2 will do.

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

### Flathub ###

Install via `flatpak install io.github.saeugetier.photobooth`. Run with `flatpak run io.github.saeugetier.photobooth` or via desktop item.

### Raspbian an a Raspberry Pi

Minimum Raspbian Buster must be used, which provides QT 6.5 development packages. In order to use GPhoto2 cameras, the Qt GPhoto2 plugin (https://github.com/saeugetier/qtmultimedia-gphoto) must be installed.

For now there is no further support for deployment on Raspbian. Please compile the program yourself from sources.

### Local PC

At least QT 6.5 development packages must be installed in order to compile the application. In order to use GPhoto2 cameras, the Qt GPhoto2 plugin (https://github.com/saeugetier/qtmultimedia-gphoto) must be installed.

For now there is no further support for deployment on local pc. Please compile the program yourself from sources.

### Yocto Linux on a Raspberry Pi

**Deprecated**

In order to get the best performance and integration, the recipes for Yocto can be used: https://github.com/saeugetier/poky-photobooth
 
Prebuild images will be available soon. For more information how to create an own image for deployment on SD card, please look into the repository.

## Configuration

### Configuration file
The local configuration file is stored in /home/<user>/.config/saeugetier/qtbooth.conf
The file contains all application settings and the pin code for the settings menu password protection (only numbers are supported). Default pin code is: 0815

### Template files
The local template files for your collage images are stored in /home/user/.local/share/saeugetier/qtbooth

It contains:
- Background images for the image collages
- The "Collages.xml" describing all image collages
- Border images

The templates can be imported from USB storage. All files with extension "xml,jpg,png,svg" in the folder "layout" will be copied to local template folder.

### How to create own templates
Create your own "Collages.xml" file. You can use the file "XmlData.xml" to customize.

The root node of the XML is named "catalog". It contains nodes for the templates for collages named "collage". The collage must contain at least one "image", a "name", a "background", a "foreground" and an "icon". You can use builtin backgrounds like "WhiteBackground.png" or create your custom one. Forground will be painted in the front layer. So it is highly recommended to have an alpha channel and some cutouts for your photos.

Images will need information about the position and size. The range of the values for position and size is between 0.0 and 1.0. It is possible to define a image boarder for each image.

There are optional properties like "printable", which generates a none printable collage (just a single image). So you can use the photobox as an simple camera without printing capabilities.

Example for "Collages.xml":

```xml
<?xml version="1.0" encoding="UTF-8"?>
<catalog version="1.0">
    <collage>
        <name>Single Image</name>
        <printable>false</printable>
        <icon>Single.svg</icon>
        <background>WhiteBackground.png</background>
        <foreground>ExampleForeground.png</foreground>
        <images>
            <image><position x="0.0" y="0.0"/><size width="1.0" height="1.0"/></image>
        </images>
    </collage>
    <collage>
        <name>Four Images Border</name>
        <icon>FourBorder.svg</icon>
        <background>StarsBackground.jpg</background>
        <images>
            <image>
                <position x="0.0" y="0.0"/><size width="0.5" height="0.5"/>
                <border><file>RedBorder.png</file><margin top="10" left="10" right="10" bottom="10"/></border>
            </image>
            <image>
                <position x="0.5" y="0.0"/><size width="0.5" height="0.5"/>
                <border><file>RedBorder.png</file><margin top="30" left="30" right="30" bottom="30"/></border>
            </image>
            <image>
                <position x="0.0" y="0.5"/><size width="0.5" height="0.5"/>
                <border><file>RedBorder.png</file><margin top="20" left="20" right="20" bottom="20"/></border>
            </image>
            <image>
                <position x="0.5" y="0.5"/><size width="0.5" height="0.5"/>
                <border><file>RedBorder.png</file><margin top="50" left="50" right="50" bottom="50"/></border>
            </image>
        </images>
    </collage>
</catalog>
```

## Issue reporting
Please use the [issue tracker](https://github.com/saeugetier/photobooth/issues) for bug reporting and feature request. If there are specific bugs or feature request belonging to the Yocto image,  please use [the issue tracker for poky-photobooth](https://github.com/saeugetier/poky-photobooth).
