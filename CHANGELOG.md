# Changelog
All notable changes to PixelMaestro Studio will be documented in this file.

The format is loosely based on [Keep a Changelog](http://keepachangelog.com/).

## [v0.13] - In Progress
### Added
- Animation Editor
	- Added support for the Fire animation introduced in PixelMaestro 0.13.
	- Added `skew` parameters for Merge and Wave animations.
	- Readded ability to reorder Show Events.

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
