#include "sparkleanimationcontrolwidget.h"
#include "ui_sparkleanimationcontrol.h"

namespace PixelMaestroStudio {
	SparkleAnimationControlWidget::SparkleAnimationControlWidget(SparkleAnimation* animation, MaestroControlWidget* controller, QWidget *parent) :
		QWidget(parent),
		ui(new Ui::SparkleAnimationControlWidget) {
		ui->setupUi(this);
		this->animation_ = animation;
		this->maestro_control_ = controller;
		ui->thresholdSpinBox->setValue(animation->get_threshold());
	}

	SparkleAnimationControlWidget::~SparkleAnimationControlWidget() {
		delete ui;
	}

	void SparkleAnimationControlWidget::on_thresholdSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_sparkle_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), arg1));
	}
}