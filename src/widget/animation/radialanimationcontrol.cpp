#include "radialanimationcontrol.h"
#include "ui_radialanimationcontrol.h"

RadialAnimationControl::RadialAnimationControl(RadialAnimation* animation, MaestroControl* maestro_control, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RadialAnimationControl) {
	this->animation_ = animation;
	this->maestro_control_ = maestro_control;
	ui->setupUi(this);
}

void RadialAnimationControl::on_resolutionSpinBox_valueChanged(int arg1) {
	maestro_control_->execute_cue(maestro_control_->animation_handler->set_radial_options(maestro_control_->get_section_index(), maestro_control_->get_overlay_index(), arg1));
}

RadialAnimationControl::~RadialAnimationControl() {
	delete ui;
}
