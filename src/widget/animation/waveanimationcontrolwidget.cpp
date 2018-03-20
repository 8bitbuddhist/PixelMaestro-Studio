#include "waveanimationcontrolwidget.h"
#include "ui_waveanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	WaveAnimationControlWidget::WaveAnimationControlWidget(WaveAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent) : QWidget(parent), ui(new Ui::WaveAnimationControlWidget) {
		ui->setupUi(this);

		this->animation_ = animation;
		this->maestro_control_widget_ = maestro_control_widget;

		ui->mirrorCheckBox->blockSignals(true);
		ui->mirrorCheckBox->setChecked(animation->get_mirror());
		ui->mirrorCheckBox->blockSignals(false);

		ui->skewSpinBox->blockSignals(true);
		ui->skewSpinBox->setValue(animation->get_skew());
		ui->skewSpinBox->blockSignals(false);
	}

	void WaveAnimationControlWidget::on_mirrorCheckBox_stateChanged(int arg1) {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->animation_handler->set_wave_options(
				maestro_control_widget_->get_section_index(),
				maestro_control_widget_->get_layer_index(),
				(arg1 > 0),
				ui->skewSpinBox->value()
			)
		);
	}

	void WaveAnimationControlWidget::on_skewSpinBox_editingFinished() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->animation_handler->set_wave_options(
				maestro_control_widget_->get_section_index(),
				maestro_control_widget_->get_layer_index(),
				ui->mirrorCheckBox->isChecked(),
				ui->skewSpinBox->value())
		);
	}

	WaveAnimationControlWidget::~WaveAnimationControlWidget() {
		delete ui;
	}
}
