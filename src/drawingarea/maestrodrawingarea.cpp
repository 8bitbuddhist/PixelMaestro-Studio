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

		section_layout_ = new QHBoxLayout(this);
	}

	/**
	 * Adds a new SectionDrawingArea to the widget.
	 * @param section Section to draw.
	 * @return New SectionDrawingArea.
	 */
	SectionDrawingArea* MaestroDrawingArea::add_section_drawing_area(Section* section) {
		section_drawing_areas_.push_back(
			QSharedPointer<SectionDrawingArea>(
				new SectionDrawingArea(this, section)
			)
		);
		QWidget* drawing_area = section_drawing_areas_.last().data();
		section_layout_->addWidget(drawing_area);
		return static_cast<SectionDrawingArea*>(drawing_area);
	}

	/**
	 * Returns this DrawingArea's MaestroControlWidget (if applicable).
	 * @return DrawingArea's controlling widget.
	 */
	MaestroControlWidget* MaestroDrawingArea::get_maestro_control_widget() const {
		return maestro_control_widget_;
	}

	/**
	 * Changes the active Section, which highlights the appropriate SectionDrawingArea.
	 * @param section Active Section.
	 */
	void MaestroDrawingArea::frame_active_section(Section *section) {
		active_section_ = section;

		if (maestro_control_widget_ == nullptr) return;

		int section_id = maestro_control_widget_->get_section_index(section);

		for (uint8_t i = 0; i < section_drawing_areas_.size(); i++) {
			if (i == section_id) {
				int layer_id = maestro_control_widget_->get_layer_index(section);

				if (layer_id > 0) {
					section_drawing_areas_[i]->draw_frame(SectionDrawingArea::FrameType::Layer);
				}
				else {
					section_drawing_areas_[i]->draw_frame(SectionDrawingArea::FrameType::Section);
				}
			}
			else {
				section_drawing_areas_[i]->draw_frame(SectionDrawingArea::FrameType::Inactive);
			}
		}
	}

	/**
	 * Sets the MaestroControlWidget used to control this DrawingArea.
	 * @param widget Controlling MaestroControlWidget.
	 */
	void MaestroDrawingArea::set_maestro_control_widget(MaestroControlWidget *widget) {
		this->maestro_control_widget_ = widget;
	}

	/**
	 * Redraws the DrawingArea.
	 */
	void MaestroDrawingArea::update() {
		// Checks to see if the active Section is currently highlighted.
		if (maestro_control_widget_ != nullptr && maestro_control_widget_->get_active_section() != active_section_) {
			frame_active_section(maestro_control_widget_->get_active_section());
		}

		// Updates all DrawingAreas
		for (uint16_t i = 0; i < section_drawing_areas_.size(); i++) {
			section_drawing_areas_[i]->update();
		}
	}

	MaestroDrawingArea::~MaestroDrawingArea() {
		maestro_controller_->remove_drawing_area(this);
	}
}