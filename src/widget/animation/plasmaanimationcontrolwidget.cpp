#include "plasmaanimationcontrolwidget.h"
#include "ui_plasmaanimationcontrol.h"

namespace PixelMaestroStudio {
	PlasmaAnimationControlWidget::PlasmaAnimationControlWidget(PlasmaAnimation* animation, MaestroControlWidget* controller, QWidget *parent) :
		QWidget(parent),
		ui(new Ui::PlasmaAnimationControlWidget) {
		ui->setupUi(this);
		this->animation_ = animation;
		this->maestro_control_ = controller;
		ui->resolutionDoubleSpinBox->setValue(animation->get_resolution());
		ui->sizeDoubleSpinBox->setValue(animation->get_size());
	}

	PlasmaAnimationControlWidget::~PlasmaAnimationControlWidget() {
		delete ui;
	}

	void PlasmaAnimationControlWidget::on_resolutionDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_plasma_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), ui->sizeDoubleSpinBox->value(), arg1));
	}

	void PlasmaAnimationControlWidget::on_sizeDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_plasma_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), arg1, ui->resolutionDoubleSpinBox->value()));
	}
}