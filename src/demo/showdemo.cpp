/*
 * ShowDemo - Demonstrates a Maestro using a Show.
 */

#include "colorpresets.h"
#include "controller/maestrocontroller.h"
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/event.h"
#include "drawingarea/maestrodrawingarea.h"
#include <memory>
#include "showdemo.h"

namespace PixelMaestroStudio {
	ShowDemo::ShowDemo(QWidget* parent, MaestroController* maestro_controller) : MaestroDrawingArea(parent, maestro_controller) {
		maestro_controller_->set_sections(1, Point(25, 10));

		CueController* controller = maestro_controller_->get_maestro()->set_cue_controller();

		SectionCueHandler* section_handler = static_cast<SectionCueHandler*>(controller->enable_handler(CueController::Handler::SectionHandler));
		CanvasCueHandler* canvas_handler = static_cast<CanvasCueHandler*>(controller->enable_handler(CueController::Handler::CanvasHandler));

		controller->run(section_handler->set_canvas(0, 0, CanvasType::PaletteCanvas));
		controller->run(canvas_handler->set_palette(0, 0, &ColorPresets::Colorwheel_Palette));

		events_ = new Event[5];

		std::string text[] = {"1", "2", "3", "4"};
		for (uint8_t i = 0; i < 4; i++) {
			events_[i].set_cue(canvas_handler->draw_text(0, 0, i, i * 6, 1, Font::Type::Font5x8, text[i].c_str(), 1));
			events_[i].set_time(1000);
		}

		events_[4].set_cue(canvas_handler->clear(0, 0));
		events_[4].set_time(1000);

		Show* show = maestro_controller_->get_maestro()->set_show(events_, 5);
		show->set_timing_mode(Show::TimingMode::Relative);
		show->set_looping(true);
	}

	ShowDemo::~ShowDemo() {
		delete[] events_;
	}
}