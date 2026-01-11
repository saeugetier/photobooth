# Overview

This project contains yet another [Photobooth](https://github.com/saeugetier/photobooth). The software is intended to run on a Raspberry Pi 3B and upwards, but it can run on any Linux PC. At least a Raspberry Pi 5 is recommended when using the AI background replacement. A preview image is displayed in capture mode. After countdown, a picture is taken and can be discarded or saved. Saved images can be printed immediately or printed as a collage image with multiple photos. All saved images are shown on the front page when the photobox is idle.

Photo capture is supported via multiple camera backends:

| Backend | Camera Types | Notes |
|---------|-------------|-------|
| **V4L2** | USB Webcams, capture cards | Standard Linux video interface |
| **libcamera** | Raspberry Pi Camera Modules | Pi Camera v1/v2/v3, HQ Camera |
| **GPhoto2** | DSLR, mirrorless cameras | See [supported cameras](http://www.gphoto.org/proj/libgphoto2/support.php) |

The application is distributed via Flathub. But interested people can compile the software by themselves. The photobooth application used a [Yocto Linux deployment](https://github.com/saeugetier/poky-photobooth/) in the past. This deployment is deprecated now, because the distribution via Flatpak is more convenient.

# Installation

## Flathub (Recommended)

The primary distribution method is via Flatpak. Flatpak can be installed on most x86_64 and aarch64 Linux distributions and includes all dependencies.

```bash
# Install from Flathub
flatpak install io.github.saeugetier.photobooth

# Run the application
flatpak run io.github.saeugetier.photobooth
```

Or launch via the desktop application menu.

## Raspberry Pi with libcamera

For Raspberry Pi Camera support, ensure your camera is properly configured:

1. Enable the camera in `/boot/firmware/config.txt`:
   ```
   # For Pi Camera Module 3
   dtoverlay=imx708
   
   # For Pi Camera Module 2
   dtoverlay=imx219
   
   # For Pi Camera Module 1
   dtoverlay=ov5647
   ```

2. Reboot and verify the camera is detected:
   ```bash
   libcamera-hello --list-cameras
   ```

3. Install and run the Flatpak as described above.

# Configuration

## Configuration File

The local configuration file is stored in:
```
~/.config/saeugetier/qtbooth.conf
```

The file contains all application settings including:

- Selected camera backend and device
- Neural network runtime settings
- Printer configuration
- PIN code for settings menu (default: `0815`)

## Template Files

The local template files for your collage images are stored in:
```
~/.local/share/saeugetier/qtbooth/
```

It contains:

- Background images for the image collages
- The `Collages.xml` describing all image collages
- Border images

Templates can be imported from USB storage. All files with extensions `xml`, `jpg`, `png`, `svg` in the folder `layout` will be copied to the local template folder.

For instructions on creating custom templates, see the [README](https://github.com/saeugetier/photobooth#how-to-create-own-templates).

# Basic Instructions

* Type of collage can be selected in the main menu.
* Settings menu can be opened in the main menu page via gears symbol. Password is "0815" if enabled.
* During snapshot (after selecting a collage type) a live preview is shown. The shutter button will trigger a countdown. AI background removal can be enabled via Snapshot Settings.
* A preview of the taken photo is displayed after snapshot. The photo can be confirmed or discarded. Effects can be applied to the photo via wizard menu.
* A preview of the collage is displayed as the next step.
* Next photo is taken afterwards.
* When all photos are taken, the collage can be printed out.
* Previous photos and collages can be printed again via the Gallery.

## Video

[![](https://markdown-videos-api.jorgenkh.no/youtube/yZxKuiFJEoE?width=640&height=360)](https://youtu.be/yZxKuiFJEoE)
(Link to Youtube)

# Technology

## Software

**Framework:** Qt 6.5 or higher - [https://qt.io](https://qt.io)

**Distribution:** Flatpak (primary), native builds supported

**Neural Network Runtime:** The application supports real-time background removal using neural networks. Available runtimes:

- ONNX: currently only CPU supported. Runs best on X86 CPU with more than 4 threads.
- NCNN: runs best on ARM platform (linited to 4 threads)

## Tested Hardware

**Platforms:**

- PC (x86_64)
- Raspberry Pi 3B, 4, 5 (aarch64). Raspberry Pi 5 for Neural Network recommended.

**Cameras:**

- USB Webcams (V4L2)
- Raspberry Pi Camera Module v1, v2, v3 (libcamera)
- Canon EOS 450D or newer (GPhoto2), Nikon cameras untested.

**Printers:**

- Canon Selphy Photo Printer CP910 over WiFi via [go-selphy-cp](https://github.com/saeugetier/go-selphy-cp)
- Standard inkjet/laser printers via CUPS

**Light/Flash:**
LED flash can be driven via Raspberry Pi GPIO. Tested configuration:
- LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html
- 20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

**Display:** A touchscreen connected via HDMI is highly recommended.

**I2C RTC:** If using a Raspberry Pi, it is recommended to use a realtime clock.

**Housing:** Plywood 8mm - Cutting via laser cutter. Template generated with http://festi.info/boxes.py/

My own housing is documented in a separate git repository: https://github.com/saeugetier/photobooth_hardware

## System Diagram

```
                                 .------------. Current .---------.
                                 | LED Driver |-------->| 20W LED |
                                 '------------'         '---------'
                                    ^
                                    |
                          ENABLE/PWM|          .-----------------------.
                                    |          |                       |
                                    |          |  Camera               |
                    .------------------.       |  - USB Webcam (V4L2)  |
                    |      Raspi       |<------|  - Pi Camera          |          -.-.-,~ .   
                    |    Application   |       |    (libcamera)        |          )     (     
                    |                  |       |  - DSLR (GPhoto2)     |          |_    |     
                    |     based on     |       '-----------------------'          /(_)---`\   
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
```



