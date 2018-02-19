#ifndef COLORCANVASDEMO_H
#define COLORCANVASDEMO_H

#include "controller/maestrocontroller.h"
#include "drawingarea/maestrodrawingarea.h"

namespace PixelMaestroStudio {
	class ColorCanvasDemo : public MaestroDrawingArea {
		public:
			ColorCanvasDemo(QWidget* parent, MaestroController* maestro_controller);
	};
}

#endif // COLORCANVASDEMO_H
