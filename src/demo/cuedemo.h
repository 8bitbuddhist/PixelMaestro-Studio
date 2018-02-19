#ifndef COMMANDDEMO_H
#define COMMANDDEMO_H

#include "controller/maestrocontroller.h"
#include "drawingarea/maestrodrawingarea.h"

namespace PixelMaestroStudio {
	class CueDemo : public MaestroDrawingArea {
		public:
			CueDemo(QWidget* parent, MaestroController* maestro_controller);
			~CueDemo();
	};
}

#endif // COMMANDDEMO_H