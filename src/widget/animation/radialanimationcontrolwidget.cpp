#include "radialanimationcontrolwidget.h"
#include "ui_radialanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	RadialAnimationControlWidget::RadialAnimationControlWidget(RadialAnimation* animation, MaestroControlWidget* maestro_control, QWidget *parent) :
		QWidget(parent),
		ui(new Ui::RadialAnimationControlWidget) {
		this->animation_ = animation;
		this->maestro_control_ = maestro_control;
		ui->setupUi(this);
	}

	void RadialAnimationControlWidget::on_resolutionSpinBox_valueChanged(int arg1) {
		maestro_control_->run_cue(maestro_control_->animation_handler->set_radial_options(maestro_control_->get_section_index(), maestro_control_->get_layer_index(), arg1));
	}

	RadialAnimationControlWidget::~RadialAnimationControlWidget() {
		delete ui;
	}
}