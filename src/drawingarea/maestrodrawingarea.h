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
			MaestroControlWidget* get_maestro_control_widget();
			void set_maestro_control_widget(MaestroControlWidget* widget);

		public slots:
			void refresh();

		protected:
			MaestroControlWidget* maestro_control_widget_ = nullptr;

			/// The MaestroController managed by this DrawingArea.
			MaestroController* maestro_controller_;

			QVector<QSharedPointer<SectionDrawingArea>> section_drawing_areas_;

			QSettings settings_;
	};
}

#endif // MAESTRODRAWINGAREA_H
