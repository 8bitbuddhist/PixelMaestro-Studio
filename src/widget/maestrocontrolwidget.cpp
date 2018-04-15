#include <QColorDialog>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include <QKeyEvent>
#include <QSettings>
#include <QString>
#include "animation/fireanimation.h"
#include "animation/fireanimationcontrolwidget.h"
#include "animation/lightninganimation.h"
#include "animation/lightninganimationcontrolwidget.h"
#include "animation/plasmaanimation.h"
#include "animation/plasmaanimationcontrolwidget.h"
#include "animation/radialanimation.h"
#include "animation/radialanimationcontrolwidget.h"
#include "animation/sparkleanimation.h"
#include "animation/sparkleanimationcontrolwidget.h"
#include "animation/waveanimation.h"
#include "animation/waveanimationcontrolwidget.h"
#include "colorpresets.h"
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
	 * @param maestro_controller The MaestroController being controlled.
	 */
	MaestroControlWidget::MaestroControlWidget(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent), ui(new Ui::MaestroControlWidget) {
		ui->setupUi(this);

		// Capture button key presses
		qApp->installEventFilter(this);

		this->maestro_controller_ = maestro_controller;

		QSettings settings;

		// Open separate window if necessary
		if (settings.value(PreferencesDialog::separate_window_option, false).toBool() == true) {
			drawing_area_dialog_ = std::unique_ptr<MaestroDrawingAreaDialog>(new MaestroDrawingAreaDialog(this, this->maestro_controller_));
			drawing_area_dialog_.get()->show();
		}

		device_extra_control_widget_ = QSharedPointer<DeviceControlWidget>(new DeviceControlWidget(this, nullptr));
		ui->deviceTab->findChild<QLayout*>("deviceTabLayout")->addWidget(device_extra_control_widget_.data());

		// Set the Maestro's Sections
		maestro_controller_->set_sections(settings.value(PreferencesDialog::num_sections, 1).toInt());

		// Initialize Cue Handlers
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

		// Check to see if we need to pause the Maestro
		if (settings.value(PreferencesDialog::pause_on_start, false).toBool()) {
			on_showPauseButton_clicked();
		}

		// Finally, initialize the MaestroControlWidget UI
		initialize();
	}

	/**
	 * Adds an action to the event history.
	 * @param cue Action performed.
	 */
	void MaestroControlWidget::add_cue_to_history(uint8_t *cue) {
		ui->eventHistoryWidget->addItem(cue_interpreter_.interpret_cue(cue));

		// Convert the Cue into an actual byte array, which we'll store in the Event History for later use.
		uint16_t cue_size = cue_controller_->get_cue_size(cue);
		QVector<uint8_t> cue_vector(cue_size);
		for (uint16_t byte = 0; byte < cue_size; byte++) {
			cue_vector[byte] = cue[byte];
		}

		event_history_.push_back(cue_vector);

		// Remove all but the last 50 Events
		if (event_history_.size() >= 50) {
			ui->eventHistoryWidget->takeItem(0);
			event_history_.remove(0);
		}
	}

	/**
	 * Enables Show Edit mode, which disables updates to the Maestro.
	 * @param enable If true, don't update the Maestro.
	 */
	void MaestroControlWidget::enable_show_edit_mode(bool enable) {
		show_mode_enabled_ = enable;
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool MaestroControlWidget::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			// Handle Event list keys
			if (watched == ui->eventListWidget) {
				if (key_event->key() == Qt::Key_Delete) {
					on_removeEventButton_clicked();
					return true;
				}
			}
			// Handle Show tab keys
			else if (watched == ui->showScrollArea) {
				if (key_event->key() == Qt::Key_Space) {
					on_showPauseButton_clicked();
				}
			}
			// Handle Canvas tab keys
			else if (watched == ui->canvasScrollArea && active_section_->get_canvas() != nullptr) {
				if (key_event->key() == Qt::Key_Left) {
					on_canvasPlaybackBackToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Right) {
					on_canvasPlaybackNextToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Space) {
					ui->canvasPlaybackStartStopToolButton->setChecked(!ui->canvasPlaybackStartStopToolButton->isChecked());
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	/**
	 * Returns the Section currently being edited.
	 * @return Active Section.
	 */
	Section* MaestroControlWidget::get_active_section() {
		return active_section_;
	}

	/**
	 * Returns the index of the selected Canvas Palette color.
	 * @return Canvas Palette color index.
	 */
	uint8_t MaestroControlWidget::get_canvas_color_index() const {
		return canvas_color_index_;
	}

	/**
	 * Returns whether the Canvas paint tool button is currently active.
	 * @return True if the paintToolButton is active.
	 */
	bool MaestroControlWidget::get_canvas_painting_enabled() {
		return (ui->paintToolButton->isEnabled() && ui->paintToolButton->isChecked());
	}

	/**
	 * Returns the index of the current Layer.
	 * @return Layer index.
	 */
	uint8_t MaestroControlWidget::get_layer_index() {
		return get_layer_index(active_section_);
	}

	/**
	 * Returns the index of the specified Layer.
	 * @param section Section belonging to the Layer.
	 * @return Layer index.
	 */
	uint8_t MaestroControlWidget::get_layer_index(Section* section) {
		/*
		 * Find the depth of the current Section by traversing parent_section_.
		 * Once parent_section == nullptr, we know we've hit the base Section.
		 */
		uint8_t level = 0;
		Section* test_section = section;
		while (test_section->get_parent_section() != nullptr) {
			test_section = test_section->get_parent_section();
			level++;
		}
		return level;
	}

	/**
	 * Returns the control widget's MaestroController.
	 * @return MaestroController.
	 */
	MaestroController* MaestroControlWidget::get_maestro_controller() {
		return maestro_controller_;
	}

	/**
	 * Returns the index of the specified Section.
	 * @param section Section to ID.
	 * @return Section index.
	 */
	uint8_t MaestroControlWidget::get_section_index(Section* section) {
		Section* target_section = section;

		// If this is an Layer, iterate until we find the parent ID
		while (target_section->get_parent_section() != nullptr) {
			target_section = target_section->get_parent_section();
		}

		// Iterate until we find the Section that active_section_ points to
		uint8_t index = 0;
		Section* test_section = maestro_controller_->get_maestro()->get_section(0);
		while (test_section != target_section) {
			index++;
			test_section = maestro_controller_->get_maestro()->get_section(index);
		}

		return index;
	}

	/**
	 * Returns the index of the current Section.
	 * @return Current Section index (or 0 if not found).
	 */
	uint8_t MaestroControlWidget::get_section_index() {
		return get_section_index(active_section_);
	}

	/**
	 * Gets the number of Layers belonging to the Section.
	 * @param section Section to scan.
	 * @return Number of Layers.
	 */
	uint8_t MaestroControlWidget::get_num_layers(Section *section) {
		uint8_t count = 0;
		Section::Layer* layer = section->get_layer();
		while (layer != nullptr) {
			layer = layer->section->get_layer();
			count++;
		}
		return count;
	}

	/**
	 * Build the MaestroControl UI.
	 */
	void MaestroControlWidget::initialize() {
		initialize_palettes();

		// There must be a better way to bulk block signals.
		ui->sectionComboBox->blockSignals(true);
		ui->layerComboBox->blockSignals(true);
		ui->alphaSpinBox->blockSignals(true);

		ui->sectionComboBox->clear();
		ui->layerComboBox->clear();
		ui->alphaSpinBox->clear();

		// Set Section/Layer
		for (uint8_t section = 0; section < maestro_controller_->get_maestro()->get_num_sections(); section++) {
			ui->sectionComboBox->addItem(QString("Section ") + QString::number(section));
		}
		ui->sectionComboBox->setCurrentIndex(0);

		// Unblock signals (*grumble grumble*)
		ui->sectionComboBox->blockSignals(false);
		ui->layerComboBox->blockSignals(false);
		ui->alphaSpinBox->blockSignals(false);

		// Disable advanced controls until they're activated manually
		ui->animationAdvancedSettingsGroupBox->setVisible(false);
		set_canvas_controls_enabled(0);
		set_layer_controls_enabled(false);
		set_show_controls_enabled(false);

		// Initialize Show timer
		show_timer_.setTimerType(Qt::CoarseTimer);
		show_timer_.setInterval(100);
		connect(&show_timer_, SIGNAL(timeout()), this, SLOT(update_maestro_last_time()));

		// Initialize Canvas elements
		// Add drawing buttons to group
		canvas_shape_type_group_.addButton(ui->circleToolButton);
		canvas_shape_type_group_.addButton(ui->lineToolButton);
		canvas_shape_type_group_.addButton(ui->paintToolButton);
		canvas_shape_type_group_.addButton(ui->rectToolButton);
		canvas_shape_type_group_.addButton(ui->textToolButton);
		canvas_shape_type_group_.addButton(ui->triangleToolButton);

		// Set Canvas defaults
		ui->currentFrameSpinBox->setEnabled(false);

		// Add an event handler to the Canvas color picker cancel button
		connect(ui->canvasColorPickerCancelButton, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);

		// Finally, show the default Section
		set_active_section(maestro_controller_->get_maestro()->get_section(0));
		populate_layer_combobox();
	}

	/// Reinitializes Palettes from the Palette Dialog.
	void MaestroControlWidget::initialize_palettes() {
		QString color_palette = ui->animationPaletteComboBox->currentText();
		QString canvas_palette = ui->canvasPaletteComboBox->currentText();

		// Repopulate color combo boxes
		ui->animationPaletteComboBox->blockSignals(true);
		ui->canvasPaletteComboBox->blockSignals(true);

		ui->animationPaletteComboBox->clear();
		ui->canvasPaletteComboBox->clear();

		for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
			ui->animationPaletteComboBox->addItem(palette_controller_.get_palette(i)->name);
			ui->canvasPaletteComboBox->addItem(palette_controller_.get_palette(i)->name);
		}

		ui->animationPaletteComboBox->setCurrentText(color_palette);
		ui->canvasPaletteComboBox->setCurrentText(canvas_palette);

		ui->animationPaletteComboBox->blockSignals(false);
		ui->canvasPaletteComboBox->blockSignals(false);
	}

	/**
	 * Loads a Cuefile into the Maestro.
	 * @param filename Path to the Cuefile.
	 */
	void MaestroControlWidget::load_cuefile(QString filename) {
		QFile file(filename);

		if (file.open(QFile::ReadOnly)) {
			this->loading_cue_ = true;
			QByteArray bytes = file.readAll();
			for (int i = 0; i < bytes.size(); i++) {
				uint8_t byte = (uint8_t)bytes.at(i);
				if (cue_controller_->read(byte)) {
					run_cue(cue_controller_->get_buffer(), true);
				}
			}
			file.close();
			this->loading_cue_ = false;
		}
	}

	/**
	 * Adds the selected Event(s) to the Show's Event list.
	 */
	void MaestroControlWidget::on_addEventButton_clicked() {
		// Add selected Cues to the Show
		for (QModelIndex index : ui->eventHistoryWidget->selectionModel()->selectedIndexes()) {
			Event* event = show_controller_->add_event(ui->eventTimeSpinBox->value(), (uint8_t*)&event_history_.at(index.row()).at(0));
			ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));
		}
		run_cue(
			show_handler->set_events(show_controller_->get_events()->data(), show_controller_->get_events()->size(), true)
		);
	}

	/**
	 * Sets the Layer's transparency level.
	 */
	void MaestroControlWidget::on_alphaSpinBox_editingFinished() {
		run_cue(
			section_handler->set_layer(get_section_index(),
				get_layer_index(active_section_->get_parent_section()),
				active_section_->get_parent_section()->get_layer()->mix_mode,
				ui->alphaSpinBox->value())
		);
	}

	/**
	 * Changes the current animation.
	 * TODO: Allow for no animation
	 * @param index Index of the new animation.
	 */
	void MaestroControlWidget::on_animationComboBox_currentIndexChanged(int index) {
		// If the animation is set to "None", remove the Animation
		if (index == 0) {
			run_cue(section_handler->remove_animation(get_section_index(), get_layer_index()));
		}
		else {

			// If the Section already has an Animation set, check to make sure the new Animation is different
			Animation* animation = active_section_->get_animation();
			if (animation != nullptr) {
				if (animation->get_type() == (AnimationType)(index - 1)) {
					return;
				}

				// Preserve options between animations
				run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), (AnimationType)(index - 1), true));
			}
			else {	// If the Section had no prior Animation, re-apply settings shown in the UI.
				run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), (AnimationType)(index - 1), false));
				on_animationPaletteComboBox_currentIndexChanged(ui->animationPaletteComboBox->currentIndex());
				on_fadeCheckBox_toggled(ui->fadeCheckBox->isChecked());
				on_orientationComboBox_currentIndexChanged(ui->orientationComboBox->currentIndex());
				on_reverse_animationCheckBox_toggled(ui->reverse_animationCheckBox->isChecked());
				on_animationIntervalSpinBox_editingFinished();
				on_delaySpinBox_editingFinished();
			}

			set_advanced_animation_controls(active_section_->get_animation());
		}

		set_animation_controls_enabled(index > 0);
	}

	/**
	 * Changes the Animation's Palette.
	 * @param index Index of the new Palette.
	 */
	void MaestroControlWidget::on_animationPaletteComboBox_currentIndexChanged(int index) {
		PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette(index);
		run_cue(animation_handler->set_palette(get_section_index(), get_layer_index(), &palette_wrapper->palette));
	}

	/// Opens the Palette Editor from the Canvas tab.
	void MaestroControlWidget::on_canvasEditPaletteButton_clicked() {
		QString palette_name = ui->canvasPaletteComboBox->currentText();

		PaletteControlWidget palette_control(&palette_controller_, palette_name);
		palette_control.exec();
		initialize_palettes();
		ui->canvasPaletteComboBox->setCurrentText(palette_name);
	}

	/**
	 * Activates the Canvas.
	 * @param checked If true, Canvas is enabled.
	 */
	void MaestroControlWidget::on_canvasEnableCheckBox_toggled(bool checked) {
		// Check to see if a Canvas already exists. If it does, warn the user that the current Canvas will be erased.
		if (!checked && active_section_->get_canvas() != nullptr) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				run_cue(section_handler->remove_canvas(get_section_index(), get_layer_index()));
			}
			else {
				// Return to the previous state and exit.
				ui->canvasEnableCheckBox->blockSignals(true);
				ui->canvasEnableCheckBox->setChecked(true);
				ui->canvasEnableCheckBox->blockSignals(false);
				return;
			}
		}

		// Display/hide controls as necessary
		set_canvas_controls_enabled(checked);
		if (checked) {
			run_cue(section_handler->set_canvas(get_section_index(), get_layer_index()));

			// Default to the circle radio button so that the controls can be refreshed
			ui->circleToolButton->setChecked(true);
			on_circleToolButton_toggled(true);

			// Select a palette
			on_canvasPaletteComboBox_currentIndexChanged(0);
		}
		else {
			// Disable Canvas playback
			ui->canvasPlaybackStartStopToolButton->setChecked(false);
			on_canvasPlaybackStartStopToolButton_toggled(false);
		}
	}

	/**
	 * Changes the Canvas' palette.
	 * @param index New index.
	 */
	void MaestroControlWidget::on_canvasPaletteComboBox_currentIndexChanged(int index) {
		PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette(index);
		run_cue(canvas_handler->set_palette(get_section_index(), get_layer_index(), &palette_wrapper->palette));

		populate_palette_canvas_color_selection(palette_wrapper);
	}

	/**
	 * Moves the Canvas animation back a frame.
	 */
	void MaestroControlWidget::on_canvasPlaybackBackToolButton_clicked() {
		run_cue(
			canvas_handler->previous_frame(get_section_index(), get_layer_index())
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(active_section_->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Advances the Canvas animation by a single frame.
	 */
	void MaestroControlWidget::on_canvasPlaybackNextToolButton_clicked() {
		run_cue(
			canvas_handler->next_frame(get_section_index(), get_layer_index())
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(active_section_->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Toggles Canvas animation playback.
	 * @param checked If true, run the Canvas animation.
	 */
	void MaestroControlWidget::on_canvasPlaybackStartStopToolButton_toggled(bool checked) {
		if (active_section_->get_canvas() == nullptr) return;

		// Enables/disable the 'current frame' control
		ui->currentFrameSpinBox->setEnabled(!checked);

		if (checked) {
			on_frameIntervalSpinBox_editingFinished();
		}
		else {
			run_cue(canvas_handler->remove_frame_timer(get_section_index(), get_layer_index()));
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(active_section_->get_canvas()->get_current_frame_index());
			ui->currentFrameSpinBox->blockSignals(false);
		}
	}

	/**
	 * Sets the offset x value.
	 */
	void MaestroControlWidget::on_offsetXSpinBox_editingFinished() {
		set_offset();
	}

	/**
	 * Sets the offset y value.
	 */
	void MaestroControlWidget::on_offsetYSpinBox_editingFinished() {
		set_offset();
	}

	/**
	 * Selects a circle for drawing.
	 * @param checked If true, next shape will be a circle.
	 */
	void MaestroControlWidget::on_circleToolButton_toggled(bool checked) {
		set_circle_controls_enabled(checked);
	}

	/**
	 * Clears the current Canvas frame.
	 */
	void MaestroControlWidget::on_clearButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			run_cue(canvas_handler->clear(get_section_index(), get_layer_index()));
		}
	}

	/**
	 * Changes the number of columns in the display grid.
	 */
	void MaestroControlWidget::on_columnsSpinBox_editingFinished() {
		on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
	}

	/**
	 * Switches to the Canvas Frame index specified in currentFrameSpinBox.
	 */
	void MaestroControlWidget::on_currentFrameSpinBox_editingFinished() {
		int frame = ui->currentFrameSpinBox->value();

		// If the selected frame exceeds the number of frames, set to the number of frames.
		if (frame >= active_section_->get_canvas()->get_num_frames()) {
			frame = active_section_->get_canvas()->get_num_frames() - 1;
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(frame);
			ui->currentFrameSpinBox->blockSignals(false);
		}

		run_cue(canvas_handler->set_current_frame_index(get_section_index(), get_layer_index(), frame));
	}

	/**
	 * Changes the animation timer.
	 * @param value New timer.
	 */
	void MaestroControlWidget::on_animationTimerSlider_valueChanged(int value) {
		ui->animationIntervalSpinBox->blockSignals(true);
		ui->animationIntervalSpinBox->setValue(value);
		ui->animationIntervalSpinBox->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Changes the animation timer.
	 */
	void MaestroControlWidget::on_animationIntervalSpinBox_editingFinished() {
		ui->animationTimerSlider->blockSignals(true);
		ui->animationTimerSlider->setValue(ui->animationIntervalSpinBox->value());
		ui->animationTimerSlider->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Handles drawing onto the current Canvas frame.
	 */
	void MaestroControlWidget::on_drawButton_clicked() {
		QAbstractButton* checked_button = canvas_shape_type_group_.checkedButton();

		if (checked_button == ui->circleToolButton) {
			run_cue(canvas_handler->draw_circle(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->lineToolButton) {
			run_cue(canvas_handler->draw_line(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value()));
		}
		else if (checked_button == ui->paintToolButton) {
			run_cue(canvas_handler->draw_point(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value()));
		}
		else if (checked_button == ui->rectToolButton) {
			run_cue(canvas_handler->draw_rect(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->textToolButton) {
			run_cue(canvas_handler->draw_text(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), (Font::Type)ui->fontComboBox->currentIndex(), ui->textLineEdit->text().toLatin1().data(), ui->textLineEdit->text().size()));
		}
		else if (checked_button == ui->triangleToolButton) {
			run_cue(canvas_handler->draw_triangle(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->target2XSpinBox->value(), ui->target2YSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
	}

	/**
	 * Initializes the Maestro's Show.
	 * @param checked If true, creates a new Show.
	 */
	void MaestroControlWidget::on_enableShowCheckBox_toggled(bool checked) {
		set_show_controls_enabled(checked);

		// Initialize Show if necessary
		if (checked) {
			if (show_controller_ == nullptr) {
				show_controller_ = new ShowController(maestro_controller_);
			}
			show_timer_.start();
		}
		else {
			// Disable show edit mode if enabled
			if (show_mode_enabled_) {
				on_toggleShowModeCheckBox_clicked(false);
			}
			show_timer_.stop();
		}
	}

	/**
	 * Toggles fading.
	 * @param checked If true, fading is enabled.
	 */
	void MaestroControlWidget::on_fadeCheckBox_toggled(bool checked) {
		run_cue(animation_handler->set_fade(get_section_index(), get_layer_index(), checked));
	}

	/**
	 * Sets the number of Canvas frames.
	 */
	void MaestroControlWidget::on_frameCountSpinBox_editingFinished() {
		int new_max = ui->frameCountSpinBox->value();
		if (new_max != active_section_->get_canvas()->get_num_frames()) {
			if (new_max < ui->currentFrameSpinBox->value()) {
				ui->currentFrameSpinBox->setValue(new_max);
			}
			ui->currentFrameSpinBox->setMaximum(new_max);
			run_cue(canvas_handler->set_num_frames(get_section_index(), get_layer_index(), new_max));

			// Set the new maximum for the current_frame spinbox
			ui->currentFrameSpinBox->setMaximum(new_max);
		}
	}

	/**
	 * Sets the interval between Canvas frames.
	 * @param value New interval.
	 */
	void MaestroControlWidget::on_frameIntervalSlider_valueChanged(int value) {
		ui->frameIntervalSpinBox->blockSignals(true);
		ui->frameIntervalSpinBox->setValue(value);
		ui->frameIntervalSpinBox->blockSignals(false);

		set_canvas_frame_interval();
	}

	/**
	 * Sets the interval between Canvas frames.
	 */
	void MaestroControlWidget::on_frameIntervalSpinBox_editingFinished() {
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSlider->setValue(ui->frameIntervalSpinBox->value());
		ui->frameIntervalSlider->blockSignals(false);

		set_canvas_frame_interval();
	}

	/**
	 * Changes the Layer's mix mode.
	 * @param index
	 */
	void MaestroControlWidget::on_mix_modeComboBox_currentIndexChanged(int index) {
		if (active_section_->get_parent_section() == nullptr) {
			return;
		}

		if ((Colors::MixMode)index != active_section_->get_parent_section()->get_layer()->mix_mode) {
			run_cue(section_handler->set_layer(get_section_index(), get_layer_index(active_section_->get_parent_section()), (Colors::MixMode)index, ui->alphaSpinBox->value()));
		}

		// Enable spin box for alpha only
		ui->alphaLabel->setEnabled((Colors::MixMode)index == Colors::MixMode::Alpha);
		ui->alphaSpinBox->setEnabled((Colors::MixMode)index == Colors::MixMode::Alpha);
	}

	/// Moves the selected Show Event down by one.
	void MaestroControlWidget::on_moveEventDownButton_clicked() {
		int current_row = ui->eventListWidget->currentRow();

		if (current_row != ui->eventListWidget->count() - 1) {
			show_controller_->move(current_row, current_row	+ 1);
			run_cue(
				show_handler->set_events(show_controller_->get_events()->data(), show_controller_->get_events()->size())
			);

			QListWidgetItem* current_item = ui->eventListWidget->takeItem(current_row);
			ui->eventListWidget->insertItem(current_row + 1, current_item);
			ui->eventListWidget->setCurrentRow(current_row + 1);
		}
	}

	/// Moves the selected Show Event up by one.
	void MaestroControlWidget::on_moveEventUpButton_clicked() {
		int current_row = ui->eventListWidget->currentRow();

		if (current_row != 0) {
			show_controller_->move(current_row, current_row - 1);
			run_cue(
				show_handler->set_events(show_controller_->get_events()->data(), show_controller_->get_events()->size())
			);

			QListWidgetItem* current_item = ui->eventListWidget->takeItem(current_row);
			ui->eventListWidget->insertItem(current_row - 1, current_item);
			ui->eventListWidget->setCurrentRow(current_row - 1);
		}
	}

	/**
	 * Selects a rectangle for the next shape.
	 * @param checked If true, the next shape will be a rectangle.
	 */
	void MaestroControlWidget::on_rectToolButton_toggled(bool checked) {
		set_rect_controls_enabled(checked);
	}

	/**
	 * Sets the Canvas scroll rate along the x axis.
	 */
	void MaestroControlWidget::on_scrollXSpinBox_editingFinished() {
		set_scroll();
	}

	/**
	 * Sets the Canvas scroll rate along the y axis.
	 */
	void MaestroControlWidget::on_scrollYSpinBox_editingFinished() {
		set_scroll();
	}

	/**
	 * Sets the animation's orientation.
	 * @param index New orientation.
	 */
	void MaestroControlWidget::on_orientationComboBox_currentIndexChanged(int index) {
		if ((Animation::Orientation)index != active_section_->get_animation()->get_orientation()) {
			run_cue(animation_handler->set_orientation(get_section_index(), get_layer_index(), (Animation::Orientation)index));
		}
	}

	/**
	 * Changes the current Layer.
	 * @param index Index of the desired Layer.
	 */
	void MaestroControlWidget::on_layerComboBox_currentIndexChanged(int index) {
		/*
		 * If we selected an Layer, iterate through the Section's nested Layers until we find it.
		 * If we selected 'None', use the base Section as the active Section.
		 */
		Section* layer_section = maestro_controller_->get_maestro()->get_section(get_section_index());
		for (int i = 0; i < index; i++) {
			layer_section = layer_section->get_layer()->section;
		}

		// Show Layer controls
		set_layer_controls_enabled(index > 0);

		// Set active Section to Layer Section
		set_active_section(layer_section);
	}

	/**
	 * Sets the number of Layers for the Section.
	 */
	void MaestroControlWidget::on_layerSpinBox_editingFinished() {
		Section* base_section = maestro_controller_->get_maestro()->get_section(get_section_index());
		Section* last_section = base_section;

		// Get the number of Layers (and the index of the last Layer) in the Section
		int num_layers = 0;
		while (last_section->get_layer() != nullptr) {
			last_section = last_section->get_layer()->section;
			num_layers++;
		}
		int last_layer_index = num_layers;

		/*
		 * Get the difference between the current Layer count and the new Layer count.
		 * If diff is positive, add more Layers. Otherwise, remove Layers.
		 */
		int diff = ui->layerSpinBox->value() - num_layers;
		if (diff > 0) {
			while (diff > 0) {
				run_cue(section_handler->set_layer(get_section_index(), last_layer_index, Colors::MixMode::None, 0));
				last_layer_index++;
				diff--;
			}
		}
		// If the diff is negative, remove Layers
		else if (diff < 0) {
			while (diff < 0) {
				last_section = last_section->get_parent_section();
				run_cue(section_handler->remove_layer(get_section_index(), last_layer_index - 1));
				last_layer_index--;
				diff++;
			}

			// If the active Layer no longer exists, jump to the last available Layer
			if (ui->layerSpinBox->value() >= last_layer_index) {
				set_active_section(last_section);
			}
		}

		// Refresh layer combobox
		populate_layer_combobox();
	}

	/**
	 * Selects a line for the next shape.
	 * @param checked If true, the next shape will be a line.
	 */
	void MaestroControlWidget::on_lineToolButton_toggled(bool checked) {
		set_line_controls_enabled(checked);
	}

	/**
	 * Loads an image into the Canvas.
	 * This overwrites all Canvas frames.
	 */
	void MaestroControlWidget::on_loadImageButton_clicked() {
		QString filename = QFileDialog::getOpenFileName(this,
			QString("Open Image"),
			QString(),
			QString("Images (*.bmp *.gif *.jpg *.png)"));

		if (!filename.isEmpty()) {
			CanvasUtility::load_image(filename, active_section_->get_canvas(), this);

			Canvas* canvas = active_section_->get_canvas();

			// Set frame options
			ui->frameCountSpinBox->blockSignals(true);
			ui->frameCountSpinBox->setValue(canvas->get_num_frames());
			ui->frameCountSpinBox->blockSignals(false);

			if (canvas->get_frame_timer() != nullptr) {
				ui->frameIntervalSpinBox->blockSignals(true);
				ui->frameIntervalSpinBox->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameIntervalSpinBox->blockSignals(false);
			}

			// Disable frame count spin box if necessary
			if (ui->canvasPlaybackStartStopToolButton->isChecked()) {
				ui->canvasPlaybackStartStopToolButton->setChecked(false);
				on_canvasPlaybackStartStopToolButton_toggled(false);
			}

			// Add Palette to list
			QString name = "Section " + QString::number(get_section_index()) +
						   " Layer " + QString::number(get_layer_index()) +
						   " Canvas";
			palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
			ui->canvasPaletteComboBox->blockSignals(true);
			ui->canvasPaletteComboBox->addItem(name);
			ui->canvasPaletteComboBox->blockSignals(false);
			ui->canvasPaletteComboBox->setCurrentText(name);

			// Start playback
			ui->canvasPlaybackStartStopToolButton->setChecked(true);
		}
	}

	/**
	 * Enables/disables Show looping.
	 * @param checked If true, the Show will loop its Event queue.
	 */
	void MaestroControlWidget::on_loopCheckBox_toggled(bool checked) {
		run_cue(show_handler->set_looping(checked));
	}

	void MaestroControlWidget::on_paintToolButton_toggled(bool checked) {
		set_paint_controls_enabled(checked);
	}

	/**
	 * Sets the active drawing color.
	 * This gets passed to the CanvasCueHandler on each draw.
	 */
	void MaestroControlWidget::on_canvas_color_clicked() {
		QPushButton* sender = (QPushButton*)QObject::sender();

		if (sender != ui->canvasColorPickerCancelButton) {
			canvas_color_index_ = sender->objectName().toInt();

			// Change the color of the Color button to reflect the selection
			PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette(ui->canvasPaletteComboBox->currentIndex());
			Colors::RGB color = palette_wrapper->palette.get_colors()[canvas_color_index_];
			ui->selectColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));
		}
		else {
			canvas_color_index_ = 255;
			ui->selectColorButton->setStyleSheet(QString("background-color: transparent;"));
		}

		run_cue(canvas_handler->set_drawing_color(get_section_index(), get_layer_index(), canvas_color_index_));
	}

	/**
	 * Opens the Palette Editor.
	 */
	void MaestroControlWidget::on_paletteControlButton_clicked() {
		QString palette_name = ui->animationPaletteComboBox->currentText();

		PaletteControlWidget palette_control(&palette_controller_, palette_name);
		palette_control.exec();
		initialize_palettes();
	}

	/**
	 * Sets the delay between Animation cycles.
	 */
	void MaestroControlWidget::on_delaySpinBox_editingFinished() {
		ui->delaySlider->blockSignals(true);
		ui->delaySlider->setValue(ui->delaySpinBox->value());
		ui->delaySlider->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Sets the delay between Animation cycles.
	 * @param value New pause interval.
	 */
	void MaestroControlWidget::on_delaySlider_valueChanged(int value) {
		ui->delaySpinBox->blockSignals(true);
		ui->delaySpinBox->setValue(value);
		ui->delaySpinBox->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Removes the selected Event(s) from the Show.
	 */
	void MaestroControlWidget::on_removeEventButton_clicked() {

		// Sort selected rows by index before removing them
		QModelIndexList list = ui->eventListWidget->selectionModel()->selectedRows();
		qSort(list.begin(), list.end(), qGreater<QModelIndex>());
		for (QModelIndex index : list) {
			show_controller_->remove_event(index.row());
			ui->eventListWidget->takeItem(index.row());
		}

		// Re-initialize the Event list
		run_cue(
			show_handler->set_events(show_controller_->get_events()->data(), show_controller_->get_events()->size(), true)
		);
	}

	/**
	 * Resyncs Maestro components.
	 */
	void MaestroControlWidget::on_resyncMaestroButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Resync Timers", "This will sync all timers (such as Animation and Canvas timers) by setting their last run time to 0. Are you sure you want to continue?", QMessageBox::Yes | QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			maestro_controller_->get_maestro()->sync();
		}
	}

	/**
	 * Toggles whether the color animation is shown in reverse.
	 * @param checked If true, reverse the animation.
	 */
	void MaestroControlWidget::on_reverse_animationCheckBox_toggled(bool checked) {
		run_cue(animation_handler->set_reverse(get_section_index(), get_layer_index(), checked));
	}

	/**
	 * Changes the number of rows in the displayed grid.
	 */
	void MaestroControlWidget::on_rowsSpinBox_editingFinished() {
		on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
	}

	/**
	 * Changes the current Section.
	 * @param index Index of the desired Section.
	 */
	void MaestroControlWidget::on_sectionComboBox_currentIndexChanged(int index) {
		// Hide Layer controls
		set_layer_controls_enabled(false);

		Section* section = maestro_controller_->get_maestro()->get_section(index);

		// Set active controller
		set_active_section(section);
	}

	/**
	 * Sets the size of the active Section.
	 * @param x Number of rows.
	 * @param y Number of columns.
	 */
	void MaestroControlWidget::on_section_resize(uint16_t x, uint16_t y) {
		if ((x != active_section_->get_dimensions()->x) || (y != active_section_->get_dimensions()->y)) {

			/*
			 * Check for a Canvas.
			 * The old Canvas gets deleted on resize.
			 * If one is set, copy its contents to a temporary buffer, then copy it back once the new Canvas is created.
			 */
			if (active_section_->get_canvas() != nullptr) {
				Point frame_bounds(active_section_->get_dimensions()->x, active_section_->get_dimensions()->y);

				Canvas* canvas = active_section_->get_canvas();

				uint8_t** frames = new uint8_t*[canvas->get_num_frames()];
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					frames[frame] = new uint8_t[frame_bounds.size()];
				}
				CanvasUtility::copy_from_canvas(canvas, frames, frame_bounds.x, frame_bounds.y);

				run_cue(section_handler->set_dimensions(get_section_index(), 0, x, y));

				CanvasUtility::copy_to_canvas(canvas, frames, frame_bounds.x, frame_bounds.y, this);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
			else {	// No Canvas set
				run_cue(section_handler->set_dimensions(get_section_index(), 0, x, y));
			}
		}
	}

	/**
	 * Toggles between stopping and running the Maestro.
	 */
	void MaestroControlWidget::on_showPauseButton_clicked() {
		if (maestro_controller_->get_running()) {
			// Stop the Maestro
			maestro_controller_->stop();
			ui->showPauseButton->setToolTip("Resume the Maestro");
		}
		else {
			// Start the Maestro
			maestro_controller_->start();
			ui->showPauseButton->setToolTip("Pause the Maestro");
		}
	}

	/**
	 * Selects text for the next Canvas shape.
	 * @param checked If true, the next shape will be text.
	 */
	void MaestroControlWidget::on_textToolButton_toggled(bool checked) {
		set_text_controls_enabled(checked);
	}

	/**
	 * Changes the Show's timing method.
	 * @param index Index of the timing method in showTimingMethodComboBox.
	 */
	void MaestroControlWidget::on_showTimingMethodComboBox_currentIndexChanged(int index) {
		run_cue(show_handler->set_timing_mode((Show::TimingMode)index));

		// Enable/disable loop controls for relative mode
		ui->loopCheckBox->setEnabled((Show::TimingMode)index == Show::TimingMode::Relative);
	}

	/**
	 * Toggles Show Edit mode. Show Edit mode lets the user populate the Event History without affecting the output.
	 * @param checked If true, Show Edit mode is enabled.
	 */
	void MaestroControlWidget::on_toggleShowModeCheckBox_clicked(bool checked) {
		enable_show_edit_mode(checked);
	}

	/**
	 * Sets the next shape to triangle.
	 * @param checked If true, the next Canvas shape drawn will be a triangle.
	 */
	void MaestroControlWidget::on_triangleToolButton_toggled(bool checked) {
		set_triangle_controls_enabled(checked);
	}

	/**
	 * Rebuilds the Layer combobox using the current active Section.
	 */
	void MaestroControlWidget::populate_layer_combobox() {
		ui->layerComboBox->blockSignals(true);
		ui->layerComboBox->clear();
		ui->layerComboBox->addItem("Base Section");

		for (uint8_t layer = 0; layer < get_num_layers(maestro_controller_->get_maestro()->get_section(get_section_index())); layer++) {
			ui->layerComboBox->addItem(QString("Layer ") + QString::number(layer + 1));
		}

		ui->layerComboBox->blockSignals(false);
	}

	/**
	 * Rebuilds the Palette Canvas color selection scroll area.
	 */
	void MaestroControlWidget::populate_palette_canvas_color_selection(PaletteController::PaletteWrapper* palette_wrapper) {
		// Add color buttons to canvasColorPickerLayout. This functions identically to palette switching in the Palette Editor.
		// Delete existing color buttons
		QList<QPushButton*> buttons = ui->canvasColorPickerScrollArea->findChildren<QPushButton*>(QString(), Qt::FindChildOption::FindChildrenRecursively);
		for (QPushButton* button : buttons) {
			// Remove all but the Cancel button
			if (button != ui->canvasColorPickerCancelButton) {
				disconnect(button, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
				delete button;
			}
		}

		// Create new buttons and add an event handler that triggers on_canvas_color_clicked()
		QLayout* layout = ui->canvasColorPickerScrollArea->findChild<QLayout*>("canvasColorPickerLayout");
		for (uint8_t color_index = 0; color_index < palette_wrapper->palette.get_num_colors(); color_index++) {
			Colors::RGB color = palette_wrapper->palette.get_colors()[color_index];
			QPushButton* button = new QPushButton();
			button->setVisible(true);
			button->setObjectName(QString::number(color_index));
			button->setToolTip(QString::number(color_index + 1));
			button->setMaximumWidth(40);
			button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));

			layout->addWidget(button);
			connect(button, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
		}
	}

	/**
	 * Updates the UI to reflect the Maestro's current settings.
	 */
	void MaestroControlWidget::refresh_maestro_settings() {
		Show* show = maestro_controller_->get_maestro()->get_show();
		if (show != nullptr) {
			//on_enableShowCheckBox_toggled(true);
			ui->enableShowCheckBox->setChecked(true);

			for (uint16_t i = 0; i < show->get_num_events(); i++) {
				Event* event = &show->get_events()[i];
				show_controller_->add_event(event->get_time(), event->get_cue());
				ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));
			}

			run_cue(
				show_handler->set_events(show_controller_->get_events()->data(), show_controller_->get_events()->size())
			);

			ui->showTimingMethodComboBox->blockSignals(true);
			ui->showTimingMethodComboBox->setCurrentIndex((uint8_t)show->get_timing());
			ui->showTimingMethodComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(show->get_looping());
			ui->loopCheckBox->blockSignals(false);
		}

		// Recalculate the Maestro Cuefile
		if (device_extra_control_widget_ != nullptr) {
			device_extra_control_widget_->update_cuefile_size();
		}
	}

	/**
	 * Forwards the specified Cue to the drawing area and/or serial device.
	 * @param cue Cue to perform.
	 * @param serial_only If true, don't run the Cue on the local Maestro.
	 */
	void MaestroControlWidget::run_cue(uint8_t *cue, bool remote_only) {
		// Update the ShowControl's history
		if (show_controller_ != nullptr) {
			add_cue_to_history(cue);
		}

		/*
		 * Only run the Cue if Show mode isn't enabled, or if the Cue is a Show Cue
		 */
		if (!show_mode_enabled_ || cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)CueController::Handler::ShowCueHandler) {
			if (!remote_only) {
				cue_controller_->run(cue);
			}

			// Send to serial device controller.
			if (device_extra_control_widget_ != nullptr) {
				device_extra_control_widget_->run_cue(cue, cue_controller_->get_cue_size(cue));
			}
		}
	}

	/**
	 * Changes the active Section and sets all GUI controls to the Section's settings.
	 * @param section New active Section.
	 */
	void MaestroControlWidget::set_active_section(Section* section) {
		if (section == nullptr) {
			return;
		}

		active_section_ = section;

		// Handle the Section and Layer comboboxes

		// If the current Section doesn't match the Section drop-down, update the drop-down
		if (ui->sectionComboBox->currentIndex() != get_section_index()) {
			ui->sectionComboBox->blockSignals(true);
			ui->sectionComboBox->setCurrentIndex(get_section_index());
			ui->sectionComboBox->blockSignals(false);
		}

		// Update the Layer combo box based on our selection
		if (get_layer_index() == 0) {
			// Set Layer count
			ui->layerSpinBox->blockSignals(true);
			ui->layerSpinBox->setValue(get_num_layers(section));
			ui->layerSpinBox->blockSignals(false);

			populate_layer_combobox();
		}

		// Handle Layer combobox
		if (ui->layerComboBox->currentIndex() != get_layer_index()) {
			ui->layerComboBox->blockSignals(true);
			ui->layerComboBox->setCurrentIndex(get_layer_index());
			ui->layerComboBox->blockSignals(false);
		}

		// Set dimensions
		ui->columnsSpinBox->blockSignals(true);
		ui->rowsSpinBox->blockSignals(true);
		// These are intentionally reversed
		ui->columnsSpinBox->setValue(section->get_dimensions()->y);
		ui->rowsSpinBox->setValue(section->get_dimensions()->x);
		ui->columnsSpinBox->blockSignals(false);
		ui->rowsSpinBox->blockSignals(false);

		// Get offset and scroll settings
		ui->offsetXSpinBox->blockSignals(true);
		ui->offsetYSpinBox->blockSignals(true);
		ui->offsetXSpinBox->setValue(section->get_offset()->x);
		ui->offsetYSpinBox->setValue(section->get_offset()->y);
		ui->offsetXSpinBox->blockSignals(false);
		ui->offsetYSpinBox->blockSignals(false);
		ui->scrollXSpinBox->blockSignals(true);
		ui->scrollYSpinBox->blockSignals(true);
		int32_t interval_x = 0;
		int32_t interval_y = 0;
		if (section->get_scroll() != nullptr) {
			Section::Scroll* scroll = section->get_scroll();
			uint16_t refresh = maestro_controller_->get_maestro()->get_timer()->get_interval();
			// x axis
			if (scroll->timer_x != nullptr) {
				float x = refresh / (float)scroll->timer_x->get_interval();
				interval_x = (section->get_dimensions()->x * refresh) / x;
			}
			else {
				if (scroll->step_x > 0) {
					interval_x = (section->get_dimensions()->x * refresh) / (float)scroll->step_x;
				}
			}
			if (scroll->reverse_x) interval_x *= -1;

			// y axis
			if (scroll->timer_y != nullptr) {
				float y = refresh / (float)scroll->timer_y->get_interval();
				interval_y = (section->get_dimensions()->y * refresh) / y;
			}
			else {
				if (scroll->step_y > 0) {
					interval_y = (section->get_dimensions()->y * refresh) / (float)scroll->step_y;
				}
			}
			if (scroll->reverse_y) interval_y *= -1;
		}
		ui->scrollXSpinBox->setValue(interval_x);
		ui->scrollYSpinBox->setValue(interval_y);

		ui->scrollXSpinBox->blockSignals(false);
		ui->scrollYSpinBox->blockSignals(false);

		// If this is a Layer, get the MixMode and alpha
		if (section->get_parent_section() != nullptr) {
			ui->mix_modeComboBox->blockSignals(true);
			ui->alphaSpinBox->blockSignals(true);
			ui->mix_modeComboBox->setCurrentIndex((uint8_t)section->get_parent_section()->get_layer()->mix_mode);
			ui->alphaSpinBox->setValue(section->get_parent_section()->get_layer()->alpha);
			ui->mix_modeComboBox->blockSignals(false);
			ui->alphaSpinBox->blockSignals(false);
		}

		// Get animation options and timer.
		// If no animation is set, initialize one.
		Animation* animation = section->get_animation();
		if (animation == nullptr) {
			PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette("Color Wheel");
			run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), AnimationType::Solid));
			run_cue(animation_handler->set_palette(get_section_index(), get_layer_index(), &palette_wrapper->palette));
			run_cue(animation_handler->set_timer(get_section_index(), get_layer_index(), 1000));
			animation = section->get_animation();
		}

		// Animation settings
		ui->orientationComboBox->blockSignals(true);
		ui->reverse_animationCheckBox->blockSignals(true);
		ui->fadeCheckBox->blockSignals(true);
		ui->animationTimerSlider->blockSignals(true);
		ui->animationIntervalSpinBox->blockSignals(true);
		ui->delaySlider->blockSignals(true);
		ui->delaySpinBox->blockSignals(true);
		ui->orientationComboBox->setCurrentIndex((uint8_t)animation->get_orientation());
		ui->reverse_animationCheckBox->setChecked(animation->get_reverse());
		ui->fadeCheckBox->setChecked(animation->get_fade());
		ui->animationTimerSlider->setValue(animation->get_timer()->get_interval());
		ui->animationIntervalSpinBox->setValue(animation->get_timer()->get_interval());
		ui->delaySlider->setValue(animation->get_timer()->get_delay());
		ui->delaySpinBox->setValue(animation->get_timer()->get_delay());
		ui->orientationComboBox->blockSignals(false);
		ui->reverse_animationCheckBox->blockSignals(false);
		ui->fadeCheckBox->blockSignals(false);
		ui->animationTimerSlider->blockSignals(false);
		ui->animationIntervalSpinBox->blockSignals(false);
		ui->delaySlider->blockSignals(false);
		ui->delaySpinBox->blockSignals(false);

		// Palette not found
		int palette_index = palette_controller_.find(animation->get_palette()->get_colors());
		if (palette_index >= 0) {
			ui->animationPaletteComboBox->blockSignals(true);
			ui->animationPaletteComboBox->setCurrentIndex(palette_index);
			ui->animationPaletteComboBox->blockSignals(false);
		}
		else {
			QString name = "Section " + QString::number(get_section_index()) +
						   " Layer " + QString::number(get_layer_index()) +
						   " Animation";
			palette_controller_.add_palette(name, animation->get_palette()->get_colors(), animation->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
			ui->animationPaletteComboBox->blockSignals(true);
			ui->animationPaletteComboBox->addItem(name);
			ui->animationPaletteComboBox->setCurrentText(name);
			ui->animationPaletteComboBox->blockSignals(false);
		}

		// Set the animation
		ui->animationComboBox->blockSignals(true);
		ui->animationComboBox->setCurrentIndex((uint8_t)animation->get_type() + 1);
		ui->animationComboBox->blockSignals(false);
		set_advanced_animation_controls(animation);

		// Get Canvas
		ui->canvasEnableCheckBox->blockSignals(true);
		ui->frameCountSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->blockSignals(true);
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSpinBox->blockSignals(true);

		Canvas* canvas = section->get_canvas();
		if (canvas != nullptr) {
			ui->canvasEnableCheckBox->setChecked(true);
			ui->frameCountSpinBox->setValue(canvas->get_num_frames());
			ui->currentFrameSpinBox->setValue(canvas->get_current_frame_index());
			if (canvas->get_frame_timer() != nullptr) {
				ui->frameIntervalSlider->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameIntervalSpinBox->setValue(canvas->get_frame_timer()->get_interval());
			}
			set_canvas_controls_enabled(true);

			// Find the corresponding palette in the Palette Controller.
			if (canvas->get_palette() != nullptr) {
				palette_index = palette_controller_.find(canvas->get_palette()->get_colors());
				if (palette_index >= 0) {
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->setCurrentIndex(palette_index);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
				else {
					QString name = "Section " + QString::number(get_section_index()) +
								   " Layer " + QString::number(get_layer_index()) +
								   " Canvas";
					palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->addItem(name);
					ui->canvasPaletteComboBox->setCurrentText(name);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
			}

			populate_palette_canvas_color_selection(palette_controller_.get_palette(ui->canvasPaletteComboBox->currentIndex()));
		}
		else {
			ui->canvasEnableCheckBox->setChecked(false);
			ui->frameCountSpinBox->setValue(1);
			ui->currentFrameSpinBox->setValue(0);
			ui->frameIntervalSlider->setValue(100);
			ui->frameIntervalSpinBox->setValue(100);
			set_canvas_controls_enabled(0);
		}

		ui->frameCountSpinBox->blockSignals(false);
		ui->currentFrameSpinBox->blockSignals(false);
		ui->frameIntervalSlider->blockSignals(false);
		ui->frameIntervalSpinBox->blockSignals(false);
		ui->canvasEnableCheckBox->blockSignals(false);
	}

	/**
	 * Toggles whether Animation controls are usable.
	 * @param enabled If true, controls are enabled.
	 */
	void MaestroControlWidget::set_animation_controls_enabled(bool enabled) {
		ui->orientationComboBox->setEnabled(enabled);
		ui->reverse_animationCheckBox->setEnabled(enabled);
		ui->animationPaletteComboBox->setEnabled(enabled);
		ui->fadeCheckBox->setEnabled(enabled);
		ui->animationAdvancedSettingsGroupBox->setEnabled(enabled);
		ui->animationTimersGroupBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas controls.
	 * @param enabled If true, enable the controls.
	 */
	void MaestroControlWidget::set_canvas_controls_enabled(bool enabled) {
		ui->drawingToolsGroupBox->setEnabled(enabled);
		ui->animationToolsGroupBox->setEnabled(enabled);
		ui->loadImageButton->setEnabled(enabled);

		// Canvas-specific controls
		ui->canvasPaletteComboBox->setEnabled(enabled);
		ui->canvasPaletteLabel->setEnabled(enabled);
		ui->canvasEditPaletteButton->setEnabled(enabled);
		ui->canvasColorPickerScrollArea->setEnabled(enabled);
	}

	/**
	 * Sets the Canvas' origin to the specified coordinates.
	 */
	void MaestroControlWidget::set_canvas_origin(Point* coordinates) {
		ui->originXSpinBox->blockSignals(true);
		ui->originYSpinBox->blockSignals(true);
		ui->originXSpinBox->setValue(coordinates->x);
		ui->originYSpinBox->setValue(coordinates->y);
		ui->originXSpinBox->blockSignals(false);
		ui->originYSpinBox->blockSignals(false);
	}

	/**
	 * Enables Canvas circle controls.
	 * @param enabled If true, circle controls are enabled.
	 */
	void MaestroControlWidget::set_circle_controls_enabled(bool enabled) {
		if (enabled) {
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);

			ui->targetLabel->setText("Radius");
			ui->targetLabel->setEnabled(true);
			ui->targetXSpinBox->setEnabled(true);
		}
		else {
			ui->targetLabel->setText("Target");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
		ui->fillCheckBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas line controls.
	 * @param enabled If true, line controls are enabled.
	 */
	void MaestroControlWidget::set_line_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->fillCheckBox->setEnabled(false);
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas paint controls.
	 * @param enabled If true, paint controls are enabled.
	 */
	void MaestroControlWidget::set_paint_controls_enabled(bool enabled) {
		set_circle_controls_enabled(false);
		set_line_controls_enabled(false);
		set_rect_controls_enabled(false);
		set_text_controls_enabled(false);
		set_triangle_controls_enabled(false);

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
		ui->originLabel->setText("Cursor");
	}

	/**
	 * Enables Canvas rectangle controls.
	 * @param enabled If true, rectangle controls are enabled.
	 */
	void MaestroControlWidget::set_rect_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->targetLabel->setText("Size");
		}
		else {
			ui->targetLabel->setText("Target");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);

		ui->fillCheckBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas text controls.
	 * @param enabled If true, text controls are enabled.
	 */
	void MaestroControlWidget::set_text_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->fillCheckBox->setEnabled(false);
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->fontLabel->setEnabled(enabled);
		ui->fontComboBox->setEnabled(enabled);

		ui->textLabel->setEnabled(enabled);
		ui->textLineEdit->setEnabled(enabled);
	}

	/**
	 * Enables Canvas triangle controls.
	 * @param enabled If true, triangle controls are enabled.
	 */
	void MaestroControlWidget::set_triangle_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);

			ui->originLabel->setText("Point A");
			ui->targetLabel->setText("Point B");
			ui->target2Label->setText("Point C");
		}
		else {
			ui->originLabel->setText("Cursor");
			ui->targetLabel->setText("Target");
			ui->target2Label->setText("Target 2");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);

		ui->target2Label->setEnabled(enabled);
		ui->target2XSpinBox->setEnabled(enabled);
		ui->target2YSpinBox->setEnabled(enabled);

		ui->fillCheckBox->setEnabled(enabled);
	}
	// End Canvas-specific methods

	/**
	 * Sets the Animation's center to the specified coordinates.
	 */
	void MaestroControlWidget::set_offset() {
		run_cue(section_handler->set_offset(get_section_index(), get_layer_index(), ui->offsetXSpinBox->value(), ui->offsetYSpinBox->value()));
	}

	/**
	 * Sets the Canvas' scrolling behavior.
	 */
	void MaestroControlWidget::set_scroll() {
		int new_x = ui->scrollXSpinBox->value();
		int new_y = ui->scrollYSpinBox->value();

		run_cue(section_handler->set_scroll(get_section_index(), get_layer_index(), Utility::abs_int(new_x), Utility::abs_int(new_y), (new_x < 0), (new_y < 0)));

		// Enable/disable offset controls
		ui->offsetXSpinBox->setEnabled(new_x == 0);
		ui->offsetYSpinBox->setEnabled(new_y == 0);

		if (new_x == 0) {
			ui->offsetXSpinBox->blockSignals(true);
			ui->offsetXSpinBox->setValue(active_section_->get_offset()->x);
			ui->offsetYSpinBox->blockSignals(false);
		}
		if (new_y == 0) {
			ui->offsetYSpinBox->blockSignals(true);
			ui->offsetYSpinBox->setValue(active_section_->get_offset()->y);
			ui->offsetYSpinBox->blockSignals(false);
		}
	}

	/**
	 * Enables Show controls.
	 * @param enabled If true, Show controls are enabled.
	 */
	void MaestroControlWidget::set_show_controls_enabled(bool enabled) {
		ui->showSettingsGroupBox->setEnabled(enabled);
		ui->eventGroupBox->setEnabled(enabled);
		ui->toggleShowModeCheckBox->setEnabled(enabled);
	}

	/**
	 * Displays extra controls for Animations that take custom parameters.
	 * @param animation Pointer to the Animation being displayed.
	 */
	void MaestroControlWidget::set_advanced_animation_controls(Animation* animation) {
		// First, remove any existing extra control widgets
		if (animation_extra_control_widget_ != nullptr) {
			this->findChild<QLayout*>("animationAdvancedSettingsLayout")->removeWidget(animation_extra_control_widget_.get());
			animation_extra_control_widget_.reset();
		}

		QLayout* layout = this->findChild<QLayout*>("animationAdvancedSettingsLayout");

		switch(animation->get_type()) {
			case AnimationType::Fire:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new FireAnimationControlWidget((FireAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Lightning:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new LightningAnimationControlWidget((LightningAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Plasma:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new PlasmaAnimationControlWidget((PlasmaAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Radial:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new RadialAnimationControlWidget((RadialAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Sparkle:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new SparkleAnimationControlWidget((SparkleAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Wave:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new WaveAnimationControlWidget((WaveAnimation*)animation, this, layout->widget()));
			default:
				break;
		}

		ui->animationAdvancedSettingsGroupBox->setVisible(animation_extra_control_widget_ != nullptr);

		if (animation_extra_control_widget_) {
			layout->addWidget(animation_extra_control_widget_.get());
		}
	}

	/// Sets the active Animation's timer.
	void MaestroControlWidget::set_animation_timer() {
		uint16_t pause = ui->delaySpinBox->value();
		uint16_t new_interval = ui->animationIntervalSpinBox->value();
		AnimationTimer* timer = active_section_->get_animation()->get_timer();
		if (new_interval != timer->get_interval() || pause != timer->get_delay()) {
			run_cue(animation_handler->set_timer(get_section_index(), get_layer_index(), new_interval, pause));
		}
	}

	/// Sets the interval between Canvas frames.
	void MaestroControlWidget::set_canvas_frame_interval() {
		run_cue(canvas_handler->set_frame_timer(get_section_index(), get_layer_index(), ui->frameIntervalSpinBox->value()));
	}

	/**
	 * Sets Layer-related controls enabled.
	 * @param visible True if you want to enable the controls.
	 */
	void MaestroControlWidget::set_layer_controls_enabled(bool enabled) {
		// If visible, enable Layer controls
		ui->mixModeLabel->setEnabled(enabled);
		ui->mix_modeComboBox->setEnabled(enabled);
		ui->alphaLabel->setEnabled(enabled);
		ui->alphaSpinBox->setEnabled(enabled);
	}

	/**
	 * Renders the Maestro's last update time in currentTimeLineEdit.
	 */
	void MaestroControlWidget::update_maestro_last_time() {
		uint last_time = (uint)maestro_controller_->get_total_elapsed_time();
		ui->currentTimeLineEdit->setText(locale_.toString(last_time));

		int current_index = maestro_controller_->get_maestro()->get_show()->get_current_index();

		// Get the index of the last Event that ran
		Show* show = maestro_controller_->get_maestro()->get_show();
		int last_index = -1;
		if (current_index == 0) {
			if (show->get_looping()) {
				last_index = show->get_num_events() - 1;
			}
		}
		else {
			last_index = show->get_current_index() - 1;
		}

		// If Relative mode is enabled, calculate the time since the last Event
		if (last_index == -1) {
			ui->relativeTimeLineEdit->setEnabled(false);
		}
		else {
			ui->relativeTimeLineEdit->setEnabled(true);
			ui->relativeTimeLineEdit->setText(locale().toString((uint)maestro_controller_->get_total_elapsed_time() - (uint)show->get_last_time()));
		}


		// Visually disable events that have recently ran.
		for (int index = 0; index < ui->eventListWidget->count(); index++) {
			if (current_index > 0 && index < current_index) {
				ui->eventListWidget->item(index)->setTextColor(Qt::GlobalColor::darkGray);
			}
			else {
				ui->eventListWidget->item(index)->setTextColor(Qt::GlobalColor::white);
			}
		}
	}

	/**
	 * Destructor.
	 */
	MaestroControlWidget::~MaestroControlWidget() {
		delete show_controller_;
		delete ui;
	}
}
