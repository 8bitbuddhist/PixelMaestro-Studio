#include <QPainter>
#include "sectiondrawingarea.h"
#include "dialog/preferencesdialog.h"

namespace PixelMaestroStudio {
	SectionDrawingArea::SectionDrawingArea(QWidget* parent, Section* section) : QFrame(parent) {
		this->maestro_drawing_area_ = static_cast<MaestroDrawingArea*>(parent);
		this->section_ = section;

		// Enable mouse tracking
		this->setMouseTracking(true);

		// Default to dim frame
		this->setStyleSheet("color: #505050;");
	}

	/**
	 * Translates the mouse cursor to a PixelMaestro Pixel.
	 * @param cursor Mouse cursor coordinates.
	 * @return Pixel coordinate.
	 */
	Point SectionDrawingArea::map_cursor_to_pixel(const QPoint cursor) {
		uint16_t x = (cursor.x() - cursor_.x) / radius_;
		uint16_t y = (cursor.y() - cursor_.y) / radius_;
		return Point(x, y);
	}

	/**
	 * Handles mouse clicks.
	 * When Canvas is enabled, users can draw onto the Section using the mouse.
	 * Left-click activates and right-click deactiviates.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::mouseMoveEvent(QMouseEvent *event) {
		if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton) {
			Canvas* canvas = section_->get_canvas();
			if (canvas != nullptr) {
				Point pixel = map_cursor_to_pixel(event->pos());

				// If there's a MaestroControlWidget, use run_cue instead of modifying the Canvas directly.
				MaestroControlWidget* widget = maestro_drawing_area_->get_maestro_control_widget();
				if (widget != nullptr) {
					if (event->buttons() == Qt::LeftButton) {
						widget->run_cue(
							widget->canvas_handler->activate(
								widget->get_section_index(),
								widget->get_layer_index(),
								pixel.x,
								pixel.y)
						);
					}
					else if (event->buttons() == Qt::RightButton) {
						widget->run_cue(
							widget->canvas_handler->deactivate(
								widget->get_section_index(),
								widget->get_layer_index(),
								pixel.x,
								pixel.y)
						);
					}
				}
				else {
					if (event->buttons() == Qt::LeftButton) {
						canvas->activate(pixel.x, pixel.y);
					}
					else if (event->buttons() == Qt::RightButton) {
						canvas->deactivate(pixel.x, pixel.y);
					}
				}
			}
		}
	}

	/**
	 * Changes the current Pixel on mouse click.
	 * Otherwise, the Pixel would only change on click + drag.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::mousePressEvent(QMouseEvent *event) {
		// If there's a MaestroControlWidget and this isn't the curent active Section, activate it.
		MaestroControlWidget* widget = maestro_drawing_area_->get_maestro_control_widget();
		if (widget != nullptr && event->buttons() == Qt::LeftButton) {
			widget->set_active_section(this->section_);
		}
		mouseMoveEvent(event);
	}

	/**
	 * Handles drawing the Section.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::paintEvent(QPaintEvent *event) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);

		/*
		 * Check to see if the Section's changed sizes.
		 * If so, recalculate the drawing area's dimensions.
		 */
		if (last_pixel_count_ != section_->get_dimensions()->size()) {
			resizeEvent(nullptr);
			last_pixel_count_ = section_->get_dimensions()->size();
		}

		/*
		 * If this is the active Section, highlight the frame, otherwise dim the frame.
		 * Only applies if maestro_drawing_area_::maestro_control_widget_ is set.
		 */
		if (maestro_drawing_area_->get_maestro_control_widget() != nullptr) {

			// Display a frame if none is set.
			if (this->frameStyle() == QFrame::Plain) {
				this->setFrameStyle(QFrame::Box | QFrame::Plain);
			}

			/*
			 * Set the frame color.
			 * The current Section has a white border, and other Sections have gray borders.
			 */
			bool active = (this->section_ == maestro_drawing_area_->get_maestro_control_widget()->get_active_section());
			if (active != this->is_active_) {
				if (active) {
					this->setStyleSheet("color: #FFFFFF;");
					this->is_active_ = true;
				}
				else {
					this->setStyleSheet("color: #505050;");
					this->is_active_ = false;
				}
			}
		}

		/*
		 * Draw each Pixel.
		 * For each Pixel, translate it's RGB color into a QColor.
		 * Then, depending on the user's preferenes, draw it as either a circle or a square.
		 */
		for (uint16_t row = 0; row < section_->get_dimensions()->y; row++) {
			for (uint16_t column = 0; column < section_->get_dimensions()->x; column++) {
				Colors::RGB rgb = section_->get_pixel_color(column, row);
				QColor qcolor;
				QBrush brush;
				QRect rect;

				qcolor.setRgb(rgb.r, rgb.g, rgb.b);
				brush.setColor(qcolor);
				brush.setStyle(Qt::BrushStyle::SolidPattern);

				/*
				 * Draw the Pixel.
				 * First, calculate the bounds of the Pixel.
				 * Then, set the color of the pen to the color of the Pixel.
				 * Finally, draw the Pixel to the screen.
				 */
				rect.setRect(cursor_.x + (column * pad_), cursor_.y + (row * pad_), radius_, radius_);
				painter.setBrush(brush);
				painter.setPen(Qt::PenStyle::NoPen);

				/*
				 * Determine which shape to draw.
				 * If none is set, default to "Square"
				 */
				switch (settings_.value(PreferencesDialog::pixel_shape, 1).toInt()) {
					case 0:	// Circle
						painter.drawEllipse(rect);
						break;
					case 1:	// Rect
						painter.drawRect(rect);
						break;
				}
			}
		}

		QFrame::paintEvent(event);
	}

	/**
	 * Handles resizing the widget.
	 * On each resize, the widget recalculates the optimal size of each Pixel so that the entire Section fits.
	 * It also centers the Section.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::resizeEvent(QResizeEvent *event) {
		QSize widget_size = this->size();

		// Next, get the max size of each Pixel via the window size.
		uint16_t max_pixel_width = widget_size.width() / section_->get_dimensions()->x;
		uint16_t max_pixel_height = widget_size.height() / section_->get_dimensions()->y;

		// Find the smaller dimension
		if (max_pixel_width < max_pixel_height) {
			radius_ = max_pixel_width;
		}
		else {
			radius_ = max_pixel_height;
		}

		pad_ = radius_;

		// Sets the Section's starting point so that it's aligned horizontally and vertically.
		cursor_.x = (widget_size.width() - (section_->get_dimensions()->x * radius_)) / 2;
		cursor_.y = (widget_size.height() - (section_->get_dimensions()->y * radius_)) / 2;

		// Calculate the actual size of each Pixel
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
	 * Used mostly for alignment.
	 * @return Section size.
	 */
	QSize SectionDrawingArea::sizeHint() const {
		/*return QSize(
			section_->get_dimensions()->x * radius_,
			section_->get_dimensions()->y * radius_
		);
		*/
		return QFrame::sizeHint();
	}

	SectionDrawingArea::~SectionDrawingArea() {}
}