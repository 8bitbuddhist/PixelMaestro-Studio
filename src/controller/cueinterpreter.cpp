#include "cueinterpreter.h"

namespace PixelMaestroStudio {
	const QStringList CueInterpreter::Handlers({"Animation",
												"Canvas",
												"Maestro",
												"Section",
												"Show"});

	const QStringList CueInterpreter::AnimationActions({"Set Cycle Index",
														"Set Fade",
														"Set Lightning Options",
														"Set Orientation",
														"Set Palette",
														"Set Plasma Options",
														"Set Radial Options",
														"Set Reverse",
														"Set Sparkle Options",
														"Set Timer",
														"Set Fire Options",
														"Start",
														"Stop",
														"Set Wave Options"});

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
													  "Lightning",
													  "Mandelbrot",
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
		QString result = Handlers.at(cue[(uint8_t)CueController::Byte::PayloadByte]) + QString(", ");
		// Delegate to the correct handler
		switch ((CueController::Handler)cue[(uint8_t)CueController::Byte::PayloadByte]) {
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
		result->append("Section " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::LayerByte]) + ", ");
		result->append(AnimationActions.at(cue[(uint8_t)AnimationCueHandler::Byte::ActionByte]));

		switch((AnimationCueHandler::Action)cue[(uint8_t)AnimationCueHandler::Byte::ActionByte]) {
			case AnimationCueHandler::Action::SetPalette:
				{
					uint8_t num_colors = cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte];
					Colors::RGB base_color(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1],
											cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 2],
											cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 3]);
					result->append(": " + QString::number(num_colors) + " colors");
					result->append(", Base Color: {" +
								   QString::number(base_color.r) + ", " +
								   QString::number(base_color.g) + ", " +
								   QString::number(base_color.b) + "}");
				}
				break;
			case AnimationCueHandler::Action::SetCycleIndex:
				result->append(": " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetFade:
				append_bool((bool)cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::Action::SetFireOptions:
				{
					result->append(", Multiplier: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
					result->append(", Divisor: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1]));
				}
				break;
			case AnimationCueHandler::Action::SetLightningOptions:
				{
					result->append(", Bolt Chance: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
					result->append(", Thresholds: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1]));
					result->append(" " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 2]));
					result->append(", Fork Chance: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 3]));
				}
				break;
			case AnimationCueHandler::Action::SetOrientation:
				result->append(": " + AnimationOrientations.at(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetPlasmaOptions:
				{
					result->append(", Size: " + QString::number(FloatByteConvert::byte_to_float(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte])));
					result->append(", Resolution: " + QString::number(FloatByteConvert::byte_to_float(&cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 4])));
				}
				break;
			case AnimationCueHandler::Action::SetRadialOptions:
				result->append(", Resolution: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetReverse:
				append_bool((bool)cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte], result);
				break;
			case AnimationCueHandler::Action::SetSparkleOptions:
				result->append(", Threshold: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
				break;
			case AnimationCueHandler::Action::SetTimer:
				{
					result->append(", Timing: " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte])));
					result->append(", Pause: " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte + 2])));
				}
				break;
			case AnimationCueHandler::Action::SetWaveOptions:
				{
					result->append(", Mirror: " +  QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte]));
					result->append(", Skew: " + QString::number(cue[(uint8_t)AnimationCueHandler::Byte::OptionsByte + 1]));
				}
				break;
			case AnimationCueHandler::Action::Start:
				result->append(", Start");
				break;
			case AnimationCueHandler::Action::Stop:
				result->append(", Stop");
				break;
		}
	}

	void CueInterpreter::interpret_canvas_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[(uint8_t)CanvasCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[(uint8_t)CanvasCueHandler::Byte::LayerByte]) + ", ");
		if (cue[(uint8_t)CanvasCueHandler::Byte::TypeByte] != 255) {
			result->append(CanvasTypes.at(cue[(uint8_t)CanvasCueHandler::Byte::TypeByte]) + "Canvas, ");
		}
		result->append(CanvasActions.at(cue[(uint8_t)CanvasCueHandler::Byte::ActionByte]));

		switch((CanvasCueHandler::Action)cue[(uint8_t)CanvasCueHandler::Byte::ActionByte]) {
			case CanvasCueHandler::Action::Clear:
				break;
			case CanvasCueHandler::Action::NextFrame:
				break;
			case CanvasCueHandler::Action::RemoveFrameTimer:
				break;
			case CanvasCueHandler::Action::SetCurrentFrameIndex:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::SetFrameTimer:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::SetNumFrames:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte])));
				break;
			case CanvasCueHandler::Action::DrawText:
				result->append(": \"" + QString::fromLatin1((char*)&cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte + 6], cue[(uint8_t)CanvasCueHandler::Byte::OptionsByte + 5]) + "\"");
				break;
			default:	// TODO: Interpret other draw actions
				break;
		}
	}

	void CueInterpreter::interpret_maestro_cue(uint8_t* cue, QString* result) {
		result->append(MaestroActions.at(cue[(uint8_t)MaestroCueHandler::Byte::ActionByte]));

		switch((MaestroCueHandler::Action)cue[(uint8_t)MaestroCueHandler::Byte::ActionByte]) {
			case MaestroCueHandler::Action::SetBrightness:
				result->append(": "	+ cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte]);
				break;
			case MaestroCueHandler::Action::SetShow:
				break;
			case MaestroCueHandler::Action::SetTimer:
			case MaestroCueHandler::Action::Sync:
				result->append(": " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)MaestroCueHandler::Byte::OptionsByte])));
				break;
			case MaestroCueHandler::Action::Start:
				result->append(", Start");
				break;
			case MaestroCueHandler::Action::Stop:
				result->append(", Stop");
				break;
		}
	}

	void CueInterpreter::interpret_section_cue(uint8_t* cue, QString* result) {
		result->append("Section " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::SectionByte]) + ", ");
		result->append("Layer " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::LayerByte]) + ", ");
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
				{
					result->append(", " + CanvasTypes.at(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte]));
					result->append(", " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 1])) + " frames";
				}
				break;
			case SectionCueHandler::Action::SetDimensions:
				{
					result->append(", " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
					result->append(" x " + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 1])));
				}
				break;
			case SectionCueHandler::Action::SetLayer:
				{
					result->append(", Mix Mode: " + ColorMixModes.at(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte]));
					result->append(", Alpha: " + QString::number(cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 1]));
				}
				break;
			case SectionCueHandler::Action::SetOffset:
				{
					result->append(", (" + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
					result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				}
				break;
			case SectionCueHandler::Action::SetScroll:
				{
					result->append(", (" + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte])));
					result->append("," + QString::number(IntByteConvert::byte_to_int(&cue[(uint8_t)SectionCueHandler::Byte::OptionsByte + 2])) + ")");
				}
				break;
		}
	}

	void CueInterpreter::interpret_show_cue(uint8_t *cue, QString *result) {
		result->append(ShowActions.at(cue[(uint8_t)ShowCueHandler::Byte::ActionByte]));

		switch ((ShowCueHandler::Action)cue[(uint8_t)ShowCueHandler::Byte::ActionByte]) {
			case ShowCueHandler::Action::SetLooping:
				if ((bool)cue[(uint8_t)ShowCueHandler::Byte::OptionsByte]) {
					result->append(", true");
				}
				else {
					result->append(", false");
				}
				break;
			case ShowCueHandler::Action::SetTimingMode:
				result->append(", " + ShowTimings.at(cue[(uint8_t)ShowCueHandler::Byte::OptionsByte]));
				break;
		}
	}
}