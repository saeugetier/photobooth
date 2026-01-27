# Photobooth

## Overview

The goal of the project is to provide a configurable photobooth software for Raspberry Pi or a PC.

Main features are:
- User Interface with touchscreen input (input via mouse is not recommended). Control via buttons may be added in future.
- Photo preview and capture via multiple camera backends:
  - **V4L2** - USB Webcams and other Video4Linux2 compatible cameras
  - **libcamera** - Raspberry Pi Camera Modules (Pi Camera v1, v2, v3, HQ Camera)
  - **GPhoto2** - DSLR and mirrorless cameras connected via USB
- **Neural Network background removal** - Remove or replace photo backgrounds in real-time
- Printout with Canon Selphy photo printer or standard printer via CUPS
- Configurable image collages with own templates. User can select current template in application.
- Password protected settings menu:
  - Copy all photos to USB storage (currently not supported via Flatpak)
  - Copy collage templates from USB storage (currently not supported via Flatpak)
  - Disable printing
- Brightness of LED preview and flash lights can be adjusted

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

## Technology

### Software
- **Framework:** Qt 6.5 or higher - https://qt.io
- **Distribution:** Flatpak (primary), native builds supported
- **Neural Network Runtime:** ONNX Runtime or NCNN for background removal

### Camera Support

| Backend | Camera Types | Notes |
|---------|-------------|-------|
| V4L2 | USB Webcams, capture cards | Standard Linux video interface |
| libcamera | Raspberry Pi Camera Modules | Pi Camera v1/v2/v3, HQ Camera |
| GPhoto2 | DSLR, mirrorless cameras | See [supported cameras](http://www.gphoto.org/proj/libgphoto2/support.php) |

### Tested Hardware

**Platforms:**
- PC (x86_64)
- Raspberry Pi 3B, 4, 5 (aarch64)

**Cameras:**
- USB Webcams (V4L2)
- Raspberry Pi Camera Module v2, v3 (libcamera)
- Canon EOS 450D (GPhoto2)

**Printers:**
- Canon Selphy Photo Printer CP910 over WiFi via https://github.com/saeugetier/go-selphy-cp
- Standard inkjet/laser printers via CUPS
- Fake printer implementation for testing
- Printing can also be disabled

**Light/Flash:**
LED flash can be driven via Raspberry Pi GPIO. Tested configuration:
- LED Driver: https://www.aliexpress.com/item/14-37-Inch-LED-LCD-Universal-TV-Backlight-Constant-Current-Board-Driver-Boost-Structure-Step-Up/32834942970.html
- 20W LED: https://www.aliexpress.com/item/1Pcs-High-Power-10W-20W-30W-50W-100W-COB-Integrated-LED-Lamp-Chip-SMD-Bead-DC/32822371892.html

**Display:** A touchscreen connected via HDMI is highly recommended.

**I2C RTC:** If using a Raspberry Pi, it is recommended to use a realtime clock for timestamping the photos.

### Housing / Electronics

My own housing is documented in a separate git repository: https://github.com/saeugetier/photobooth_hardware

Housing: Plywood 8mm - Cutting via laser cutter. Template generated with http://festi.info/boxes.py/

## Installation

### Flathub (Recommended)

The primary distribution method is via Flatpak. Flatpak can be installed on most x86_64 and aarch64 Linux distributions and includes all dependencies.

```bash
# Install from Flathub
flatpak install io.github.saeugetier.photobooth

# Run the application
flatpak run io.github.saeugetier.photobooth
```

Or launch via the desktop application menu.

### Raspberry Pi with libcamera

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

### Building from Source

#### Requirements
- Qt 6.5 or higher development packages
- OpenCV
- For GPhoto2 support: libgphoto2-dev
- For libcamera support: libcamera-dev
- For neural network: ONNX Runtime, NCNN

#### Build Steps

```bash
git clone https://github.com/saeugetier/photobooth.git
cd photobooth
mkdir build && cd build
cmake ..
make -j$(nproc)
./qtbooth
```

## Configuration

### Configuration File

The local configuration file is stored in:
```
~/.config/saeugetier/qtbooth.conf
```

The file contains all application settings including:
- Selected camera backend and device
- Neural network runtime settings
- Printer configuration
- PIN code for settings menu (default: `0815`)

### Template Files

The local template files for your collage images are stored in:
```
~/.local/share/saeugetier/qtbooth/
```

It contains:
- Background images for the image collages
- The `Collages.xml` describing all image collages
- Border images

Templates can be imported from USB storage. All files with extensions `xml`, `jpg`, `png`, `svg` in the folder `layout` will be copied to the local template folder.

### How to Create Own Templates

Create your own `Collages.xml` file. You can use the existing file as a reference.

The root node of the XML is named `catalog`. It contains nodes for the templates for collages named `collage`. Each collage must contain at least one `image`, a `name`, a `background`, a `foreground` and an `icon`.

You can use built-in backgrounds like `WhiteBackground.png` or create your custom one. The foreground will be painted in the front layer, so it is highly recommended to have an alpha channel and some cutouts for your photos.

Images require position and size information. The range of values for position and size is between `0.0` and `1.0`. It is possible to define an image border for each image.

Optional properties:
- `printable` - Set to `false` to create a non-printable collage (single image mode)

#### Example `Collages.xml`:

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
            <image>
                <position x="0.0" y="0.0"/>
                <size width="1.0" height="1.0"/>
            </image>
        </images>
    </collage>
    <collage>
        <name>Four Images Border</name>
        <icon>FourBorder.svg</icon>
        <background>StarsBackground.jpg</background>
        <images>
            <image>
                <position x="0.0" y="0.0"/>
                <size width="0.5" height="0.5"/>
                <border>
                    <file>RedBorder.png</file>
                    <margin top="10" left="10" right="10" bottom="10"/>
                </border>
            </image>
            <image>
                <position x="0.5" y="0.0"/>
                <size width="0.5" height="0.5"/>
                <border>
                    <file>RedBorder.png</file>
                    <margin top="30" left="30" right="30" bottom="30"/>
                </border>
            </image>
            <image>
                <position x="0.0" y="0.5"/>
                <size width="0.5" height="0.5"/>
                <border>
                    <file>RedBorder.png</file>
                    <margin top="20" left="20" right="20" bottom="20"/>
                </border>
            </image>
            <image>
                <position x="0.5" y="0.5"/>
                <size width="0.5" height="0.5"/>
                <border>
                    <file>RedBorder.png</file>
                    <margin top="50" left="50" right="50" bottom="50"/>
                </border>
            </image>
        </images>
    </collage>
</catalog>
```

## Neural Network Background Removal

The application supports real-time background removal using neural networks. Available runtimes:
- ONNX: currently only CPU supported. Runs best on X86 CPU with more than 4 threads.
- NCNN: runs best on ARM platform (limited to 4 threads)

Configure the neural network runtime in the settings menu.

## Issue Reporting

Please use the [issue tracker](https://github.com/saeugetier/photobooth/issues) for bug reporting and feature requests.

## License

See [LICENSE](LICENSE) for details.
