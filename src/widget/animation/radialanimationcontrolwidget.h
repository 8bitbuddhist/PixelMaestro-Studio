#ifndef RADIALANIMATIONCONTROLWIDGET_H
#define RADIALANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/radialanimation.h"
#include "widget/maestrocontrolwidget.h"

using namespace PixelMaestro;

namespace Ui {
	class RadialAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class RadialAnimationControlWidget : public QWidget {
			Q_OBJECT

		public:
			explicit RadialAnimationControlWidget(RadialAnimation* animation, MaestroControlWidget* maestro_control, QWidget *parent = 0);
			~RadialAnimationControlWidget();

		private slots:
			void on_resolutionSpinBox_valueChanged(int arg1);

		private:
			RadialAnimation* animation_;
			MaestroControlWidget* maestro_control_;
			Ui::RadialAnimationControlWidget *ui;
	};
}

#endif // RADIALANIMATIONCONTROLWIDGET_H
