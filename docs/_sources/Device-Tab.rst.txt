
Device Tab
==========

The Device Tab lets you connect devices to PixelMaestro Studio via USB. This allows you to send commands to devices that interact directly with LED hardware, such as an Arduino. You can connect multiple devices and control them in real-time, or send your entire Maestro configuration to a device.


.. image:: images/device-tab.png
   :target: images/device-tab.png
   :alt: Device tab

Your device must be running software capable of receiving and interpreting serial commands from PixelMaestro Studio. The PixelMaestro library includes :pmarduino:`several Arduino sketches <>` that demonstrate this.

Adding a Device
---------------

Before you can connect to a device, you first need to add it. Click the *Add* button to open the new device dialog. Select the port that the device is connected to from the drop-down menu (or enter it manually). Check *Auto-connect* to automatically connect to the device when PixelMaestro Studio starts. Live Updates are explained in more detail in the next section. Click *Ok* to save your device and add it to the Device List.

Enabling Live Updates
^^^^^^^^^^^^^^^^^^^^^

With Live Updates enabled, any actions you perform in PixelMaestro Studio are automatically mirrored to a connected device in real-time. For an example, see the :pmarduino:`USB Arduino sketch <USB_Live>` included in the PixelMaestro library.

Check the *Live Updates* box to enable live updates. This also enables the Map Sections button, which is explained in the next section.

Mapping Sections
^^^^^^^^^^^^^^^^

The number of Sections you manage in PixelMaestro Studio will likely be different than the number of Sections managed on your device. Because Sections are identified by their index, this means that most commands generated in PixelMaestro Studio won't work as expected on your device.

Section maps fix this by letting you map one Section index to another. For example, you might want to run commands for Section 2 on a device that only has one Section (Section 0). You can use a Section map to map Section 2 to Section 0. The next time you perform an action, PixelMaestro Studio automatically translates the command's Section index before sending it to the device.

To create a map, click the *Map Sections...* button. This opens a dialog box showing a table with two columns:


* The ``Local Section`` column is the index of each Section in PixelMaestro Studio.
* The ``Remote Section`` column is the index of the target Section on the remote device.

In the following image, commands that run on Sections 3 and 4 (indices 2 and 3) in PixelMaestro Studio will run on Sections 1 and 2 (indices 0 and 1) on the remote device. Setting the remote Section to -1 prevents the command from being uploaded to the remote device.


.. image:: images/section-map-dialog.png
   :target: images/section-map-dialog.png
   :alt: Section Map Dialog


By default, Sections are mapped directly to each other. You can change the ``Remote Section`` column to any numeric value. If the device doesn't contain a Section with that ID, then the Cue simply won't run. Click *OK* to save your changes, or click *Reset* to revert back to the default mapping.

Connecting to a Device
----------------------

Once a device has been added, it appears in the device list. Disconnected devices appear gray, while connected devices appear white. Select a device and click *Connect* to open a serial connection to it. Connecting to a device will also display a connection icon next to the tab name. If the device has live updates enabled, will automatically receive commands as you perform actions in PixelMaestro Studio.

When you are done, click *Disconnect* to close the connection to a device. Devices are also disconnected automatically when PixelMaestro Studio closes.

Uploading Cuefiles
------------------

Live updates only send individual events to your devices. If you want to send your entire Maestro configuration (called the Cuefile), use the *Upload* button.

First, select a device in the Device List. The *Size* text box shows the current size of the Cuefile in bytes. You should check this value against your device's RAM capacity, as well as the size of its CueController buffer. If the Cuefile is larger than either of those, then the Cuefile might not transfer properly.

Click *Upload* to send the Cuefile to your device. The progress bar shows how much of the Cue has been uploaded. You can see the contents of the Cuefile by clicking the *Preview* button.

The Cuefile contains every instruction needed to reproduce your configuration. As such, you can store the Cuefile on your device and restore it when the device boots. On an Arduino, you can do this by writing the Cuefile to EEPROM as shown in :pmarduino:`this example sketch <USB_EEPROM>`. If your device has a storage device like an SD card, you can copy any ``.PMC`` file created in PixelMaestro Studio to the card and read it on the device.
