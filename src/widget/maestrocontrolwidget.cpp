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
	 * Loads a Cuefile into the Maestro.
	 * @param byte_array Byte array containing the Cuefile.
	 */
	void MaestroControlWidget::load_cuefile(QByteArray byte_array) {
		this->loading_cue_ = true;
		for (int i = 0; i < byte_array.size(); i++) {
			uint8_t byte = (uint8_t)byte_array.at(i);
			if (cue_controller_->read(byte)) {
				run_cue(cue_controller_->get_buffer(), true);
			}
		}
		this->loading_cue_ = false;
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
		// TODO: Indicate that the Cue is unsaved

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
			}

			// Send to serial device controller.
			if (device_control_widget_ != nullptr) {
				device_control_widget_->run_cue(cue, cue_controller_->get_cue_size(cue));
			}
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
		animation_handler = static_cast<AnimationCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::AnimationCueHandler)
		);
		canvas_handler = static_cast<CanvasCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::CanvasCueHandler)
		);
		maestro_handler = static_cast<MaestroCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::MaestroCueHandler)
		);
		section_handler = static_cast<SectionCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::SectionCueHandler)
		);
		show_handler = static_cast<ShowCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::ShowCueHandler)
		);

		// Initialize UI components and controllers
		section_control_widget_->set_active_section(maestro_controller_->get_maestro()->get_section(0));
		section_control_widget_->initialize();
		animation_control_widget_->initialize();
		canvas_control_widget_->initialize();
		refresh_maestro_settings();
	}

	MaestroControlWidget::~MaestroControlWidget() {
		delete ui;
	}
}
