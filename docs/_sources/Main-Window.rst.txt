
Main Window
===========

The main window provides all of the interactive functionality of PixelMaestro Studio. Here you can interact with a Maestro, create Animations and Canvases, create Shows, and manage attached devices.

Layout
------

The main window is split into two sections: the *Maestro Viewer* and the *Maestro Editor*.

Maestro Viewer
^^^^^^^^^^^^^^

The top half of the main window contains the Maestro Viewer, which renders the output from the active Maestro. Each of the Maestro's Sections is displayed in order from left to right. The Section that currently has focus (called the *Active Section*) is shown with a white frame. If a Layer is the Active Section, then the Section is shown with a light gray frame. Inactive sections have no frame.

.. Note:: The Maestro Viewer may not be visible if *Main Window* is not selected as an output in the :doc:`Preferences <Preferences>` screen.

Maestro Editor
^^^^^^^^^^^^^^

The bottom half of the main window contains the Maestro Editor, which provides controls for interacting with the Maestro and performing other actions. It's split into five tabs:


* :doc:`Section Tab <Section-Tab>`
* :doc:`Animation Tab <Animation-Tab>`
* :doc:`Canvas Tab <Canvas-Tab>`
* :doc:`Show Tab <Show-Tab>`
* :doc:`Device Tab <Device-Tab>`

Control Buttons
~~~~~~~~~~~~~~~

At the top of the Maestro Editor are four buttons:

The *Lock Button* prevents any actions you perform from modifying the Maestro. While the Maestro is locked, a padlock icon appears next to each tab and group of controls that is locked. Certain Maestro-level actions, such as enabling or disabling a :doc:`Show <Show-Tab>`, will remain unlocked. Note that performing a locked action will still generate an :doc:`event <Show-Tab>`.

The *Play/Pause Button* controls the playback of the Maestro. It will start/stop any Show timers, Animations, Canvas animations, and communication with USB devices.

The *Sync Button* synchronizes any and all timer-based components including Animations, Canvas animations, and Shows.

The *Refresh Button* refreshes the Maestro Editor based on the current Maestro configuration. This is used to synchronize the UI with the Maestro when the Maestro is changed by a non-user initiated action (e.g. a Show). If an action causes the UI to go out of sync, this button appears highlighted.