#include "core/maestro.h"
#include "maestrodrawingarea.h"
#include "dialog/preferencesdialog.h"
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QTimer>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param parent The parent QWidget.
	 * @param maestro_controller The MaestroController rendered by this DrawingArea.
	 */
	MaestroDrawingArea::MaestroDrawingArea(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent) {
		set_maestro_controller(maestro_controller);

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
		return dynamic_cast<SectionDrawingArea*>(drawing_area);
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

		int section_id = maestro_control_widget_->section_control_widget_->get_section_index(section);

		for (uint8_t i = 0; i < section_drawing_areas_.size(); i++) {
			if (i == section_id) {
				int layer_id = maestro_control_widget_->section_control_widget_->get_layer_index(section);

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
	 * Removes a Section drawing areas.
	 * @param Section Pointer to the Section to remove. Leave blank to remove all Sections.
	 */
	void MaestroDrawingArea::remove_section_drawing_area(Section* section) {
		if (section != nullptr) {
			for (uint8_t da_index = 0; da_index < this->section_drawing_areas_.size(); da_index++) {
				SectionDrawingArea* drawing_area = this->section_drawing_areas_.at(da_index).get();
				if (drawing_area->get_section() == section) {
					section_drawing_areas_.removeAt(da_index);
					return;
				}
			}
		}
		else {
			section_drawing_areas_.clear();
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
	 * Sets the MaestroController that this DrawingArea will render.
	 * @param maestro_controller New MaestroController.
	 */
	void MaestroDrawingArea::set_maestro_controller(MaestroController *maestro_controller) {
		this->maestro_controller_ = maestro_controller;
	}

	/**
	 * Redraws the DrawingArea.
	 */
	void MaestroDrawingArea::update() {
		// Checks to see if the active Section is currently highlighted.
		if (maestro_control_widget_ != nullptr && maestro_control_widget_->section_control_widget_->get_active_section() != active_section_) {
			frame_active_section(maestro_control_widget_->section_control_widget_->get_active_section());
		}

		// Update all DrawingAreas
		for (uint16_t i = 0; i < section_drawing_areas_.size(); i++) {
			section_drawing_areas_[i]->update();
		}
	}

	MaestroDrawingArea::~MaestroDrawingArea() { }
}
