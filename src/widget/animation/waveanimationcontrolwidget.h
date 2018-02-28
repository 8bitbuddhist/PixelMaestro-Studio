#ifndef WAVEANIMATIONCONTROLWIDGET_H
#define WAVEANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/waveanimation.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class WaveAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class WaveAnimationControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit WaveAnimationControlWidget(WaveAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent = 0);
			~WaveAnimationControlWidget();

		private slots:
			void on_skewSpinBox_editingFinished();

			void on_mirrorCheckBox_stateChanged(int arg1);

		private:
			WaveAnimation* animation_;
			MaestroControlWidget* maestro_control_widget_;
			Ui::WaveAnimationControlWidget *ui;
	};
}

#endif // WAVEANIMATIONCONTROLWIDGET_H
