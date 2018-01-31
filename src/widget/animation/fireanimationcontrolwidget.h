#ifndef FIREANIMATIONCONTROLWIDGET_H
#define FIREANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/fireanimation.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class FireAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class FireAnimationControlWidget : public QWidget {
			Q_OBJECT

		public:
			explicit FireAnimationControlWidget(FireAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent = 0);
			~FireAnimationControlWidget();

		private slots:
			void on_multiplierSpinBox_valueChanged(int arg1);

			void on_divisorSpinBox_valueChanged(int arg1);

		private:
			FireAnimation* animation_;
			MaestroControlWidget* maestro_control_widget_;
			Ui::FireAnimationControlWidget *ui;
	};
}

#endif // FIREANIMATIONCONTROLWIDGET_H
