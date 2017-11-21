#include <QListWidget>
#include <QMessageBox>
#include <QVariant>
#include "cue/cuecontroller.h"
#include "showcontrol.h"
#include "ui_showcontrol.h"

ShowControl::ShowControl(ShowController* show_controller, CueController* cue_controller, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ShowControl) {
	this->show_controller_ = show_controller;
	this->cue_controller_ = cue_controller;
	this->maestro_control_ = static_cast<MaestroControl*>(parent);
	ui->setupUi(this);

	// Initialize timer and update it kinda frequently-ish
	timer_.setTimerType(Qt::CoarseTimer);
	connect(&timer_, SIGNAL(timeout()), this, SLOT(refresh_maestro_last_time()));
	timer_.start(250);

	// Redraw Event list
	ui->eventListWidget->clear();
	for (Event event : show_controller_->get_events()) {
		ui->eventListWidget->addItem(locale_.toString(event.get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event.get_cue()));
	}
}

void ShowControl::add_cue_to_history(uint8_t *cue) {
	ui->eventHistoryWidget->addItem(cue_interpreter_.interpret_cue(cue));
	history_.push_back(cue);

	// Start removing older items
	if (history_.size() >= 10) {
		ui->eventHistoryWidget->takeItem(0);
		history_.remove(0);
	}
}

void ShowControl::on_addEventButton_clicked() {
	// Add selected Cues to the Show
	for (QModelIndex index : ui->eventHistoryWidget->selectionModel()->selectedIndexes()) {
		// TODO: Verify that the Cue isn't already in the Event list
		Event* event = show_controller_->add_event(ui->eventTimeSpinBox->value(), history_.at(index.row()));
		ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));
	}
}

void ShowControl::on_lockMaestroCheckBox_toggled(bool checked) {
	maestro_control_->enable_show_edit_mode(checked);
}

/// Redraws the Event list.
void ShowControl::refresh_event_list() {
	ui->eventListWidget->clear();
	for (Event event : show_controller_->get_events()) {
		ui->eventListWidget->addItem(cue_interpreter_.interpret_cue(event.get_cue()));
	}
}

/**
 * Displays the Maestro's last run time.
 */
void ShowControl::refresh_maestro_last_time() {
	ui->runtimeLineEdit->setText(locale_.toString(cue_controller_->get_maestro()->get_timing()->get_last_time()));
}

ShowControl::~ShowControl() {
	delete ui;
}
