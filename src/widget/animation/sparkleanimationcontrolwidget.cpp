#include "sparkleanimationcontrolwidget.h"
#include "ui_sparkleanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	SparkleAnimationControlWidget::SparkleAnimationControlWidget(SparkleAnimation& animation, MaestroControlWidget& maestro_control_widget, QWidget *parent) :
			QWidget(parent),
			ui(new Ui::SparkleAnimationControlWidget),
			animation_(animation),
			maestro_control_widget_(maestro_control_widget) {
		ui->setupUi(this);

		ui->thresholdSpinBox->setValue(animation.get_threshold());
	}

	SparkleAnimationControlWidget::~SparkleAnimationControlWidget() {
		delete ui;
	}

	void SparkleAnimationControlWidget::on_thresholdSpinBox_editingFinished() {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.animation_handler->set_sparkle_options(
				maestro_control_widget_.section_control_widget_->get_section_index(),
				maestro_control_widget_.section_control_widget_->get_layer_index(),
				ui->thresholdSpinBox->value()
			)
		);
	}
}
