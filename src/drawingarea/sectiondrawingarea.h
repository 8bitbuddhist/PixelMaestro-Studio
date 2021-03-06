#ifndef SECTIONDRAWINGAREA_H
#define SECTIONDRAWINGAREA_H

#include <QFrame>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QResizeEvent>
#include <QWidget>
#include "core/section.h"
#include "drawingarea/maestrodrawingarea.h"

namespace PixelMaestroStudio {
	class MaestroDrawingArea;
	class SectionDrawingArea : public QFrame {
		Q_OBJECT

		public:
			enum class FrameType : uint8_t {
				Inactive,
				Active
			};

			SectionDrawingArea(QWidget* parent, Section& section, uint8_t section_id);
			~SectionDrawingArea() = default;
			void draw_frame(FrameType type);
			Section& get_section() const;

		protected:
			Section& section_;

			void mouseMoveEvent(QMouseEvent* event) override;
			void mousePressEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent *event) override;
			void resizeEvent(QResizeEvent *event) override;

		private:
			/// The last location of the mouse cursor.
			QPoint cursor_pos_;

			/// The Section's last recorded size. Used to determine when to resize the output.
			uint32_t last_pixel_count_ = 0;

			/// The parent MaestroDrawingArea.
			MaestroDrawingArea& maestro_drawing_area_;

			/// The shape that each Pixel is drawn in. Defaults to square.
			int pixel_shape_ = 1;

			/// The size of each rendered Pixel.
			uint16_t radius_ = 20;

			/// The location where the Section will be rendered.
			Point section_cursor_;

			/// Index of the Section in the Maestro.
			uint8_t section_id_;

			Point map_cursor_to_pixel(const QPoint cursor);
	};
}

#endif // SECTIONDRAWINGAREA_H
