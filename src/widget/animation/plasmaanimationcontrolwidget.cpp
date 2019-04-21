#include "plasmaanimationcontrolwidget.h"
#include "ui_plasmaanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	PlasmaAnimationControlWidget::PlasmaAnimationControlWidget(PlasmaAnimation& animation, MaestroControlWidget& maestro_control_widget, QWidget *parent) :
			QWidget(parent),
			ui(new Ui::PlasmaAnimationControlWidget),
			animation_(animation),
			maestro_control_widget_(maestro_control_widget) {
		ui->setupUi(this);

		ui->resolutionDoubleSpinBox->setValue(animation.get_resolution());
		ui->sizeDoubleSpinBox->setValue(animation.get_size());
	}

	PlasmaAnimationControlWidget::~PlasmaAnimationControlWidget() {
		delete ui;
	}

	void PlasmaAnimationControlWidget::on_resolutionDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.animation_handler->set_plasma_options(
				maestro_control_widget_.section_control_widget_->get_section_index(),
				maestro_control_widget_.section_control_widget_->get_layer_index(),
				ui->sizeDoubleSpinBox->value(), arg1
			)
		);
	}

	void PlasmaAnimationControlWidget::on_sizeDoubleSpinBox_valueChanged(double arg1) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.animation_handler->set_plasma_options(
				maestro_control_widget_.section_control_widget_->get_section_index(),
				maestro_control_widget_.section_control_widget_->get_layer_index(),
				arg1,
				ui->resolutionDoubleSpinBox->value()
			)
		);
	}
}
