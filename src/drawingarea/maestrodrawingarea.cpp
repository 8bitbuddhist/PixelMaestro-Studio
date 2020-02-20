#include <QApplication>
#include "core/maestro.h"
#include "maestrodrawingarea.h"
#include "dialog/preferencesdialog.h"
#include <QElapsedTimer>
#include <QGridLayout>
#include <QTimer>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param parent The parent QWidget.
	 * @param maestro_controller The MaestroController rendered by this DrawingArea.
	 */
	MaestroDrawingArea::MaestroDrawingArea(QWidget* parent, MaestroController& maestro_controller) : QFrame(parent), maestro_controller_(maestro_controller) {
		section_layout_ = new QGridLayout(this);

		// Hide frame by default
		this->setFrameStyle(QFrame::Box | QFrame::Plain);
		this->setStyleSheet("color: #333333;");
	}

	/**
	 * Adds a new SectionDrawingArea to the widget.
	 * @param section Section to draw.
	 * @return New SectionDrawingArea.
	 */
	SectionDrawingArea* MaestroDrawingArea::add_section_drawing_area(Section& section, const uint8_t section_id) {
		section_drawing_areas_.push_back(
			QSharedPointer<SectionDrawingArea>(
				new SectionDrawingArea(this, section, section_id)
			)
		);
		QWidget* drawing_area = section_drawing_areas_.last().data();

		QSettings settings;
		int grid_width = settings.value(PreferencesDialog::num_sections_per_row, 1).toInt();
		int row_num = section_id / grid_width;
		int column_num = section_id % grid_width;
		section_layout_->addWidget(drawing_area, row_num, column_num);
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
	void MaestroDrawingArea::frame_active_section(Section& section) {
		if (maestro_control_widget_ == nullptr) return;

		int section_id = maestro_control_widget_->section_control_widget_->get_section_index(section);

		for (uint8_t i = 0; i < section_drawing_areas_.size(); i++) {
			if (i == section_id) {
				section_drawing_areas_[i]->draw_frame(SectionDrawingArea::FrameType::Active);
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
	void MaestroDrawingArea::remove_section_drawing_areas() {
		section_drawing_areas_.clear();
	}

	void MaestroDrawingArea::set_locked(bool locked) {
		if (locked) {
			QColor highlight_color = qApp->palette().highlight().color();
			this->setStyleSheet(QString("color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));
		}
		else {
			this->setStyleSheet("color: #333333;");
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
		// Update all DrawingAreas
		for (uint16_t i = 0; i < section_drawing_areas_.size(); i++) {
			section_drawing_areas_[i]->update();
		}
	}
}
