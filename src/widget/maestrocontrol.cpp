#include <QColorDialog>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
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

	// Open serial connections to output devices
	QSettings settings;
	int serial_count = settings.beginReadArray(SettingsDialog::output_devices);
	for (int device = 0; device < serial_count; device++) {
		settings.setArrayIndex(device);
		if (settings.value(SettingsDialog::output_enabled).toInt() > 0) {
			// Detect and skip over the screen
			if (settings.value(SettingsDialog::output_name).toString().compare(SettingsDialog::main_window_option, Qt::CaseInsensitive) == 0) { }
			// Detect and initialize the simulated serial device
			else if (settings.value(SettingsDialog::output_name).toString().compare(SettingsDialog::detached_window_option, Qt::CaseInsensitive) == 0) {
				drawing_area_dialog_ = std::unique_ptr<SimpleDrawingAreaDialog>(new SimpleDrawingAreaDialog(this, this->maestro_controller_));
				drawing_area_dialog_.get()->show();
			}
			// Detect all other devices (serial/USB)
			else {
				// Initialize the serial device
				QSharedPointer<QSerialPort> serial_device(new QSerialPort());
				serial_device->setPortName(QString(settings.value(SettingsDialog::serial_port).toString()));
				serial_device->setBaudRate(9600);

				// https://stackoverflow.com/questions/13312869/serial-communication-with-arduino-fails-only-on-the-first-message-after-restart
				serial_device->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
				serial_device->setParity(QSerialPort::Parity::NoParity);
				serial_device->setDataBits(QSerialPort::DataBits::Data8);
				serial_device->setStopBits(QSerialPort::StopBits::OneStop);

				if (!serial_device->open(QIODevice::WriteOnly)) {
					QMessageBox::warning(nullptr, QString("Serial Failure"), QString("Failed to open serial device: " + serial_device->errorString()));
				}
				serial_devices_.push_back(serial_device);
			}
		}
	}
	settings.endArray();

	// Initialize Cue Controller
	cue_controller_ = maestro_controller_->get_maestro()->set_cue_controller(UINT16_MAX);
	animation_handler = static_cast<AnimationCueHandler*>(cue_controller_->enable_handler(CueController::Handler::AnimationHandler));
	canvas_handler = static_cast<CanvasCueHandler*>(cue_controller_->enable_handler(CueController::Handler::CanvasHandler));
	maestro_handler = static_cast<MaestroCueHandler*>(cue_controller_->enable_handler(CueController::Handler::MaestroHandler));
	section_handler = static_cast<SectionCueHandler*>(cue_controller_->enable_handler(CueController::Handler::SectionHandler));
	show_handler = static_cast<ShowCueHandler*>(cue_controller_->enable_handler(CueController::Handler::ShowHandler));

	// Clear out and re-add the Maestro's Sections
	active_section_ = nullptr;
	maestro_controller_->reset_sections();

	maestro_controller_->set_sections(settings.value(SettingsDialog::num_sections, 1).toInt());

	// Finally, initialize the MaestroControl
	initialize();
}

/**
 * Adds an action to the event history.
 * @param cue Action performed.
 */
void MaestroControl::add_cue_to_history(uint8_t *cue) {
	ui->eventHistoryWidget->addItem(cue_interpreter_.interpret_cue(cue));

	// Convert the Cue into an actual byte array, which we'll store in the Event History
	uint16_t cue_size = cue_controller_->get_cue_size(cue);
	QVector<uint8_t> cue_vector(cue_size);
	for (uint16_t byte = 0; byte < cue_size; byte++) {
		cue_vector[byte] = cue[byte];
	}

	event_history_.push_back(cue_vector);

	// Start removing older items
	if (event_history_.size() >= 10) {
		ui->eventHistoryWidget->takeItem(0);
		event_history_.remove(0);
	}
}

/**
 * Enables Show Edit mode, which disables updates to the Maestro.
 * @param enable If true, don't update the Maestro.
 */
void MaestroControl::enable_show_edit_mode(bool enable) {
	show_mode_enabled_ = enable;
}

/**
 * Returns the index of the current Layer.
 * @return Layer index.
 */
int16_t MaestroControl::get_layer_index() {
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

/**
 * Returns the index of the specified Layer.
 * @param section Section belonging to the Layer.
 * @return Layer index.
 */
uint8_t MaestroControl::get_layer_index(Section* section) {
	uint8_t level = 0;
	Section* test_section = section;
	while (test_section->get_parent_section() != nullptr) {
		test_section = test_section->get_parent_section();
		level++;
	}
	return level;
}

/**
 * Returns the index of the specified Section.
 * @param section Section to ID.
 * @return Section index.
 */
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
 * Gets the number of Layers belonging to the Section.
 * @param section Section to scan.
 * @return Number of Layers.
 */
uint8_t MaestroControl::get_num_layers(Section *section) {
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
void MaestroControl::initialize() {
	// Get the base Section
	Section* section = maestro_controller_->get_maestro()->get_section(0);

	initialize_palettes();

	// There must be a better way to bulk block signals.
	ui->sectionComboBox->blockSignals(true);
	ui->layerComboBox->blockSignals(true);
	ui->alphaSpinBox->blockSignals(true);

	ui->sectionComboBox->clear();
	ui->layerComboBox->clear();
	ui->alphaSpinBox->clear();

	// Set Section/Layer
	for (uint8_t section = 1; section <= maestro_controller_->get_maestro()->get_num_sections(); section++) {
		ui->sectionComboBox->addItem(QString("Section ") + QString::number(section));
	}
	ui->sectionComboBox->setCurrentIndex(0);

	// Unblock signals (*grumble grumble*)
	ui->sectionComboBox->blockSignals(false);
	ui->layerComboBox->blockSignals(false);
	ui->alphaSpinBox->blockSignals(false);

	// Disable advanced controls until they're activated manually
	set_canvas_controls_enabled(false, CanvasType::AnimationCanvas);
	set_layer_controls_enabled(false);
	set_show_controls_enabled(false);

	// Initialize Show timer
	show_timer_.setTimerType(Qt::CoarseTimer);
	show_timer_.setInterval(250);
	connect(&show_timer_, SIGNAL(timeout()), this, SLOT(update_maestro_last_time()));

	// Set Show controls
	Show* show = maestro_controller_->get_maestro()->get_show();
	if (show != nullptr) {
		on_enableShowCheckBox_toggled(true);
		ui->enableShowCheckBox->setChecked(true);

		for (uint16_t i = 0; i < show->get_num_events(); i++) {
			Event* event = &show->get_events()[i];
			show_controller_->add_event(event->get_time(), event->get_cue());
			ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));
		}

		show_controller_->initialize_events();

		ui->timingMethodComboBox->setEnabled(false);
		ui->timingMethodComboBox->setCurrentIndex(show->get_timing());
		ui->timingMethodComboBox->setEnabled(true);
	}

	// Initialize Canvas elements
	// Add radio buttons to groups
	canvas_shape_type_group_.addButton(ui->circleRadioButton);
	canvas_shape_type_group_.addButton(ui->lineRadioButton);
	canvas_shape_type_group_.addButton(ui->rectRadioButton);
	canvas_shape_type_group_.addButton(ui->textRadioButton);
	canvas_shape_type_group_.addButton(ui->triangleRadioButton);

	// Set Canvas defaults
	ui->currentFrameSpinBox->setEnabled(false);

	// Add fonts
	ui->fontComboBox->addItems({"5x8"});

	// Finally, open up the default Section
	set_active_section(section);
	populate_layer_combobox();
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
 * Adds the selected Event(s) to the Show's Event list.
 */
void MaestroControl::on_addEventButton_clicked() {
	// Add selected Cues to the Show
	for (QModelIndex index : ui->eventHistoryWidget->selectionModel()->selectedIndexes()) {
		Event* event = show_controller_->add_event(ui->eventTimeSpinBox->value(), (uint8_t*)&event_history_.at(index.row()).at(0));
		ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));
	}
	show_controller_->initialize_events();
}

/**
 * Sets the Layer's transparency level.
 * @param arg1 Transparency level from 0 - 255.
 */
void MaestroControl::on_alphaSpinBox_valueChanged(int arg1) {
	if (active_section_->get_parent_section() == nullptr) {
		return;
	}

	run_cue(section_handler->set_layer(get_section_index(), get_layer_index(active_section_->get_parent_section()), active_section_->get_parent_section()->get_layer()->mix_mode, arg1));
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
	run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), (AnimationType::Type)index, true, nullptr, 0));
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
	run_cue(section_handler->remove_canvas(get_section_index(), get_layer_index()));

	// Add the new Canvas
	if (index > 0) {
		run_cue(section_handler->set_canvas(get_section_index(), get_layer_index(), (CanvasType::Type)(index - 1)));
		set_canvas_controls_enabled(true, CanvasType::Type(index - 1));

		// Default to circle radio button so that the controls can be refreshed
		ui->circleRadioButton->setChecked(true);
		set_circle_controls_enabled(true);
	}
	else {
		set_canvas_controls_enabled(false, CanvasType::Type(index - 1));

		// Check if Edit Frame mode is enabled, and disable it
		if (ui->toggleCanvasModeCheckBox->isChecked()) {
			on_toggleCanvasModeCheckBox_toggled(false);
		}
	}
}

/**
 * Sets the offset x value.
 */
void MaestroControl::on_offsetXSpinBox_editingFinished() {
	set_offset();
}

/**
 * Sets the offset y value.
 */
void MaestroControl::on_offsetYSpinBox_editingFinished() {
	set_offset();
}

/**
 * Selects a circle for drawing.
 * @param checked If true, next shape will be a circle.
 */
void MaestroControl::on_circleRadioButton_toggled(bool checked) {
	set_circle_controls_enabled(checked);
}

/**
 * Clears the current Canvas frame.
 */
void MaestroControl::on_clearButton_clicked() {
	QMessageBox::StandardButton confirm;
	confirm = QMessageBox::question(this, "Clear Canvas Frame", "This action will clear the current Canvas frame. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
	if (confirm == QMessageBox::Yes) {
		run_cue(canvas_handler->clear(get_section_index(), get_layer_index()));
	}
}

/**
 * Changes the color scheme.
 * @param index Index of the new color scheme.
 */
void MaestroControl::on_colorComboBox_currentIndexChanged(int index) {
	PaletteController::Palette* palette = palette_controller_.get_palette(index);
	run_cue(animation_handler->set_colors(get_section_index(), get_layer_index(), &palette->colors[0], palette->colors.size()));
}

/**
 * Changes the number of columns in the display grid.
 */
void MaestroControl::on_columnsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

/**
 * Switches to the Canvas Frame index specified in currentFrameSpinBox.
 */
void MaestroControl::on_currentFrameSpinBox_editingFinished() {
	int frame = ui->currentFrameSpinBox->value();

	// If the selected frame exceeds the number of frames, set to the number of frames.
	if (frame >= active_section_->get_canvas()->get_num_frames()) {
		frame = active_section_->get_canvas()->get_num_frames() - 1;
		ui->currentFrameSpinBox->setValue(frame);
	}
	else {
		run_cue(canvas_handler->set_current_frame_index(get_section_index(), get_layer_index(), frame));
	}
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

/**
 * Changes the cycle speed.
 */
void MaestroControl::on_cycleSpinBox_editingFinished() {
	ui->cycleSlider->blockSignals(true);
	ui->cycleSlider->setValue(ui->cycleSpinBox->value());
	ui->cycleSlider->blockSignals(false);

	set_speed();
}

/**
 * Handles drawing onto the current Canvas frame.
 */
void MaestroControl::on_drawButton_clicked() {
	QAbstractButton* checked_button = canvas_shape_type_group_.checkedButton();

	if ((CanvasType::Type)(ui->canvasComboBox->currentIndex() - 1) == CanvasType::ColorCanvas) {
		if (checked_button == ui->circleRadioButton) {
			run_cue(canvas_handler->draw_circle(get_section_index(), get_layer_index(), canvas_rgb_color_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->lineRadioButton) {
			run_cue(canvas_handler->draw_line(get_section_index(), get_layer_index(), canvas_rgb_color_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value()));
		}
		else if (checked_button == ui->rectRadioButton) {
			run_cue(canvas_handler->draw_rect(get_section_index(), get_layer_index(), canvas_rgb_color_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->textRadioButton) {
			run_cue(canvas_handler->draw_text(get_section_index(), get_layer_index(), canvas_rgb_color_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), (Font::Type)ui->fontComboBox->currentIndex(), ui->textLineEdit->text().toLatin1().data(), ui->textLineEdit->text().size()));
		}
		else {	// Triangle
			run_cue(canvas_handler->draw_triangle(get_section_index(), get_layer_index(), canvas_rgb_color_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->target2XSpinBox->value(), ui->target2YSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
	}
	else {
		if (checked_button == ui->circleRadioButton) {
			run_cue(canvas_handler->draw_circle(get_section_index(), get_layer_index(), ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->lineRadioButton) {
			run_cue(canvas_handler->draw_line(get_section_index(), get_layer_index(), ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value()));
		}
		else if (checked_button == ui->rectRadioButton) {
			run_cue(canvas_handler->draw_rect(get_section_index(), get_layer_index(), ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
		else if (checked_button == ui->textRadioButton) {
			run_cue(canvas_handler->draw_text(get_section_index(), get_layer_index(), ui->originXSpinBox->value(), ui->originYSpinBox->value(), (Font::Type)ui->fontComboBox->currentIndex(), ui->textLineEdit->text().toLatin1().data(), ui->textLineEdit->text().size()));
		}
		else {	// Triangle
			run_cue(canvas_handler->draw_triangle(get_section_index(), get_layer_index(), ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->target2XSpinBox->value(), ui->target2YSpinBox->value(), ui->fillCheckBox->isChecked()));
		}
	}
}

/**
 * Initializes the Maestro's Show.
 * @param checked If true, creates a new Show.
 */
void MaestroControl::on_enableShowCheckBox_toggled(bool checked) {
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
void MaestroControl::on_fadeCheckBox_toggled(bool checked) {
	run_cue(animation_handler->set_fade(get_section_index(), get_layer_index(), checked));
}

/**
 * Sets the number of Canvas frames.
 */
void MaestroControl::on_frameCountSpinBox_editingFinished() {
	int new_max = ui->frameCountSpinBox->value();
	if (new_max < ui->currentFrameSpinBox->value()) {
		ui->currentFrameSpinBox->setValue(new_max);
	}
	ui->currentFrameSpinBox->setMaximum(new_max);
	run_cue(canvas_handler->set_num_frames(get_section_index(), get_layer_index(), new_max));

	// Set the new maximum for the current_frame spinbox
	ui->currentFrameSpinBox->setMaximum(new_max);
}

/**
 * Changes the Canvas' frame rate.
 */
void MaestroControl::on_frameRateSpinBox_editingFinished() {
	if (!ui->toggleCanvasModeCheckBox->isChecked()) {
		run_cue(canvas_handler->set_frame_timing(get_section_index(), get_layer_index(), ui->frameRateSpinBox->value()));
	}
}

/**
 * Changes the Layer's mix mode.
 * @param index
 */
void MaestroControl::on_mix_modeComboBox_currentIndexChanged(int index) {
	if (active_section_->get_parent_section() == nullptr) {
		return;
	}

	if ((Colors::MixMode)index != active_section_->get_parent_section()->get_layer()->mix_mode) {
		run_cue(section_handler->set_layer(get_section_index(), get_layer_index(active_section_->get_parent_section()), (Colors::MixMode)index, ui->alphaSpinBox->value()));

		// Enable spin box for alpha only
		if ((Colors::MixMode)index == Colors::MixMode::Alpha) {
			ui->alphaSpinBox->setEnabled(true);
		}
		else {
			ui->alphaSpinBox->setEnabled(false);
		}
	}
}

/**
 * Selects a rectangle for the next shape.
 * @param checked If true, the next shape will be a rectangle.
 */
void MaestroControl::on_rectRadioButton_toggled(bool checked) {
	set_rect_controls_enabled(checked);
}

/**
 * Sets the Canvas scroll rate along the x axis.
 */
void MaestroControl::on_scrollXSpinBox_editingFinished() {
	set_scroll();
}

/**
 * Sets the Canvas scroll rate along the y axis.
 */
void MaestroControl::on_scrollYSpinBox_editingFinished() {
	set_scroll();
}

/**
 * Sets the animation's orientation.
 * @param index New orientation.
 */
void MaestroControl::on_orientationComboBox_currentIndexChanged(int index) {
	if ((Animation::Orientation)index != active_section_->get_animation()->get_orientation()) {
		run_cue(animation_handler->set_orientation(get_section_index(), get_layer_index(), (Animation::Orientation)index));
	}
}

/**
 * Changes the current Layer.
 * @param index Index of the desired Layer.
 */
void MaestroControl::on_layerComboBox_currentIndexChanged(int index) {
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
void MaestroControl::on_layerSpinBox_editingFinished() {
	Section* base_section = maestro_controller_->get_maestro()->get_section(get_section_index());
	Section* last_section = base_section;

	// Get the current number of Layers
	int num_layers = get_num_layers(base_section);

	// Get the last Layer in the Maestro
	while (last_section->get_layer() != nullptr) {
		last_section = last_section->get_layer()->section;
	}

	int diff = ui->layerSpinBox->value() - num_layers;

	// If diff is positive, add more Layers
	if (diff > 0) {
		while (diff > 0) {
			run_cue(section_handler->set_layer(get_section_index(base_section), get_layer_index(last_section), Colors::MixMode::None, 0));
			diff--;
		}
	}
	// If the diff is negative, remove Layers
	else if (diff < 0) {
		// To be safe, jump back to the base Section
		on_layerComboBox_currentIndexChanged(0);

		while (diff < 0) {
			last_section = last_section->get_parent_section();
			run_cue(section_handler->remove_layer(get_section_index(base_section), get_layer_index(last_section)));
			diff++;
		}
	}

	populate_layer_combobox();
}

/**
 * Selects a line for the next shape.
 * @param checked If true, the next shape will be a line.
 */
void MaestroControl::on_lineRadioButton_toggled(bool checked) {
	set_line_controls_enabled(checked);
}

/**
 * Loads an image into the Canvas.
 * This overwrites all Canvas frames.
 */
void MaestroControl::on_loadImageButton_clicked() {
	QString filename = QFileDialog::getOpenFileName(this,
		QString("Open Image"),
		QDir::home().path(),
		QString("Images (*.bmp *.gif *.jpg *.png)"));

	CanvasUtility::load_image(filename, active_section_->get_canvas(), this);

	// Set the number of frames
	ui->frameCountSpinBox->blockSignals(true);
	ui->frameCountSpinBox->setValue(active_section_->get_canvas()->get_num_frames());
	ui->frameCountSpinBox->blockSignals(false);
}

/**
 * Enables/disables Show looping.
 * @param checked If true, the Show will loop its Event queue.
 */
void MaestroControl::on_loopCheckBox_toggled(bool checked) {
	run_cue(show_handler->set_looping(checked));
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
 * Removes the selected Event(s) from the Show.
 */
void MaestroControl::on_removeEventButton_clicked() {
	for (QModelIndex index : ui->eventListWidget->selectionModel()->selectedIndexes()) {
		show_controller_->remove_event(index.row());
		ui->eventListWidget->takeItem(index.row());
	}
	show_controller_->initialize_events();
}

/**
 * Toggles whether the color animation is shown in reverse.
 * @param checked If true, reverse the animation.
 */
void MaestroControl::on_reverse_animationCheckBox_toggled(bool checked) {
	run_cue(animation_handler->set_reverse(get_section_index(), get_layer_index(), checked));
}

/**
 * Changes the number of rows in the displayed grid.
 */
void MaestroControl::on_rowsSpinBox_editingFinished() {
	on_section_resize(ui->rowsSpinBox->value(), ui->columnsSpinBox->value());
}

/**
 * Changes the current Section.
 * @param index Index of the desired Section.
 */
void MaestroControl::on_sectionComboBox_currentIndexChanged(int index) {
	// Hide Layer controls
	set_layer_controls_enabled(false);

	Section* section = maestro_controller_->get_maestro()->get_section(index);

	// Set active controller
	set_active_section(section);

	// Set Layer count
	ui->layerSpinBox->blockSignals(true);
	ui->layerSpinBox->setValue(get_num_layers(section));
	ui->layerSpinBox->blockSignals(false);

	populate_layer_combobox();
}

/**
 * Sets the size of the active Section.
 * @param x Number of rows.
 * @param y Number of columns.
 */
void MaestroControl::on_section_resize(uint16_t x, uint16_t y) {
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

				run_cue(section_handler->set_dimensions(get_section_index(), get_layer_index(), x, y));

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

				run_cue(section_handler->set_dimensions(get_section_index(), get_layer_index(), x, y));

				CanvasUtility::copy_to_canvas(canvas, frames, frame_bounds.x, frame_bounds.y, this);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
		}
		else {	// No Canvas set
			run_cue(section_handler->set_dimensions(get_section_index(), get_layer_index(), x, y));
		}
	}
}

/**
 * Sets the ColorCanvas drawing color.
 */
void MaestroControl::on_selectColorButton_clicked() {
	QColor new_color = QColorDialog::getColor(canvas_color_, this);

	if (new_color != canvas_color_) {
		canvas_color_ = new_color;
	}

	ui->selectColorButton->setStyleSheet("background-color: " + canvas_color_.name());

	canvas_rgb_color_.r = canvas_color_.red();
	canvas_rgb_color_.g = canvas_color_.green();
	canvas_rgb_color_.b = canvas_color_.blue();
}

/**
 * Selects text for the next Canvas shape.
 * @param checked If true, the next shape will be text.
 */
void MaestroControl::on_textRadioButton_toggled(bool checked) {
	set_text_controls_enabled(checked);
}

/**
 * Changes the Show's timing method.
 * @param index Index of the timing method in timingMethodComboBox.
 */
void MaestroControl::on_timingMethodComboBox_currentIndexChanged(int index) {
	maestro_controller_->get_maestro()->get_show()->set_timing((Show::TimingMode)index);

	// Enable/disable loop controls for relative mode
	if ((Show::TimingMode)index == Show::TimingMode::Relative) {
		ui->loopCheckBox->setEnabled(true);
		ui->loopLabel->setEnabled(true);
	}
	else {
		ui->loopCheckBox->setEnabled(false);
		ui->loopLabel->setEnabled(false);
	}
}

/**
 * Toggles Canvas Edit mode, which prevents the Canvas from automatically switching frames.
 * @param checked If true, Canvas Edit mode is enabled.
 */
void MaestroControl::on_toggleCanvasModeCheckBox_toggled(bool checked) {
	// Enables/disable the 'current frame' control
	ui->currentFrameSpinBox->setEnabled(checked);

	if (checked) {
		run_cue(canvas_handler->remove_frame_timing(get_section_index(), get_layer_index()));
		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(active_section_->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}
	else {
		on_frameRateSpinBox_editingFinished();
	}
}

/**
 * Toggles Show Edit mode. Show Edit mode lets the user populate the Event History without affecting the output.
 * @param checked If true, Show Edit mode is enabled.
 */
void MaestroControl::on_toggleShowModeCheckBox_clicked(bool checked) {
	enable_show_edit_mode(checked);
}

/**
 * Sets the next shape to triangle.
 * @param checked If true, the next Canvas shape drawn will be a triangle.
 */
void MaestroControl::on_triangleRadioButton_toggled(bool checked) {
	set_triangle_controls_enabled(checked);
}

/**
 * Rebuilds the Layer combobox using the current active Section.
 */
void MaestroControl::populate_layer_combobox() {
	ui->layerComboBox->blockSignals(true);
	ui->layerComboBox->clear();
	ui->layerComboBox->addItem("Base Section");

	for (uint8_t layer = 1; layer <= get_num_layers(maestro_controller_->get_maestro()->get_section(get_section_index())); layer++) {
		ui->layerComboBox->addItem(QString("Layer ") + QString::number(layer));
	}

	ui->layerComboBox->blockSignals(false);
}

/**
 * Reads a Cuefile.
 * @param filename The name and path of the Cuefile.
 */
void MaestroControl::read_from_file(QString filename) {
	QFile file(filename);

	if (file.open(QFile::ReadOnly)) {
		QByteArray bytes = file.readAll();
		for (int i = 0; i < bytes.size(); i++) {
			uint8_t byte = (uint8_t)bytes.at(i);
			// Send the Cue on successful read
			if(cue_controller_->read(byte)) {
				// Yes, this means executing twice in some cases, but it's a temporary convenience
				run_cue(cue_controller_->get_buffer());
			}
		}
		file.close();
	}

	// Reinitialize UI
	initialize();

	// Set starting Section to 0 (to be safe)
	on_sectionComboBox_currentIndexChanged(0);
}

/**
 * Forwards the specified Cue to the drawing area and/or serial device.
 * @param cue Cue to perform.
 */
void MaestroControl::run_cue(uint8_t *cue) {
	// Only render to outputs if Show Edit mode isn't enabled
	if (!show_mode_enabled_) {
		cue_controller_->run(cue);

		/*
		 * Send to serial devices.
		 * Certain actions (e.g. grid resizing) should be caught here and prevented from running.
		 */
		if (cue[CueController::Byte::PayloadByte] != CueController::Handler::SectionHandler &&
			cue[SectionCueHandler::Byte::ActionByte] != SectionCueHandler::Action::SetDimensions) {
			for (int i = 0; i < serial_devices_.size(); i++) {
				if (serial_devices_[i]->isOpen()) {
					serial_devices_[i]->write((const char*)cue, cue_controller_->get_cue_size(cue));
				}
			}
		}
	}

	// Update the ShowControl's history
	if (show_controller_ != nullptr) {
		add_cue_to_history(cue);
	}
}

/**
 * Saves the Maestro to a Cuefile.
 * @param filename Target filename.
 */
void MaestroControl::save_to_file(QString filename) {
	if (!filename.endsWith(".pmc", Qt::CaseInsensitive)) {
		filename.append(".pmc");
	}
	QFile file(filename);
	if (file.open(QFile::WriteOnly)) {
		QDataStream datastream(&file);

		save_maestro_settings(&datastream);
		for (uint8_t i = 0; i < maestro_controller_->get_maestro()->get_num_sections(); i++) {
			save_section_settings(&datastream, i, 0);
		}

		file.close();
	}
}

/**
 * Saves Maestro-specific settings as Cues.
 * @param datastream Stream to save the Cues to.
 */
void MaestroControl::save_maestro_settings(QDataStream *datastream) {
	// Timing
	write_cue_to_stream(datastream, maestro_handler->set_timing(maestro_controller_->get_maestro()->get_timing()->get_interval()));

	// Save Show settings
	Show* show = maestro_controller_->get_maestro()->get_show();
	if (show != nullptr) {
		write_cue_to_stream(datastream, maestro_handler->set_show());
		write_cue_to_stream(datastream, show_handler->set_events(show->get_events(), show->get_num_events(), true));
		write_cue_to_stream(datastream, show_handler->set_looping(show->get_looping()));
		write_cue_to_stream(datastream, show_handler->set_timing(show->get_timing()));
	}
}

/**
 * Saves Section-specific settings as Cues.
 * @param datastream Stream to save the Cues to.
 * @param section_id The index of the Section to save.
 * @param layer_id The index of the Layer to save.
 */
void MaestroControl::save_section_settings(QDataStream* datastream, uint8_t section_id, uint8_t layer_id) {

	Section* section = maestro_controller_->get_maestro()->get_section(section_id);

	if (layer_id > 0) {
		for (uint8_t i = 0; i < layer_id; i++) {
			section = section->get_layer()->section;
		}
	}

	// Dimensions
	write_cue_to_stream(datastream, section_handler->set_dimensions(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y));

	// Animation & Colors
	Animation* animation = section->get_animation();
	write_cue_to_stream(datastream,section_handler->set_animation(section_id, layer_id, animation->get_type(), false, animation->get_colors(), animation->get_num_colors(), false));
	write_cue_to_stream(datastream, animation_handler->set_orientation(section_id, layer_id, animation->get_orientation()));
	write_cue_to_stream(datastream, animation_handler->set_reverse(section_id, layer_id, animation->get_reverse()));
	write_cue_to_stream(datastream, animation_handler->set_fade(section_id, layer_id, animation->get_fade()));
	write_cue_to_stream(datastream, animation_handler->set_timing(section_id, layer_id, animation->get_timing()->get_interval(), animation->get_timing()->get_pause()));
	// Save Animation-specific settings
	switch(animation->get_type()) {
		case AnimationType::Lightning:
			{
				LightningAnimation* la = static_cast<LightningAnimation*>(animation);
				write_cue_to_stream(datastream, animation_handler->set_lightning_options(section_id, layer_id, la->get_bolt_count(), la->get_down_threshold(), la->get_up_threshold(), la->get_fork_chance()));
			}
			break;
		case AnimationType::Plasma:
			{
				PlasmaAnimation* pa = static_cast<PlasmaAnimation*>(animation);
				write_cue_to_stream(datastream, animation_handler->set_plasma_options(section_id, layer_id, pa->get_size(), pa->get_resolution()));
			}
			break;
		case AnimationType::Radial:
			{
				RadialAnimation* ra = static_cast<RadialAnimation*>(animation);
				write_cue_to_stream(datastream, animation_handler->set_radial_options(section_id, layer_id, ra->get_resolution()));
			}
			break;
		case AnimationType::Sparkle:
			{
				SparkleAnimation* sa = static_cast<SparkleAnimation*>(animation);
				write_cue_to_stream(datastream, animation_handler->set_sparkle_options(section_id, layer_id, sa->get_threshold()));
			}
			break;
		default:
			break;
	}

	// Scrolling and offset
	write_cue_to_stream(datastream, section_handler->set_offset(section_id, layer_id, section->get_offset()->x, section->get_offset()->y));
	if (section->get_scroll()) {
		write_cue_to_stream(datastream, section_handler->set_scroll(section_id, layer_id, section->get_scroll()->interval_x, section->get_scroll()->interval_y));
	}

	// Save Canvas settings
	Canvas* canvas = section->get_canvas();
	if (canvas != nullptr) {
		write_cue_to_stream(datastream, section_handler->set_canvas(section_id, layer_id, canvas->get_type(), canvas->get_num_frames()));

		if (canvas->get_frame_timing()) {
			write_cue_to_stream(datastream, canvas_handler->set_frame_timing(section_id, layer_id, canvas->get_frame_timing()->get_interval()));
		}

		// Draw and save each frame
		for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
			switch (canvas->get_type()) {
				case CanvasType::AnimationCanvas:
					write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<AnimationCanvas*>(canvas)->get_frame(frame)));
					break;
				case CanvasType::ColorCanvas:
					write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<ColorCanvas*>(canvas)->get_frame(frame)));
					break;
			}
			if (canvas->get_current_frame_index() != canvas->get_num_frames() - 1) {
				write_cue_to_stream(datastream, canvas_handler->next_frame(section_id, layer_id));
			}
		}
		write_cue_to_stream(datastream, canvas_handler->set_current_frame_index(get_section_index(), get_layer_index(), 0));
	}

	// Layers
	Section::Layer* layer = section->get_layer();
	if (layer != nullptr) {
		write_cue_to_stream(datastream, section_handler->set_layer(section_id, layer_id, layer->mix_mode, layer->alpha));
		save_section_settings(datastream, section_id, layer_id + 1);
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

	// Get scroll settings
	ui->scrollXSpinBox->blockSignals(true);
	ui->scrollYSpinBox->blockSignals(true);
	if (section->get_scroll() != nullptr) {
		ui->scrollXSpinBox->setValue(section->get_scroll()->interval_x);
		ui->scrollYSpinBox->setValue(section->get_scroll()->interval_y);
	}
	ui->scrollXSpinBox->blockSignals(false);
	ui->scrollYSpinBox->blockSignals(false);

	// Get Layer settings
	if (section->get_layer()) {
		ui->layerSpinBox->blockSignals(true);
		ui->layerSpinBox->setValue(get_num_layers(section));
		ui->layerSpinBox->blockSignals(false);
	}

	// If this is an Layer, get the MixMode and alpha
	if (section->get_parent_section() != nullptr) {
		ui->mix_modeComboBox->blockSignals(true);
		ui->alphaSpinBox->blockSignals(true);
		ui->mix_modeComboBox->setCurrentIndex(section->get_parent_section()->get_layer()->mix_mode);
		ui->alphaSpinBox->setValue(section->get_parent_section()->get_layer()->alpha);
		ui->mix_modeComboBox->blockSignals(false);
		ui->alphaSpinBox->blockSignals(false);
	}

	// Get animation options and speed
	// If no animation is set, initialize one.
	if (section->get_animation() == nullptr) {
		PaletteController::Palette* palette = palette_controller_.get_palette("Color Wheel");
		run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), AnimationType::Solid, true, &palette->colors[0], palette->colors.size()));
		run_cue(animation_handler->set_timing(get_section_index(), get_layer_index(), 1000));
	}

	// Animation settings
	Animation* animation = section->get_animation();
	ui->orientationComboBox->blockSignals(true);
	ui->reverse_animationCheckBox->blockSignals(true);
	ui->fadeCheckBox->blockSignals(true);
	ui->cycleSlider->blockSignals(true);
	ui->cycleSpinBox->blockSignals(true);
	ui->pauseSlider->blockSignals(true);
	ui->pauseSpinBox->blockSignals(true);
	ui->orientationComboBox->setCurrentIndex(animation->get_orientation());
	ui->reverse_animationCheckBox->setChecked(animation->get_reverse());
	ui->fadeCheckBox->setChecked(animation->get_fade());
	ui->cycleSlider->setValue(animation->get_timing()->get_interval());
	ui->cycleSpinBox->setValue(animation->get_timing()->get_interval());
	ui->pauseSlider->setValue(animation->get_timing()->get_pause());
	ui->pauseSpinBox->setValue(animation->get_timing()->get_pause());
	ui->orientationComboBox->blockSignals(false);
	ui->reverse_animationCheckBox->blockSignals(false);
	ui->fadeCheckBox->blockSignals(false);
	ui->cycleSlider->blockSignals(false);
	ui->cycleSpinBox->blockSignals(false);
	ui->pauseSlider->blockSignals(false);
	ui->pauseSpinBox->blockSignals(false);

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
			name += "Layer " + QString::number(get_layer_index());
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

	// Get Canvas
	ui->canvasComboBox->blockSignals(true);
	ui->frameCountSpinBox->blockSignals(true);
	ui->currentFrameSpinBox->blockSignals(true);
	ui->frameRateSpinBox->blockSignals(true);

	Canvas* canvas = section->get_canvas();
	if (canvas != nullptr) {
		ui->canvasComboBox->setCurrentIndex((int)canvas->get_type() + 1);
		ui->frameCountSpinBox->setValue(canvas->get_num_frames());
		ui->currentFrameSpinBox->setValue(canvas->get_current_frame_index());
		if (canvas->get_frame_timing() != nullptr) {
			ui->frameRateSpinBox->setValue(canvas->get_frame_timing()->get_interval());
		}
		set_canvas_controls_enabled(true, canvas->get_type());
	}
	else {
		ui->canvasComboBox->setCurrentIndex(0);
		ui->frameCountSpinBox->setValue(1);
		ui->currentFrameSpinBox->setValue(1);
		ui->frameRateSpinBox->setValue(100);
		set_canvas_controls_enabled(false, CanvasType::Type::AnimationCanvas);
	}

	ui->frameCountSpinBox->blockSignals(false);
	ui->currentFrameSpinBox->blockSignals(false);
	ui->frameRateSpinBox->blockSignals(false);
	ui->canvasComboBox->blockSignals(false);
}

/**
 * Enables Canvas controls.
 * @param enabled If true, controls are enabled.
 * @param type The type of Canvas.
 */
void MaestroControl::set_canvas_controls_enabled(bool enabled, CanvasType::Type type) {
	ui->toggleCanvasModeLabel->setEnabled(enabled);
	ui->toggleCanvasModeCheckBox->setEnabled(enabled);
	ui->frameCountLabel->setEnabled(enabled);
	ui->frameCountSpinBox->setEnabled(enabled);
	ui->currentFrameLabel->setEnabled(enabled);
	ui->currentFrameSpinBox->setEnabled(enabled);
	ui->frameRateLabel->setEnabled(enabled);
	ui->frameRateSpinBox->setEnabled(enabled);
	ui->drawingToolsLabel->setEnabled(enabled);
	ui->circleRadioButton->setEnabled(enabled);
	ui->lineRadioButton->setEnabled(enabled);
	ui->triangleRadioButton->setEnabled(enabled);
	ui->textRadioButton->setEnabled(enabled);
	ui->rectRadioButton->setEnabled(enabled);
	ui->originLabel->setEnabled(enabled);
	ui->originXSpinBox->setEnabled(enabled);
	ui->originYSpinBox->setEnabled(enabled);
	ui->targetLabel->setEnabled(enabled);
	ui->targetXSpinBox->setEnabled(enabled);
	ui->targetYSpinBox->setEnabled(enabled);
	ui->target2Label->setEnabled(enabled);
	ui->target2XSpinBox->setEnabled(enabled);
	ui->target2YSpinBox->setEnabled(enabled);
	ui->fontLabel->setEnabled(enabled);
	ui->fontComboBox->setEnabled(enabled);
	ui->textLabel->setEnabled(enabled);
	ui->textLineEdit->setEnabled(enabled);
	ui->selectColorButton->setEnabled(enabled && type == CanvasType::ColorCanvas);
	ui->fillCheckBox->setEnabled(enabled);
	ui->loadImageButton->setEnabled(enabled);
	ui->drawButton->setEnabled(enabled);
	ui->clearButton->setEnabled(enabled);
}

/**
 * Enables Canvas circle controls.
 * @param enabled If true, circle controls are enabled.
 */
void MaestroControl::set_circle_controls_enabled(bool enabled) {
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
void MaestroControl::set_line_controls_enabled(bool enabled) {
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
 * Enables Canvas rectangle controls.
 * @param enabled If true, rectangle controls are enabled.
 */
void MaestroControl::set_rect_controls_enabled(bool enabled) {
	if (enabled) {
		set_circle_controls_enabled(false);
		set_line_controls_enabled(false);
		set_text_controls_enabled(false);
		set_triangle_controls_enabled(false);
		ui->targetLabel->setText(QString("Size"));
	}
	else {
		ui->targetLabel->setText(QString("Target"));
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
void MaestroControl::set_text_controls_enabled(bool enabled) {
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
void MaestroControl::set_triangle_controls_enabled(bool enabled) {
	if (enabled) {
		set_circle_controls_enabled(false);
		set_line_controls_enabled(false);
		set_rect_controls_enabled(false);
		set_text_controls_enabled(false);

		ui->originLabel->setText(QString("Point A"));
		ui->targetLabel->setText(QString("Point B"));
		ui->target2Label->setText(QString("Point C"));
	}
	else {
		ui->originLabel->setText(QString("Origin"));
		ui->targetLabel->setText(QString("Target"));
		ui->target2Label->setText(QString("Target 2"));
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
void MaestroControl::set_offset() {
	// Don't allow changes if scrolling is enabled
	if (ui->scrollXSpinBox->value() == 0 && ui->scrollYSpinBox->value() == 0) {
		run_cue(section_handler->set_offset(get_section_index(), get_layer_index(), ui->offsetXSpinBox->value(), ui->offsetYSpinBox->value()));
	}
}

/**
 * Sets the Canvas' scrolling behavior.
 */
void MaestroControl::set_scroll() {
	// Only update if the scroll doesn't exist, or the scroll does exist and its values don't match the new values
	int new_x = ui->scrollXSpinBox->value();
	int new_y = ui->scrollYSpinBox->value();
	Section::Scroll* scroll = active_section_->get_scroll();
	if (scroll == nullptr ||
		(scroll != nullptr &&
		 (scroll->interval_x != new_x ||
		 scroll->interval_y != new_y))) {
		run_cue(section_handler->set_scroll(get_section_index(), get_layer_index(), new_x, new_y));
	}

	// Enable/disable offset controls
	if (new_x != 0 || new_y != 0) {
		ui->offsetLabel->setEnabled(false);
		ui->offsetXSpinBox->setEnabled(false);
		ui->offsetYSpinBox->setEnabled(false);
	}
	else {
		ui->offsetLabel->setEnabled(true);
		ui->offsetXSpinBox->setEnabled(true);
		ui->offsetYSpinBox->setEnabled(true);

		ui->offsetXSpinBox->blockSignals(true);
		ui->offsetYSpinBox->blockSignals(true);
		ui->offsetXSpinBox->setValue(active_section_->get_offset()->x);
		ui->offsetYSpinBox->setValue(active_section_->get_offset()->y);
		ui->offsetXSpinBox->blockSignals(false);
		ui->offsetYSpinBox->blockSignals(false);
	}
}

/**
 * Enables Show controls.
 * @param enabled If true, Show controls are enabled.
 */
void MaestroControl::set_show_controls_enabled(bool enabled) {
	ui->currentTimeLabel->setEnabled(enabled);
	ui->currentTimeLineEdit->setEnabled(enabled);
	ui->toggleShowModeCheckBox->setEnabled(enabled);
	ui->toggleShowModeLabel->setEnabled(enabled);
	ui->timingMethodLabel->setEnabled(enabled);
	ui->timingMethodComboBox->setEnabled(enabled);
	ui->eventHistoryLabel->setEnabled(enabled);
	ui->eventHistoryWidget->setEnabled(enabled);
	ui->eventListLabel->setEnabled(enabled);
	ui->eventListWidget->setEnabled(enabled);
	ui->eventTimeLabel->setEnabled(enabled);
	ui->eventTimeSpinBox->setEnabled(enabled);
	ui->addEventButton->setEnabled(enabled);
	ui->removeEventButton->setEnabled(enabled);
	ui->loopLabel->setEnabled(enabled);
	ui->loopCheckBox->setEnabled(enabled);
}

/// Sets the speed and/or pause interval for the active Animation.
void MaestroControl::set_speed() {
	uint16_t pause = ui->pauseSpinBox->value();
	uint16_t speed = ui->cycleSpinBox->value();
	AnimationTiming* timing = active_section_->get_animation()->get_timing();
	if (speed != timing->get_interval() || pause != timing->get_pause()) {
		run_cue(animation_handler->set_timing(get_section_index(), get_layer_index(), speed, pause));
	}
}

/**
 * Sets Layer-related controls enabled.
 * @param visible True if you want to enable the controls.
 */
void MaestroControl::set_layer_controls_enabled(bool enabled) {
	// If visible, enable Layer controls
	ui->mixModeLabel->setEnabled(enabled);
	ui->mix_modeComboBox->setEnabled(enabled);
	ui->alphaSpinBox->setEnabled(enabled);

	// Invert layout controls
	ui->gridSizeLabel->setEnabled(!enabled);
	ui->columnsSpinBox->setEnabled(!enabled);
	ui->rowsSpinBox->setEnabled(!enabled);
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

/**
 * Renders the Maestro's last update time in currentTimeLineEdit.
 */
void MaestroControl::update_maestro_last_time() {
	ui->currentTimeLineEdit->setText(locale_.toString(maestro_controller_->get_maestro()->get_timing()->get_last_time()));

	// Visually disable events that have recently ran.
	int current_index = maestro_controller_->get_show()->get_current_index();
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
 * Appends a Cue to a stream.
 * @param stream Stream to append to.
 * @param cue Cue to append.
 */
void MaestroControl::write_cue_to_stream(QDataStream* stream, uint8_t* cue) {
	if (cue == nullptr) {
		return;
	}
	stream->writeRawData((const char*)cue, cue_controller_->get_cue_size(cue));
}

/**
 * Destructor.
 */
MaestroControl::~MaestroControl() {
	// Close all open serial devices
	for (int i = 0;	i < serial_devices_.size(); i++) {
		if (serial_devices_[i]->isOpen()) {
			serial_devices_[i]->close();
		}
	}

	delete show_controller_;
	delete ui;
}
