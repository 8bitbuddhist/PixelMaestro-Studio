#include "plasmaanimationcontrol.h"
#include "ui_plasmaanimationcontrol.h"

namespace PixelMaestroStudio {
	PlasmaAnimationControl::PlasmaAnimationControl(PlasmaAnimation* animation, MaestroControl* controller, QWidget *parent) :
		QWidget(parent),
		ui(new Ui::PlasmaAnimationControl) {
		ui->setupUi(this);
		this->animation_ = animation;
		this->maestro_control_ = controller;
		ui->resolutionDoubleSpinBox->setValue(animation->get_resolution());
		ui->sizeDoubleSpinBox->setValue(animation->get_size());
	}

	PlasmaAnimationControl::~PlasmaAnimationControl() {
		delete ui;
	}

	void PlasmaAnimationControl::on_resolutionDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_plasma_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), ui->sizeDoubleSpinBox->value(), arg1));
	}

	void PlasmaAnimationControl::on_sizeDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_plasma_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), arg1, ui->resolutionDoubleSpinBox->value()));
	}
}