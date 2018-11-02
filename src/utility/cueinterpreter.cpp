#include "cueinterpreter.h"
#include "dialog/preferencesdialog.h"

namespace PixelMaestroStudio {
	const QString CueInterpreter::delimiter = QString(" | ");

	const QStringList CueInterpreter::Handlers({"Animation",
												"Canvas",
												"Maestro",
												"Section",
												"Show"});

	const QStringList CueInterpreter::AnimationActions({"Set Cycle Index",
														"Set Fade",
														"Set Fire Options",
														"Set Lightning Options",
														"Set Orientation",
														"Set Palette",
														"Set Plasma Options",
														"Set Radial Options",
														"Set Reverse",
														"Set Sparkle Options",
														"Set Timer",
														"Set Wave Options",
														"Start",
														"Stop"});

	const QStringList CueInterpreter::CanvasActions({"Clear",
													 "Draw Circle",
													 "Draw Frame",
													 "Draw Line",
													 "Draw Point",
													 "Draw Rect",
													 "Draw Text",
													 "Draw Triangle",
													 "Erase Point",
													 "Next Frame",
													 "Previous Frame",
													 "Remove Frame Timer",
													 "Set Current Frame Index",
													 "Set Drawing Color",
													 "Set Frame Timer",
													 "Set Num Frames",
													 "Set Palette",
													 "Start Frame Timer",
													 "Stop Frame Timer"});

	const QStringList CueInterpreter::MaestroActions({"Set Brightness",
													  "Set Show",
													  "Set Timer",
													  "Start",
													  "Stop"
													  "Sync"});

	const QStringList CueInterpreter::SectionActions({"Remove Animation",
													  "Remove Canvas",
													  "Remove Layer",
													  "Set Animation",
													  "Set Canvas",
													  "Set Dimensions",
													  "Set Layer",
													  "Set Offset",
													  "Set Scroll"});

	const QStringList CueInterpreter::ShowActions({"Set Events",
												   "Set Looping",
												   "Set Timing Mode"});

	const QStringList CueInterpreter::ShowTimings({"Absolute",
												   "Relative"});

	const QStringList CueInterpreter::AnimationTypes({"Blink",
													  "Cycle",
													  "Fire",
													  "Lightning",
													  "Mandelbrot",
													  "Plasma",
													  "Radial",
													  "Random",
													  "Solid",
													  "Sparkle",
													  "Wave"});

	const QStringList CueInterpreter::AnimationOrientations({"Horizontal",
															 "Vertical"});

	const QStringList CueInterpreter::ColorMixModes({"None",
													 "Alpha",
													 "Multiply",
													 "Overlay"});

	CueInterpreter::CueInterpreter() { }

	void CueInterpreter::append_bool(bool value, QString* result) {
		if (value) {
			result->append(": On");
		}
		else {
			result->append(": Off");
		}
	}

	void CueInterpreter::append_animation_timer(uint16_t interval, uint16_t delay, QString *result) {
		append_timer(interval, result);
		result->append(delimiter + "Delay: " + QString::number(delay));
	}

	void CueInterpreter::append_timer(uint16_t interval, QString *result) {
		result->append(delimiter + "Interval: " + QString::number(interval));
	}

	QString CueInterpreter::interpret_cue(uint8_t* cue) {
		QString result;
		// Delegate to the correct handler
		switch ((CueController::Handler)cue[(uint8_t)CueController::Byte::PayloadByte]) {
			case CueController::Handler::AnimationCueHandler:
				interpret_animation_cue(cue, &result);
				break;
			case CueController::Handler::CanvasCueHandler:
				interpret_canvas_cue(cue, &result);
				break;
			case CueController::Handler::MaestroCueHandler:
				interpret_maestro_cue(cue, &result);
				break;
			case CueController::Handler::SectionCueHandler:
				interpret_section_cue(cue, &result);
				break;
			case CueController::Handler::ShowCueHandler:
				interpret_show_cue(cue, &result);
				break;
		}

		return result;
	}

	void CueInterpreter::interpret_animation_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::SectionByte]) + delimiter);
		result->append("Layer " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::LayerByte]) + delimiter);
		result->append("Animation" + delimiter);
		result->append(AnimationActions.at(cue[(uint8_t)AnimationCueHandler::Byte::ActionByte]));

		switch((AnimationCueHandler::Action)cue[(uint8_t)AnimationCueHandler::Byte::ActionByte]) {
			case AnimationCueHandler::Action::SetPalette:
				result->append(": " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]) + " colors");
				break;
			case AnimationCueHandler::Action::SetCycleIndex:
				result->append(": " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetFade:
				append_bool((bool)cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::Action::SetFireOptions:
				result->append(delimiter + "Multiplier: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetLightningOptions:
				{
					result->append(delimiter+ "Bolt Chance: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
					result->append(delimiter+ "Drift: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1]));
					result->append(" " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 2]));
					result->append(delimiter + "Fork Chance: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 3]));
				}
				break;
			case AnimationCueHandler::Action::SetOrientation:
				result->append(": " + AnimationOrientations.at(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetPlasmaOptions:
				{
					result->append(delimiter + "Size: " + QString::number(FloatByteConvert::byte_to_float(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte])));
					result->append(delimiter + "Resolution: " + QString::number(FloatByteConvert::byte_to_float(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 4])));
				}
				break;
			case AnimationCueHandler::Action::SetRadialOptions:
				result->append(delimiter + "Resolution: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetReverse:
				append_bool((bool)cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::Action::SetSparkleOptions:
				result->append(delimiter + "Threshold: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetTimer:
				append_animation_timer(
					IntByteConvert::byte_to_int(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]),
					IntByteConvert::byte_to_int(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 2]),
					result
				);
				break;
			case AnimationCueHandler::Action::SetWaveOptions:
				{
					result->append(delimiter + "Mirror: " +  QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
					result->append(delimiter + "Skew: " + QString::number((int8_t)cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1]));
				}
				break;
			case AnimationCueHandler::Action::Start:
				// Do nothing
			case AnimationCueHandler::Action::Stop:
				// Do nothing
				break;
		}
	}

	void CueInterpreter::interpret_canvas_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[(uint8_t)CanvasCueHandler::Byte::SectionByte]) + delimiter);
		result->append("Layer " + QString::number(cue[(uint8_t)CanvasCueHandler::Byte::LayerByte]) + delimiter);
		result->append("Canvas" + delimiter);
		result->append(CanvasActions.at(cue[(uint8_t)CanvasCueHandler::Byte::ActionByte]));

		switch((CanvasCueHandler::Action)cue[(uint8_t)CanvasCueHandler::Byte::ActionByte]) {
			case CanvasCueHandler::Action::Clear:
				break;
			case CanvasCueHandler::Action::DrawText:
				{
					int start = (uint8_t)CanvasCueHandler::Byte::OptionsByte + 7;
					int size = cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte + 6];
					QString text = QString::fromUtf8((char*)&cue[start], size);
					result->append(": \"" + text + "\"");
				}
				break;
			case CanvasCueHandler::Action::ErasePoint:
				result->append(": (" + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])) + ", ");
				result->append(QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte + 2])) + ")");
				break;
			case CanvasCueHandler::Action::NextFrame:
				break;
			case CanvasCueHandler::Action::RemoveFrameTimer:
				break;
			case CanvasCueHandler::Action::SetCurrentFrameIndex:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::SetFrameTimer:
				append_timer(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte]), result);
				break;
			case CanvasCueHandler::Action::SetNumFrames:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])));
				break;
			default:
				break;
		}
	}

	void CueInterpreter::interpret_maestro_cue(uint8_t* cue, QString* result) {
		result->append("Maestro" + delimiter);
		result->append(MaestroActions.at(cue[(uint8_t)MaestroCueHandler::Byte::ActionByte]));

		switch((MaestroCueHandler::Action)cue[(uint8_t)MaestroCueHandler::Byte::ActionByte]) {
			case MaestroCueHandler::Action::SetBrightness:
				result->append(": ");
				result->append(cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte]);
				break;
			case MaestroCueHandler::Action::SetShow:
				break;
			case MaestroCueHandler::Action::SetTimer:
			case MaestroCueHandler::Action::Sync:
				append_timer(IntByteConvert::byte_to_int(&cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte]), result);
				break;
			case MaestroCueHandler::Action::Start:
				// Do nothing
			case MaestroCueHandler::Action::Stop:
				// Do nothing
				break;
		}
	}

	void CueInterpreter::interpret_section_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::SectionByte]) + delimiter);
		result->append("Layer " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::LayerByte]) + delimiter);
		result->append("Section" + delimiter);
		result->append(SectionActions.at(cue[(uint8_t)SectionCueHandler::Byte::ActionByte]));

		switch ((SectionCueHandler::Action)cue[(uint8_t)SectionCueHandler::Byte::ActionByte]) {
			case SectionCueHandler::Action::RemoveAnimation:
				break;
			case SectionCueHandler::Action::RemoveCanvas:
				break;
			case SectionCueHandler::Action::RemoveLayer:
				break;
			case SectionCueHandler::Action::SetAnimation:
				result->append(": " + AnimationTypes.at(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte]));
				break;
			case SectionCueHandler::Action::SetCanvas:
				result->append(", " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])) + " frame(s)";
				break;
			case SectionCueHandler::Action::SetDimensions:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
				result->append(" x " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 2])));
				break;
			case SectionCueHandler::Action::SetLayer:
				result->append(": " + ColorMixModes.at(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte]) + " Mix Mode, ");
				result->append("Alpha = " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 1]));
				break;
			case SectionCueHandler::Action::SetOffset:
				result->append(": (" + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
				result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				break;
			case SectionCueHandler::Action::SetScroll:
				result->append(": (" + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
				result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				break;
		}
	}

	void CueInterpreter::interpret_show_cue(uint8_t *cue, QString *result) {
		result->append("Show" + delimiter);
		result->append(ShowActions.at(cue[(uint8_t)ShowCueHandler::Byte::ActionByte]));

		switch ((ShowCueHandler::Action)cue[(uint8_t)ShowCueHandler::Byte::ActionByte]) {
			case ShowCueHandler::Action::SetEvents:
				result->append(delimiter + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)ShowCueHandler::Byte::OptionsByte])) + " events");
				break;
			case ShowCueHandler::Action::SetLooping:
				append_bool((bool)cue[(uint8_t)ShowCueHandler::Byte::OptionsByte], result);
				break;
			case ShowCueHandler::Action::SetTimingMode:
				result->append(": " + ShowTimings.at(cue[(uint8_t)ShowCueHandler::Byte::OptionsByte]));
				break;
		}
	}
}
