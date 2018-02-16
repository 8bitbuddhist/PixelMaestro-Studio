#include "colorpresets.h"
#include "cuedemo.h"
#include "cue/animationcuehandler.h"
#include "cue/cuecontroller.h"
#include "cue/canvascuehandler.h"
#include "cue/sectioncuehandler.h"

namespace PixelMaestroStudio {
	CueDemo::CueDemo(QWidget* parent, MaestroController* maestro_controller) : SimpleDrawingArea(parent, maestro_controller) {
		maestro_controller_->set_sections(1, Point(10, 10));

		Maestro* maestro = maestro_controller_->get_maestro();

		CueController* controller = maestro->set_cue_controller();

		SectionCueHandler *section_handler = static_cast<SectionCueHandler*>(controller->enable_handler(CueController::Handler::SectionHandler));
		section_handler->set_layer(0, 0, Colors::MixMode::Overlay, 0);
		controller->run();

		section_handler->set_dimensions(0, 0, 62, 9);
		controller->run();

		Colors::RGB colors_black[] = {ColorPresets::Black, ColorPresets::White};
		section_handler->set_animation(0, 0, AnimationType::Cycle, false, colors_black, 2);
		controller->run();

		AnimationCueHandler* animation_handler = static_cast<AnimationCueHandler*>(controller->enable_handler(CueController::Handler::AnimationHandler));
		animation_handler->set_timer(0, 0, 2000, 1500);
		controller->run();

		// Note that this isn't true black because the Layer mixmode treats true black as transparent.
		Colors::RGB colors_white[] = {ColorPresets::White, {0, 0, 1}};
		section_handler->set_animation(0, 1, AnimationType::Cycle, false, colors_white, 2);
		controller->run();

		animation_handler->set_timer(0, 1, 2000, 1500);
		controller->run();

		section_handler->set_canvas(0, 1, CanvasType::AnimationCanvas);
		controller->run();

		CanvasCueHandler* canvas_handler = static_cast<CanvasCueHandler*>(controller->enable_handler(CueController::Handler::CanvasHandler));
		canvas_handler->draw_text(0, 1, 1, 1, Font::Type::Font5x8, "PixelMaestro", 12);
		controller->run();
	}

	CueDemo::~CueDemo() {}
}