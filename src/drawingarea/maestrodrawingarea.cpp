#include "core/maestro.h"
#include "maestrodrawingarea.h"
#include "window/preferencesdialog.h"
#include <QElapsedTimer>
#include <QSettings>
#include <QTimer>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param parent The parent QWidget.
	 * @param maestro_controller The MaestroController rendered by this DrawingArea.
	 */
	MaestroDrawingArea::MaestroDrawingArea(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent) {
		this->maestro_controller_ = maestro_controller;

		// Let the DrawingArea be managed by the MaestroController
		maestro_controller->add_drawing_area(this);
	}

	MaestroController* MaestroDrawingArea::get_maestro_controller() {
		return maestro_controller_;
	}

	void MaestroDrawingArea::refresh() {
		update();
	}

	MaestroDrawingArea::~MaestroDrawingArea() {
		maestro_controller_->remove_drawing_area(this);
	}
}