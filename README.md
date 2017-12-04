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

## Contents
1. [Main Window](#main-window)
	1. [Navigating Workspaces](#navigating-workspaces)
	2. [Changing Settings](#changing-settings)
2. [Using the Animation Previewer](#animation-previewer)
	1. [Additional Animation Parameters](#additional-animation-parameters)
	2. [Custom Color Schemes](#custom-color-schemes)
	3. [Using Canvases](#using-canvases)
	4. [Saving and Loading Configurations](#saving-and-loading-configurations)

## Main Window
When first started, the application shows a blank window. Use the *File* menu to access different areas of the program such as the *Animation Editor* or various demos. Use the *Edit* menu to access the program's Settings. Use the *Help* menu to access documentation.

### Navigating Workspaces
Opening a link in the *File* menu creates a new *Workspace*. A Workspace is any screen that shows a running Maestro, such as a demo. After opening a Workspace, you can see details about the Workspace in the status bar at the bottom of the window. To close the current workspace, click *Close* in the File menu, or select another Workspace.

### Changing Settings
You can change the program's settings by clicking *Edit* > *Settings*.

#### Rendering Options
*Rendering Options* control how the Maestro is rendered to the screen. The *Pixel Size* and *Pixel Shape* options define the appearance of each Pixel on the screen. The *Refresh Rate* defines how quickly the Maestro updates. This may impact how smooth certain animations appear.

**Tip:** For best performance, set the Pixel Size to *Full* and the Pixel Shape to *Square*.

#### Animation Editor Options
*Animation Editor Options* control options specific to the Animation Editor. The *Number of Sections* option sets the number of Sections assigned to the Maestro when opening the Animation Editor.

The *Output devices* list is where you select the output devices that are controlled by the Animation Editor. Two options are available by default:

* *Main window* displays the Maestro above the Animation Editor controls.
* *Detached window* displays the Maestro in a new window separate from the Animation Editor.

This list also displays devices connected via USB. You can select any USB device currently attached to your PC. Note that the device must be actively listening for PixelMaestro Cues.

You may need to reopen any open Workspaces before setting changes will take effect.

## Using the Animation Editor
The Animation Editor is an interactive tool for modifying a Maestro in real-time. When you open the Animation Editor, it displays a set of controls for modifying Sections. You can perform actions such as adjusting the Section's size, configuring Animations, and adding Canvases.

To modify a Section, you must first set it as the *Active Section*. Despite its name, the Active Section is determined by both the *Section* and *Layout* drop-down boxes. That is, if Section 1 and Layout 3 are selected, then the active Section is Section 1's third Layout. If the Layout box is set to "Base Section", then Section 1 becomes the active Section.

The output of the Animation Editor depends on your configuration in the Settings dialog. The Animation Editor will update any devices checked in the *Output devices* list, including connected USB devices. Checking *Main window* or *Detached window* opens a visual representation of the Maestro on your PC. If no devices are selected, you won't be able to see any changes to your Maestro.

### Additional Animation Parameters
TODO

### Custom Color Schemes
TODO

### Using Canvases
TODO

### Saving and Loading Configurations
You can save the current Maestro configuration to a file by clicking *File* > *Save...*. Likewise, you can open a Maestro configuration using *File* > *Open...*. This file works on any device running PixelMaestro.

[![Support this project on Patreon](https://c5.patreon.com/external/logo/become_a_patron_button@2x.png)](https://www.patreon.com/bePatron?u=8547028)
