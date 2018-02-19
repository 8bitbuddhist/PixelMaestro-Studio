/*
 * BlinkDemo - Displays a simple blink animation.
 */

#ifndef BLINKDEMO_H
#define BLINKDEMO_H

#include "drawingarea/maestrodrawingarea.h"
#include "controller/maestrocontroller.h"

namespace PixelMaestroStudio {
	class BlinkDemo : public MaestroDrawingArea {
		public:
			BlinkDemo(QWidget* parent, MaestroController* maestro_controller);
	};
}

#endif // BLINKDEMO_H
