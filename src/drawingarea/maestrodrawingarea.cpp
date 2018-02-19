#include "core/maestro.h"
#include "maestrodrawingarea.h"
#include "dialog/preferencesdialog.h"
#include <QElapsedTimer>
#include <QHBoxLayout>
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

		QHBoxLayout* layout	= new QHBoxLayout(this);
		for (uint8_t section = 0; section < maestro_controller->get_maestro()->get_num_sections(); section++) {
			section_drawing_areas_.push_back(
				QSharedPointer<SectionDrawingArea>(
					new SectionDrawingArea(this, maestro_controller->get_maestro()->get_section(section))
				)
			);
			QWidget* drawing_area = section_drawing_areas_[section].data();
			layout->addWidget(drawing_area);
		}
	}

	MaestroControlWidget* MaestroDrawingArea::get_maestro_control_widget() {
		return maestro_control_widget_;
	}

	MaestroController* MaestroDrawingArea::get_maestro_controller() {
		return maestro_controller_;
	}

	void MaestroDrawingArea::refresh() {
		for (uint16_t i = 0; i < section_drawing_areas_.size(); i++) {
			section_drawing_areas_[i]->update();
		}
	}

	void MaestroDrawingArea::set_maestro_control_widget(MaestroControlWidget *widget) {
		this->maestro_control_widget_ = widget;
	}

	MaestroDrawingArea::~MaestroDrawingArea() {
		maestro_controller_->remove_drawing_area(this);
	}
}