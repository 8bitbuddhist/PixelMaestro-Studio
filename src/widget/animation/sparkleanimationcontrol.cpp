#include "sparkleanimationcontrol.h"
#include "ui_sparkleanimationcontrol.h"

SparkleAnimationControl::SparkleAnimationControl(SparkleAnimation* animation, MaestroControl* controller, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SparkleAnimationControl) {
	ui->setupUi(this);
	this->animation_ = animation;
	this->maestro_control_ = controller;
	ui->thresholdSpinBox->setValue(animation->get_threshold());
}

SparkleAnimationControl::~SparkleAnimationControl() {
	delete ui;
}

void SparkleAnimationControl::on_thresholdSpinBox_valueChanged(int arg1) {
	maestro_control_->execute_cue(maestro_control_->animation_handler->set_sparkle_options(maestro_control_->get_section_index(), maestro_control_->get_overlay_index(), arg1));
}
