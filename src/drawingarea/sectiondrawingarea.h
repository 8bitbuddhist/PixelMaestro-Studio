#ifndef SECTIONDRAWINGAREA_H
#define SECTIONDRAWINGAREA_H

#include <QFrame>
#include <QMouseEvent>
#include <QSettings>
#include <QWidget>
#include "core/section.h"
#include "drawingarea/maestrodrawingarea.h"

namespace PixelMaestroStudio {
	class MaestroDrawingArea;
	class SectionDrawingArea : public QFrame {
		Q_OBJECT

		public:
			SectionDrawingArea(QWidget* parent, Section* section);
			~SectionDrawingArea();

		protected:
			Section* section_;

			/// tmp_rgb_ converted to QColor
			QColor tmp_color_;

			/// Brush used to paint tmp_color
			QBrush tmp_brush_;

			/// Size and location of the Pixel to draw using tmp_brush
			QRect tmp_rect_;

			/// Colors::RGB output from each Pixel
			Colors::RGB tmp_rgb_;

			void mousePressEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent *event) override;
			void resizeEvent(QResizeEvent *event) override;
			QSize sizeHint() const override;

		private:
			/// The location where the Section will be rendered.
			Point cursor_;

			/// The Section's last recorded size. Used to determine when to resize the output.
			uint32_t last_pixel_count_;

			MaestroDrawingArea* maestro_drawing_area_ = nullptr;

			/// The size of each rendered Pixel.
			uint8_t radius_ = 20;
			/// The amount of space between each Pixel. Gets initialized in resizeEvent().
			uint8_t pad_;

			QSettings settings_;
	};
}

#endif // SECTIONDRAWINGAREA_H