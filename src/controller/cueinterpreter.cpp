#include "cueinterpreter.h"

namespace PixelMaestroStudio {
	const QStringList CueInterpreter::Handlers({"Animation",
												"Canvas",
												"Maestro",
												"Section",
												"Show"});

	const QStringList CueInterpreter::AnimationActions({"Set Colors",
														"Set Cycle Index",
														"Set Fade",
														"Set Lightning Options",
														"Set Orientation",
														"Set Plasma Options",
														"Set Radial Options",
														"Set Reverse",
														"Set Sparkle Options",
														"Set Timer",
														"Set Fire Options",
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
													 "Next Frame",
													 "Remove Frame Timer",
													 "Set Colors",
													 "Set Current Frame Index",
													 "Set Frame Timer",
													 "Set Num Frames",
													 "Start Frame Timer",
													 "Stop Frame Timer"});

	const QStringList CueInterpreter::MaestroActions({"Set Show",
													  "Set Timer",
													  "Start",
													  "Stop"
													  "Sync"});

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
													  "Wave",
													  "Fire"});

	const QStringList CueInterpreter::AnimationOrientations({"Horizontal",
															 "Vertical"});

	const QStringList CueInterpreter::CanvasTypes({"Animation",
												   "Color",
												   "Palette"});

	const QStringList CueInterpreter::ColorMixModes({"Alpha",
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

	QString CueInterpreter::interpret_cue(uint8_t* cue) {
		QString result = Handlers.at(cue[CueController::Byte::PayloadByte]) + QString(", ");
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
		result->append("Section " + QString::number(cue[AnimationCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[AnimationCueHandler::Byte::LayerByte]) + ", ");
		result->append(AnimationActions.at(cue[AnimationCueHandler::Byte::ActionByte]));

		switch((AnimationCueHandler::Action)cue[AnimationCueHandler::Byte::ActionByte]) {
			case AnimationCueHandler::SetColors:
				{
					uint8_t num_colors = cue[AnimationCueHandler::Byte::OptionsByte];
					Colors::RGB base_color(cue[AnimationCueHandler::Byte::OptionsByte + 1],
											cue[AnimationCueHandler::Byte::OptionsByte + 2],
											cue[AnimationCueHandler::Byte::OptionsByte + 3]);
					result->append(": " + QString::number(num_colors) + " colors");
					result->append(", Base Color: {" +
								   QString::number(base_color.r) + ", " +
								   QString::number(base_color.g) + ", " +
								   QString::number(base_color.b) + "}");
				}
				break;
			case AnimationCueHandler::SetCycleIndex:
				result->append(": " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::SetFade:
				append_bool((bool)cue[AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::SetFireOptions:
				{
					result->append(", Multiplier: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte]));
					result->append(", Divisor: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte + 1]));
				}
				break;
			case AnimationCueHandler::SetLightningOptions:
				{
					result->append(", Bolt Chance: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte]));
					result->append(", Thresholds: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte + 1]));
					result->append(" " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte + 2]));
					result->append(", Fork Chance: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte + 3]));
				}
				break;
			case AnimationCueHandler::SetOrientation:
				result->append(": " + AnimationOrientations.at(cue[AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::SetPlasmaOptions:
				{
					result->append(", Size: " + QString::number(FloatByteConvert::byte_to_float(&cue[AnimationCueHandler::Byte::OptionsByte])));
					result->append(", Resolution: " + QString::number(FloatByteConvert::byte_to_float(&cue[AnimationCueHandler::Byte::OptionsByte + 4])));
				}
				break;
			case AnimationCueHandler::SetRadialOptions:
				result->append(", Resolution: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::SetReverse:
				append_bool((bool)cue[AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::SetSparkleOptions:
				result->append(", Threshold: " + QString::number(cue[AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::SetTimer:
				{
					result->append(", Timing: " + QString::number(IntByteConvert::byte_to_int(&cue[MaestroCueHandler::Byte::OptionsByte])));
					result->append(", Pause: " + QString::number(IntByteConvert::byte_to_int(&cue[MaestroCueHandler::Byte::OptionsByte + 2])));
				}
				break;
			case AnimationCueHandler::Start:
				result->append(", Start");
				break;
			case AnimationCueHandler::Stop:
				result->append(", Stop");
				break;
		}
	}

	void CueInterpreter::interpret_canvas_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[CanvasCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[CanvasCueHandler::Byte::LayerByte]) + ", ");
		if (cue[CanvasCueHandler::Byte::TypeByte] != 255) {
			result->append(CanvasTypes.at(cue[CanvasCueHandler::Byte::TypeByte]) + "Canvas, ");
		}
		result->append(CanvasActions.at(cue[CanvasCueHandler::Byte::ActionByte]));

		switch((CanvasCueHandler::Action)cue[CanvasCueHandler::Byte::ActionByte]) {
			case CanvasCueHandler::Action::Clear:
				break;
			case CanvasCueHandler::Action::NextFrame:
				break;
			case CanvasCueHandler::Action::RemoveFrameTimer:
				break;
			case CanvasCueHandler::Action::SetCurrentFrameIndex:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::SetFrameTimer:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::SetNumFrames:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[CanvasCueHandler::Byte::OptionsByte])));
				break;
			default:	// Draw actions
				break;
		}
	}

	void CueInterpreter::interpret_maestro_cue(uint8_t* cue, QString* result) {
		result->append(MaestroActions.at(cue[MaestroCueHandler::Byte::ActionByte]));

		switch((MaestroCueHandler::Action)cue[MaestroCueHandler::Byte::ActionByte]) {
			case MaestroCueHandler::Action::SetShow:
				break;
			case MaestroCueHandler::Action::SetTimer:
			case MaestroCueHandler::Action::Sync:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[MaestroCueHandler::Byte::OptionsByte])));
				break;
			case MaestroCueHandler::Start:
				result->append(", Start");
				break;
			case MaestroCueHandler::Stop:
				result->append(", Stop");
				break;
		}
	}

	void CueInterpreter::interpret_section_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[SectionCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[SectionCueHandler::Byte::LayerByte]) + ", ");
		result->append(SectionActions.at(cue[SectionCueHandler::Byte::ActionByte]));

		switch ((SectionCueHandler::Action)cue[SectionCueHandler::Byte::ActionByte]) {
			case SectionCueHandler::Action::RemoveCanvas:
				break;
			case SectionCueHandler::Action::RemoveLayer:
				break;
			case SectionCueHandler::Action::SetAnimation:
				result->append(": " + AnimationTypes.at(cue[SectionCueHandler::Byte::OptionsByte]));
				break;
			case SectionCueHandler::Action::SetCanvas:
				{
					result->append(", " + CanvasTypes.at(cue[SectionCueHandler::Byte::OptionsByte]));
					result->append(", " + QString::number(cue[SectionCueHandler::Byte::OptionsByte + 1])) + " frames";
				}
				break;
			case SectionCueHandler::Action::SetDimensions:
				{
					result->append(", " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
					result->append(" x " + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 1])));
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
					result->append(", (" + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
					result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				}
				break;
			case SectionCueHandler::Action::SetScroll:
				{
					result->append(", (" + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte])));
					result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				}
				break;
		}
	}

	void CueInterpreter::interpret_show_cue(uint8_t *cue, QString *result) {
		result->append(ShowActions.at(cue[ShowCueHandler::Byte::ActionByte]));
	}
}