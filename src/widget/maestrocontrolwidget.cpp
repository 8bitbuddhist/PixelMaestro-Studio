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
#include "animation/mergeanimation.h"
#include "animation/mergeanimationcontrolwidget.h"
#include "animation/plasmaanimation.h"
#include "animation/plasmaanimationcontrolwidget.h"
#include "animation/radialanimation.h"
#include "animation/radialanimationcontrolwidget.h"
#include "animation/sparkleanimation.h"
#include "animation/sparkleanimationcontrolwidget.h"
#include "animation/waveanimation.h"
#include "animation/waveanimationcontrolwidget.h"
#include "canvas/animationcanvas.h"
#include "canvas/colorcanvas.h"
#include "canvas/palettecanvas.h"
#include "colorpresets.h"
#include "controller/maestrocontroller.h"
#include "core/section.h"
#include "drawingarea/simpledrawingarea.h"
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
		// Capture button key presses
		qApp->installEventFilter(this);

		ui->setupUi(this);

		this->maestro_controller_ = maestro_controller;

		QSettings settings;

		// Open serial connections to output devices
		int serial_count = settings.beginReadArray(PreferencesDialog::output_devices);
		for (int device = 0; device < serial_count; device++) {
			settings.setArrayIndex(device);
			if (settings.value(PreferencesDialog::output_enabled).toInt() > 0) {
				// Detect and skip over the screen
				if (settings.value(PreferencesDialog::output_name).toString().compare(PreferencesDialog::main_window_option, Qt::CaseInsensitive) == 0) { }
				// Detect and initialize the simulated serial device
				else if (settings.value(PreferencesDialog::output_name).toString().compare(PreferencesDialog::detached_window_option, Qt::CaseInsensitive) == 0) {
					drawing_area_dialog_ = std::unique_ptr<SimpleDrawingAreaDialog>(new SimpleDrawingAreaDialog(this, this->maestro_controller_));
					drawing_area_dialog_.get()->show();
				}
				// Detect serial/USB devices
				else {
					// Initialize the serial device
					QSharedPointer<QSerialPort> serial_device(new QSerialPort());
					serial_device->setPortName(settings.value(PreferencesDialog::output_name).toString());
					serial_device->setBaudRate(9600);

					// https://stackoverflow.com/questions/13312869/serial-communication-with-arduino-fails-only-on-the-first-message-after-restart
					serial_device->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
					serial_device->setParity(QSerialPort::Parity::NoParity);
					serial_device->setDataBits(QSerialPort::DataBits::Data8);
					serial_device->setStopBits(QSerialPort::StopBits::OneStop);

					if (serial_device->open(QIODevice::WriteOnly)) {
						serial_devices_.push_back(serial_device);
					}
				}
			}
		}
		settings.endArray();

		// Get Cue Handlers
		cue_controller_ = maestro_controller_->get_maestro()->get_cue_controller();
		animation_handler = static_cast<AnimationCueHandler*>(cue_controller_->get_handler(CueController::Handler::AnimationHandler));
		canvas_handler = static_cast<CanvasCueHandler*>(cue_controller_->get_handler(CueController::Handler::CanvasHandler));
		maestro_handler = static_cast<MaestroCueHandler*>(cue_controller_->get_handler(CueController::Handler::MaestroHandler));
		section_handler = static_cast<SectionCueHandler*>(cue_controller_->get_handler(CueController::Handler::SectionHandler));
		show_handler = static_cast<ShowCueHandler*>(cue_controller_->get_handler(CueController::Handler::ShowHandler));

		// Check to see if we need to pause the Maestro
		if (settings.value(PreferencesDialog::pause_on_start, false).toBool()) {
			on_showPauseButton_clicked();
		}

		// Finally, initialize the MaestroControl
		initialize();
	}

	/**
	 * Adds an action to the event history.
	 * @param cue Action performed.
	 */
	void MaestroControlWidget::add_cue_to_history(uint8_t *cue) {
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
			// Handle delete key where 'watched' == EventList widget
			if (watched == ui->eventListWidget) {
				QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
				if (key_event->key() == Qt::Key_Delete) {
					on_removeEventButton_clicked();
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	/**
	 * Returns the index of the current Layer.
	 * @return Layer index.
	 */
	uint8_t MaestroControlWidget::get_layer_index() {
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
	uint8_t MaestroControlWidget::get_layer_index(Section* section) {
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
	uint8_t MaestroControlWidget::get_section_index(Section* section) {

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
	uint8_t MaestroControlWidget::get_section_index() {
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
		set_canvas_controls_enabled(0);
		set_layer_controls_enabled(false);
		set_show_controls_enabled(false);

		// Initialize Show timer
		show_timer_.setTimerType(Qt::CoarseTimer);
		show_timer_.setInterval(100);
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

			ui->showTimingMethodComboBox->blockSignals(true);
			ui->showTimingMethodComboBox->setCurrentIndex((uint8_t)show->get_timing());
			ui->showTimingMethodComboBox->blockSignals(false);

			ui->loopCheckBox->blockSignals(true);
			ui->loopCheckBox->setChecked(show->get_looping());
			ui->loopCheckBox->blockSignals(false);
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

		// Finally, show the default Section
		set_active_section(section);
		populate_layer_combobox();
	}

	/// Reinitializes Palettes from the Palette Dialog.
	void MaestroControlWidget::initialize_palettes() {
		// Repopulate color combo boxes
		ui->colorComboBox->blockSignals(true);
		ui->canvasPaletteComboBox->blockSignals(true);

		ui->colorComboBox->clear();
		ui->canvasPaletteComboBox->clear();

		for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
			ui->colorComboBox->addItem(palette_controller_.get_palette(i)->name);
			ui->canvasPaletteComboBox->addItem(palette_controller_.get_palette(i)->name);
		}

		ui->colorComboBox->blockSignals(false);
		ui->canvasPaletteComboBox->blockSignals(false);
	}

	/**
	 * Adds the selected Event(s) to the Show's Event list.
	 */
	void MaestroControlWidget::on_addEventButton_clicked() {
		// Add selected Cues to the Show
		for (QModelIndex index : ui->eventHistoryWidget->selectionModel()->selectedIndexes()) {
			Event* event = show_controller_->add_event(ui->eventTimeSpinBox->value(), (uint8_t*)&event_history_.at(index.row()).at(0));
			ui->eventListWidget->addItem(locale_.toString(event->get_time()) + QString(": ") + cue_interpreter_.interpret_cue(event->get_cue()));

			int row = ui->eventListWidget->count() - 1;
			QListWidgetItem* item = ui->eventListWidget->item(row);
			item->setData(0, event->get_time());
			item->setData(1, *event->get_cue());
		}
		show_controller_->initialize_events();
	}

	/**
	 * Sets the Layer's transparency level.
	 * @param arg1 Transparency level from 0 - 255.
	 */
	void MaestroControlWidget::on_alphaSpinBox_valueChanged(int arg1) {
		run_cue(section_handler->set_layer(get_section_index(), get_layer_index(active_section_->get_parent_section()), active_section_->get_parent_section()->get_layer()->mix_mode, arg1));
	}

	/**
	 * Changes the current animation.
	 * @param index Index of the new animation.
	 */
	void MaestroControlWidget::on_animationComboBox_currentIndexChanged(int index) {
		// Only change if the animation is different
		if (active_section_->get_animation()->get_type() == (AnimationType)index) {
			return;
		}

		// Preserve the animation cycle between changes
		PaletteController::Palette* palette = palette_controller_.get_palette(ui->colorComboBox->currentIndex());
		run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), (AnimationType)index, true, &palette->colors[0], palette->colors.size(), true));
		show_extra_controls(active_section_->get_animation());
	}

	/**
	 * Sets the active drawing color when using PaletteCanvases.
	 */
	void MaestroControlWidget::on_canvas_color_clicked() {
		QPushButton* sender = (QPushButton*)QObject::sender();
		canvas_color_index_ = sender->objectName().toInt();

		PaletteController::Palette* palette = palette_controller_.get_palette(ui->canvasPaletteComboBox->currentIndex());
		Colors::RGB color = palette->colors.at(canvas_color_index_);

		// Change the color of the Color button to reflect the selection
		ui->selectColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));
	}

	/**
	 * Changes the current Canvas.
	 * @param index Index of the new Canvas type.
	 */
	void MaestroControlWidget::on_canvasComboBox_currentIndexChanged(int index) {
		CanvasType new_canvas_type = (CanvasType)(index - 1);

		// Check to see if a Canvas already exists. If it does, warn the user that the current Canvas will be erased.
		if (active_section_->get_canvas() && active_section_->get_canvas()->get_type() != new_canvas_type) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This action will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm != QMessageBox::Yes) {
				ui->canvasComboBox->blockSignals(true);
				ui->canvasComboBox->setCurrentIndex((int)active_section_->get_canvas()->get_type() + 1);
				ui->canvasComboBox->blockSignals(false);
				return;
			}
		}

		// Add the new Canvas
		set_canvas_controls_enabled(index);
		if (index > 0) {
			run_cue(section_handler->set_canvas(get_section_index(), get_layer_index(), new_canvas_type));

			// Default to the circle radio button so that the controls can be refreshed
			ui->circleRadioButton->setChecked(true);
			on_circleRadioButton_toggled(true);

			// Select a palette
			if (new_canvas_type == CanvasType::PaletteCanvas) {
				on_canvasPaletteComboBox_currentIndexChanged(0);
			}
		}
		else {
			// Check if Edit Frame mode is enabled. If it is, disable it
			if (ui->toggleCanvasModeCheckBox->isChecked()) {
				on_toggleCanvasModeCheckBox_toggled(false);
			}
		}
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
	 * Changes the Canvas' palette (PaletteCanvases only)
	 * @param index New index.
	 */
	void MaestroControlWidget::on_canvasPaletteComboBox_currentIndexChanged(int index) {
		PaletteController::Palette* palette = palette_controller_.get_palette(index);
		run_cue(canvas_handler->set_colors(get_section_index(), get_layer_index(), &palette->colors[0], palette->colors.size()));

		// Add color buttons to canvasColorPickerLayout. This functions identically to palette switching in the Palette Editor.
		// Delete existing color buttons
		QList<QPushButton*> buttons = ui->canvasColorPickerScrollArea->findChildren<QPushButton*>(QString(), Qt::FindChildOption::FindChildrenRecursively);
		for (QPushButton* button : buttons) {
			disconnect(button, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
			delete button;
		}

		// Create new buttons and add an event handler that triggers on_canvas_color_clicked()
		QLayout* layout = ui->canvasColorPickerScrollArea->findChild<QLayout*>("canvasColorPickerLayout");
		for (uint8_t color_index = 0; color_index < palette->colors.size(); color_index++) {
			Colors::RGB color = palette->colors.at(color_index);
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
	void MaestroControlWidget::on_circleRadioButton_toggled(bool checked) {
		set_circle_controls_enabled(checked);
	}

	/**
	 * Clears the current Canvas frame.
	 */
	void MaestroControlWidget::on_clearButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Clear Canvas", "This action will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			run_cue(canvas_handler->clear(get_section_index(), get_layer_index()));
		}
	}

	/**
	 * Changes the color scheme.
	 * @param index Index of the new color scheme.
	 */
	void MaestroControlWidget::on_colorComboBox_currentIndexChanged(int index) {
		PaletteController::Palette* palette = palette_controller_.get_palette(index);
		run_cue(animation_handler->set_colors(get_section_index(), get_layer_index(), &palette->colors[0], palette->colors.size()));
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

		switch ((CanvasType)(ui->canvasComboBox->currentIndex() - 1)) {
			case CanvasType::AnimationCanvas:
				{
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
				break;
			case CanvasType::ColorCanvas:
				{
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
				break;
			case CanvasType::PaletteCanvas:
				{
					if (checked_button == ui->circleRadioButton) {
						run_cue(canvas_handler->draw_circle(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->fillCheckBox->isChecked()));
					}
					else if (checked_button == ui->lineRadioButton) {
						run_cue(canvas_handler->draw_line(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value()));
					}
					else if (checked_button == ui->rectRadioButton) {
						run_cue(canvas_handler->draw_rect(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->fillCheckBox->isChecked()));
					}
					else if (checked_button == ui->textRadioButton) {
						run_cue(canvas_handler->draw_text(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), (Font::Type)ui->fontComboBox->currentIndex(), ui->textLineEdit->text().toLatin1().data(), ui->textLineEdit->text().size()));
					}
					else {	// Triangle
						run_cue(canvas_handler->draw_triangle(get_section_index(), get_layer_index(), canvas_color_index_, ui->originXSpinBox->value(), ui->originYSpinBox->value(), ui->targetXSpinBox->value(), ui->targetYSpinBox->value(), ui->target2XSpinBox->value(), ui->target2YSpinBox->value(), ui->fillCheckBox->isChecked()));
					}
				}
				break;
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
	void MaestroControlWidget::on_frameRateSpinBox_editingFinished() {
		if (!ui->toggleCanvasModeCheckBox->isChecked()) {
			run_cue(canvas_handler->set_frame_timer(get_section_index(), get_layer_index(), ui->frameRateSpinBox->value()));
		}
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

			// Enable spin box for alpha only
			if ((Colors::MixMode)index == Colors::MixMode::Alpha) {
				ui->alphaSpinBox->setEnabled(true);
			}
			else {
				ui->alphaSpinBox->setEnabled(false);
			}
		}
	}

	/// Moves the selected Show Event down by one.
	void MaestroControlWidget::on_moveEventDownButton_clicked() {
		int current_row = ui->eventListWidget->currentRow();

		if (current_row != ui->eventListWidget->count() - 1) {
			show_controller_->move(current_row, current_row	+ 1);

			QListWidgetItem* current_item = ui->eventListWidget->takeItem(current_row);
			ui->eventListWidget->insertItem(current_row + 1, current_item);
		}
	}

	/// Moves the selected Show Event up by one.
	void MaestroControlWidget::on_moveEventUpButton_clicked() {
		int current_row = ui->eventListWidget->currentRow();

		if (current_row != 0) {
			show_controller_->move(current_row, current_row - 1);

			QListWidgetItem* current_item = ui->eventListWidget->takeItem(current_row);
			ui->eventListWidget->insertItem(current_row - 1, current_item);
		}
	}

	/**
	 * Selects a rectangle for the next shape.
	 * @param checked If true, the next shape will be a rectangle.
	 */
	void MaestroControlWidget::on_rectRadioButton_toggled(bool checked) {
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
	void MaestroControlWidget::on_lineRadioButton_toggled(bool checked) {
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
				ui->frameRateSpinBox->blockSignals(true);
				ui->frameRateSpinBox->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameRateSpinBox->blockSignals(false);
			}

			// Disable frame count spin box if necessary
			if (ui->toggleCanvasModeCheckBox->isChecked()) {
				ui->toggleCanvasModeCheckBox->setChecked(false);
				on_toggleCanvasModeCheckBox_toggled(false);
			}

			// If PaletteCanvas, add Palette to list
			if (canvas->get_type() == CanvasType::PaletteCanvas) {
				QString name = "Section " + QString::number(get_section_index() + 1) +
							   " Layer " + QString::number(get_layer_index()) +
							   " Canvas";
				palette_controller_.add_palette(name, static_cast<PaletteCanvas*>(canvas)->get_colors(), static_cast<PaletteCanvas*>(canvas)->get_num_colors());
				ui->canvasPaletteComboBox->blockSignals(true);
				ui->canvasPaletteComboBox->addItem(name);
				ui->canvasPaletteComboBox->blockSignals(false);
				ui->canvasPaletteComboBox->setCurrentText(name);
			}
		}
	}

	/**
	 * Enables/disables Show looping.
	 * @param checked If true, the Show will loop its Event queue.
	 */
	void MaestroControlWidget::on_loopCheckBox_toggled(bool checked) {
		run_cue(show_handler->set_looping(checked));
	}


	/**
	 * Opens the Palette Editor.
	 */
	void MaestroControlWidget::on_paletteControlButton_clicked() {
		QString palette_name = ui->colorComboBox->currentText();

		PaletteControlWidget palette_control(&palette_controller_, palette_name);
		palette_control.exec();
		initialize_palettes();
	}

	/**
	 * Sets the delay between Animation cycles.
	 * @param value New pause interval.
	 */
	void MaestroControlWidget::on_pauseSlider_valueChanged(int value) {
		ui->pauseSpinBox->blockSignals(true);
		ui->pauseSpinBox->setValue(value);
		ui->pauseSpinBox->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Sets the delay between Animation cycles.
	 * @param arg1 New pause interval.
	 */
	void MaestroControlWidget::on_pauseSpinBox_valueChanged(int arg1) {
		ui->pauseSlider->blockSignals(true);
		ui->pauseSlider->setValue(arg1);
		ui->pauseSlider->blockSignals(false);

		set_animation_timer();
	}

	/**
	 * Removes the selected Event(s) from the Show.
	 */
	void MaestroControlWidget::on_removeEventButton_clicked() {
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
	void MaestroControlWidget::on_section_resize(uint16_t x, uint16_t y) {
		if ((x != active_section_->get_dimensions()->x) || (y != active_section_->get_dimensions()->y)) {

			/*
			 * Check for a Canvas.
			 * The old Canvas gets deleted on resize.
			 * If one is set, copy its contents to a temporary buffer, then copy it back once the new Canvas is created.
			 */
			if (active_section_->get_canvas() != nullptr) {
				Point frame_bounds(active_section_->get_dimensions()->x, active_section_->get_dimensions()->y);
				if (active_section_->get_canvas()->get_type() == CanvasType::AnimationCanvas) {
					AnimationCanvas* canvas = static_cast<AnimationCanvas*>(active_section_->get_canvas());

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
				else if (active_section_->get_canvas()->get_type() == CanvasType::PaletteCanvas) {
					PaletteCanvas* canvas = static_cast<PaletteCanvas*>(active_section_->get_canvas());

					uint8_t** frames = new uint8_t*[canvas->get_num_frames()];
					for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
						frames[frame] = new uint8_t[frame_bounds.size()];
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
	void MaestroControlWidget::on_selectColorButton_clicked() {
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
	 * Toggles between stopping and running the Maestro.
	 */
	void MaestroControlWidget::on_showPauseButton_clicked() {
		if (maestro_controller_->get_running()) {
			// Stop the Maestro
			maestro_controller_->stop();
			ui->showPauseButton->setText("Resume");
			ui->showPauseButton->setToolTip("Resume the Maestro");
		}
		else {
			// Start the Maestro
			maestro_controller_->start();
			ui->showPauseButton->setText("Pause");
			ui->showPauseButton->setToolTip("Pause the Maestro");
		}
	}

	/**
	 * Selects text for the next Canvas shape.
	 * @param checked If true, the next shape will be text.
	 */
	void MaestroControlWidget::on_textRadioButton_toggled(bool checked) {
		set_text_controls_enabled(checked);
	}

	/**
	 * Changes the Show's timing method.
	 * @param index Index of the timing method in showTimingMethodComboBox.
	 */
	void MaestroControlWidget::on_showTimingMethodComboBox_currentIndexChanged(int index) {
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
	void MaestroControlWidget::on_toggleCanvasModeCheckBox_toggled(bool checked) {
		// Enables/disable the 'current frame' control
		ui->currentFrameSpinBox->setEnabled(checked);

		if (checked) {
			run_cue(canvas_handler->remove_frame_timer(get_section_index(), get_layer_index()));
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
	void MaestroControlWidget::on_toggleShowModeCheckBox_clicked(bool checked) {
		enable_show_edit_mode(checked);
	}

	/**
	 * Sets the next shape to triangle.
	 * @param checked If true, the next Canvas shape drawn will be a triangle.
	 */
	void MaestroControlWidget::on_triangleRadioButton_toggled(bool checked) {
		set_triangle_controls_enabled(checked);
	}

	/**
	 * Rebuilds the Layer combobox using the current active Section.
	 */
	void MaestroControlWidget::populate_layer_combobox() {
		ui->layerComboBox->blockSignals(true);
		ui->layerComboBox->clear();
		ui->layerComboBox->addItem("Base Section");

		for (uint8_t layer = 1; layer <= get_num_layers(maestro_controller_->get_maestro()->get_section(get_section_index())); layer++) {
			ui->layerComboBox->addItem(QString("Layer ") + QString::number(layer));
		}

		ui->layerComboBox->blockSignals(false);
	}

	/**
	 * Forwards the specified Cue to the drawing area and/or serial device.
	 * @param cue Cue to perform.
	 */
	void MaestroControlWidget::run_cue(uint8_t *cue) {
		// Only render to outputs if Show Edit mode isn't enabled
		if (!show_mode_enabled_) {
			cue_controller_->run(cue);

			/*
			 * Send to serial devices.
			 * Certain actions (e.g. grid resizing) should be caught here and prevented from running.
			 */
			if (!(cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)CueController::Handler::SectionHandler &&
				cue[(uint8_t)SectionCueHandler::Byte::ActionByte] == (uint8_t)SectionCueHandler::Action::SetDimensions)) {
				for (int i = 0; i < serial_devices_.size(); i++) {
					if (serial_devices_[i]->isOpen()) {
						int size = cue_controller_->get_cue_size(cue);
						serial_devices_[i]->write((const char*)cue, size);
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
	 * Changes the active Section and sets all GUI controls to the Section's settings.
	 * @param section New active Section.
	 */
	void MaestroControlWidget::set_active_section(Section* section) {
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

		// Get Layer settings
		if (section->get_layer()) {
			ui->layerSpinBox->blockSignals(true);
			ui->layerSpinBox->setValue(get_num_layers(section));
			ui->layerSpinBox->blockSignals(false);
		}

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
			PaletteController::Palette* palette = palette_controller_.get_palette("Color Wheel");
			run_cue(section_handler->set_animation(get_section_index(), get_layer_index(), AnimationType::Solid, true, &palette->colors[0], palette->colors.size()));
			run_cue(animation_handler->set_timer(get_section_index(), get_layer_index(), 1000));
			animation = section->get_animation();
		}

		// Animation settings
		ui->orientationComboBox->blockSignals(true);
		ui->reverse_animationCheckBox->blockSignals(true);
		ui->fadeCheckBox->blockSignals(true);
		ui->animationTimerSlider->blockSignals(true);
		ui->animationIntervalSpinBox->blockSignals(true);
		ui->pauseSlider->blockSignals(true);
		ui->pauseSpinBox->blockSignals(true);
		ui->orientationComboBox->setCurrentIndex((uint8_t)animation->get_orientation());
		ui->reverse_animationCheckBox->setChecked(animation->get_reverse());
		ui->fadeCheckBox->setChecked(animation->get_fade());
		ui->animationTimerSlider->setValue(animation->get_timer()->get_interval());
		ui->animationIntervalSpinBox->setValue(animation->get_timer()->get_interval());
		ui->pauseSlider->setValue(animation->get_timer()->get_pause());
		ui->pauseSpinBox->setValue(animation->get_timer()->get_pause());
		ui->orientationComboBox->blockSignals(false);
		ui->reverse_animationCheckBox->blockSignals(false);
		ui->fadeCheckBox->blockSignals(false);
		ui->animationTimerSlider->blockSignals(false);
		ui->animationIntervalSpinBox->blockSignals(false);
		ui->pauseSlider->blockSignals(false);
		ui->pauseSpinBox->blockSignals(false);

		// Palette not found
		int palette_index = palette_controller_.find(animation->get_colors());
		if (palette_index >= 0) {
			ui->colorComboBox->blockSignals(true);
			ui->colorComboBox->setCurrentIndex(palette_index);
			ui->colorComboBox->blockSignals(false);
		}
		else {
			QString name = "Section " + QString::number(get_section_index() + 1) +
						   " Layer " + QString::number(get_layer_index()) +
						   " Animation";
			palette_controller_.add_palette(name, animation->get_colors(), animation->get_num_colors());
			ui->colorComboBox->blockSignals(true);
			ui->colorComboBox->addItem(name);
			ui->colorComboBox->setCurrentText(name);
			ui->colorComboBox->blockSignals(false);
		}

		// Set the animation
		ui->animationComboBox->blockSignals(true);
		ui->animationComboBox->setCurrentIndex((uint8_t)animation->get_type());
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
			if (canvas->get_frame_timer() != nullptr) {
				ui->frameRateSpinBox->setValue(canvas->get_frame_timer()->get_interval());
			}
			set_canvas_controls_enabled(((uint8_t)canvas->get_type() + 1));

			if (canvas->get_type() == CanvasType::PaletteCanvas) {
				// Find the corresponding palette in the Palette Controller.
				int palette_index = palette_controller_.find(static_cast<PaletteCanvas*>(canvas)->get_colors());
				if (palette_index >= 0) {
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->setCurrentIndex(palette_index);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
				else {
					QString name = "Section " + QString::number(get_section_index() + 1) +
								   " Layer " + QString::number(get_layer_index()) +
								   " Canvas";
					palette_controller_.add_palette(name, static_cast<PaletteCanvas*>(canvas)->get_colors(), static_cast<PaletteCanvas*>(canvas)->get_num_colors());
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->addItem(name);
					ui->canvasPaletteComboBox->setCurrentText(name);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
			}
		}
		else {
			ui->canvasComboBox->setCurrentIndex(0);
			ui->frameCountSpinBox->setValue(1);
			ui->currentFrameSpinBox->setValue(0);
			ui->frameRateSpinBox->setValue(100);
			set_canvas_controls_enabled(0);
		}

		ui->frameCountSpinBox->blockSignals(false);
		ui->currentFrameSpinBox->blockSignals(false);
		ui->frameRateSpinBox->blockSignals(false);
		ui->canvasComboBox->blockSignals(false);
	}

	/**
	 * Enables Canvas controls.
	 * @param index Index of the Canvas in the drop-down list. 0 for no Canvas.
	 */
	void MaestroControlWidget::set_canvas_controls_enabled(uint8_t index) {
		ui->toggleCanvasModeLabel->setEnabled(index);
		ui->toggleCanvasModeCheckBox->setEnabled(index);
		ui->frameCountLabel->setEnabled(index);
		ui->frameCountSpinBox->setEnabled(index);
		ui->currentFrameLabel->setEnabled(index);
		ui->currentFrameSpinBox->setEnabled(index);
		ui->frameRateLabel->setEnabled(index);
		ui->frameRateSpinBox->setEnabled(index);
		ui->drawingToolsLabel->setEnabled(index);
		ui->circleRadioButton->setEnabled(index);
		ui->lineRadioButton->setEnabled(index);
		ui->triangleRadioButton->setEnabled(index);
		ui->textRadioButton->setEnabled(index);
		ui->rectRadioButton->setEnabled(index);
		ui->originLabel->setEnabled(index);
		ui->originXSpinBox->setEnabled(index);
		ui->originYSpinBox->setEnabled(index);
		ui->targetLabel->setEnabled(index);
		ui->targetXSpinBox->setEnabled(index);
		ui->targetYSpinBox->setEnabled(index);
		ui->target2Label->setEnabled(index);
		ui->target2XSpinBox->setEnabled(index);
		ui->target2YSpinBox->setEnabled(index);
		ui->fontLabel->setEnabled(index);
		ui->fontComboBox->setEnabled(index);
		ui->textLabel->setEnabled(index);
		ui->textLineEdit->setEnabled(index);
		ui->fillCheckBox->setEnabled(index);
		ui->loadImageButton->setEnabled(index);
		ui->drawButton->setEnabled(index);
		ui->clearButton->setEnabled(index);

		// Canvas-specific controls
		CanvasType type = (CanvasType)(index - 1);
		ui->selectColorButton->setEnabled(index && type  == CanvasType::ColorCanvas);
		ui->canvasPaletteComboBox->setEnabled(index && type == CanvasType::PaletteCanvas);
		ui->canvasPaletteLabel->setEnabled(index && type == CanvasType::PaletteCanvas);
		ui->canvasEditPaletteButton->setEnabled(index && type == CanvasType::PaletteCanvas);
		ui->canvasColorPickerScrollArea->setVisible(index && type == CanvasType::PaletteCanvas);
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
	 * Enables Canvas rectangle controls.
	 * @param enabled If true, rectangle controls are enabled.
	 */
	void MaestroControlWidget::set_rect_controls_enabled(bool enabled) {
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
	void MaestroControlWidget::set_offset() {
		// Don't allow changes if scrolling is enabled
		if (ui->scrollXSpinBox->value() == 0 && ui->scrollYSpinBox->value() == 0) {
			run_cue(section_handler->set_offset(get_section_index(), get_layer_index(), ui->offsetXSpinBox->value(), ui->offsetYSpinBox->value()));
		}
	}

	/**
	 * Sets the Canvas' scrolling behavior.
	 */
	void MaestroControlWidget::set_scroll() {
		int new_x = ui->scrollXSpinBox->value();
		int new_y = ui->scrollYSpinBox->value();
		run_cue(section_handler->set_scroll(get_section_index(), get_layer_index(), Utility::abs_int(new_x), Utility::abs_int(new_y), (new_x < 0), (new_y < 0)));

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
	void MaestroControlWidget::set_show_controls_enabled(bool enabled) {
		ui->currentTimeLabel->setEnabled(enabled);
		ui->currentTimeLineEdit->setEnabled(enabled);
		ui->toggleShowModeCheckBox->setEnabled(enabled);
		ui->toggleShowModeLabel->setEnabled(enabled);
		ui->showTimingMethodLabel->setEnabled(enabled);
		ui->showTimingMethodComboBox->setEnabled(enabled);
		ui->eventHistoryLabel->setEnabled(enabled);
		ui->eventHistoryWidget->setEnabled(enabled);
		ui->eventListLabel->setEnabled(enabled);
		ui->eventListWidget->setEnabled(enabled);
		ui->eventTimeLabel->setEnabled(enabled);
		ui->eventTimeSpinBox->setEnabled(enabled);
		ui->addEventButton->setEnabled(enabled);
		ui->removeEventButton->setEnabled(enabled);
		ui->moveEventDownButton->setEnabled(enabled);
		ui->moveEventUpButton->setEnabled(enabled);
		ui->loopLabel->setEnabled(enabled);
		ui->loopCheckBox->setEnabled(enabled);
		ui->relativeTimeLabel->setEnabled(enabled);
		ui->relativeTimeLineEdit->setEnabled(enabled);
	}

	/// Sets the active Animation's timer.
	void MaestroControlWidget::set_animation_timer() {
		uint16_t pause = ui->pauseSpinBox->value();
		uint16_t new_interval = ui->animationIntervalSpinBox->value();
		AnimationTimer* timer = active_section_->get_animation()->get_timer();
		if (new_interval != timer->get_interval() || pause != timer->get_pause()) {
			run_cue(animation_handler->set_timer(get_section_index(), get_layer_index(), new_interval, pause));
		}
	}

	/**
	 * Sets Layer-related controls enabled.
	 * @param visible True if you want to enable the controls.
	 */
	void MaestroControlWidget::set_layer_controls_enabled(bool enabled) {
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
	void MaestroControlWidget::show_extra_controls(Animation* animation) {
		// First, remove any existing extra control widgets
		if (animation_extra_control_widget_ != nullptr) {
			this->findChild<QLayout*>("animationExtraOptionsLayout")->removeWidget(animation_extra_control_widget_.get());
			animation_extra_control_widget_.reset();
		}

		QLayout* layout = this->findChild<QLayout*>("animationExtraOptionsLayout");

		switch(animation->get_type()) {
			case AnimationType::Fire:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new FireAnimationControlWidget((FireAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Lightning:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new LightningAnimationControlWidget((LightningAnimation*)animation, this, layout->widget()));
				break;
			case AnimationType::Merge:
				animation_extra_control_widget_ = std::unique_ptr<QWidget>(new MergeAnimationControlWidget((MergeAnimation*)animation, this, layout->widget()));
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

		if (animation_extra_control_widget_) {
			layout->addWidget(animation_extra_control_widget_.get());
		}
	}

	/**
	 * Renders the Maestro's last update time in currentTimeLineEdit.
	 */
	void MaestroControlWidget::update_maestro_last_time() {
		uint last_time = (uint)maestro_controller_->get_total_elapsed_time();
		ui->currentTimeLineEdit->setText(locale_.toString(last_time));

		int current_index = maestro_controller_->get_show()->get_current_index();

		// Get the index of the last Event that ran
		Show* show = maestro_controller_->get_show();
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
		// Close all open serial devices
		for (int i = 0;	i < serial_devices_.size(); i++) {
			if (serial_devices_[i]->isOpen()) {
				serial_devices_[i]->close();
			}
		}

		delete show_controller_;
		delete ui;
	}
}
