#include <QKeyEvent>
#include <QMessageBox>
#include <QModelIndex>
#include <QRegExp>
#include "showcontrolwidget.h"
#include "ui_showcontrolwidget.h"
#include "controller/showcontroller.h"

namespace PixelMaestroStudio {
	ShowControlWidget::ShowControlWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ShowControlWidget) {
		ui->setupUi(this);

		// Capture key presses
		qApp->installEventFilter(this);

		this->maestro_control_widget_ = static_cast<MaestroControlWidget*>(parent);

		show_timer_.start();
	}

	/**
	 * Adds a Cue to the event history list.
	 * @param cue Cue to add.
	 */
	void ShowControlWidget::add_event_to_history(uint8_t *cue) {
		ui->eventHistoryWidget->addItem(cue_interpreter_.interpret_cue(cue));

		// Convert the Cue into an actual byte array, which we'll store in the Event History for later use.
		uint16_t cue_size = maestro_control_widget_->cue_controller_->get_cue_size(cue);
		QVector<uint8_t> cue_vector(cue_size);
		for (uint16_t byte = 0; byte < cue_size; byte++) {
			cue_vector[byte] = cue[byte];
		}

		event_history_.push_back(cue_vector);

		// Remove all but the last XX events
		if (event_history_.size() >= event_history_max_) {
			ui->eventHistoryWidget->takeItem(0);
			event_history_.remove(0);
		}
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool ShowControlWidget::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			// Delete queued events only when when Event queue has focus
			if (watched == ui->eventQueueWidget) {
				if (key_event->key() == Qt::Key_Delete) {
					on_removeEventButton_clicked();
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	void ShowControlWidget::initialize() {
		// Disable controls by default
		set_show_controls_enabled(false);
		ui->advancedLockedLabel->setVisible(false);

		// Setup timer
		show_timer_.setTimerType(Qt::CoarseTimer);
		show_timer_.setInterval(TIMER_INTERVAL_);
		connect(&show_timer_, SIGNAL(timeout()), this, SLOT(timer_refresh()));

		// Initialize ShowController
		if (show_controller_ == nullptr) {
			show_controller_ = new ShowController(maestro_control_widget_->get_maestro_controller());
		}
		else {
			show_controller_->clear();
		}

		on_clearHistoryButton_clicked();
		on_clearQueueButton_clicked();
	}

	/**
	 * Returns whether the user has locked the Maestro for editing.
	 * @return If true, Maestro is locked.
	 */
	bool ShowControlWidget::get_maestro_locked() const {
		return maestro_locked_;
	}

	/**
	 * Adds the selected Event(s) to the Show's Event queue.
	 */
	void ShowControlWidget::on_addEventButton_clicked() {
		for (QModelIndex index : ui->eventHistoryWidget->selectionModel()->selectedIndexes()) {
			Event* event = show_controller_->add_event(
				ui->eventTimeSpinBox->value(),
				(uint8_t*)&event_history_.at(index.row()).at(0)
			);

			ui->eventQueueWidget->addItem(
				locale_.toString(event->get_time()) +
				QString(": ") +
				cue_interpreter_.interpret_cue(event->get_cue())
			);
		}

		maestro_control_widget_->run_cue(
			maestro_control_widget_->show_handler->set_events(
				show_controller_->get_events()->data(),
				show_controller_->get_events()->size(),
				true
			)
		);
	}

	/**
	 * Clears the Event History.
	 */
	void ShowControlWidget::on_clearHistoryButton_clicked() {
		ui->eventHistoryWidget->clear();
		event_history_.clear();
	}

	/**
	 * Removes all Events from the Show's queue.
	 */
	void ShowControlWidget::on_clearQueueButton_clicked() {
		ui->eventQueueWidget->clear();
		show_controller_->get_events()->clear();
		maestro_control_widget_->run_cue(
			maestro_control_widget_->show_handler->set_events(
				nullptr,
				0,
				false
			)
		);
	}

	void ShowControlWidget::on_enableCheckBox_toggled(bool checked) {
		set_show_controls_enabled(checked);

		if (checked) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->maestro_handler->set_show()
			);
		}
		else {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->maestro_handler->remove_show()
			);
		}
	}

	/**
	 * Toggles Event looping.
	 * @param checked If true, events will loop after the Show ends.
	 */
	void ShowControlWidget::on_loopCheckBox_toggled(bool checked) {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->show_handler->set_looping(checked)
		);
	}

	/// Moves the selected Show Event down one spot.
	void ShowControlWidget::on_moveEventDownButton_clicked() {
		int current_row = ui->eventQueueWidget->currentRow();

		if (current_row != ui->eventQueueWidget->count() - 1) {
			show_controller_->move(current_row, current_row	+ 1);
			maestro_control_widget_->run_cue(
				maestro_control_widget_->show_handler->set_events(
					show_controller_->get_events()->data(),
					show_controller_->get_events()->size()
				)
			);

			QListWidgetItem* current_item = ui->eventQueueWidget->takeItem(current_row);
			ui->eventQueueWidget->insertItem(current_row + 1, current_item);
			ui->eventQueueWidget->setCurrentRow(current_row + 1);
		}
	}

	/// Moves the selected Show Event up one spot.
	void ShowControlWidget::on_moveEventUpButton_clicked() {
		int current_row = ui->eventQueueWidget->currentRow();

		if (current_row != 0) {
			show_controller_->move(current_row, current_row - 1);
			maestro_control_widget_->run_cue(
				maestro_control_widget_->show_handler->set_events(
					show_controller_->get_events()->data(),
					show_controller_->get_events()->size()
				)
			);

			QListWidgetItem* current_item = ui->eventQueueWidget->takeItem(current_row);
			ui->eventQueueWidget->insertItem(current_row - 1, current_item);
			ui->eventQueueWidget->setCurrentRow(current_row - 1);
		}
	}

	/**
	 * Removes the selected Event(s) from the Show.
	 */
	void ShowControlWidget::on_removeEventButton_clicked() {
		// Sort selected rows by index before removing them
		QModelIndexList list = ui->eventQueueWidget->selectionModel()->selectedRows();
		qSort(list.begin(), list.end(), qGreater<QModelIndex>());
		for (QModelIndex index : list) {
			show_controller_->remove_event(index.row());
			ui->eventQueueWidget->takeItem(index.row());
		}

		// Re-initialize the Event list
		maestro_control_widget_->run_cue(
			maestro_control_widget_->show_handler->set_events(
				show_controller_->get_events()->data(),
				show_controller_->get_events()->size(),
				true
			)
		);
	}

	/**
	 * Changes the Show's timing mode.
	 * @param index New timing mode.
	 */
	void ShowControlWidget::on_timingModeComboBox_currentIndexChanged(int index) {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->show_handler->set_timing_mode(
				(Show::TimingMode)index
			)
		);

		// Enable loop checkbox if we're in relative mode
		ui->loopCheckBox->setEnabled((Show::TimingMode)index == Show::TimingMode::Relative);
	}

	/**
	 * Updates the UI in the event of a Maestro change.
	 */
	void ShowControlWidget::refresh() {
		Show* show = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_show();
		ui->enableCheckBox->blockSignals(true);
		ui->enableCheckBox->setChecked(show != nullptr);
		set_show_controls_enabled(show != nullptr);
		ui->enableCheckBox->blockSignals(false);

		if (show != nullptr) {
			for (uint16_t i = 0; i < show->get_num_events(); i++) {
				Event* event = &show->get_events()[i];

				// Check for Events before adding them to prevent duplicates
				if (show_controller_->get_event_index(event) < 0) {
					show_controller_->add_event(event->get_time(), event->get_cue());
					ui->eventQueueWidget->addItem(
						locale_.toString(event->get_time()) +
								QString(": ") +
								cue_interpreter_.interpret_cue(event->get_cue())
					);
				}
			}

			ui->timingModeComboBox->blockSignals(true);
			ui->timingModeComboBox->setCurrentIndex((uint8_t)show->get_timing());
			ui->timingModeComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(show->get_looping());
			ui->loopCheckBox->blockSignals(false);
		}
		else {
			ui->timingModeComboBox->blockSignals(true);
			ui->timingModeComboBox->setCurrentIndex(0);
			ui->timingModeComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(0);
			ui->loopCheckBox->blockSignals(false);
		}
	}

	/**
	 * Sets whether the Maestro is locked for editing.
	 * @param locked If true, Cues won't change the Maestro.
	 */
	void ShowControlWidget::set_maestro_locked(bool locked) {
		this->maestro_locked_ = locked;
		ui->advancedLockedLabel->setVisible(locked);
	}

	/**
	 * Updates fields in the widget when the timer fires.
	 */
	void ShowControlWidget::timer_refresh() {
		uint last_time = (uint)maestro_control_widget_->get_maestro_controller()->get_total_elapsed_time();
		ui->absoluteTimeLineEdit->setText(locale_.toString(last_time));

		Show* show = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_show();

		if (show == nullptr) return;

		// Get the last event that ran
		int last_index = -1;
		uint16_t current_index = show->get_current_index();
		if (current_index == 0) {
			if (show->get_looping()) {
				last_index = show->get_num_events() - 1;
			}
		}
		else {
			last_index = show->get_current_index() - 1;
		}

		// If relative mode is enabled, calculate the time since the last Event
		ui->relativeTimeLineEdit->setEnabled(last_index	>= 0);
		if (last_index >= 0) {
			ui->relativeTimeLineEdit->setText(locale_.toString(
				(uint)maestro_control_widget_->get_maestro_controller()->get_total_elapsed_time() - (uint)show->get_last_time()
			));
		}

		// Darken events that have already ran
		for (int i = 0; i < ui->eventQueueWidget->count(); i++) {
			if (current_index > 0 && i < current_index) {
				ui->eventQueueWidget->item(i)->setTextColor(Qt::GlobalColor::darkGray);
			}
			else {
				ui->eventQueueWidget->item(i)->setTextColor(Qt::GlobalColor::white);
			}
		}

		/*
		 * If the last event is different from the one on record, that means an Event ran.
		 * We refresh the UI to account for any Maestro changes.
		 *
		 * WARNING: Whenever the UI refreshes, any widgets that are actively in use are reset.
		 * This makes it impossible to use the editor with Shows that update frequently.
		 * Look into alternate solutions.
		 */
		/*
		if (last_index != this->last_index_) {
			//maestro_control_widget_->refresh_section_settings();
			this->last_index_ = last_index;
		}
		*/
	}

	/**
	 * Toggles whether Show controls are enabled.
	 * @param enabled If true, controls are enabled and usable.
	 */
	void ShowControlWidget::set_show_controls_enabled(bool enabled) {
		ui->advancedSettingsGroupBox->setEnabled(enabled);
		ui->eventsGroupBox->setEnabled(enabled);
	}

	ShowControlWidget::~ShowControlWidget() {
		delete ui;
	}
}
