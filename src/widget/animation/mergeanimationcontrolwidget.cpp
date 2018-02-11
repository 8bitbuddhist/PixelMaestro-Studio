#include "mergeanimationcontrolwidget.h"
#include "ui_mergeanimationcontrolwidget.h"

namespace PixelMaestroStudio {
	MergeAnimationControlWidget::MergeAnimationControlWidget(MergeAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent) :	QWidget(parent), ui(new Ui::MergeAnimationControlWidget) {
		ui->setupUi(this);

		this->animation_ = animation;
		this->maestro_control_widget_ = maestro_control_widget;

		ui->skewSpinBox->blockSignals(true);
		ui->skewSpinBox->setValue(animation->get_skew());
		ui->skewSpinBox->blockSignals(false);
	}

	void PixelMaestroStudio::MergeAnimationControlWidget::on_skewSpinBox_editingFinished() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->animation_handler->set_merge_options(
				maestro_control_widget_->get_section_index(),
				maestro_control_widget_->get_layer_index(),
				ui->skewSpinBox->value())
		);
	}

	MergeAnimationControlWidget::~MergeAnimationControlWidget() {
		delete ui;
	}
}