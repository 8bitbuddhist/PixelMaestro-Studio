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
		ui->sectionTab->findChild<QLayout*>("sectionLayout")->addWidget(section_control_widget_.data());

		show_control_widget_ = QSharedPointer<ShowControlWidget>(new ShowControlWidget(this));
		ui->showTab->findChild<QLayout*>("showLayout")->addWidget(show_control_widget_.data());

		// Initialize Palette-containing subwidgets
		animation_control_widget_->refresh_palettes();
		canvas_control_widget_->refresh_palettes();
	}

	void MaestroControlWidget::edit_palettes(QString palette) {
		PaletteControlWidget palette_control(&palette_controller_, palette);
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

			if (virtual_maestro.get_cue_controller()->read(byte)) {
				run_cue(virtual_maestro.get_cue_controller()->get_buffer(), false);
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

		/*
		 * Toggle the lock icon displayed in each Tab.
		 * Display it for all Tabs except for the Show and Device Tabs.
		 * Also display it for the Advanced Settings Group Box.
		 */
		QList<QWidget*> tabs = ui->tabWidget->findChildren<QWidget*>(QRegExp("^((?!show)(?!device).)*Tab$"));

		if (checked) {
			for(QWidget* tab : tabs) {
				ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(tab), QIcon(":/icon_lock.png"));
			}
		}
		else { // Refresh Maestro settings when unlocked
			for(QWidget* tab : tabs) {
				ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(tab), QIcon());
			}
			refresh_maestro_settings();
			refresh_section_settings();
		}
	}

	/**
	 * Toggles whether the Maestro is running or paused.
	 * @param checked If true, the Maestro is paused.
	 */
	void MaestroControlWidget::on_playPauseButton_toggled(bool checked) {
		if (checked) { // Stop the Maestro
			maestro_controller_->stop();
			run_cue(
				maestro_handler->stop()
			);

			ui->playPauseButton->setToolTip("Start playback");
		}
		else {
			maestro_controller_->start();
			run_cue(
				maestro_handler->start()
			);

			ui->playPauseButton->setToolTip("Stop playback");
		}
	}

	void MaestroControlWidget::on_syncButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Sync Timers", "This will sync all timers to the Maestro's current time, which might interrupt Animations, Shows, and Canvases. Are you sure you want to continue?", QMessageBox::Yes | QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			maestro_controller_->get_maestro()->sync(maestro_controller_->get_total_elapsed_time());
		}
	}

	/**
	 * Updates the UI to reflect a Section change.
	 */
	void MaestroControlWidget::refresh_section_settings() {
		animation_control_widget_->refresh();
		canvas_control_widget_->refresh();
		section_control_widget_->refresh();
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
	 * @param serial_only If true, don't run the Cue on the local Maestro.
	 */
	void MaestroControlWidget::run_cue(uint8_t *cue, bool remote_only) {
		show_control_widget_->add_event_to_history(cue);

		/*
		 * Only run the Cue if:
		 *	1) The Maestro isn't locked
		 *	2) The Maestro is locked, but the Cue is a Show Cue
		 */
		if (!show_control_widget_->get_maestro_locked() ||
			(show_control_widget_->get_maestro_locked() && cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)CueController::Handler::ShowCueHandler)) {
			if (!remote_only) {
				cue_controller_->run(cue);
				set_maestro_modified(true);
			}

			// Send to serial device controller
			device_control_widget_->run_cue(cue, cue_controller_->get_cue_size(cue));
		}
	}

	/**
	 * Sets the widget's target MaestroController.
	 * @param maestro_controller New MaestroController.
	 */
	void MaestroControlWidget::set_maestro_controller(MaestroController *maestro_controller) {
		this->maestro_controller_ = maestro_controller;

		// Get Maestro's Cue Handlers for convenience
		cue_controller_ = maestro_controller_->get_maestro()->get_cue_controller();
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
		if (!maestro_controller->get_running()) {
			ui->playPauseButton->setChecked(true);
		}

		// Initialize UI components and controllers
		section_control_widget_->set_active_section(maestro_controller_->get_maestro()->get_section(0));
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
		this->parentWidget()->parentWidget()->parentWidget()->setWindowModified(modified);
	}

	MaestroControlWidget::~MaestroControlWidget() {
		delete ui;
	}
}
