
Canvas Tab
==========

The Canvas tab lets you create and customize :pmdocs:`Canvases <Canvases.html>`. Canvases let you draw shapes and load images to display on the Section.

To use a Canvas on the active Section, click *Enable Canvas*.


.. image:: images/canvas-controls.png
   :target: _images/canvas-controls.png
   :alt: Canvas Tab


Like Animations, Canvases use color palettes. You can use the same palettes with both components. However, Canvases are limited to the first 255 colors in a palette, since the last color is reserved for transparency.

.. Tip:: After modifying a palette, reselect it from the palette drop-down to reload the palette.

Loading Images
--------------

You can load an image file into the Canvas by clicking the *Open Image...* button. You can select a PNG, JPEG, or GIF image file. If the image is animated, PixelMaestro Studio will load each frame as a new Canvas frame and immediately begin playback. You can play, pause, or step through the animation using the playback controls on the right side of the screen.

Drawing
-------

To draw on the Canvas, select the drawing tool you wish to use, select the location where you want to draw, set any additional parameters, select the palette and color you want to draw with, then click ``Draw``.  When using the :doc:`free drawing tool <Canvas-Tab.html#free-drawing>`, you can use your mouse to draw directly on the Canvas by left-clicking on the Section.

Drawing Tools
-------------

The *Drawing Tools* box provides controls for drawing shapes, text, and other objects on the Canvas. Select a drawing tool using the icons provided. Depending on the tool you select, one or more text boxes will become enabled. These boxes allow you to set the coordinates, size, and any other options needed to draw the object.

.. Tip:: You can use the brush tool to free draw directly on the Canvas using your mouse.

When drawing shapes, only the outline of the shape is drawn by default. Checking *Fill* draws the shape filled in.

Before drawing an object, you'll need to select a color. First, select a palette using the *Palette* drop-down, then click on the color that you want to draw the object with. Finally, click *Draw* to draw the object.

To erase a color drawn on the Canvas, click the **X** button at the far left of the color palette. This sets the drawing color to transparent. Click *Draw* to "erase" the pixel.

Specifying Coordinates
^^^^^^^^^^^^^^^^^^^^^^

When drawing a shape, you will need to set the location where you want to draw the shape. Canvases use a `Cartesian coordinate system <https://en.wikipedia.org/wiki/Cartesian_coordinate_system>`_ with the origin (0,0) starting in the top-left corner and increasing as you move away from the corner. For example, the point (2, 5) is 2 pixels to the right of the origin and 5 pixels down.

The coordinate boxes are found on the right-hand side of the Drawing Tools box:


* The *Cursor* (*Point A* for triangles) is the shape's starting point. For rectangles and text this is the top-left corner, but for circles this is the center.
* The *Target* is the shape's ending point.

  * For lines and rectangles, this is the *size*, or how long the shape is from the cursor.
  * For circles, this is the *radius*.
  * For triangles, this is *Point B*, or the location of the triangle's second corner.

* For triangles, *Point C* is the location of the triangle's third corner.

Free Drawing
^^^^^^^^^^^^

You can draw directly onto the Canvas by selecting the brush icon, selecting a color, and left-clicking directly on the Section. Use the right mouse button to erase.

You can also click on the Section with the replace tool.

Replacing Colors
^^^^^^^^^^^^^^^^

You can replace a single color with another color by using the *Replace* tool. The Replace tool replaces all instances of one color across the entire frame with another color. Select the pixel containing the color you want to replace, select the new color in the Palette, then click *Draw*. You can also select the color by left-clicking directly on the Section.

Managing Animations
-------------------

The animations box contains controls for managing and editing animations. Any Canvas with more than one drawing surface (called a *frame*\ ) is an animated Canvas.  *Frame Count* displays the total number of frames, while *Current Frame* displays the index of the current frame (while the animation is stopped). You can start or stop playback and step through frames using the playback buttons. *Frame Interval* sets the amount of time (in milliseconds) between each frame.

.. Tip:: Use the arrow keys to step through individual frames while the animation is paused.

Clearing the Canvas
-------------------

To erase the Canvas, click the *Clear...* button.

.. Warning:: This will erase *every* frame in the Canvas!
