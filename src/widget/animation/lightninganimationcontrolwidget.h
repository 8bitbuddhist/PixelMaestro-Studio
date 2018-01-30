#ifndef LIGHTNINGANIMATIONCONTROLWIDGET_H
#define LIGHTNINGANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/lightninganimation.h"
#include "widget/maestrocontrolwidget.h"

using namespace PixelMaestro;

namespace Ui {
	class LightningAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class LightningAnimationControlWidget : public QWidget {
			Q_OBJECT

		public:
			explicit LightningAnimationControlWidget(LightningAnimation* animation, MaestroControlWidget* controller, QWidget *parent = 0);
			~LightningAnimationControlWidget();

		private slots:
			void on_forkChanceSpinBox_valueChanged(int arg1);

			void on_spreadDownSpinBox_valueChanged(int arg1);

			void on_spreadUpSpinBox_valueChanged(int arg1);

			void on_boltCountSpinBox_valueChanged(int arg1);

		private:
			LightningAnimation* animation_;
			MaestroControlWidget* maestro_control_;
			Ui::LightningAnimationControlWidget *ui;
	};
}

#endif // LIGHTNINGANIMATIONCONTROLWIDGET_H
