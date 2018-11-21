/*
 * MaestroDrawingArea - Renders output from a MaestroController to a DrawingArea. Forms the basis for other DrawingAreas.
 */

#ifndef MAESTRODRAWINGAREA_H
#define MAESTRODRAWINGAREA_H

#include "controller/maestrocontroller.h"
#include "core/colors.h"
#include "core/maestro.h"
#include "core/pixel.h"
#include "core/section.h"
#include "maestrodrawingarea.h"
#include "sectiondrawingarea.h"
#include "widget/maestrocontrolwidget.h"
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QTimer>
#include <QWidget>
#include <vector>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class MaestroController;
	class MaestroControlWidget;
	class SectionDrawingArea;

	class MaestroDrawingArea : public QWidget {
		Q_OBJECT

		public:
			MaestroDrawingArea(QWidget* parent, MaestroController* maestro_controller);
			~MaestroDrawingArea();
			SectionDrawingArea* add_section_drawing_area(Section* section);
			MaestroControlWidget* get_maestro_control_widget() const;
			void frame_active_section(Section* section);
			void remove_section_drawing_area(Section* section = nullptr);
			void set_maestro_control_widget(MaestroControlWidget* widget);
			void set_maestro_controller(MaestroController* maestro_controller);

		public slots:
			void update();

		protected:
			/// The MaestroControlWidget controlling this DrawingArea (if applicable).
			MaestroControlWidget* maestro_control_widget_ = nullptr;

			/// The MaestroController managed by this DrawingArea.
			MaestroController* maestro_controller_ = nullptr;

			/// The SectionDrawingAreas managed by this DrawingArea
			QVector<QSharedPointer<SectionDrawingArea>> section_drawing_areas_;

		private:
			/// Layout containing SectionDrawingAreas.
			QHBoxLayout* section_layout_ = nullptr;

			/// Tracks the active Section in order to provide highlighting.
			Section* active_section_ = nullptr;
	};
}

#endif // MAESTRODRAWINGAREA_H
