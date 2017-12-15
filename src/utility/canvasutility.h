/*
 * CanvasUtility - Tools for managing Canvases.
 */

#ifndef CANVASUTILITY_H
#define CANVASUTILITY_H

#include <QByteArray>
#include <QString>
#include "canvas/animationcanvas.h"
#include "canvas/canvas.h"
#include "canvas/colorcanvas.h"
#include "canvas/palettecanvas.h"
#include "core/colors.h"
#include "widget/maestrocontrol.h"

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class CanvasUtility {
		public:
			static void copy_from_canvas(AnimationCanvas* canvas, bool** target, uint16_t target_x, uint16_t target_y);
			static void copy_from_canvas(ColorCanvas* canvas, Colors::RGB** target, uint16_t target_x, uint16_t target_y);
			static void copy_from_canvas(PaletteCanvas* canvas, uint8_t** target, uint16_t target_x, uint16_t target_y);
			static void copy_to_canvas(AnimationCanvas* canvas, bool** source, uint16_t target_x, uint16_t target_y, MaestroControl* maestro_control);
			static void copy_to_canvas(ColorCanvas* canvas, Colors::RGB** source, uint16_t target_x, uint16_t target_y, MaestroControl* maestro_control);
			static void copy_to_canvas(PaletteCanvas* canvas, uint8_t** source, uint16_t target_x, uint16_t target_y, MaestroControl* maestro_control);
			static void load_image(QString filename, Canvas* canvas, MaestroControl* maestro_control = nullptr);
	};
}

#endif // CANVASUTILITY_H
