# PixelMaestro Studio

![latest release](https://img.shields.io/github/release/8bitbuddhist/pixelmaestro-studio.svg) ![license](https://img.shields.io/github/license/8bitbuddhist/pixelmaestro-studio.svg)

PixelMaestro Studio is a desktop application for controlling LED displays, built on the [PixelMaestro library](https://github.com/8bitbuddhist/PixelMaestro). Using PixelMaestro Studio, you can design and preview custom animations, save and load presets, and control devices over USB.

## Usage

Download and run the latest version of PixelMaestro Studio from the [Releases](https://github.com/8bitbuddhist/PixelMaestro-Studio/releases) page. The [Wiki](https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki) has more information on PixelMaestro Studio's features and how to use them.

For information about the library itself, refer to the [PixelMaestro Wiki](https://github.com/8bitbuddhist/PixelMaestro/wiki).

## Building PixelMaestro Studio

### Build Requirements
- QT 5 with the QtSerialPort module
- [PixelMaestro core library](https://github.com/8bitbuddhist/PixelMaestro) (included as a submodule)

### Build Instructions (Linux)
1. Install Qt 5 with the QtSerialPort module
2. Clone the repository to a folder on your computer
3. Navigate to the newly created repository folder and use `git submodule init` and `git submodule update` to fetch the PixelMaestro core library.
4. Open the `PixelMaestro_Studio.pro` file in Qt Creator, or use Qmake to build the project.

## Credits

Icons from [Openclipart](https://openclipart.org/).

## Donate

If you find this project useful, cool, or just plain interesting, consider [making a donation](https://www.patreon.com/bePatron?c=1348704).
