#ifndef SECTIONDRAWINGAREA_H
#define SECTIONDRAWINGAREA_H

#include <QFrame>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
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

			void mouseMoveEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent *event) override;
			void resizeEvent(QResizeEvent *event) override;
			QSize sizeHint() const override;

		private:
			/// Tracks whether this is the active Section.
			bool is_active_ = false;

			/// The location where the Section will be rendered.
			Point cursor_;

			/// The Section's last recorded size. Used to determine when to resize the output.
			uint32_t last_pixel_count_ = 0;

			/// The parent MaestroDrawingArea.
			MaestroDrawingArea* maestro_drawing_area_ = nullptr;

			/// The size of each rendered Pixel.
			uint8_t radius_ = 20;

			/// The amount of space between each Pixel. Gets initialized in resizeEvent().
			uint8_t pad_ = 0;

			QSettings settings_;

			Point map_cursor_to_pixel(const QPoint cursor);
	};
}

#endif // SECTIONDRAWINGAREA_H