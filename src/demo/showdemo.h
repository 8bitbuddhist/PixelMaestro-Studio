/*
 * ShowDemo - Demonstrates a Maestro using a Show.
 */

#ifndef SHOWDEMO_H
#define SHOWDEMO_H

#include "drawingarea/maestrodrawingarea.h"
#include "controller/maestrocontroller.h"

namespace PixelMaestroStudio {
	class ShowDemo : public MaestroDrawingArea {
		public:
			ShowDemo(QWidget* parent, MaestroController* maestro_controller);
			~ShowDemo();

		private:
			Event* events_;

	};
}

#endif // SHOWDEMO_H
