# PixelMaestro Studio

![latest release](https://img.shields.io/github/release/8bitbuddhist/pixelmaestro-studio.svg) ![license](https://img.shields.io/github/license/8bitbuddhist/pixelmaestro-studio.svg)

PixelMaestro Studio is a desktop application for controlling LED displays using the [PixelMaestro library](https://github.com/8bitbuddhist/PixelMaestro). Using PixelMaestro Studio, you can design and preview custom animations, save and load presets, and control devices over USB.

## Usage

Download and run the latest version of PixelMaestro Studio from the [Releases](https://github.com/8bitbuddhist/PixelMaestro-Studio/releases) page. The [Wiki](https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki) has more information on PixelMaestro Studio's features and how to use them.

For information about the PixelMaestro library, visit the [PixelMaestro Wiki](https://github.com/8bitbuddhist/PixelMaestro/wiki) or go straight to the [repository](https://github.com/8bitbuddhist/PixelMaestro/).

## Running PixelMaestro Studio

### Windows

Download and run `PixelMaestro_Studio.exe`.

### Linux

Download and run `PixelMaestro_Studio`.

## Building PixelMaestro Studio

### Build Requirements
- QT 5.11.2 or higher with the QtSerialPort module
- [PixelMaestro core library](https://github.com/8bitbuddhist/PixelMaestro) (included as a submodule)

### Build Instructions (Linux)
1. [Download and install Qt 5.11.2 or higher](https://www.qt.io/download). Include the QtSerialPort module
2. Clone the GitHub repository to a folder on your computer by running `git clone https://github.com/8bitbuddhist/PixelMaestro-Studio.git`
3. Navigate to the newly created repository folder and run `git submodule init` and `git submodule update` to download the PixelMaestro core library.
4. Launch the Qt Creator application and open the `PixelMaestro_Studio.pro` file, or use Qmake to build the project directly.

## Credits

Icons from [Openclipart](https://openclipart.org/).
