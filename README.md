# PixelMaestro Studio

![latest release](https://img.shields.io/github/release/8bitbuddhist/pixelmaestro-studio.svg) ![license](https://img.shields.io/github/license/8bitbuddhist/pixelmaestro-studio.svg)

PixelMaestro Studio is a desktop application for controlling LED displays using the [PixelMaestro library](https://github.com/8bitbuddhist/PixelMaestro). Using PixelMaestro Studio, you can create custom LED animations, save and load presets, and control LEDs over USB.

![screenshot](screenshot.png)

## Features

- Design custom animations and patterns using a fully interactive, real-time, visual editing tool
- Control up to 256 independent, multi-layered drawing surfaces simultaneously
- Create beautiful, 24-bit color palettes with support for over 250 colors per palette
- Use powerful image editing tools to design your own still images and animations
- Set dynamic time-based animations and transitions using a powerful yet intuitive event scheduling system
- Upload your changes in real-time to devices connected via USB or serial port
- Save, load, and share your custom animations to and from your computer and other devices

## Usage

Download the latest version of PixelMaestro Studio from the [Releases](https://github.com/8bitbuddhist/PixelMaestro-Studio/releases) page. The [Wiki](https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki) contians information on how to use PixelMaestro Studio and its various features.

To learn more about the PixelMaestro library itself, [visit the repository](https://github.com/8bitbuddhist/PixelMaestro/) or check out the [official Wiki](https://github.com/8bitbuddhist/PixelMaestro/wiki).

## Running PixelMaestro Studio

### Windows

Download and run `PixelMaestro_Studio.exe`.

### Linux

Download `PixelMaestro_Studio`, mark it as executable, then run it.

```bash
$ wget https://github.com/8bitbuddhist/PixelMaestro-Studio/releases/download/{version tag}/PixelMaestro_Studio
$ chmod +x PixelMaestro_Studio
$ ./PixelMaestro_Studio
```

## Building PixelMaestro Studio

### Build Requirements
- QT 5.11.2 or higher with the QtSerialPort module
- [PixelMaestro core library](https://github.com/8bitbuddhist/PixelMaestro) (included as a submodule)

### Build Instructions (Linux)
1. [Download and install Qt 5.11.2 or higher](https://www.qt.io/download) along with the QtSerialPort module.
2. Clone the GitHub repository to your computer:
	- `git clone https://github.com/8bitbuddhist/PixelMaestro-Studio.git`
3. Navigate to the newly created repository folder and use `git submodule` to download the PixelMaestro core library:
	```bash
	$ cd PixelMaestro-Studio
	$ git submodule init
	$ git submodule update
	```
4. Use `qmake` to build the project, or if you have the Qt Creator application installed, open the `PixelMaestro_Studio.pro` file.
	```bash
	qmake PixelMaestro-Studio.pro && make qmake_all
	```

## Credits

Icons based on the [Monochrome icon set by Danny Allen](https://store.kde.org/p/1002558), retrieved from [Openclipart](https://openclipart.org/).
