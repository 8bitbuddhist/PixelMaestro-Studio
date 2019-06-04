#include <QKeyEvent>
#include <QListIterator>
#include <QListWidget>
#include <QModelIndex>
#include <QRegExp>
#include <QSettings>
#include "dialog/preferencesdialog.h"
#include "dialog/editeventdialog.h"
#include "showcontrolwidget.h"
#include "ui_showcontrolwidget.h"
#include "controller/showcontroller.h"

namespace PixelMaestroStudio {
	ShowControlWidget::ShowControlWidget(QWidget *parent) :
			QWidget(parent),
			ui(new Ui::ShowControlWidget),
			maestro_control_widget_(*dynamic_cast<MaestroControlWidget*>(parent)) {
		ui->setupUi(this);

		// Capture key presses
		qApp->installEventFilter(this);

		show_timer_.start();

		connect(ui->eventQueueWidget->model(), &QAbstractItemModel::rowsMoved, this, &ShowControlWidget::on_eventQueueWidget_rowsMoved);
	}

	/**
	 * Adds a Cue to the event history list.
	 * @param cue Cue to add.
	 */
	void ShowControlWidget::add_event_to_history(uint8_t *cue) {
		ui->eventHistoryWidget->addItem(CueInterpreter::interpret_cue(cue));

		// Convert the Cue into an actual byte array, which we'll store in the Event History for later use.
		uint32_t cue_size = maestro_control_widget_.cue_controller_->get_cue_size(cue);
		QVector<uint8_t> cue_vector(cue_size);
		for (uint16_t byte = 0; byte < cue_size; byte++) {
			cue_vector[byte] = cue[byte];
		}

		event_history_.push_back(cue_vector);

		// Remove all but the last XX events
		QSettings settings;
		if (event_history_.size() >= settings.value(PreferencesDialog::event_history_max).toInt()) {
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
			QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
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
			show_controller_ = new ShowController();
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

	void ShowControlWidget::move_event(int current_index, int target_index) {
		QListWidgetItem* item = ui->eventQueueWidget->takeItem(current_index);

		show_controller_->move(current_index, target_index);
		ui->eventQueueWidget->insertItem(target_index, item);
		ui->eventQueueWidget->setCurrentRow(target_index);
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
				CueInterpreter::interpret_cue(event->get_cue())
			);
		}

		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_events(
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
		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_events(
				nullptr,
				0,
				false
			)
		);
	}

	void ShowControlWidget::on_enableCheckBox_toggled(bool checked) {
		set_show_controls_enabled(checked);

		if (checked) {
			maestro_control_widget_.run_cue(
				maestro_control_widget_.maestro_handler->set_show()
			);
		}
		else {
			maestro_control_widget_.run_cue(
				maestro_control_widget_.maestro_handler->remove_show()
			);
		}
	}

	void ShowControlWidget::on_eventQueueWidget_itemDoubleClicked(QListWidgetItem *item) {
		int row = ui->eventQueueWidget->row(item);

		// Make sure the row is in the bounds of the Event queue
		if (row > -1 && row < show_controller_->get_events()->size()) {
			Event* event = &show_controller_->get_events()->data()[row];
			EditEventDialog dialog(*event, this);
			if (dialog.exec() == QDialog::Accepted) {
				// Update the UI and actual Show Event
				item->setText(locale_.toString(event->get_time()) +
							  QString(": ") +
							  CueInterpreter::interpret_cue(event->get_cue()));

				maestro_control_widget_.run_cue(
					maestro_control_widget_.show_handler->set_events(
						show_controller_->get_events()->data(),
						show_controller_->get_events()->size()
					)
				);
			}
		}
	}

	void ShowControlWidget::on_eventQueueWidget_rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row) {
		if (parent == destination) {
			for	(int i = start; i < end; i++) {
				int dest_row = row;
				if (dest_row >= show_controller_->get_events()->size()) {
					dest_row = show_controller_->get_events()->size() - 1;
				}
				show_controller_->move(start, dest_row);
			}

			maestro_control_widget_.run_cue(
				maestro_control_widget_.show_handler->set_events(
					show_controller_->get_events()->data(),
					show_controller_->get_events()->size()
				)
			);
		}
	}

	/**
	 * Toggles Event looping.
	 * @param checked If true, events will loop after the Show ends.
	 */
	void ShowControlWidget::on_loopCheckBox_toggled(bool checked) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_looping(checked)
		);
	}

	/// Moves the selected Event(s) down one spot.
	void ShowControlWidget::on_moveEventDownButton_clicked() {
		// FIXME: Breaks when triggered multiple times sequentially with multi-select enabled
		QModelIndexList list = ui->eventQueueWidget->selectionModel()->selectedIndexes();
		for (int i = list.size() - 1; i >= 0; i--) {
			int current_index = list.at(i).row();
			if (current_index != ui->eventQueueWidget->count() - 1) {
				move_event(current_index, current_index + 1);
			}
		}

		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_events(
				show_controller_->get_events()->data(),
				show_controller_->get_events()->size()
			)
		);
	}

	/// Moves the selected Event(s) up one spot.
	void ShowControlWidget::on_moveEventUpButton_clicked() {
		QModelIndexList list = ui->eventQueueWidget->selectionModel()->selectedIndexes();
		for (int i = 0; i < list.size(); i++) {
			int current_index = list.at(i).row();
			if (current_index != 0) {
				move_event(current_index, current_index - 1);
			}
		}

		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_events(
				show_controller_->get_events()->data(),
				show_controller_->get_events()->size()
			)
		);
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
		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_events(
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
		maestro_control_widget_.run_cue(
			maestro_control_widget_.show_handler->set_timing_mode(
				(Show::TimingMode)index
			)
		);

		// Enable loop checkbox if we're in relative mode
		ui->loopCheckBox->setEnabled((Show::TimingMode)index == Show::TimingMode::Relative);
		ui->relativeTimeLineEdit->setEnabled((Show::TimingMode)index == Show::TimingMode::Relative);
	}

	/**
	 * Updates the UI in the event of a Maestro change.
	 */
	void ShowControlWidget::refresh() {
		Show* show = maestro_control_widget_.get_maestro_controller()->get_maestro().get_show();
		ui->enableCheckBox->blockSignals(true);
		ui->enableCheckBox->setChecked(show != nullptr);
		set_show_controls_enabled(show != nullptr);
		ui->enableCheckBox->blockSignals(false);

		if (show != nullptr) {
			for (uint16_t i = 0; i < show->get_num_events(); i++) {
				Event* event = show->get_event_at_index(i);
				if (event != nullptr) {
					// Check for Events before adding them to prevent duplicates
					if (show_controller_->get_event_index(event) < 0) {
						show_controller_->add_event(event->get_time(), event->get_cue());
						ui->eventQueueWidget->addItem(
							locale_.toString(event->get_time()) +
									QString(": ") +
									CueInterpreter::interpret_cue(event->get_cue())
						);
					}
				}
			}

			ui->timingModeComboBox->blockSignals(true);
			ui->timingModeComboBox->setCurrentIndex((uint8_t)show->get_timing());
			ui->timingModeComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(show->get_looping());
			ui->loopCheckBox->blockSignals(false);

			ui->relativeTimeLineEdit->blockSignals(true);
			ui->relativeTimeLineEdit->setEnabled(show->get_timing() == Show::TimingMode::Relative);
			ui->relativeTimeLineEdit->blockSignals(false);
		}
		else {
			ui->timingModeComboBox->blockSignals(true);
			ui->timingModeComboBox->setCurrentIndex(0);
			ui->timingModeComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(false);
			ui->loopCheckBox->blockSignals(false);

			ui->relativeTimeLineEdit->blockSignals(true);
			ui->relativeTimeLineEdit->setEnabled(false);
			ui->relativeTimeLineEdit->blockSignals(false);
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
		// Update 'Absolute Time' text box
		uint last_time = (uint)maestro_control_widget_.get_maestro_controller()->get_total_elapsed_time();
		ui->absoluteTimeLineEdit->setText(locale_.toString(last_time));

		Show* show = maestro_control_widget_.get_maestro_controller()->get_maestro().get_show();
		if (show == nullptr) return;

		// If relative mode is enabled, calculate the time since the last Event
		bool relative_time_enabled = show->get_timing() == Show::TimingMode::Relative;
		if (relative_time_enabled) {
			uint relative_time = maestro_control_widget_.get_maestro_controller()->get_total_elapsed_time() - show->get_last_time();
			ui->relativeTimeLineEdit->setText(locale_.toString(relative_time));
		}

		// Get the last event that ran, and if it differs from the Show's current index, update the Event Queue
		if (last_event_time_ != show->get_last_time()) {
			last_event_time_ = show->get_last_time();

			// Darken events that have already ran
			for (int i = 0; i < ui->eventQueueWidget->count(); i++) {
				if (i < show->get_current_index()) {
					ui->eventQueueWidget->item(i)->setTextColor(Qt::GlobalColor::darkGray);
				}
				else {
					ui->eventQueueWidget->item(i)->setTextColor(Qt::GlobalColor::white);
				}
			}

			// If live update triggers are enabled, send the last Event's queue to the DeviceControlWidget to be sent to remote devices
			QSettings settings;
			if (settings.value(PreferencesDialog::events_trigger_device_updates, false).toBool()) {
				Event* event = show->get_event_at_index(show->get_current_index());
				if (event != nullptr) {
					CueController* cue_controller = &maestro_control_widget_.get_maestro_controller()->get_maestro().get_cue_controller();
					maestro_control_widget_.device_control_widget_->run_cue(event->get_cue(), cue_controller->get_cue_size(event->get_cue()));
				}
			}

			maestro_control_widget_.set_refresh_needed(true);
		}
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
		qApp->removeEventFilter(this);
		delete show_controller_;
		delete ui;
	}
}
