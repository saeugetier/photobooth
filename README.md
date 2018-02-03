# Overview


                                 .------------. Current .---------.
                                 | LED Driver |-------->| 20W LED |
                                 '------------'         '---------'
                                    ^
                                    |
                                 PWM|          .-----------------------.
                                    |          |        Camera         |
                                    |          | connected via GPhoto2 |
                    .------------------.  USB  |                       |
                    |      Raspi       |<------|                       |          -.-.-,~ .   
                    | Kivy Application |       |                       |          )     (     
                    |                  |       '-----------------------'          |_    |     
                    |                  |                                          /(_)---`\   
                    |                  |                                         (_      -'   
                    |                  |           .-------------.                ]      |    
                    |                  |           | Touchscreen |                |    _,')   
                    |                  |  USB/VGA  |             |                [_,-'_-'(   
                    |                  |---------->|             |               (_).-'    \  
                    |                  |           |             |               / /        \ 
                    '------------------'           |             |
                             |       |             '-------------'
                         USB |       |
                             |       |
                             v       |                         .--------------.
                          Storage    '------------------------>| Wii Nunchuck |
                         _.-----._                             '--------------'
                       .-         -.
                       |-_       _-|
                       |  ~-----~  |
                       |           |
                       `._       _.'
                          "-----"   
                          
# Technology
## Software
Framework: Python --> Kivy - https://kivy.org

Used camera: Photo camera connected via USB with GPhoto2 - See supported cameras: http://www.gphoto.org/proj/libgphoto2/support.php

Wii Nunchuck control: https://computers.tutsplus.com/tutorials/using-a-wii-nunchuck-to-control-python-turtle--cms-20984

Software PWM for LED driver: https://projects.drogon.net/raspberry-pi/wiringpi/software-pwm-library/

## Hardware
Camera: Canon EOS 450D

Printer: Canon Selphy Photo Printer

LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html

20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

Wii Nunchuck: https://www.aliexpress.com/item/Nunchuck-Nunchuk-Video-Game-Controller-Remote-For-Nintendo-For-Wii-Console-5-Colors-R179T-Drop-shipping/32809515241.html

Housing: Plywood 8mm - Cutting via lasercutter. Template generated with http://festi.info/boxes.py/



## Ideas:

https://gist.github.com/saeugetier/641647d13465b1b412e531fb1147c462
