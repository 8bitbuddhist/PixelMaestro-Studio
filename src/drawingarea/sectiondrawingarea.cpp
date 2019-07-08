#include <QPainter>
#include <QSettings>
#include "sectiondrawingarea.h"
#include "dialog/preferencesdialog.h"

namespace PixelMaestroStudio {
	SectionDrawingArea::SectionDrawingArea(QWidget* parent, Section& section, uint8_t section_id) : QFrame(parent), maestro_drawing_area_(*dynamic_cast<MaestroDrawingArea*>(parent)), section_(section) {
		this->section_id_ = section_id;

		// Enable mouse tracking
		this->setMouseTracking(true);

		// Display a frame if none is set and default to inactive.
		if (this->frameStyle() == QFrame::Plain) {
			this->setFrameStyle(QFrame::Box | QFrame::Plain);
			this->draw_frame(FrameType::Inactive);
		}

		QSettings settings;
		this->pixel_shape_ = settings.value(PreferencesDialog::pixel_shape, 1).toInt();
	}

	/*
	 * If this is the active Section, highlight the frame, otherwise dim the frame.
	 * Only applies if maestro_drawing_area_::maestro_control_widget_ is set.
	 */
	/**
	 * Draws a frame around the widget.
	 * Whether the Section is active determines the color of the frame.
	 * @param type They type of Section and whether it's active or inactive.
	 */
	void SectionDrawingArea::draw_frame(FrameType type) {
		/*
		 * Set the frame color.
		 * The current Section has a white frame, Layers have a light gray frame, and inactive Sections have dark frames.
		 */
		switch (type) {
			case FrameType::Inactive:
				this->setStyleSheet("color: #333333;");
				break;
			case FrameType::Layer:
				this->setStyleSheet("color: #808080");
				break;
			case FrameType::Section:
				this->setStyleSheet("color: #FFFFFF;");
				break;
		}
	}

	/**
	 * Returns the underlying Section.
	 * @return Section rendered by this DrawingArea.
	 */
	Section& SectionDrawingArea::get_section() const {
		return this->section_;
	}

	/**
	 * Translates the mouse cursor to a PixelMaestro Pixel.
	 * @param cursor Mouse cursor coordinates.
	 * @return Pixel coordinate.
	 */
	Point SectionDrawingArea::map_cursor_to_pixel(const QPoint cursor) {
		uint16_t x = cursor.x() - section_cursor_.x;
		uint16_t y = cursor.y() - section_cursor_.y;

		if (radius_ > 0) {
			x /= radius_;
			y /= radius_;
			if (x >= section_.get_dimensions().x) {
				x = section_.get_dimensions().x - 1;
			}
			if (y >= section_.get_dimensions().y) {
				y = section_.get_dimensions().y - 1;
			}

			return Point(x, y);
		}
		else {
			return Point(0, 0);
		}
	}

	/**
	 * Handles mouse clicks.
	 * When Canvas is enabled, users can draw onto the Section using the mouse.
	 * Left-click activates and right-click deactiviates.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::mouseMoveEvent(QMouseEvent *event) {
		// Store the cursor position for Canvas editing
		cursor_pos_ = event->pos();

		if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton) {
			Canvas* canvas = section_.get_canvas();
			if (canvas != nullptr) {
				Point pixel = map_cursor_to_pixel(cursor_pos_);

				// If there's a MaestroControlWidget, use run_cue instead of modifying the Canvas directly.
				MaestroControlWidget* widget = maestro_drawing_area_.get_maestro_control_widget();
				if (widget != nullptr) {
					// Set the cursor location in the MaestroControlWidget
					widget->canvas_control_widget_->set_canvas_origin(pixel);

					// Check to see if paint mode is enabled.
					if (widget->canvas_control_widget_->get_painting_enabled()) {
						if (event->buttons() == Qt::LeftButton) {
							widget->run_cue(
								widget->canvas_handler->draw_point(
									widget->section_control_widget_->get_section_index(),
									widget->section_control_widget_->get_layer_index(),
									canvas->get_current_frame_index(),
									widget->canvas_control_widget_->get_selected_color_index(),
									pixel.x,
									pixel.y)
							);
						}
						else if (event->buttons() == Qt::RightButton) {
							widget->run_cue(
								widget->canvas_handler->erase_point(
									widget->section_control_widget_->get_section_index(),
									widget->section_control_widget_->get_layer_index(),
									canvas->get_current_frame_index(),
									pixel.x,
									pixel.y)
							);
						}
					}

					if (widget->canvas_control_widget_->get_replace_enabled()) {
						if (event->buttons() == Qt::LeftButton) {
							widget->canvas_control_widget_->on_drawButton_clicked();
						}
					}
				}
			}
		}
	}

	/**
	 * Triggers a mouse move event on mouse press.
	 * @param event Event parameters.
	 */
	void SectionDrawingArea::mousePressEvent(QMouseEvent *event) {
		// Sets the current Section as the active Section on left click
		if (event->buttons() == Qt::LeftButton) {
			Section& active_section = maestro_drawing_area_.get_maestro_control_widget()->section_control_widget_->get_active_section();
			if (&active_section != &this->section_) {
				maestro_drawing_area_.get_maestro_control_widget()->section_control_widget_->set_active_section(&this->section_);
			}
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
		if (last_pixel_count_ != section_.get_dimensions().size()) {
			resizeEvent(nullptr);
			last_pixel_count_ = section_.get_dimensions().size();
		}

		/*
		 * Draw each Pixel.
		 * For each Pixel, translate it's RGB color into a QColor.
		 * Then, depending on the user's preferenes, draw it as either a circle or a square.
		 */
		for (uint16_t row = 0; row < section_.get_dimensions().y; row++) {
			for (uint16_t column = 0; column < section_.get_dimensions().x; column++) {
				Colors::RGB rgb = section_.get_pixel_color(column, row);
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
				rect.setRect(section_cursor_.x + (column * radius_), section_cursor_.y + (row * radius_), radius_, radius_);
				painter.setBrush(brush);

				// Set Pen style.
				// If Canvas is enabled, draw a light border around the Pixel if the cursor is over it
				painter.setPen(Qt::PenStyle::NoPen);
				if (section_.get_canvas() != nullptr) {
					Point pixel_pos = map_cursor_to_pixel(cursor_pos_);
					if (pixel_pos.x == column && pixel_pos.y == row) {
						painter.setPen(Qt::PenStyle::SolidLine);
					}
				}

				/*
				 * Determine which shape to draw.
				 * If none is set, default to "Square"
				 */
				switch (pixel_shape_) {
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
		uint16_t max_pixel_width = static_cast<uint16_t>(widget_size.width() / section_.get_dimensions().x);
		uint16_t max_pixel_height = static_cast<uint16_t>(widget_size.height() / section_.get_dimensions().y);

		// Find the smaller dimension
		if (max_pixel_width < max_pixel_height) {
			radius_ = max_pixel_width;
		}
		else {
			radius_ = max_pixel_height;
		}

		// Sets the Section's starting point so that it's aligned horizontally and vertically.
		section_cursor_.x = (widget_size.width() - (section_.get_dimensions().x * radius_)) / 2;
		section_cursor_.y = (widget_size.height() - (section_.get_dimensions().y * radius_)) / 2;
	}
}
