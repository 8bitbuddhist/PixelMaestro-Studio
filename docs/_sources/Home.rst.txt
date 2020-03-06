
Home
====

Welcome to the PixelMaestro Studio documentation!

What is PixelMaestro Studio?
----------------------------

PixelMaestro Studio is an application for controlling RGB LEDs, built on the `PixelMaestro library <https://github.com/8bitbuddhist/PixelMaestro/>`_. With PixelMaestro Studio, you can design animations, create dynamic light shows, control lights over USB, and more.

.. image:: images/screenshot.png
   :target: _images/screenshot.png
   :alt: PixelMaestro Studio screenshot

To get started with PixelMaestro Studio, check out the :doc:`Getting Started <Getting-Started>` guide. If you want to learn more about the PixelMaestro library, visit the `PixelMaestro GitHub page <https://github.com/8bitbuddhist/PixelMaestro/>`_ or the :pmdocs:`PixelMaestro documentation <index.html>`.

FAQ
---

**What devices are supported by PixelMaestro Studio?**

PixelMaestro Studio can control any device running the PixelMaestro library over a serial or TCP/IP connection. This includes Arduino, Raspberry Pi, Teensy, etc.

**How do I connect my device to PixelMaestro Studio?**

Follow the instructions in the :doc:`Device Tab <Device-Tab>` page.

**When I click Connect on the Device Tab, nothing happens or I get an error message**

If you're connecting over serial, make sure you have read and write access to the serial port that the device is connected on. Also, check that the port isn't currently being used by another process.

If you're using TCP/IP, make sure the IP address is correct, that the port is open on the device, and that your firewall isn't blocking traffic.

**My device is connected, but when I upload a Cuefile or enable Live Updates, nothing happens**

In your device's sketch, make sure you :pmdocs:`enabled the CueController <Cues.html#initializing-the-cuecontroller>` and appropriate :pmdocs:`CueHandlers <Cues.html#enabling-cuehandlers>`. If you are sending larger Cues (especially Canvas Cues), you might want to increase the size of the CueController's buffer when initializing it.

Also, make sure that you pass the output from ``serial::read()`` to :pmdocs:`CueController::read() <Cues.html#running-cues>` *before* running ``Maestro::update()``. For an example, check the :pmarduino:`Arduino sketches <>` included with PixelMaestro.

The CueController on your device might also be blocking certain Cues. You can tell when Cues are being blocked if your sketch contains statements similar to:

.. code-block:: c++

   cue_controller.set_blocked_cues(...)

Lastly, you might be hitting the memory limits of your device. Try disabling or removing features from your sketch, especially Canvases and Shows as they can use up large amounts of RAM. Or, use a device with a larger amount of RAM.

**The app runs really slow at high resolutions**

All rendering is done on the CPU, so the amount of processing power required increases as your resolution increases. You can try:

* Reducing the size of your Sections
* Not rendering the Maestro by unchecking the output options shown in the *Window* menu
* :pmdocs:`Tweaking Pixel performance options <Pixels.html#performance-options>`

**You should add X animation or Y feature**

Submit an issue, or better yet, a pull request!
