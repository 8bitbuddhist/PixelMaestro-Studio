
Section Tab
===========

The Section tab provides controls for editing :pmdocs:`Sections <Sections.html>`. Here, you can change settings such as the size, offset, scrolling behavior, and number of Layers for each Section in your Maestro.


.. image:: images/section-controls.png
   :target: images/section-controls.png
   :alt: Section Controls


Selecting the Active Section
--------------------------

PixelMaestro Studio can only modify one Section at a time. In order to edit a Section or Layer, you must first set it as the *Active Section*. All Section-related actions performed in PixelMaestro Studio modify the Active Section. Use the Active Section drop-down to select the Active Section. Note that if you are rendering the Maestro on-screen, the Active Section will be shown with a white frame.

You can also set a Layer as the Active Section. In order to make a Layer active, you must first select its base Section in the *Active Section* drop-down menu. The *Active Layer* drop-down lists every Layer belonging to that Section. Once you select your desired Layer, it will be shown in the Maestro Viewer with a light gray frame.

For example, if ``Section 1`` is selected in the *Active Section* drop-down and *Base Section* is selected in the *Active Layer* drop-down, then any actions performed will affect the Section. If you select *Layer 1* in the *Active Layer* drop-down, then the Section's first Layer will become the Active Section.

Section Settings
----------------

Section settings affect the behavior of the Section.

The *Layers* text box changes the number of Layers belonging to the Section. Increasing the number adds new blank Layers to the Section, while decreasing the number removes Layers.

If *wrap* is checked, when the Section is scrolling or offset, Pixels that are moved off of the Section will reappear on the opposite side of the Section.

Layer Settings
--------------

Layer settings affect the behavior of the selected Layer.

The *Active Layer* sets a Layer as the Active Section. Select *Base Section* to change the Active Section back to the Section.

*Mix Mode* determines how the Layer is rendered on top of the Section. The *Alpha* spin box controls the amount of blending between the Layer and the base Section, and is only available when the Alpha mix mode is selected. To learn more, read the PixelMaestro :pmdocs:`color mixing documentation <Colors.html#mixing-colors>`.

General Settings
----------------

General settings affect the Active Section as well as its Layers.

*Grid Size* changes the horizontal and vertical dimensions of the base Section. Dimensions are measured in individual pixels. The new dimensions automatically apply to all components belonging to the Section including Layers, Canvases, and Animations.

.. Warning:: High resolutions can cause high CPU usage. If you're experiencing performance problems, try reducing the grid size.

*Scroll* scrolls the contents of the Section across the grid. The values indicate how much time (in milliseconds) it takes to complete a single scroll across either the horizontal or vertical axis. For example, a value of ``1000`` in the first text box means it will take 1000 milliseconds to scroll the Section once along the x axis. Setting either number negative reverses the direction of the scroll.

*Offset* shifts the Section the specified number of pixels along the x or y axis. This is disabled while *Scroll* is set.

Brightness
----------

Brightness changes how bright or dim the Section appears. Brightness is a numeric value between 0 and 255, where 0 is completely dark and 255 is fully lit.

.. Tip:: You can control each Layer's brightness independently of its parent Section, but a Layer can only be as bright as its parent.
