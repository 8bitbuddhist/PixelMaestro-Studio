#ifndef ANIMATIONCONTROLWIDGET_H
#define ANIMATIONCONTROLWIDGET_H

#include <QSharedPointer>
#include <QWidget>
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class AnimationControlWidget;
}

namespace PixelMaestroStudio {
	class AnimationControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit AnimationControlWidget(QWidget *parent = nullptr);
			~AnimationControlWidget();
			MaestroControlWidget* maestro_control_widget = nullptr;

			void initialize();
			void refresh();
			void refresh_palettes();

		private slots:
			void on_typeComboBox_currentIndexChanged(int index);
			void on_paletteComboBox_currentIndexChanged(int index);
			void on_fadeCheckBox_toggled(bool checked);
			void on_reverseCheckBox_toggled(bool checked);
			void on_orientationComboBox_currentIndexChanged(int index);
			void on_cycleIntervalSpinBox_editingFinished();
			void on_delayIntervalSpinBox_editingFinished();

			void on_paletteEditButton_clicked();

			void on_cycleIntervalSlider_valueChanged(int value);

			void on_delayIntervalSlider_valueChanged(int value);

		private:
			QSharedPointer<QWidget> advanced_controls_widget_;
			Ui::AnimationControlWidget *ui;

			void set_advanced_controls(Animation* animation);
			void set_animation_timer();
			void set_controls_enabled(bool);
	};
}

#endif // ANIMATIONCONTROLWIDGET_H
