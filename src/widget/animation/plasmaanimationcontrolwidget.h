#ifndef PLASMAANIMATIONCONTROLWIDGET_H
#define PLASMAANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/plasmaanimation.h"
#include "widget/maestrocontrolwidget.h"

using namespace PixelMaestro;

namespace Ui {
	class PlasmaAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class PlasmaAnimationControlWidget : public QWidget {
			Q_OBJECT

		public:
			explicit PlasmaAnimationControlWidget(PlasmaAnimation& animation, MaestroControlWidget& maestro_control_widget, QWidget *parent = 0);
			~PlasmaAnimationControlWidget();

		private slots:
			void on_sizeDoubleSpinBox_valueChanged(double arg1);

			void on_resolutionDoubleSpinBox_valueChanged(double arg1);

		private:
			PlasmaAnimation& animation_;
			MaestroControlWidget& maestro_control_widget_;
			Ui::PlasmaAnimationControlWidget *ui;
	};
}

#endif // PLASMAANIMATIONCONTROLWIDGET_H
