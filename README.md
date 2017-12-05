# PixelMaestro Studio
PixelMaestro Studio is a Qt-based desktop application that lets you run the PixelMaestro library on a PC. PixelMaestro Studio lets you run demos, mix custom animations, save and load configurations, and control other devices running PixelMaestro over USB.

## Build Requirements
- QT 5 with the QtSerialPort module
- [PixelMaestro core library](https://github.com/Anewman2/PixelMaestro) (included as submodule)

## Build Instructions
1. Install Qt 5 with the QtSerialPort module
2. Clone the repository to a folder on your computer
3. Navigate to the newly created repository folder and use `git submodule init` and `git submodule update` to fetch the PixelMaestro core library.
4. Open the `PixelMaestro_Studio.pro` file in Qt Creator, or use Qmake to build the project.

[![Support this project on Patreon](https://c5.patreon.com/external/logo/become_a_patron_button@2x.png)](https://www.patreon.com/bePatron?u=8547028)
