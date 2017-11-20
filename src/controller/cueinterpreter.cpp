#include "cueinterpreter.h"

const QStringList CueInterpreter::Handlers({"Animation Handler", "Canvas Handler", "Maestro Handler", "Section Handler", "Show Handler"});

const QStringList CueInterpreter::AnimationActions({"Set Center", "Set Colors", "Set Cycle Index", "Set Fade", "Set Lightning Options", "Set Orientation", "Set Plasma Options", "Set Radial Options", "Set Reverse", "Set Sparkle Options", "Set Timing"});
const QStringList CueInterpreter::CanvasActions({"Canvas Clear", "Draw Circle", "Draw Frame", "Draw Line", "Draw Point", "Draw Rect", "Draw Text", "Draw Triangle", "Next Frame", "Set Current Frame Index", "Set Frame Timing", "Set Num Frames", "Set Offset", "Set Scroll"});
const QStringList CueInterpreter::MaestroActions({"Set Timing"});
const QStringList CueInterpreter::SectionActions({"Remove Canvas", "Set Animation", "Set Canvas", "Set Dimensions", "Set Overlay"});
const QStringList CueInterpreter::ShowActions({"Set Events", "Set Looping", "Set Timing"});

const QStringList CueInterpreter::AnimationTypes({"Blink", "Cycle", "Lightning", "Mandelbrot", "Merge", "Plasma", "Radial", "Random", "Solid", "Sparkle", "Wave"});

const QStringList CueInterpreter::CanvasTypes({"Animation", "Color"});


CueInterpreter::CueInterpreter() { }

QString CueInterpreter::interpret_cue(uint8_t* cue) {
	QString result = QString("Handler: ") + Handlers.at(cue[CueController::Byte::PayloadByte]) + QString(", ");
	// Delegate to the correct handler
	switch ((CueController::Handler)cue[CueController::Byte::PayloadByte]) {
		case CueController::Handler::AnimationHandler:
			interpret_animation_cue(cue, &result);
			break;
		case CueController::Handler::CanvasHandler:
			interpret_canvas_cue(cue, &result);
			break;
		case CueController::Handler::MaestroHandler:
			interpret_maestro_cue(cue, &result);
			break;
		case CueController::Handler::SectionHandler:
			interpret_section_cue(cue, &result);
			break;
		case CueController::Handler::ShowHandler:
			interpret_show_cue(cue, &result);
			break;
	}

	return result;
}

void CueInterpreter::interpret_animation_cue(uint8_t* cue, QString* result) {
	result->append("Section: " + QString(cue[AnimationCueHandler::Byte::SectionByte]) + ", ");
	result->append("Overlay: " + QString(cue[AnimationCueHandler::Byte::OverlayByte]) + ", ");
	result->append("Animation Type: " + AnimationTypes.at(cue[AnimationCueHandler::Byte::ActionByte]) + ", ");
	result->append("Action: " + AnimationActions.at(cue[AnimationCueHandler::Byte::ActionByte]));
}

void CueInterpreter::interpret_canvas_cue(uint8_t* cue, QString* result) {
	result->append("Section: " + QString(cue[CanvasCueHandler::Byte::SectionByte]) + ", ");
	result->append("Overlay: " + QString(cue[CanvasCueHandler::Byte::OverlayByte]) + ", ");
	result->append("Canvas Type: " + CanvasTypes.at(cue[CanvasCueHandler::Byte::TypeByte]) + ", ");
	result->append("Action: " + CanvasActions.at(cue[CanvasCueHandler::Byte::ActionByte]));
}

void CueInterpreter::interpret_maestro_cue(uint8_t* cue, QString* result) {
	result->append("Action: " + MaestroActions.at(cue[MaestroCueHandler::Byte::ActionByte]));
}

void CueInterpreter::interpret_section_cue(uint8_t* cue, QString* result) {
	result->append("Section: " + QString(cue[SectionCueHandler::Byte::SectionByte]) + ", ");
	result->append("Overlay: " + QString(cue[SectionCueHandler::Byte::OverlayByte]) + ", ");
	result->append("Action: " + SectionActions.at(cue[SectionCueHandler::Byte::ActionByte]));
}

void CueInterpreter::interpret_show_cue(uint8_t *cue, QString *result) {
	result->append("Action: " + ShowActions.at(cue[ShowCueHandler::Byte::ActionByte]));
}