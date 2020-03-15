# Photobooth

## Overview


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
Framework: Qt 5.9 or higher - https://qt.io

Used camera: Photo camera connected via USB with GPhoto2 - See supported cameras: http://www.gphoto.org/proj/libgphoto2/support.php
Qt GPhoto2 plugin: https://github.com/dept2/qtmultimedia-gphoto

The software can be build to run on a local PC. Or it can run on an Raspberry Pi. In order to get the best performance and integration, the recipes for Yocto can be used: https://github.com/saeugetier/poky-photobooth

### Tested Hardware
Platform: Tested on PC plattform, Raspberry Pi 3B and Raspbarry Pi 4 (Raspberry Pi 3 might also work)

Camera: Tested with Canon EOS 450D and Raspberry Pi camera. Every camera compatible with gPhoto2 or v4l2 will do.

Printer: Tested Canon Selphy Photo Printer CP910

Light/Flash: 
- LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html
- 20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Display: A touchscreen is highly recommended

RTC: If using a Raspberry Pi, it is recommended to use a realtime clock.

### Housing
Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/

