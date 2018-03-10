/*
 * BlinkDemo - Displays a simple blink animation.
 */

#include <memory>
#include "blinkdemo.h"
#include "colorpresets.h"
#include "controller/maestrocontroller.h"
#include "core/colors.h"
#include "drawingarea/maestrodrawingarea.h"

namespace PixelMaestroStudio {
	BlinkDemo::BlinkDemo(QWidget* parent, MaestroController* maestro_controller) : MaestroDrawingArea(parent, maestro_controller) {
		Section* section = maestro_controller_->set_sections(1, Point(10, 10));

		Animation* animation = section->set_animation(AnimationType::Blink);
		animation->set_palette(&ColorPresets::Colorwheel_Palette);
		animation->set_timer(1000);
	}
}