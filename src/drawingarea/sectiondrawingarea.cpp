#include <QPainter>
#include "sectiondrawingarea.h"
#include "dialog/preferencesdialog.h"

namespace PixelMaestroStudio {
	SectionDrawingArea::SectionDrawingArea(QWidget* parent, Section* section) : QWidget(parent) {
		this->maestro_drawing_area_ = (MaestroDrawingArea*)parent;
		this->section_ = section;
	}

	void SectionDrawingArea::mousePressEvent(QMouseEvent *event) {
		MaestroControlWidget* widget = maestro_drawing_area_->get_maestro_control_widget();
		if (widget != nullptr && event->buttons() == Qt::LeftButton) {
			// FIXME: Update Section selection combobox as well
			widget->set_active_section(this->section_);
		}
	}

	void SectionDrawingArea::paintEvent(QPaintEvent *event) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);

		if (last_pixel_count_ != section_->get_dimensions()->size()) {
			resizeEvent(nullptr);
			last_pixel_count_ = section_->get_dimensions()->size();
		}

		for (uint16_t row = 0; row < section_->get_dimensions()->y; row++) {
			for (uint16_t column = 0; column < section_->get_dimensions()->x; column++) {
				tmp_rgb_ = section_->get_pixel_color(column, row);
				tmp_color_.setRgb(tmp_rgb_.r, tmp_rgb_.g, tmp_rgb_.b);
				tmp_brush_.setColor(tmp_color_);
				tmp_brush_.setStyle(Qt::BrushStyle::SolidPattern);

				/*
				 * Draw the Pixel.
				 * First, calculate the bounds of the Pixel.
				 * Then, set the color of the pen to the color of the Pixel.
				 * Finally, draw the Pixel to the screen.
				 */
				tmp_rect_.setRect(column * pad_, row * pad_, radius_, radius_);
				painter.setBrush(tmp_brush_);
				painter.setPen(Qt::PenStyle::NoPen);

				/*
				 * Determine which shape to draw.
				 * If none is set, default to "Square"
				 */
				switch (settings_.value(PreferencesDialog::pixel_shape, 1).toInt()) {
					case 0:	// Circle
						painter.drawEllipse(tmp_rect_);
						break;
					case 1:	// Rect
						painter.drawRect(tmp_rect_);
						break;
				}
			}
		}
	}

	void SectionDrawingArea::resizeEvent(QResizeEvent *event) {
		QSize widget_size = this->size();

		// Next, get the max size of each Pixel via the window size.
		uint16_t max_width = widget_size.width() / section_->get_dimensions()->x;
		uint16_t max_height = widget_size.height() / section_->get_dimensions()->y;

		// Find the smaller dimension
		if (max_width < max_height) {
			radius_ = max_width;
		}
		else {
			radius_ = max_height;
		}

		pad_ = radius_;

		// Finally, calculate the radius using the Settings dialog
		switch (settings_.value(PreferencesDialog::pixel_padding).toInt()) {
			case 1:	// Small
				radius_ *= 0.8;
				break;
			case 2:	// Medium
				radius_ *= 0.6;
				break;
			case 3:	// Large
				radius_ *= 0.4;
				break;
		}
	}

	/**
	 * Sets the size of the Widget to the size of the Section output.
	 * Used to provide horizontal alignmet.
	 * @return Section size.
	 */
	QSize SectionDrawingArea::sizeHint() const {
		return QSize(
			section_->get_dimensions()->x * radius_,
			section_->get_dimensions()->y * radius_
		);
	}

	SectionDrawingArea::~SectionDrawingArea() {}
}