#include <QImageReader>
#include "animatedcanvasdemo.h"
#include "canvas/colorcanvas.h"
#include "utility/canvasutility.h"

AnimatedCanvasDemo::AnimatedCanvasDemo(QWidget* parent, MaestroController* maestro_controller) : SimpleDrawingArea(parent, maestro_controller) {
	// Load image
	QImageReader gif(":/resources/nyan.gif", "GIF");
	Point canvas_size(200, 200);
	gif.setScaledSize(QSize(canvas_size.x, canvas_size.y));

	Section* section = maestro_controller_->set_sections(1, canvas_size);
	Canvas* canvas = section->set_canvas(CanvasType::ColorCanvas, gif.imageCount());
	canvas->set_frame_timing(gif.nextImageDelay());
	CanvasUtility::load_image(QString(":/resources/nyan.gif"), canvas);
}
