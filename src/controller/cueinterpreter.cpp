#include "cueinterpreter.h"

const QStringList CueInterpreter::Handlers({"Animation Handler",
											"Canvas Handler",
											"Maestro Handler",
											"Section Handler",
											"Show Handler"});

const QStringList CueInterpreter::AnimationActions({"Set Colors",
													"Set Cycle Index",
													"Set Fade",
													"Set Lightning Options",
													"Set Orientation",
													"Set Plasma Options",
													"Set Radial Options",
													"Set Reverse",
													"Set Sparkle Options",
													"Set Timing"});

const QStringList CueInterpreter::CanvasActions({"Clear",
												 "Draw Circle",
												 "Draw Frame",
												 "Draw Line",
												 "Draw Point",
												 "Draw Rect",
												 "Draw Text",
												 "Draw Triangle",
												 "Next Frame",
												 "Set Current Frame Index",
												 "Set Frame Timing",
												 "Set Num Frames"});

const QStringList CueInterpreter::MaestroActions({"Set Show",
												  "Set Timing"});

const QStringList CueInterpreter::SectionActions({"Remove Canvas",
												  "Remove Layer",
												  "Set Animation",
												  "Set Canvas",
												  "Set Dimensions",
												  "Set Layer",
												  "Set Offset",
												  "Set Scroll"});

const QStringList CueInterpreter::ShowActions({"Set Events",
											   "Set Looping",
											   "Set Timing"});

const QStringList CueInterpreter::AnimationTypes({"Blink",
												  "Cycle",
												  "Lightning",
												  "Mandelbrot",
												  "Merge",
												  "Plasma",
												  "Radial",
												  "Random",
												  "Solid",
												  "Sparkle",
												  "Wave"});

const QStringList CueInterpreter::CanvasTypes({"Animation",
											   "Color"});

const QStringList CueInterpreter::ColorMixModes({"Alpha",
												"Multiply",
												"Overlay"});


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
	result->append("Section: " + QString::number(cue[AnimationCueHandler::Byte::SectionByte]) + ", ");
	result->append("Layer: " + QString::number(cue[AnimationCueHandler::Byte::LayerByte]) + ", ");
	result->append("Action: " + AnimationActions.at(cue[AnimationCueHandler::Byte::ActionByte]));

	// TODO: Finish Animation parameters
}

void CueInterpreter::interpret_canvas_cue(uint8_t* cue, QString* result) {
	result->append("Section: " + QString::number(cue[CanvasCueHandler::Byte::SectionByte]) + ", ");
	result->append("Layer: " + QString::number(cue[CanvasCueHandler::Byte::LayerByte]) + ", ");
	result->append("Canvas Type: " + CanvasTypes.at(cue[CanvasCueHandler::Byte::TypeByte]) + ", ");
	result->append("Action: " + CanvasActions.at(cue[CanvasCueHandler::Byte::ActionByte]));

	// TODO: Canvas parameters
}

void CueInterpreter::interpret_maestro_cue(uint8_t* cue, QString* result) {
	result->append("Action: " + MaestroActions.at(cue[MaestroCueHandler::Byte::ActionByte]));

	switch((MaestroCueHandler::Action)cue[MaestroCueHandler::Byte::ActionByte]) {
		case MaestroCueHandler::Action::SetShow:
			break;
		case MaestroCueHandler::Action::SetTiming:
			result->append(", Interval: " + QString::number(IntByteConvert::byte_to_int(&cue[MaestroCueHandler::Byte::OptionsByte])));
			break;
	}
}

void CueInterpreter::interpret_section_cue(uint8_t* cue, QString* result) {
	result->append("Section: " + QString::number(cue[SectionCueHandler::Byte::SectionByte]) + ", ");
	result->append("Layer: " + QString::number(cue[SectionCueHandler::Byte::LayerByte]) + ", ");
	result->append("Action: " + SectionActions.at(cue[SectionCueHandler::Byte::ActionByte]));

	switch ((SectionCueHandler::Action)cue[SectionCueHandler::Byte::ActionByte]) {
		case SectionCueHandler::Action::RemoveCanvas:
			break;
		case SectionCueHandler::Action::RemoveLayer:
			break;
		case SectionCueHandler::Action::SetAnimation:
			result->append(", Type: " + AnimationTypes.at(cue[SectionCueHandler::Byte::OptionsByte]));
			break;
		case SectionCueHandler::Action::SetCanvas:
			{
				result->append(", Type: " + CanvasTypes.at(cue[SectionCueHandler::Byte::OptionsByte]));
				result->append(", Frames: " + QString::number(cue[SectionCueHandler::Byte::OptionsByte + 1]));
			}
			break;
		case SectionCueHandler::Action::SetDimensions:
			{
				result->append(", Width: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
				result->append(", Height: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 1])));
			}
			break;
		case SectionCueHandler::Action::SetLayer:
			{
				result->append(", Mix Mode: " + ColorMixModes.at(cue[SectionCueHandler::Byte::OptionsByte]));
				result->append(", Alpha: " + QString::number(cue[SectionCueHandler::Byte::OptionsByte + 1]));
			}
			break;
		case SectionCueHandler::Action::SetOffset:
			{
				result->append(", X offset: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
				result->append(", Y offset: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 2])));
			}
			break;
		case SectionCueHandler::Action::SetScroll:
			{
				result->append(", X rate: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
				result->append(", Y rate: " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 2])));
			}
			break;
	}
}

void CueInterpreter::interpret_show_cue(uint8_t *cue, QString *result) {
	result->append("Action: " + ShowActions.at(cue[ShowCueHandler::Byte::ActionByte]));
}