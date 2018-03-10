#include "animation/radialanimation.h"
#include "canvas/colorcanvas.h"
#include "canvas/fonts/font5x8.h"
#include "colorcanvasdemo.h"
#include "colorpresets.h"

namespace PixelMaestroStudio {
	ColorCanvasDemo::ColorCanvasDemo(QWidget* parent, MaestroController* maestro_controller) : MaestroDrawingArea(parent, maestro_controller) {
		Section* section = maestro_controller_->set_sections(1, Point(80, 80));

		Section::Layer* layer = section->set_layer(Colors::MixMode::Overlay);
		ColorCanvas* canvas = static_cast<ColorCanvas*>(layer->section->set_canvas(CanvasType::ColorCanvas));

		canvas->draw_circle(ColorPresets::Blue, 40, 40, 40, true);
		canvas->draw_circle(ColorPresets::Green, 40, 40, 30, true);
		canvas->draw_circle(ColorPresets::Red, 40, 40, 20, true);

		// Not true black since the Layer mix mode treats black as transparent
		canvas->draw_rect({0, 0, 1}, 9, 36, 62, 9, true);
		canvas->draw_text(ColorPresets::White, 10, 37, new Font5x8(), (char*)"PixelMaestro", 12);
	}
}