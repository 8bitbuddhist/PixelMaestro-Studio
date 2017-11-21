/*
 * CueInterpreter - Parses Cues and generates a string description of the Cue.
 */

#ifndef CUEINTERPRETER_H
#define CUEINTERPRETER_H

#include <QString>
#include <QStringList>
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/maestrocuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/showcuehandler.h"

using namespace PixelMaestro;

class CueInterpreter {
	public:
		CueInterpreter();

		// Map enums to QStrings
		// Handlers
		static const QStringList Handlers;
		static const QStringList AnimationActions;
		static const QStringList CanvasActions;
		static const QStringList MaestroActions;
		static const QStringList SectionActions;
		static const QStringList ShowActions;
		static const QStringList AnimationTypes;
		static const QStringList CanvasTypes;
		static const QStringList ColorMixModes;

		QString interpret_cue(uint8_t* cue);

	private:
		void interpret_animation_cue(uint8_t* cue, QString* result);
		void interpret_canvas_cue(uint8_t* cue, QString* result);
		void interpret_maestro_cue(uint8_t* cue, QString* result);
		void interpret_section_cue(uint8_t* cue, QString* result);
		void interpret_show_cue(uint8_t* cue, QString* result);
};

#endif // CUEINTERPRETER_H
