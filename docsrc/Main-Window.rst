
Main Window
===========

The main window contains all of the controls necessary for interacting with PixelMaestro. Here you can edit the Maestro, create Animations and Canvases, create Shows, and connect devices.

Layout
------

The main window is split into two sections: the *Maestro Viewer*\ , and the *Maestro Editor*.

Maestro Viewer
^^^^^^^^^^^^^^

The top half of the main window renders the output of the Maestro. Each Section is displayed in order from left to right. The Section that currently has focus (the *Active Section*\ ) is displayed with a white border. If a Layer is the Active Section, then the Section is displayed with a light gray border. Inactive sections have no border.

**Note**\ : The Maestro Viewer may not be visible if *Main Window* is not selected as an output in the `Preferences <Preferences.html>` screen.

Maestro Editor
^^^^^^^^^^^^^^

The Maestro Editor is the bottom half of the window. It provides controls for editing the Maestro. It's divided into five tabs:


* `Section Tab <Section-Tab.html>`
* `Animation Tab <Animation-Tab.html>`
* `Canvas Tab <Canvas-Tab.html>`
* `Show Tab <Show-Tab.html>`
* `Device Tab <Device-Tab.html>`

Control Buttons
~~~~~~~~~~~~~~~

At the top of the Maestro Editor are four buttons:

The *Lock Button* prevents any actions you perform from modifying the Maestro. However, it does not prevent actions from generating `events <Show-Tab.html>`. While the Maestro is locked, a padlock icon appears next to each tab and group of controls that is locked. Certain Maestro-level actions, such as enabling or disabling a `Show <Show-Tab.html>`\ , will remain unlocked.

The *Play/Pause Button* lets you start or stop playback of the Maestro at any time. This also stops any Show timers, Animations, and communications with USB devices.

The *Sync Button* resets any and all timers. This synchronizes Animations, Shows, and other timer-based components.

The *Refresh Button* updates all controls with the current Maestro configuration. This is used to synchronize the UI with the Maestro when the Maestro is changed by another process (e.g. a Show). If another process changes the Maestro and the UI is left out of sync, this button appears highlighted.
