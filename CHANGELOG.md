# Changelog
All notable changes to PixelMaestro Studio will be documented in this file.

The format is loosely based on [Keep a Changelog](http://keepachangelog.com/).

## [v0.30.1] - In Progress
### Added
- Added ability to merge a Cuefile into the current Maestro instead of replacing it.
- Added ability to resize sections of the interface.
- Added shortcut keys for Save, New, Open, etc.
- Added cursor when free drawing on a Canvas.

### Changed
- Renamed several file menu options to be more clear about their actions.
- Added tooltips to several menu options.
- Renamed `Cue Interpreter` to `Cue Previewer`.
- Updated Qt version to 5.11.2.
- Created static build for Linux.
- Fixed memory leak when loading images into a Canvas.
- Fixed custom Palettes from saving and loading when session saving was disabled.

### Removed
- Removed donation link.
- Removed Pixel Size option from Preferences dialog.

## [v0.30] - 2018-04-18
WARNING: This update breaks backwards compatability with v0.20.1 and earlier Cuefiles.

### Added
- Animation Editor
	- Added Device tab for controlling USB devices. This replaces the Serial section of the Preferences dialog.
	- Added ability to upload an entire Maestro configuration (a `Cuefile`) to a USB device.
	- Added ability to toggle live updating of USB devices.
	- Added ability to unset Animations.
	- Added ability to edit Palettes.
	- Palettes are now saved between sessions.
	- Added dialog window for previewing Cuefiles.
- Preferences
	- Added setting to toggle session auto save/load when closing/opening the application. This is enabled by default.

### Changed
- Animation Editor
	- Fixed MaestroController not properly saving Section scroll settings.
	- Palette list is now automatically saved and reopened when closing/opening PixelMaestro Studio.
	- Consolidated all three Canvas types into the base Canvas class. This new class works like a PaletteCanvas with support for transparency.
	- Replaced the "Canvas Type" drop-down with a check box.
- Preferences
	- Moved USB device settings to Animation Editor.
- File selector dialog now remembers the last directory used.
- Lowered minimum Qt version to 5.9.1 for compatibility with Ubuntu 18.04 repository Qt version.
- Code cleanup.

## [v0.20.1] - 2018-03-20

### Changed
- Fixed an issue where an extra non-existent Section was listed in the Animation Editor dropdown.
- Fixed active Section identification and highlighting.
- Fixed mirror checkbox not being checked when switching to a Section with a mirrored Wave animation.

## [v0.20] - 2018-03-17
WARNING: This update breaks backwards compatability with v0.12 and earlier Cuefiles.

### Added
- Added ability to set the Canvas cursor by clicking on a pixel.
- Added Canvas drawing tool that lets you draw directly onto a Section.
- Added playback buttons to the Canvas and Show tabs.
- Added fire animation.
- Added `skew` and `mirror` parameters to Wave animation.
- Added ability to reorder Show Events.
- Added a border to Sections. The active Section/Layer has a lighter border, while all other Sections have a dark border.
	
### Changed
- _Many_ UI changes and bugfixes.
- Fixed Maestro-level properties not updating when loading a Maestro from a Cuefile.
- Enabled changing the grid size while a Layer is active.
- Enabled changing per-axis offset values while scrolling is disabled on that axis.
- Merged Lightning animation's two separate drift controls into a single control.
- Changed Preferences dialog layout.
	
### Removed
- Removed the Merge animation. Use `WaveAnimation::set_mirror(true)` to recreate the merge effect.
- Removed standalone demos. To see a demo, open one of the example Cuefiles from the `examples` folder.

## [v0.12] - 2018-01-22
### Changed
- Animation Editor:
	- Loading an image into a PaletteCanvas now adds the palette to the palette drop-down.
- MaestroController: Changed default refresh rate to 50ms.
- SimpleDrawingAreaDialog: Added fullscreen toggle.
- Renamed `Settings` to `Preferences`.

## [v0.11] - 2017-12-23
### Added
- Moved Maestro timing to the MaestroController. This should reduce resource usage and keep screen outputs from falling out of sync with each other.
- Animation Editor
	- Added support for `PaletteCanvases`.
	- Added a pause/resume button to the Show tab. This lets you stop and start the Maestro at any time.
	- Added a relative time box to the Show tab. This shows the time elapsed since the last event that was ran.
- Demos
	- Added sample Cue files for each demo. You can load these files in the Animation Editor.
- Settings
	- Added the ability to pause the Maestro when the Animation Editor first opens.

### Changed
- Moved Cuefile processing and Maestro initialization into MaestroController.
- Animation Editor
	- Show tab: Renamed "Current Time" to "Absolute Time".

### Changed
- Animation Editor: Removed alert when a saved USB device is no longer connected. The device now appears greyed out on the Settings page.

## [v0.10] - 2017-12-05
### Added
- Added donate link to Help menu.
- Added ability to configure multiple output devices in settings.
- Animation Editor
	- Added checkbox to enable repeated Canvas scrolling.
	- Added checkbox for looping Show Events.
	- Added empty space between Sections.

### Changed
- Moved project into Beta state.
- Modified Show Event management UI
	- Added ability to remove Events from Show Event list.
	- Added timing method combo box.
	- Simplified Event History selection.
	- Events that have already run will now appear grayed out.

### Removed
- Removed device simulator.
- Removed Animated Canvas demo.

## [v0.9] - 2017-11-28
### Added
- Added frame controls to Canvas editing tools.
- Added Canvas Edit mode, which stops the Canvas from animating so you can draw on individual frames.
- Added Show Edit mode, which lets you generate Events without changing the Maestro itself.

### Changed
- Redesigned Animation Editor UI. Controls are now grouped into tabs for each component: Section, Animation, Canvas, and Show.
- Show controls are now fully functional.
