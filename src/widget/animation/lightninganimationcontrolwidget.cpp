#include "lightninganimationcontrolwidget.h"
#include "ui_lightninganimationcontrolwidget.h"

namespace PixelMaestroStudio {
	LightningAnimationControlWidget::LightningAnimationControlWidget(LightningAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent) : QWidget(parent), ui(new Ui::LightningAnimationControlWidget) {
		ui->setupUi(this);

		this->animation_ = animation;
		this->maestro_control_widget_ = maestro_control_widget;
		ui->boltCountSpinBox->setValue(animation->get_bolt_count());
		ui->forkChanceSpinBox->setValue(animation->get_fork_chance());
		ui->driftSpinBox->setValue(animation->get_drift());
	}

	LightningAnimationControlWidget::~LightningAnimationControlWidget() {
		delete ui;
	}

	void LightningAnimationControlWidget::on_forkChanceSpinBox_valueChanged(int arg1) {
		maestro_control_widget_->run_cue(maestro_control_widget_->animation_handler->set_lightning_options(maestro_control_widget_->get_section_index(), maestro_control_widget_->get_layer_index(), ui->boltCountSpinBox->value(), ui->driftSpinBox->value(), (uint8_t)arg1));
	}

	void LightningAnimationControlWidget::on_driftSpinBox_valueChanged(int arg1) {
		maestro_control_widget_->run_cue(maestro_control_widget_->animation_handler->set_lightning_options(maestro_control_widget_->get_section_index(), maestro_control_widget_->get_layer_index(), ui->boltCountSpinBox->value(), arg1, ui->forkChanceSpinBox->value()));
	}

	void LightningAnimationControlWidget::on_boltCountSpinBox_valueChanged(int arg1) {
		maestro_control_widget_->run_cue(maestro_control_widget_->animation_handler->set_lightning_options(maestro_control_widget_->get_section_index(), maestro_control_widget_->get_layer_index(), arg1, ui->driftSpinBox->value(), ui->forkChanceSpinBox->value()));
	}
}