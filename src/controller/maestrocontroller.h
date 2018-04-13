/*
 * MaestroController - Wrapper class for managing a Maestro using a DrawingArea.
 */

#ifndef MAESTROCONTROLLER_H
#define MAESTROCONTROLLER_H

#include "core/maestro.h"
#include "core/section.h"
#include "drawingarea/maestrodrawingarea.h"
#include <QDataStream>
#include <QElapsedTimer>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QVector>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class MaestroDrawingArea;

	class MaestroController : public QObject {
		Q_OBJECT
		public:
			/**
			 * Sets a Cue that should be blocked from serialization.
			 */
			struct BlockedCue {
				/// The Handler that this Cue belongs to.
				CueController::Handler handler;
				/// The action being performed.
				uint8_t action;
			};

			MaestroController();
			~MaestroController();
			void add_drawing_area(MaestroDrawingArea* drawing_area);
			void block_cue(CueController::Handler handler, uint8_t action_index);
			void clear_blocked_cues();
			Maestro* get_maestro();
			bool get_running();
			uint64_t get_total_elapsed_time();
			void remove_drawing_area(MaestroDrawingArea* drawing_area);
			void save_cuefile(QString filename);
			void save_maestro_to_datastream(QDataStream* datastream);
			void save_section_to_datastream(QDataStream* datastream, uint8_t section_id, uint8_t layer_id);
			void write_cue_to_stream(QDataStream* stream, uint8_t* cue);
			Section* set_sections(uint8_t num_sections, Point dimensions = Point(10, 10));
			void stop();
			void start();

		private:
			/// List of Cues that should be blocked from executing.
			QVector<BlockedCue> blocked_cues_;

			/// References each drawing area that the Maestro is rendering to.
			QVector<MaestroDrawingArea*> drawing_areas_;

			/// Tracks the time that the Maestro was last paused.
			uint64_t last_pause_ = 0;

			/// Updates the Maestro's runtime.
			QElapsedTimer elapsed_timer_;

			/// Maestro refresh timer.
			QTimer timer_;

			/// Maestro controlled by this controller.
			QSharedPointer<Maestro> maestro_;

			/// The number of Sections in the Maestro.
			uint8_t num_sections_;

			/// Sections belonging to the Maestro.
			Section* sections_ = nullptr;

		private slots:
			void update();
	};
}

#endif // MAESTROCONTROLLER_H
