#include "fireanimationcontrolwidget.h"
#include "ui_fireanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	FireAnimationControlWidget::FireAnimationControlWidget(FireAnimation& animation, MaestroControlWidget& maestro_control_widget, QWidget* parent) :
			QWidget(parent),
			ui(new Ui::FireAnimationControlWidget),
			animation_(animation),
			maestro_control_widget_(maestro_control_widget) {
		ui->setupUi(this);

		ui->multiplierSpinBox->blockSignals(true);
		ui->multiplierSpinBox->setValue(animation.get_multiplier());
		ui->multiplierSpinBox->blockSignals(false);
	}

	FireAnimationControlWidget::~FireAnimationControlWidget() {
		delete ui;
	}

	void FireAnimationControlWidget::on_multiplierSpinBox_valueChanged(int arg1) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.animation_handler->set_fire_options(
				maestro_control_widget_.section_control_widget_->get_section_index(),
				maestro_control_widget_.section_control_widget_->get_layer_index(),
				arg1
			)
		);
	}
}
