
Device Tab
==========

The Device Tab lets you connect PixelMaestro Studio to devices over serial, USB, or network. This lets you send commands to devices that interact directly with LED hardware, such as an Arduino.

.. image:: images/device-tab.png
   :target: images/device-tab.png
   :alt: Device tab

Your device must be running a sketch capable of receiving and interpreting serial commands from PixelMaestro Studio. You can find samples in the :pmarduino:`PixelMaestro Arduino folder <>`.

Adding a Device
---------------

To add a new device, click the *Add* button. This opens the new device dialog. If connecting to a serial device, you can use the drop-down to view a list of serial devices connected to your PC. You can also enter the location manually.

If connecting to a network device, enter the IP address (and optionally, the port number) of the device using the format [IP]:[Port].

.. Note:: PixelMaestro Studio defaults to port 8077.

If you want to automatically connect to the device when PixelMaestro Studio loads, check *Auto-connect*. The Live Updates option is explained in more detail in the next section.

Click *Ok* to save your device and add it to the Device List.

Enabling Live Updates
^^^^^^^^^^^^^^^^^^^^^

With Live Updates enabled, any actions you perform in PixelMaestro Studio are automatically sent to connected devices in real-time. Check the *Live Updates* box when adding or editing a device to enable live updates. This also enables the Map Sections button, which is explained in the next section.

For an example of a sketch that allows a device to receive live updates, see the :pmarduino:`USB Arduino sketch <USB_Live>` included in the PixelMaestro library.

Mapping Sections
^^^^^^^^^^^^^^^^

Section mapping allows you to run commands meant for one Section on another Section.

For projects with multiple devices, the number of Sections you manage in PixelMaestro Studio may be different than the number of Sections managed by any one device. Because Sections are identified by their index, this means that by default commands sent by PixelMaestro Studio won't run properly on your devices.

Section mapping fixes this by letting you change the index in real-time. For example, you might want to run commands for Section 2 on a device that only has one Section, which has an index of 0. You can use a Section map to map Section 2 to Section 0. The next time you perform an action, PixelMaestro Studio automatically translates the command's Section index from 2 to 0 before sending it to the device.

To create a map, click the *Map Sections...* button. This opens a dialog box showing a table with two columns:

* The ``Local Section`` column is the index of each Section in PixelMaestro Studio.
* The ``Remote Section`` column is the index of the target Section on the remote device.

In the following image, commands that run on Sections 3 and 4 (indices 2 and 3) in PixelMaestro Studio will run on Sections 1 and 2 (indices 0 and 1) on the remote device. Setting the remote Section to -1 prevents the command from being uploaded to the remote device.


.. image:: images/section-map-dialog.png
   :target: images/section-map-dialog.png
   :alt: Section Map Dialog


By default, Sections are mapped directly to each other. You can change the ``Remote Section`` column to any numeric value. If the device doesn't contain a Section with that ID, then the command simply won't run. Click *OK* to save your changes, or click *Reset* to revert back to the default mapping.

Connecting to a Device
----------------------

Once a device has been added, it appears in the device list. Disconnected devices appear gray, while connected devices appear white. Select a device and click *Connect* to connect to it. Sucessfully connecting to a device displays a connection icon next to the tab name. If the device has live updates enabled, it will automatically receive commands as you perform actions in PixelMaestro Studio.

When you are done, click *Disconnect* to close the connection. Devices also disconnect automatically when closing PixelMaestro Studio.

Uploading Cuefiles
------------------

Live updates only send individual events to your devices. If you want to send your entire Maestro configuration (called the Cuefile), use the *Upload* button.

First, select a device in the Device List. The *Cuefile Size* text box shows the current size of the Cuefile in bytes. You should check this value against your device's RAM capacity, as well as the size of its CueController buffer. If the Cuefile is larger than either of those, then the Cuefile might not upload properly.

Click *Upload* to send the Cuefile to your device. The progress bar shows how much of the Cue has been uploaded. You can see the contents of the Cuefile by clicking the *Preview* button.

The Cuefile contains every instruction needed to reproduce your configuration from scratch. As such, you can store the Cuefile on a device and read it when the device boots to restore your Maestro to a previous state. On an Arduino, you can do this by writing the Cuefile to EEPROM as demonstrated in :pmarduino:`this example sketch <USB_EEPROM>`. If your device has a storage device like an SD card, you can copy any ``.PMC`` file created in PixelMaestro Studio to the card and pass its contents to the ``CueController``.
