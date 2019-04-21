#include "radialanimationcontrolwidget.h"
#include "ui_radialanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	RadialAnimationControlWidget::RadialAnimationControlWidget(RadialAnimation& animation, MaestroControlWidget& maestro_control_widget, QWidget *parent) :
			QWidget(parent),
			ui(new Ui::RadialAnimationControlWidget),
			animation_(animation),
			maestro_control_widget_(maestro_control_widget) {
		ui->setupUi(this);
	}

	void RadialAnimationControlWidget::on_resolutionSpinBox_valueChanged(int arg1) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.animation_handler->set_radial_options(
				maestro_control_widget_.section_control_widget_->get_section_index(),
				maestro_control_widget_.section_control_widget_->get_layer_index(),
				arg1
			)
		);
	}

	RadialAnimationControlWidget::~RadialAnimationControlWidget() {
		delete ui;
	}
}
