#include "fireanimationcontrolwidget.h"
#include "ui_fireanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	FireAnimationControlWidget::FireAnimationControlWidget(FireAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget* parent) : QWidget(parent), ui(new Ui::FireAnimationControlWidget) {
		ui->setupUi(this);

		this->animation_ = animation;
		this->maestro_control_widget_ = maestro_control_widget;

		ui->divisorSpinBox->blockSignals(true);
		ui->multiplierSpinBox->blockSignals(true);
		ui->divisorSpinBox->setValue(animation->get_divisor());
		ui->multiplierSpinBox->setValue(animation->get_multiplier());
		ui->divisorSpinBox->blockSignals(false);
		ui->multiplierSpinBox->blockSignals(false);
	}

	FireAnimationControlWidget::~FireAnimationControlWidget() {
		delete ui;
	}

	void PixelMaestroStudio::FireAnimationControlWidget::on_divisorSpinBox_valueChanged(int arg1) {
		maestro_control_widget_->run_cue(maestro_control_widget_->animation_handler->set_fire_options(maestro_control_widget_->get_section_index(), maestro_control_widget_->get_layer_index(), ui->multiplierSpinBox->value(), arg1));
	}

	void FireAnimationControlWidget::on_multiplierSpinBox_valueChanged(int arg1) {
		maestro_control_widget_->run_cue(maestro_control_widget_->animation_handler->set_fire_options(maestro_control_widget_->get_section_index(), maestro_control_widget_->get_layer_index(), arg1, ui->divisorSpinBox->value()));
	}
}
