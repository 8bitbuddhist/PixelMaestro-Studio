#include "lightninganimationcontrolwidget.h"
#include "ui_lightninganimationcontrol.h"

namespace PixelMaestroStudio {
	LightningAnimationControlWidget::LightningAnimationControlWidget(LightningAnimation* animation, MaestroControlWidget* controller, QWidget *parent) :	QWidget(parent), ui(new Ui::LightningAnimationControlWidget) {
		ui->setupUi(this);
		this->animation_ = animation;
		this->maestro_control_ = controller;
		ui->boltCountSpinBox->setValue(animation->get_bolt_count());
		ui->forkChanceSpinBox->setValue(animation->get_fork_chance());
		ui->spreadDownSpinBox->setValue(animation->get_down_threshold());
		ui->spreadUpSpinBox->setValue(animation->get_up_threshold());
	}

	LightningAnimationControlWidget::~LightningAnimationControlWidget() {
		delete ui;
	}

	void LightningAnimationControlWidget::on_forkChanceSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_lightning_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), ui->boltCountSpinBox->value(), ui->spreadDownSpinBox->value(), ui->spreadUpSpinBox->value(), (uint8_t)arg1));
	}

	void LightningAnimationControlWidget::on_spreadDownSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_lightning_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), ui->boltCountSpinBox->value(), arg1, ui->spreadUpSpinBox->value(), ui->forkChanceSpinBox->value()));
	}

	void LightningAnimationControlWidget::on_spreadUpSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_lightning_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), ui->boltCountSpinBox->value(), ui->spreadDownSpinBox->value(), arg1, ui->forkChanceSpinBox->value()));
	}

	void LightningAnimationControlWidget::on_boltCountSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_lightning_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), arg1, ui->spreadDownSpinBox->value(), ui->spreadUpSpinBox->value(), ui->forkChanceSpinBox->value()));
	}
}