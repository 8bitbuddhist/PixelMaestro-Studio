#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include <QKeyEvent>
#include <QSettings>
#include <QString>
#include "controller/maestrocontroller.h"
#include "core/section.h"
#include "drawingarea/maestrodrawingarea.h"
#include "maestrocontrolwidget.h"
#include "dialog/preferencesdialog.h"
#include "widget/palettecontrolwidget.h"
#include "ui_maestrocontrolwidget.h"
#include "utility/canvasutility.h"
#include "utility.h"
#include "window/mainwindow.h"

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param parent The QWidget containing this controller.
	 */
	MaestroControlWidget::MaestroControlWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MaestroControlWidget) {
		ui->setupUi(this);

		// Capture button key presses
		qApp->installEventFilter(this);

		// Add control tabs
		animation_control_widget_ = QSharedPointer<AnimationControlWidget>(new AnimationControlWidget(this));
		ui->animationTab->findChild<QLayout*>("animationLayout")->addWidget(animation_control_widget_.data());

		canvas_control_widget_ = QSharedPointer<CanvasControlWidget>(new CanvasControlWidget(this));
		ui->canvasTab->findChild<QLayout*>("canvasLayout")->addWidget(canvas_control_widget_.data());

		device_control_widget_ = QSharedPointer<DeviceControlWidget>(new DeviceControlWidget(this));
		ui->deviceTab->findChild<QLayout*>("deviceLayout")->addWidget(device_control_widget_.data());

		section_control_widget_ = QSharedPointer<SectionControlWidget>(new SectionControlWidget(this));
		ui->topLayout->insertWidget(0, section_control_widget_.data());

		show_control_widget_ = QSharedPointer<ShowControlWidget>(new ShowControlWidget(this));
		ui->showTab->findChild<QLayout*>("showLayout")->addWidget(show_control_widget_.data());

		// Initialize Palette-containing subwidgets
		animation_control_widget_->refresh_palettes();
		canvas_control_widget_->refresh_palettes();
	}

	void MaestroControlWidget::edit_palettes(QString palette) {
		PaletteControlWidget palette_control(palette_controller_, palette);
		palette_control.exec();

		// Update Palette-containing subwidgets
		animation_control_widget_->refresh_palettes();
		canvas_control_widget_->refresh_palettes();
	}

	/**
	 * Returns the control widget's MaestroController.
	 * @return MaestroController.
	 */
	MaestroController* MaestroControlWidget::get_maestro_controller() {
		return maestro_controller_;
	}

	/**
	 * Returns whether the Maestro has been modified.
	 * @return If true, the Maestro has been modified.
	 */
	bool MaestroControlWidget::get_maestro_modified() const {
		return modified_;
	}

	/**
	 * Loads a Cuefile into the Maestro.
	 * @param byte_array Byte array containing the Cuefile.
	 */
	void MaestroControlWidget::load_cuefile(const QByteArray& byte_array) {
		/*
		 * To test the Cuefile, we read each byte into a virtual Maestro.
		 * If it runs, we then pass it to the actual Maestro.
		 */
		Maestro virtual_maestro(nullptr, 0);
		virtual_maestro.set_cue_controller(UINT16_MAX);
		for (char byte_char : byte_array) {
			uint8_t byte = static_cast<uint8_t>(byte_char);

			if (virtual_maestro.get_cue_controller().read(byte)) {
				run_cue(virtual_maestro.get_cue_controller().get_buffer(), RunTarget::Local);
			}
		}

		// Refresh settings
		refresh_maestro_settings();
		refresh_section_settings();

		// Refresh Palettes
		animation_control_widget_->refresh_palettes();
		canvas_control_widget_->refresh_palettes();

		set_maestro_modified(false);
	}

	/**
	 * Toggles the Maestro lock.
	 * @param checked If true, the Maestro is locked.
	 */
	void MaestroControlWidget::on_lockButton_toggled(bool checked) {
		show_control_widget_->set_maestro_locked(checked);

		if (checked) {
			QColor highlight_color = qApp->palette().highlight().color();
			ui->lockButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));
		}
		else {
			refresh_maestro_settings();
			refresh_section_settings();

			ui->lockButton->setStyleSheet(QString());
		}

		if (maestro_drawing_area_) {
			dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_)->set_locked(checked);
		}
	}

	/**
	 * Toggles whether the Maestro is running or paused.
	 * @param checked If true, the Maestro is paused.
	 */
	void MaestroControlWidget::on_playPauseButton_toggled(bool checked) {
		if (checked) { // Stop the Maestro
			QColor highlight_color = qApp->palette().highlight().color();
			ui->playPauseButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));

			maestro_controller_->stop();
			run_cue(
				maestro_handler->stop()
			);

			ui->playPauseButton->setToolTip("Start playback");
		}
		else {
			ui->playPauseButton->setStyleSheet(QString());

			maestro_controller_->start();
			run_cue(
				maestro_handler->start()
			);

			ui->playPauseButton->setToolTip("Stop playback");
		}
	}

	/**
	 * Refreshes the UI.
	 */
	void MaestroControlWidget::on_refreshButton_clicked() {
		refresh_maestro_settings();
		refresh_section_settings();

		set_refresh_needed(false);
	}

	void MaestroControlWidget::on_syncButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Sync Timers", "This will sync all timers to the Maestro's current time, which might interrupt Animations, Shows, and Canvases. Are you sure you want to continue?", QMessageBox::Yes | QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			run_cue(
				maestro_handler->sync(maestro_controller_->get_total_elapsed_time())
			);
		}
	}

	/**
	 * Updates the UI to reflect a Section change.
	 */
	void MaestroControlWidget::refresh_section_settings() {
		animation_control_widget_->refresh();
		canvas_control_widget_->refresh();
		section_control_widget_->refresh();

		if (maestro_drawing_area_) {
			dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_)->frame_active_section(section_control_widget_->get_active_section());
		}
	}

	/**
	 * Updates the UI to reflect the Maestro's current settings.
	 */
	void MaestroControlWidget::refresh_maestro_settings() {
		show_control_widget_->refresh();
		device_control_widget_->update_cuefile_size();
	}

	/**
	 * Forwards the specified Cue to the drawing area and/or serial device.
	 * @param cue Cue to perform.
	 * @param run_targets Bitmask of RunTargets specifying where to run the Cue.
	 */
	void MaestroControlWidget::run_cue(uint8_t *cue, int run_targets) {
		show_control_widget_->add_event_to_history(cue);

		// Only run the Cue if the Maestro isn't locked, or the Cue is a Show Cue.
		if (!show_control_widget_->get_maestro_locked() || cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)CueController::Handler::ShowCueHandler) {
			if ((run_targets & RunTarget::Local) == RunTarget::Local) {
				cue_controller_->run(cue);
				set_maestro_modified(true);
			}

			if ((run_targets & RunTarget::Remote) == RunTarget::Remote) {
				// Send to serial device controller
				device_control_widget_->run_cue(cue, cue_controller_->get_cue_size(cue));
			}
		}
	}

	/**
	 * Sets the widget's target MaestroController.
	 * @param maestro_controller New MaestroController.
	 */
	void MaestroControlWidget::set_maestro_controller(MaestroController& maestro_controller) {
		this->maestro_controller_ = &maestro_controller;

		// Get Maestro's Cue Handlers for convenience
		this->cue_controller_ = &maestro_controller.get_maestro().get_cue_controller();
		animation_handler = dynamic_cast<AnimationCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::AnimationCueHandler)
		);
		canvas_handler = dynamic_cast<CanvasCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::CanvasCueHandler)
		);
		maestro_handler = dynamic_cast<MaestroCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::MaestroCueHandler)
		);
		section_handler = dynamic_cast<SectionCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::SectionCueHandler)
		);
		show_handler = dynamic_cast<ShowCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::ShowCueHandler)
		);

		// Check whether the Maestro is currently running. If not, trigger pause button
		ui->playPauseButton->blockSignals(true);
		ui->playPauseButton->setChecked(!maestro_controller.get_running());
		ui->playPauseButton->blockSignals(false);

		// Initialize UI components and controllers
		section_control_widget_->set_active_section(maestro_controller_->get_maestro().get_section(0));
		section_control_widget_->initialize();
		animation_control_widget_->initialize();
		canvas_control_widget_->initialize();
		show_control_widget_->initialize();
		refresh_maestro_settings();

		set_maestro_modified(false);
	}

	/**
	 * Sets whether the Maestro has been modified.
	 * @param modified If true, Maestro has been modified.
	 */
	void MaestroControlWidget::set_maestro_modified(bool modified) {
		this->modified_ = modified;

		// Update MainWindow title
		this->parentWidget()->parentWidget()->setWindowModified(modified);
	}

	/**
	 * Sets whether the Maestro has been modified by an actor other than the user (e.g. by a Show)
	 * @param refresh_needed If true, highlight the refresh button
	 */
	void MaestroControlWidget::set_refresh_needed(bool refresh_needed) {
		if (refresh_needed) {
			QColor highlight_color = qApp->palette().highlight().color();
			ui->refreshButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));
		}
		else {
			ui->refreshButton->setStyleSheet(QString());
		}
	}

	void MaestroControlWidget::toggle_maestro_drawing_area(bool enabled) {
		if (enabled) {
			// Checked: create new DrawingArea
			maestro_drawing_area_ = new MaestroDrawingArea(ui->renderLayout->widget(), *maestro_controller_);
			ui->renderLayout->insertWidget(0, maestro_drawing_area_);
			maestro_controller_->add_drawing_area(*dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_));
			dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_)->set_maestro_control_widget(this);

			// Restore splitter position. If the position isn't saved in the user's settings, default to a 50/50 split
			QSettings settings;
			QByteArray splitter_state = settings.value(PreferencesDialog::splitter_position).toByteArray();
			if (splitter_state.size() > 0) {
				ui->splitter->restoreState(splitter_state);
			}
			else {
				ui->splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
			}
		}
		else {
			// Unchecked
			ui->renderLayout->removeWidget(maestro_drawing_area_);
			maestro_controller_->remove_drawing_area(dynamic_cast<MaestroDrawingArea&>(*maestro_drawing_area_));
			delete maestro_drawing_area_;
		}
	}

	MaestroControlWidget::~MaestroControlWidget() {
		// Save splitter position
		QSettings settings;
		settings.setValue(PreferencesDialog::splitter_position, ui->splitter->saveState());

		delete maestro_drawing_area_;
		qApp->removeEventFilter(this);
		delete ui;
	}
}
