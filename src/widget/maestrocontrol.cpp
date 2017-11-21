#include <QDataStream>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include "animation/lightninganimation.h"
#include "animation/lightninganimationcontrol.h"
#include "animation/plasmaanimation.h"
#include "animation/plasmaanimationcontrol.h"
#include "animation/radialanimation.h"
#include "animation/radialanimationcontrol.h"
#include "animation/sparkleanimation.h"
#include "animation/sparkleanimationcontrol.h"
#include "canvas/animationcanvas.h"
#include "canvas/colorcanvas.h"
#include "canvas/canvascontrol.h"
#include "colorpresets.h"
#include "controller/maestrocontroller.h"
#include "core/section.h"
#include "drawingarea/simpledrawingarea.h"
#include "maestrocontrol.h"
#include "window/settingsdialog.h"
#include "widget/palettecontrol.h"
#include "ui_maestrocontrol.h"
#include "utility/canvasutility.h"

/**
 * Constructor.
 * @param parent The QWidget containing this controller.
 * @param maestro_controller The MaestroController being controlled.
 */
MaestroControl::MaestroControl(QWidget* parent, MaestroController* maestro_controller) : QWidget(parent), ui(new Ui::MaestroControl) {
	ui->setupUi(this);

	this->maestro_controller_ = maestro_controller;

	// Open serial connection to Arduino (if configured)
	QSettings settings;
	if (settings.value(SettingsDialog::serial_enabled).toBool()) {

		/*
		 * If the virtual device is selected, connect to a new VirtualSerialDeviceDialog.
		 * Cues are sent to the device in send_to_device().
		 * Otherwise initialize the serial port like normal.
		 */
		if (settings.value(SettingsDialog::serial_port).toString().contains(SettingsDialog::virtual_device_option)) {
			virtual_device_dialog_ = std::unique_ptr<VirtualSerialDeviceDialog>(new VirtualSerialDeviceDialog(this));
			virtual_device_dialog_.get()->show();
		}
		else {
			serial_port_.setPortName(QString(settings.value(SettingsDialog::serial_port).toString()));
			serial_port_.setBaudRate(9600);

			// https://stackoverflow.com/questions/13312869/serial-communication-with-arduino-fails-only-on-the-first-message-after-restart
			serial_port_.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
			serial_port_.setParity(QSerialPort::Parity::NoParity);
			serial_port_.setDataBits(QSerialPort::DataBits::Data8);
			serial_port_.setStopBits(QSerialPort::StopBits::OneStop);

			if (!serial_port_.open(QIODevice::WriteOnly)) {
				QMessageBox::warning(nullptr, QString("Serial Failure"), QString("Failed to open serial device: " + serial_port_.errorString()));
			}
		}
	}

	// Initialize Cue Controller
	cue_controller_ = maestro_controller_->get_maestro()->set_cue_controller();
	animation_handler = static_cast<AnimationCueHandler*>(cue_controller_->enable_handler(CueController::Handler::AnimationHandler));
	canvas_handler = static_cast<CanvasCueHandler*>(cue_controller_->enable_handler(CueController::Handler::CanvasHandler));
	maestro_handler = static_cast<MaestroCueHandler*>(cue_controller_->enable_handler(CueController::Handler::MaestroHandler));
	section_handler = static_cast<SectionCueHandler*>(cue_controller_->enable_handler(CueController::Handler::SectionHandler));
	show_handler = static_cast<ShowCueHandler*>(cue_controller_->enable_handler(CueController::Handler::ShowHandler));

	// Finally, initialize the MaestroControl
	initialize();
}

/**
 * Enables Show Edit mode, which disables updates to the Maestro.
 * @param enable If true, don't update the Maestro.
 */
void MaestroControl::enable_show_edit_mode(bool enable) {
	show_mode_enabled_ = enable;
}

/**
 * Forwards the specified Cue to the drawing area and/or serial device.
 * @param cue Cue to perform.
 */
void MaestroControl::execute_cue(uint8_t *cue) {
	// Only render to outputs if Show Edit mode isn't enabled
	if (!show_mode_enabled_) {
		cue_controller_->run(cue);

		// Update the virtual Maestro
		if (virtual_device_dialog_ != nullptr) {
			virtual_device_dialog_->get_maestro()->get_cue_controller()->run(cue);
		}

		// Send to serial device
		if (serial_port_.isOpen()) {
			serial_port_.write((const char*)cue, cue_controller_->get_cue_size(cue));
		}
	}

	// Update the ShowControl's history
	if (show_control_widget_ != nullptr) {
		static_cast<ShowControl*>(show_control_widget_.get())->add_cue_to_history(cue);
	}
}

/**
 * Returns the index of the current Overlay.
 * @return Overlay index.
 */
int16_t MaestroControl::get_overlay_index() {
	/*
	 * Find the depth of the current Section by traversing parent_section_.
	 * Once parent_section == nullptr, we know we've hit the base Section.
	 */
	uint8_t level = 0;
	Section* target_section = active_section_;
	while (target_section->get_parent_section() != nullptr) {
		target_section = target_section->get_parent_section();
		level++;
	}
	return level;
}

uint8_t MaestroControl::get_overlay_index(Section* section) {
	uint8_t level = 0;
	Section* test_section = section;
	while (test_section->get_parent_section() != nullptr) {
		test_section = test_section->get_parent_section();
		level++;
	}
	return level;
}

uint8_t MaestroControl::get_section_index(Section* section) {

	uint8_t index = 0;
	Section* test_section = section;
	Section* target_section = maestro_controller_->get_maestro()->get_section(0);
	while (index < maestro_controller_->get_maestro()->get_num_sections() && target_section != test_section) {
		index++;
		target_section = maestro_controller_->get_maestro()->get_section(index);
	}

	if (index < maestro_controller_->get_maestro()->get_num_sections()) {
		return index;
	}
	else {
		return 0;
	}
}

/**
 * Returns the index of the current Section.
 * @return Current Section index (or -1 if not found).
 */
int16_t MaestroControl::get_section_index() {
	Section* target_section = active_section_;

	// If this is an Overlay, iterate until we find the parent ID
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
 * Gets the number of Overlays belonging to the Section.
 * @param section Section to scan.
 * @return Number of Overlays.
 */
uint8_t MaestroControl::get_num_overlays(Section *section) {
	uint8_t count = 0;
	Section::Overlay* overlay = section->get_overlay();
	while (overlay != nullptr) {
		overlay = overlay->section->get_overlay();
		count++;
	}
	return count;
}

/**
 * Build the initial UI.
 */
void MaestroControl::initialize() {
	// Clear out and re-add the Maestro's Sections
	active_section_ = nullptr;
	maestro_controller_->reset_sections();

	QSettings settings;
	maestro_controller_->set_sections(settings.value(SettingsDialog::num_sections, 1).toInt());
	Section* section = maestro_controller_->get_maestro()->get_section(0);

	initialize_palettes();

	// There must be a better way to bulk block signals.
	ui->animationComboBox->blockSignals(true);
	ui->orientationComboBox->blockSignals(true);
	ui->sectionComboBox->blockSignals(true);
	ui->overlayComboBox->blockSignals(true);
	ui->mix_modeComboBox->blockSignals(true);
	ui->alphaSpinBox->blockSignals(true);
	ui->canvasComboBox->blockSignals(true);

	ui->animationComboBox->clear();
	ui->orientationComboBox->clear();
	ui->sectionComboBox->clear();
	ui->overlayComboBox->clear();
	ui->mix_modeComboBox->clear();
	ui->alphaSpinBox->clear();
	ui->canvasComboBox->clear();

	// Populate Animation combo box
	ui->animationComboBox->addItems({"Blink", "Cycle", "Lightning", "Mandelbrot", "Merge", "Plasma", "Radial", "Random", "Solid", "Sparkle", "Wave"});
	ui->orientationComboBox->addItems({"Horizontal", "Vertical"});

	// Set Section/Overlay
	for (uint8_t section = 1; section <= maestro_controller_->get_maestro()->get_num_sections(); section++) {
		ui->sectionComboBox->addItem(QString("Section ") + QString::number(section));
	}
	ui->sectionComboBox->setCurrentIndex(0);

	// Initialize Overlay controls
	ui->mix_modeComboBox->addItems({"None", "Alpha", "Multiply", "Overlay"});

	// Initialize Canvas controls
	ui->canvasComboBox->addItems({"No Canvas", "Animation Canvas", "Color Canvas"});

	// Unblock signals (*grumble grumble*)
	ui->animationComboBox->blockSignals(false);
	ui->orientationComboBox->blockSignals(false);
	ui->sectionComboBox->blockSignals(false);
	ui->overlayComboBox->blockSignals(false);
	ui->mix_modeComboBox->blockSignals(false);
	ui->alphaSpinBox->blockSignals(false);
	ui->canvasComboBox->blockSignals(false);

	// Hide Overlay controls
	set_overlay_controls_visible(false);

	set_active_section(section);

	populate_overlay_combobox();
}

/// Reinitializes Palettes from Palette Dialog.
void MaestroControl::initialize_palettes() {
	ui->colorComboBox->blockSignals(true);
	ui->colorComboBox->clear();

	// Populate color combo box
	for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
		ui->colorComboBox->addItem(palette_controller_.get_palette(i)->name);
	}
	ui->colorComboBox->blockSignals(false);
}

/**
 * Sets the Overlay's transparency level.
 * @param arg1 Transparency level from 0 - 255.
 */
void MaestroControl::on_alphaSpinBox_valueChanged(int arg1) {
	if (active_section_->get_parent_section() == nullptr) {
		return;
	}

	execute_cue(section_handler->set_overlay(get_section_index(), get_overlay_index(active_section_->get_parent_section()), active_section_->get_parent_section()->get_overlay()->mix_mode, arg1));
}

/**
 * Changes the current animation.
 * @param index Index of the new animation.
 */
void MaestroControl::on_animationComboBox_currentIndexChanged(int index) {

	// Only change if the animation is different
	if (active_section_->get_animation() == nullptr || active_section_->get_animation()->get_type() == index) {
		return;
	}

	// Preserve the animation cycle between changes
	execute_cue(section_handler->set_animation(get_section_index(), get_overlay_index(), (AnimationType::Type)index, true, nullptr, 0));
	show_extra_controls(active_section_->get_animation());

	// Reapply animation settings
	on_colorComboBox_currentIndexChanged(ui->colorComboBox->currentIndex());
	on_orientationComboBox_currentIndexChanged(ui->orientationComboBox->currentIndex());
	on_fadeCheckBox_toggled(ui->fadeCheckBox->isChecked());
	on_reverse_animationCheckBox_toggled(ui->reverse_animationCheckBox->isChecked());
	on_cycleSlider_valueChanged(ui->cycleSlider->value());
}

/**
 * Changes the current Canvas.
 * @param index Index of the new Canvas type.
 */
void MaestroControl::on_canvasComboBox_currentIndexChanged(int index) {
	// Remove the existing Canvas
	execute_cue(section_handler->remove_canvas(get_section_index(), get_overlay_index()));

	// Add the new Canvas
	if (index > 0) {
		execute_cue(section_handler->set_canvas(get_section_index(), get_overlay_index(), (CanvasType::Type)(index - 1)));
	}

	show_canvas_controls(index > 0);
}

/**
 * Changes the color scheme.
 * If 'Custom' is selected, this also displays controls for adjusting the custom color scheme.
 * @param index Index of the new color scheme.
 */
void MaestroControl::on_colorComboBox_currentIndexChanged(int index) {
	PaletteController::Palette* palette = palette_controller_.get_palette(index);
	execute_cue(animation_handler->set_colors(get_section_index(), get_overlay_index(), &palette->colors[0], palette->colors.size()));
}

/**
 * Changes the number of columns in the display grid.
 */
void MaestroControl::on_columnsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

/**
 * Changes the cycle speed.
 * @param value New cycle speed.
 */
void MaestroControl::on_cycleSlider_valueChanged(int value) {
	ui->cycleSpinBox->blockSignals(true);
	ui->cycleSpinBox->setValue(value);
	ui->cycleSpinBox->blockSignals(false);

	set_speed();
}

void MaestroControl::on_cycleSpinBox_editingFinished() {
	ui->cycleSlider->blockSignals(true);
	ui->cycleSlider->setValue(ui->cycleSpinBox->value());
	ui->cycleSlider->blockSignals(false);

	set_speed();
}

void MaestroControl::on_enableShowCheckBox_toggled(bool checked) {
	QLayout* show_layout = this->findChild<QLayout*>("showControlLayout");
	show_layout->removeWidget(show_control_widget_.get());
	show_control_widget_.reset();

	// Initialize Show if necessary
	if (checked) {
		if (show_controller_ == nullptr) {
			show_controller_ = new ShowController(maestro_controller_);
		}

		show_control_widget_ = std::unique_ptr<QWidget>(new ShowControl(show_controller_, cue_controller_, this));
		show_layout->addWidget(show_control_widget_.get());
	}
}

/**
 * Toggles fading.
 * @param checked If true, fading is enabled.
 */
void MaestroControl::on_fadeCheckBox_toggled(bool checked) {
	execute_cue(animation_handler->set_fade(get_section_index(), get_overlay_index(), checked));
}

/**
 * Changes the Overlay's mix mode.
 * @param index
 */
void MaestroControl::on_mix_modeComboBox_currentIndexChanged(int index) {
	if (active_section_->get_parent_section() == nullptr) {
		return;
	}

	if ((Colors::MixMode)index != active_section_->get_parent_section()->get_overlay()->mix_mode) {
		execute_cue(section_handler->set_overlay(get_section_index(), get_overlay_index(active_section_->get_parent_section()), (Colors::MixMode)index, ui->alphaSpinBox->value()));

		// Show/hide spin box for alpha only
		if ((Colors::MixMode)index == Colors::MixMode::Alpha) {
			ui->alphaSpinBox->setVisible(true);
		}
		else {
			ui->alphaSpinBox->setVisible(false);
		}
	}
}

void MaestroControl::on_offsetResetButton_clicked() {

	execute_cue(animation_handler->reset_center(get_section_index(), get_overlay_index()));

	Point center = Point(active_section_->get_dimensions()->x / 2, active_section_->get_dimensions()->y / 2);

	ui->offsetXSpinBox->blockSignals(true);
	ui->offsetYSpinBox->blockSignals(true);
	ui->offsetXSpinBox->setValue(center.x);
	ui->offsetYSpinBox->setValue(center.y);
	ui->offsetXSpinBox->blockSignals(false);
	ui->offsetYSpinBox->blockSignals(false);

	set_center();
}

void MaestroControl::on_offsetXSpinBox_editingFinished() {
	set_center();
}

void MaestroControl::on_offsetYSpinBox_editingFinished() {
	set_center();
}

/**
 * Sets the animation's orientation.
 * @param index New orientation.
 */
void MaestroControl::on_orientationComboBox_currentIndexChanged(int index) {
	if ((Animation::Orientation)index != active_section_->get_animation()->get_orientation()) {
		execute_cue(animation_handler->set_orientation(get_section_index(), get_overlay_index(), (Animation::Orientation)index));
	}
}

void MaestroControl::on_overlayComboBox_currentIndexChanged(int index) {
	/*
	 * If we selected an Overlay, iterate through the Section's nested Overlays until we find it.
	 * If we selected 'None', use the base Section as the active Section.
	 */
	Section* overlay_section = maestro_controller_->get_maestro()->get_section(get_section_index());
	for (int i = 0; i < index; i++) {
		overlay_section = overlay_section->get_overlay()->section;
	}

	// Show Overlay controls
	set_overlay_controls_visible(index > 0);

	// Set active Section to Overlay Section
	set_active_section(overlay_section);
}

void MaestroControl::on_overlaySpinBox_editingFinished() {
	Section* base_section = maestro_controller_->get_maestro()->get_section(get_section_index());
	Section* last_section = base_section;

	// Get the current number of Overlays
	int num_overlays = get_num_overlays(base_section);

	// Get the last Overlay in the Maestro
	while (last_section->get_overlay() != nullptr) {
		last_section = last_section->get_overlay()->section;
	}

	int diff = ui->overlaySpinBox->value() - num_overlays;

	// If diff is positive, add more Overlays
	if (diff > 0) {
		while (diff > 0) {
			execute_cue(section_handler->set_overlay(get_section_index(base_section), get_overlay_index(last_section), Colors::MixMode::None, 0));
			diff--;
		}
	}
	// If the diff is negative, remove Overlays
	else if (diff < 0) {
		// To be safe, jump back to the base Section
		on_overlayComboBox_currentIndexChanged(0);

		while (diff < 0) {
			last_section = last_section->get_parent_section();
			execute_cue(section_handler->remove_overlay(get_section_index(base_section), get_overlay_index(last_section)));
			diff++;
		}
	}

	populate_overlay_combobox();
}

/**
 * Opens the Palette Editor.
 */
void MaestroControl::on_paletteControlButton_clicked() {
	QString palette_name = ui->colorComboBox->currentText();

	PaletteControl palette_control(&palette_controller_, palette_name);
	palette_control.exec();
	initialize_palettes();
	ui->colorComboBox->setCurrentText(palette_name);
}

/**
 * Sets the delay between Animation cycles.
 * @param value New pause interval.
 */
void MaestroControl::on_pauseSlider_valueChanged(int value) {
	ui->pauseSpinBox->blockSignals(true);
	ui->pauseSpinBox->setValue(value);
	ui->pauseSpinBox->blockSignals(false);

	set_speed();
}

/**
 * Sets the delay between Animation cycles.
 * @param arg1 New pause interval.
 */
void MaestroControl::on_pauseSpinBox_valueChanged(int arg1) {
	ui->pauseSlider->blockSignals(true);
	ui->pauseSlider->setValue(arg1);
	ui->pauseSlider->blockSignals(false);

	set_speed();
}

/**
 * Toggles whether the color animation is shown in reverse.
 * @param checked If true, reverse the animation.
 */
void MaestroControl::on_reverse_animationCheckBox_toggled(bool checked) {
	execute_cue(animation_handler->set_reverse(get_section_index(), get_overlay_index(), checked));
}

/**
 * Changes the number of rows in the displayed grid.
 */
void MaestroControl::on_rowsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

void MaestroControl::on_sectionComboBox_currentIndexChanged(int index) {
	// Hide Overlay controls
	set_overlay_controls_visible(false);

	Section* section = maestro_controller_->get_maestro()->get_section(index);

	// Set active controller
	set_active_section(section);

	// Set Overlay count
	ui->overlaySpinBox->blockSignals(true);
	ui->overlaySpinBox->setValue(get_num_overlays(section));
	ui->overlaySpinBox->blockSignals(false);

	populate_overlay_combobox();
}

/**
 * Sets the size of the active Section.
 * @param x Number of rows.
 * @param y Number of columns.
 */
void MaestroControl::on_section_resize(uint16_t x, uint16_t y) {
	// Note: Resizing serial devices is intentionally unavailable.
	if ((x != active_section_->get_dimensions()->x) || (y != active_section_->get_dimensions()->y)) {

		/*
		 * Check for a Canvas.
		 * The old Canvas gets deleted on resize.
		 * If one is set, copy its contents to a temporary buffer, then copy it back once the new Canvas is created.
		 */
		if (active_section_->get_canvas() != nullptr) {
			if (active_section_->get_canvas()->get_type() == CanvasType::AnimationCanvas) {
				AnimationCanvas* canvas = static_cast<AnimationCanvas*>(active_section_->get_canvas());
				// Create temporary frameset
				Point frame_bounds(canvas->get_section()->get_dimensions()->x, canvas->get_section()->get_dimensions()->y);
				bool** frames = new bool*[canvas->get_num_frames()];
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					frames[frame] = new bool[frame_bounds.size()];
				}
				CanvasUtility::copy_from_canvas(canvas, frames, frame_bounds.x, frame_bounds.y);

				execute_cue(section_handler->set_dimensions(get_section_index(), get_overlay_index(), x, y));

				CanvasUtility::copy_to_canvas(canvas, frames, frame_bounds.x, frame_bounds.y, this);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
			else if (active_section_->get_canvas()->get_type() == CanvasType::ColorCanvas) {
				ColorCanvas* canvas = static_cast<ColorCanvas*>(active_section_->get_canvas());
				// Create temporary frameset
				Point frame_bounds(canvas->get_section()->get_dimensions()->x, canvas->get_section()->get_dimensions()->y);
				Colors::RGB** frames = new Colors::RGB*[canvas->get_num_frames()];
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					frames[frame] = new Colors::RGB[frame_bounds.size()];
				}
				CanvasUtility::copy_from_canvas(canvas, frames, frame_bounds.x, frame_bounds.y);

				execute_cue(section_handler->set_dimensions(get_section_index(), get_overlay_index(), x, y));

				CanvasUtility::copy_to_canvas(canvas, frames, frame_bounds.x, frame_bounds.y, this);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
		}
		else {	// No Canvas set
			execute_cue(section_handler->set_dimensions(get_section_index(), get_overlay_index(), x, y));
		}

		// Reset Animation center
		execute_cue(animation_handler->set_center(get_section_index(), get_overlay_index(), x / 2,	y / 2));
		on_offsetResetButton_clicked();
	}
}

/**
 * Rebuilds the Overlay combobox using the current active Section.
 */
void MaestroControl::populate_overlay_combobox() {
	ui->overlayComboBox->blockSignals(true);
	ui->overlayComboBox->clear();
	ui->overlayComboBox->addItem("Base Section");

	for (uint8_t overlay = 1; overlay <= get_num_overlays(maestro_controller_->get_maestro()->get_section(get_section_index())); overlay++) {
		ui->overlayComboBox->addItem(QString("Overlay ") + QString::number(overlay));
	}

	ui->overlayComboBox->blockSignals(false);
}

void MaestroControl::read_from_file(QString filename) {
	QFile file(filename);

	if (file.open(QFile::ReadOnly)) {
		// Reinitialize UI
		//initialize();

		QByteArray bytes = file.readAll();
		for (int i = 0; i < bytes.size(); i++) {
			uint8_t byte = (uint8_t)bytes.at(i);
			// Send the Cue on successful read
			if(cue_controller_->read(byte)) {
				// Yes, this means executing twice for a DrawingArea, but it's a temporary convenience
				execute_cue(cue_controller_->get_cue());
			}
		}
		file.close();
	}

	// Reinitialize UI with base Section
	on_sectionComboBox_currentIndexChanged(0);
}

void MaestroControl::save_to_file(QString filename) {
	if (!filename.endsWith(".pmc", Qt::CaseInsensitive)) {
		filename.append(".pmc");
	}
	QFile file(filename);
	if (file.open(QFile::WriteOnly)) {
		QDataStream datastream(&file);

		for (uint8_t i = 0; i < maestro_controller_->get_maestro()->get_num_sections(); i++) {
			save_section_settings(&datastream, i, 0);
		}

		file.flush();
		file.close();
	}
}

void MaestroControl::save_maestro_settings(QDataStream *datastream) {
	// Timing
	write_cue_to_stream(datastream, maestro_handler->set_timing(maestro_controller_->get_maestro()->get_timing()->get_interval()));

	// Save Show settings
	Show* show = maestro_controller_->get_maestro()->get_show();
	if (show != nullptr) {
		write_cue_to_stream(datastream, show_handler->set_events(show->get_events(), show->get_num_events()));
		write_cue_to_stream(datastream, show_handler->set_looping(show->get_looping()));
		write_cue_to_stream(datastream, show_handler->set_timing(show->get_timing()));
	}
}

void MaestroControl::save_section_settings(QDataStream* datastream, uint8_t section_id, uint8_t overlay_id) {

	Section* section = maestro_controller_->get_maestro()->get_section(section_id);

	if (overlay_id > 0) {
		for (uint8_t i = 0; i < overlay_id; i++) {
			section = section->get_overlay()->section;
		}
	}

	// Dimensions
	write_cue_to_stream(datastream, section_handler->set_dimensions(section_id, overlay_id, section->get_dimensions()->x, section->get_dimensions()->y));

	// Animation & Colors
	Animation* animation = section->get_animation();
	write_cue_to_stream(datastream,section_handler->set_animation(section_id, overlay_id, animation->get_type(), false, animation->get_colors(), animation->get_num_colors(), false));
	write_cue_to_stream(datastream, animation_handler->set_orientation(section_id, overlay_id, animation->get_orientation()));
	write_cue_to_stream(datastream, animation_handler->set_reverse(section_id, overlay_id, animation->get_reverse()));
	write_cue_to_stream(datastream, animation_handler->set_fade(section_id, overlay_id, animation->get_fade()));
	write_cue_to_stream(datastream, animation_handler->set_timing(section_id, overlay_id, animation->get_timing()->get_interval(), animation->get_timing()->get_pause()));

	switch(animation->get_type()) {
		case AnimationType::Lightning:
			{
				LightningAnimation* la = static_cast<LightningAnimation*>(animation);
				animation_handler->set_lightning_options(section_id, overlay_id, la->get_bolt_count(), la->get_down_threshold(), la->get_up_threshold(), la->get_fork_chance());
			}
			break;
		case AnimationType::Plasma:
			{
				PlasmaAnimation* pa = static_cast<PlasmaAnimation*>(animation);
				animation_handler->set_plasma_options(section_id, overlay_id, pa->get_size(), pa->get_resolution());
			}
			break;
		case AnimationType::Radial:
			{
				RadialAnimation* ra = static_cast<RadialAnimation*>(animation);
				animation_handler->set_radial_options(section_id, overlay_id, ra->get_resolution());
			}
			break;
		case AnimationType::Sparkle:
			{
				SparkleAnimation* sa = static_cast<SparkleAnimation*>(animation);
				animation_handler->set_sparkle_options(section_id, overlay_id, sa->get_threshold());
			}
			break;
		default:
			break;
	}
	write_cue_to_stream(datastream, cue_controller_->get_cue());

	// Canvas
	Canvas* canvas = section->get_canvas();
	if (canvas != nullptr) {
		write_cue_to_stream(datastream, section_handler->set_canvas(section_id, overlay_id, canvas->get_type(), canvas->get_num_frames()));

		// Draw and save each frame
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			switch (canvas->get_type()) {
				case CanvasType::AnimationCanvas:
					canvas_handler->draw_frame(section_id, overlay_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<AnimationCanvas*>(canvas)->get_frame(frame));
					break;
				case CanvasType::ColorCanvas:
					canvas_handler->draw_frame(section_id, overlay_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<ColorCanvas*>(canvas)->get_frame(frame));
					break;
			}
			write_cue_to_stream(datastream, cue_controller_->get_cue());
		}
	}

	// Overlays
	Section::Overlay* overlay = section->get_overlay();
	if (overlay != nullptr) {
		write_cue_to_stream(datastream, section_handler->set_overlay(section_id, overlay_id, overlay->mix_mode, overlay->alpha));
		save_section_settings(datastream, section_id, overlay_id + 1);
	}
}

/**
 * Changes the active Section and sets all GUI controls to the Section's settings.
 * @param section New active Section.
 */
void MaestroControl::set_active_section(Section* section) {
	if (section == nullptr) {
		return;
	}

	active_section_ = section;

	// Set dimensions
	ui->columnsSpinBox->blockSignals(true);
	ui->rowsSpinBox->blockSignals(true);
	// These are intentionally reversed
	ui->columnsSpinBox->setValue(section->get_dimensions()->y);
	ui->rowsSpinBox->setValue(section->get_dimensions()->x);
	ui->columnsSpinBox->blockSignals(false);
	ui->rowsSpinBox->blockSignals(false);

	// Get animation options and speed
	// If no animation is set, initialize one.
	if (section->get_animation() == nullptr) {
		PaletteController::Palette* palette = palette_controller_.get_palette("Color Wheel");
		execute_cue(section_handler->set_animation(get_section_index(), get_overlay_index(), AnimationType::Solid, true, &palette->colors[0], palette->colors.size()));
		execute_cue(animation_handler->set_timing(get_section_index(), get_overlay_index(), 1000));
	}

	Animation* animation = section->get_animation();

	ui->orientationComboBox->blockSignals(true);
	ui->reverse_animationCheckBox->blockSignals(true);
	ui->fadeCheckBox->blockSignals(true);
	ui->cycleSlider->blockSignals(true);
	ui->cycleSpinBox->blockSignals(true);
	ui->pauseSlider->blockSignals(true);
	ui->pauseSpinBox->blockSignals(true);
	ui->offsetXSpinBox->blockSignals(true);
	ui->offsetYSpinBox->blockSignals(true);
	ui->orientationComboBox->setCurrentIndex(animation->get_orientation());
	ui->reverse_animationCheckBox->setChecked(animation->get_reverse());
	ui->fadeCheckBox->setChecked(animation->get_fade());
	ui->cycleSlider->setValue(animation->get_timing()->get_interval());
	ui->cycleSpinBox->setValue(animation->get_timing()->get_interval());
	ui->pauseSlider->setValue(animation->get_timing()->get_pause());
	ui->pauseSpinBox->setValue(animation->get_timing()->get_pause());
	ui->offsetXSpinBox->setValue(animation->get_center()->x);
	ui->offsetYSpinBox->setValue(animation->get_center()->y);
	ui->orientationComboBox->blockSignals(false);
	ui->reverse_animationCheckBox->blockSignals(false);
	ui->fadeCheckBox->blockSignals(false);
	ui->cycleSlider->blockSignals(false);
	ui->cycleSpinBox->blockSignals(false);
	ui->pauseSlider->blockSignals(false);
	ui->pauseSpinBox->blockSignals(false);
	ui->offsetXSpinBox->blockSignals(false);
	ui->offsetYSpinBox->blockSignals(false);

	/*
	 * Select Palette.
	 * Find the corresponding palette in the Palette Controller.
	 * If it's not there, add it.
	 */
	bool palette_found = false;
	for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
		PaletteController::Palette* palette = palette_controller_.get_palette(i);
		if (*palette == animation->get_colors()) {
			ui->colorComboBox->blockSignals(true);
			ui->colorComboBox->setCurrentText(palette->name);
			ui->colorComboBox->blockSignals(false);
			palette_found = true;
			continue;
		}
	}

	// Palette not found
	if (!palette_found) {
		QString name = "Custom - ";
		if (section->get_parent_section() == nullptr) {
			name += "Section " + QString::number(get_section_index());
		}
		else {
			name += "Overlay " + QString::number(get_overlay_index());
		}
		palette_controller_.add_palette(name, animation->get_colors(), animation->get_num_colors());
		ui->colorComboBox->blockSignals(true);
		ui->colorComboBox->addItem(name);
		ui->colorComboBox->setCurrentText(name);
		ui->colorComboBox->blockSignals(false);
	}

	// Set the animation
	ui->animationComboBox->blockSignals(true);
	ui->animationComboBox->setCurrentIndex(animation->get_type());
	ui->animationComboBox->blockSignals(false);
	show_extra_controls(animation);

	// Get Overlay MixMode and alpha from the Overlay's parent section
	if (section->get_parent_section() != nullptr) {
		ui->mix_modeComboBox->blockSignals(true);
		ui->alphaSpinBox->blockSignals(true);
		ui->mix_modeComboBox->setCurrentIndex(section->get_parent_section()->get_overlay()->mix_mode);
		ui->alphaSpinBox->setValue(section->get_parent_section()->get_overlay()->alpha);
		ui->mix_modeComboBox->blockSignals(false);
		ui->alphaSpinBox->blockSignals(false);
	}

	// Get Canvas
	ui->canvasComboBox->blockSignals(true);
	if (section->get_canvas() != nullptr) {
		Canvas* canvas = section->get_canvas();
		ui->canvasComboBox->setCurrentIndex((int)canvas->get_type() + 1);
	}
	else {
		ui->canvasComboBox->setCurrentIndex(0);
	}

	show_canvas_controls(section->get_canvas() != nullptr);
	ui->canvasComboBox->blockSignals(false);
}

void MaestroControl::set_center() {
	execute_cue(animation_handler->set_center(get_section_index(), get_overlay_index(), ui->offsetXSpinBox->value(), ui->offsetYSpinBox->value()));
}

/// Sets the speed and/or pause interval for the active Animation.
void MaestroControl::set_speed() {
	uint16_t pause = ui->pauseSpinBox->value();
	uint16_t speed = ui->cycleSpinBox->value();
	AnimationTiming* timing = active_section_->get_animation()->get_timing();
	if (speed != timing->get_interval() || pause != timing->get_pause()) {
		execute_cue(animation_handler->set_timing(get_section_index(), get_overlay_index(), speed, pause));
	}
}

/**
 * Sets the visibility of Overlay-related controls.
 * @param visible True if you want to show the controls.
 */
void MaestroControl::set_overlay_controls_visible(bool visible) {
	// If visible, show Overlay controls
	ui->mixModeLabel->setVisible(visible);
	ui->mix_modeComboBox->setVisible(visible);
	ui->alphaSpinBox->setVisible(visible);

	// Invert layout controls
	ui->gridSizeLabel->setVisible(!visible);
	ui->columnsSpinBox->setVisible(!visible);
	ui->rowsSpinBox->setVisible(!visible);
}

/**
 * Displays extra controls for animations that take custom parameters.
 * @param index Index of the animation in the animations list.
 * @param animation Pointer to the animation.
 */
void MaestroControl::show_extra_controls(Animation* animation) {
	// First, remove any existing extra control widgets
	if (animation_extra_control_widget_ != nullptr) {
		this->findChild<QLayout*>("animationExtraOptionsLayout")->removeWidget(animation_extra_control_widget_.get());
		animation_extra_control_widget_.reset();
	}

	QLayout* layout = this->findChild<QLayout*>("animationExtraOptionsLayout");

	switch(animation->get_type()) {
		case AnimationType::Lightning:
			animation_extra_control_widget_ = std::unique_ptr<QWidget>(new LightningAnimationControl((LightningAnimation*)animation, this, layout->widget()));
			break;
		case AnimationType::Plasma:
			animation_extra_control_widget_ = std::unique_ptr<QWidget>(new PlasmaAnimationControl((PlasmaAnimation*)animation, this, layout->widget()));
			break;
		case AnimationType::Radial:
			animation_extra_control_widget_ = std::unique_ptr<QWidget>(new RadialAnimationControl((RadialAnimation*)animation, this, layout->widget()));
			break;
		case AnimationType::Sparkle:
			animation_extra_control_widget_ = std::unique_ptr<QWidget>(new SparkleAnimationControl((SparkleAnimation*)animation, this, layout->widget()));
			break;
		default:
			break;
	}

	if (animation_extra_control_widget_) {
		layout->addWidget(animation_extra_control_widget_.get());
	}
}

void MaestroControl::show_canvas_controls(bool visible) {
	QLayout* layout = this->findChild<QLayout*>("canvasControlsLayout");

	// Remove the Canvas controls.
	// If a Canvas is set, re-initialize and re-add the controls.
	layout->removeWidget(canvas_control_widget_.get());
	canvas_control_widget_.reset();

	if (visible) {
		canvas_control_widget_ = std::unique_ptr<QWidget>(new CanvasControl(this));
		layout->addWidget(canvas_control_widget_.get());
	}
}

/**
 * Appends a Cue to a stream.
 * @param stream Stream to append to.
 * @param cue Cue to append.
 */
void MaestroControl::write_cue_to_stream(QDataStream* stream, uint8_t* cue) {
	stream->writeRawData((const char*)cue, cue_controller_->get_cue_size(cue));
}

/**
 * Destructor.
 */
MaestroControl::~MaestroControl() {
	if (serial_port_.isOpen()) {
		serial_port_.close();
	}
	delete show_controller_;
	delete ui;
}