#ifndef SHOWCONTROLLER_H
#define SHOWCONTROLLER_H

#include "controller/maestrocontroller.h"
#include "core/maestro.h"
#include "cue/event.h"
#include "cue/show.h"
#include <QString>
#include <QVector>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class MaestroController;

	class ShowController {
		public:
			explicit ShowController(MaestroController* maestro_controller);
			Event* add_event(uint32_t time, uint8_t* cue);
			void clear();
			int get_event_index(Event* event);
			QString get_event_description(uint16_t index);
			QVector<Event>* get_events();
			void move(uint16_t from, uint16_t to);
			void remove_event(uint16_t index);

		private:
			CueInterpreter cue_interpreter_;
			QVector<Event> event_queue_;
			Show* show_;

	};
}

#endif // SHOWCONTROLLER_H
