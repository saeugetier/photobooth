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
LED flash can be driven via GPIO on any supported ARM SBC using **libgpiod**. See [GPIO Configuration](#gpio-configuration) for setup details. Tested configuration:
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

## GPIO Configuration

The photobooth supports controlling LED lighting via GPIO pins using **libgpiod** — the standard Linux GPIO character device interface. This works on any ARM SBC with a `/dev/gpiochip*` device, including Raspberry Pi and Rockchip-based boards.

GPIO is **disabled by default** and must be enabled in **Settings → GPIO**.

### Overview

Two GPIO lines are used:

| Function | Mode | Description |
|----------|------|-------------|
| **LED Enable** | Digital Output | Turns the LED driver on/off (HIGH = on, LOW = off) |
| **LED Brightness** | Software PWM | Controls LED brightness via pulse-width modulation (0–100%) |

The software PWM runs in a dedicated thread for consistent timing, independent of the UI rendering.

### Board Presets & Default Pin Assignments

The settings menu provides board presets that auto-configure the correct GPIO chip and line numbers:

| Board | GPIO Chip | LED Enable Line | LED Brightness Line | Physical Header Pins |
|-------|-----------|----------------|---------------------|---------------------|
| **Raspberry Pi 3/4** | `/dev/gpiochip0` | 23 | 18 | Enable: Pin 16 (GPIO23), Brightness: Pin 12 (GPIO18) |
| **Raspberry Pi 5** | `/dev/gpiochip4` | 23 | 18 | Enable: Pin 16 (GPIO23), Brightness: Pin 12 (GPIO18) |
| **Orange Pi 3B (RK3566)** | `/dev/gpiochip3` | 13 (GPIO3_B5) | 14 (GPIO3_B6) | Consult board pinout diagram |
| **Custom** | User-selectable | User-selectable | User-selectable | — |

> **Note:** The Orange Pi 3B default lines (13, 14 on gpiochip3) are examples. Consult your specific board's pinout documentation and adjust in the Settings menu. RK3566 GPIO numbering uses bank notation: e.g. GPIO3_A5 = line offset `3*8+5 = 29` on the corresponding gpiochip. The settings menu enumerates all available lines with their kernel names for easy identification.

### Wiring

```
        GPIO (Enable Line)          GPIO (Brightness Line / PWM)
             │                              │
             ▼                              ▼
        ┌─────────┐                   ┌──────────┐
        │ ENABLE   │                   │ PWM/DIM  │
        │          │                   │          │
        │  LED     ├───────────────────┤  LED     │
        │  Driver  │                   │  Driver  │
        │          │                   │          │
        └────┬─────┘                   └──────────┘
             │
             ▼
        ┌─────────┐
        │  LED    │
        │ (20W)   │
        └─────────┘
```

The **Enable** line turns the LED driver on/off. The **Brightness** line controls the duty cycle via software PWM. If your LED driver uses active-low logic (brightness increases as PWM decreases), enable the **"Invert PWM"** toggle in the GPIO settings.

### Permissions / udev Setup

GPIO character devices (`/dev/gpiochip*`) require appropriate permissions. The project ships udev rules and an installer script in the `udev/` directory.

#### Automatic Installation

```bash
cd udev/
sudo ./install-gpio-rules.sh
```

The script will:
1. Auto-detect your board type from `/proc/device-tree/model`
2. Install the matching udev rules to `/etc/udev/rules.d/99-gpio-photobooth.rules`
3. Create a `gpio` group (if it doesn't exist)
4. Add your user to the `gpio` group
5. Reload udev rules

**You must log out and back in (or reboot) for the group change to take effect.**

#### Manual Installation

```bash
# Copy the appropriate rules file
sudo cp udev/99-gpio-raspberrypi.rules /etc/udev/rules.d/99-gpio-photobooth.rules

# Create gpio group and add your user
sudo groupadd -f gpio
sudo usermod -aG gpio $USER

# Reload udev
sudo udevadm control --reload-rules && sudo udevadm trigger

# Log out and back in, then verify
ls -la /dev/gpiochip*
```

#### Flatpak

When running as a Flatpak, the `--device=all` permission grants access to `/dev/gpiochip*` inside the sandbox. You still need the udev rules on the **host** system for correct group permissions.

### Settings Menu

In the application, go to **Settings → GPIO** tab:

1. **Enable GPIO** — Master switch (off by default). All other controls are hidden until enabled.
2. **Board Preset** — Select your board to auto-fill chip and line defaults. Select "Custom" to manually configure.
3. **GPIO Chip** — The character device (e.g. `/dev/gpiochip0`). Only editable in "Custom" mode.
4. **LED Enable Line** — GPIO line offset for the on/off enable signal. Shows all available lines with kernel names.
5. **LED Brightness Line** — GPIO line offset for the PWM brightness signal.
6. **PWM Frequency (Hz)** — Software PWM frequency (default: 1000 Hz). Higher values give smoother dimming but increase CPU usage. 500–2000 Hz is typical for LED drivers.
7. **Invert PWM** — When enabled, inverts the duty cycle (1.0 - brightness). Enable this for active-low LED drivers.

## Neural Network Background Removal

The application supports real-time background removal using neural networks. Available runtimes:
- ONNX: currently only CPU supported. Runs best on X86 CPU with more than 4 threads.
- NCNN: runs best on ARM platform (limited to 4 threads)

Configure the neural network runtime in the settings menu.

## Issue Reporting

Please use the [issue tracker](https://github.com/saeugetier/photobooth/issues) for bug reporting and feature requests.

## License

See [LICENSE](LICENSE) for details.
