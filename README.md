# PixelMaestro Studio

![latest release](https://img.shields.io/github/release/8bitbuddhist/pixelmaestro-studio.svg) ![license](https://img.shields.io/github/license/8bitbuddhist/pixelmaestro-studio.svg)

PixelMaestro Studio is a desktop application for controlling LED displays using the [PixelMaestro library](https://github.com/8bitbuddhist/PixelMaestro). Using PixelMaestro Studio, you can create custom LED animations, save and load presets, and control LEDs over USB.

![screenshot](screenshot.png)

## Usage

Download the latest version of PixelMaestro Studio from the [Releases](https://github.com/8bitbuddhist/PixelMaestro-Studio/releases) page. The [Wiki](https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki) contians information on how to use PixelMaestro Studio and its various features.

To learn more about the PixelMaestro library itself, visit the [PixelMaestro Wiki](https://github.com/8bitbuddhist/PixelMaestro/wiki) or [check out the repository](https://github.com/8bitbuddhist/PixelMaestro/).

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

Icons from [Openclipart](https://openclipart.org/).
